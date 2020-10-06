#include <cppcoro/sync_wait.hpp>
#include <cppitertools/range.hpp>
#include <flow/cancellation.hpp>
#include <flow/channel.hpp>
#include <flow/logging.hpp>

/**
 * This test will create a single publisher and subscriber, send 10 messages and then quit.
 */

namespace {
struct Point {
  double x;
  double y;
};

std::string to_string(Point const& p)
{
  std::stringstream ss;
  ss << "Point: x: " << p.x << " y: " << p.y;
  return ss.str();
}

static constexpr std::size_t TOTAL_MESSAGES = 10;
static constexpr std::size_t BUFFER_SIZE = 4096;

volatile std::atomic_bool application_is_running = true;
std::atomic_size_t messages_sent = 0;

cppcoro::static_thread_pool scheduler;
cppcoro::sequence_barrier<std::size_t> barrier;
cppcoro::multi_producer_sequencer<std::size_t> sequencer{ barrier, BUFFER_SIZE };

using message_buffer_t = std::array<Point, BUFFER_SIZE>;
message_buffer_t buffer{};
}// namespace

int main()
{
  flow::channel<Point> point_channel("points");
  if (point_channel.name() != "points") {
    flow::logging::error("Expected name of channel to be 'points'. Got {}", point_channel.name());
    return 1;
  }

  // given by the publisher
  auto on_request = [](Point& msg) {
    msg.x = 5.0;
    msg.y = 4.0;
  };

  flow::cancellation_handle cancellation_handle{};
  flow::cancellable_callback<void, Point&> publisher(cancellation_handle.token(), std::move(on_request));
  point_channel.push_publisher(std::move(publisher));

  // given by the subscriber
  auto on_message = [&](Point const& msg) {
    if (msg.x < 5.0 or msg.y < 4.0) {
      flow::logging::error("Got: {} Expected: {}", to_string(msg), to_string(Point{ 5.0, 4.0 }));
      application_is_running.exchange(false);
    }
    messages_sent.fetch_add(1, std::memory_order_relaxed);
  };

  flow::cancellable_callback<void, Point const&> subscription(cancellation_handle.token(), std::move(on_message));
  point_channel.push_subscription(std::move(subscription));

  auto task = point_channel.open_communications(scheduler, barrier, sequencer, buffer, application_is_running);
  std::thread task_thread([&] { cppcoro::sync_wait(std::move(task)); });

  while (messages_sent.load() < TOTAL_MESSAGES and application_is_running.load()) {}

  int EXIT_CODE = EXIT_SUCCESS;
  if (not application_is_running.load()) {
    flow::logging::error("Message published was not correct. The test has failed.");
    EXIT_CODE = EXIT_FAILURE;
  }
  else {
    flow::logging::info("Tested channel: Sent {} messages and cancelled operation.", TOTAL_MESSAGES);
    application_is_running.exchange(false);
  }

  task_thread.join();
  return EXIT_CODE;
}

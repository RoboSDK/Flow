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

bool confirm_name(auto& channel, std::string const& channel_name)
{
  const bool is_correct_name = channel.name() == channel_name;
  if (not is_correct_name) {
    flow::logging::error("Expected name of channel to be '{}'. Got {}", channel_name, channel.name());
  }
  return is_correct_name;
}

static constexpr std::size_t TOTAL_MESSAGES = 5000;
cppcoro::static_thread_pool scheduler;

volatile std::atomic_bool application_is_running = true;
std::atomic_size_t messages_sent = 0;
}// namespace

int main()
{
  flow::channel<Point> small_points_channel("small_points");
  flow::channel<Point> large_points_channel("large_points");

  if (not confirm_name(small_points_channel, "small_points") or not confirm_name(large_points_channel, "large_points")) {
    return 1;
  }

  large_points_channel.push_publisher([](Point& msg) {
    msg.x = 5.0;
    msg.y = 4.0;
  });

  small_points_channel.push_publisher([](Point& msg) {
    msg.x = 1.0;
    msg.y = 1.0;
  });

  large_points_channel.push_subscription([&](Point const& msg) {
    if (msg.x < 5.0 or msg.y < 4.0) {
      flow::logging::error("Got: {} Expected: {}", to_string(msg), to_string(Point{ 5.0, 4.0 }));
      application_is_running.exchange(false);
    }
    messages_sent.fetch_add(1, std::memory_order_relaxed);
  });

  small_points_channel.push_subscription([&](Point const& msg) {
    if (msg.x < 1.0 or msg.y < 1.0) {
      flow::logging::error("Got: {} Expected: {}", to_string(msg), to_string(Point{ 1.0, 1.0 }));
      application_is_running.exchange(false);
    }
    messages_sent.fetch_add(1, std::memory_order_relaxed);
  });

  auto small_points_task = small_points_channel.open_communications(scheduler, application_is_running);
  auto large_points_task = large_points_channel.open_communications(scheduler, application_is_running);
  std::thread task_thread([&] { cppcoro::sync_wait(cppcoro::when_all_ready(std::move(small_points_task), std::move(large_points_task))); });

  while (messages_sent.load(std::memory_order_relaxed) < TOTAL_MESSAGES and application_is_running.load()) {}

  int EXIT_CODE = EXIT_SUCCESS;
  if (not application_is_running.load()) {
    flow::logging::error("Message published was not correct. The test has failed.");
    EXIT_CODE = EXIT_FAILURE;
  }
  else {
    flow::logging::info("Tested channel: Sent {} messages and cancelled operation.", TOTAL_MESSAGES);
    application_is_running.exchange(false);
  }

  flow::logging::info("Waiting for task thread to join....");
  if (task_thread.joinable()) { task_thread.join(); }
  flow::logging::info("Joined!");
  return EXIT_CODE;
}

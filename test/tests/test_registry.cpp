#include <flow/callback_handle.hpp>
#include <flow/channel.hpp>
#include <flow/logging.hpp>
#include <flow/registry.hpp>

#include <cppcoro/sync_wait.hpp>

/**
 * This test will create a single publisher and subscriber, send 10 messages and then quit.
 */
flow::callback_handle make_points_subscription(flow::registry& channels);
flow::callback_handle make_points_publisher(flow::registry& channels);

namespace {
struct Point {
  double x;
  double y;
};

static constexpr std::size_t TOTAL_MESSAGES = 5000;
static constexpr std::size_t BUFFER_SIZE = 4096;
cppcoro::static_thread_pool scheduler;

volatile std::atomic_bool application_is_running = true;
std::atomic_size_t messages_sent = 0;
}// namespace

int main()
{
  flow::registry channel_registry;
  flow::callback_handle cb_sub_handle = make_points_subscription(channel_registry);
  flow::callback_handle cb_pub_handle = make_points_publisher(channel_registry);

  auto& point_channel = channel_registry.get_channel<Point>();

  auto communications_task = point_channel.open_communications(scheduler, application_is_running);
  std::thread task_thread([&] { cppcoro::sync_wait(std::move(communications_task)); });

  int EXIT_CODE = EXIT_SUCCESS;
  std::size_t prev_num_messages = messages_sent.load(std::memory_order_relaxed);
  while (messages_sent.load(std::memory_order_relaxed) < TOTAL_MESSAGES) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (prev_num_messages == messages_sent.load(std::memory_order_relaxed)) {
      application_is_running.exchange(false);
      flow::logging::error("Expect 5000 messages to be sent, but only sent {} and stopped.", prev_num_messages);
      EXIT_CODE = EXIT_FAILURE;
      break;
    }
    prev_num_messages = messages_sent.load(std::memory_order_relaxed);
  }

  if (application_is_running.load(std::memory_order_relaxed)) {
    flow::logging::info("Tested channel: Sent {} messages and cancelled operation.", TOTAL_MESSAGES);
    application_is_running.exchange(false);
  }

  flow::logging::info("Waiting for task thread to join....");
  if (task_thread.joinable()) { task_thread.join(); }
  flow::logging::info("Joined!");
  return EXIT_CODE;
}

flow::callback_handle make_points_subscription(flow::registry& channels)
{
  auto on_message = [](Point const& /*unused*/) {
    messages_sent.fetch_add(1, std::memory_order_relaxed);
  };

  return flow::subscribe<Point>("points", channels, on_message);
}

flow::callback_handle make_points_publisher(flow::registry& channels)
{
  auto on_request = [](Point& /*unused*/) {};
  return flow::publish<Point>("points", channels, on_request);
}

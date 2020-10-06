#include <flow/callback_handle.hpp>
#include <flow/channel.hpp>
#include <flow/logging.hpp>
#include <flow/registry.hpp>

#include <cppcoro/sync_wait.hpp>

/**
 * This test will create a single publisher and subscriber, send 10 messages and then quit.
 */

namespace {
struct Point {
  double x;
  double y;
};
}// namespace

flow::callback_handle make_points_subscription(flow::registry& channels);
flow::callback_handle make_points_publisher(flow::registry& channels);

namespace {
volatile std::atomic_bool application_is_running = true;
static constexpr std::size_t TOTAL_MESSAGES = 10;
std::atomic_size_t messages_sent = 0;

cppcoro::static_thread_pool scheduler;
cppcoro::sequence_barrier<std::size_t> barrier;
static constexpr std::size_t BUFFER_SIZE = 4096;
cppcoro::multi_producer_sequencer<std::size_t> sequencer{ barrier, BUFFER_SIZE };

using message_buffer_t = std::array<Point, BUFFER_SIZE>;
message_buffer_t buffer{};
}// namespace

int main()
{
  flow::registry channel_registry;
  flow::callback_handle cb_sub_handle = make_points_subscription(channel_registry);
  flow::callback_handle cb_pub_handle = make_points_publisher(channel_registry);

  auto& point_channel = channel_registry.get_channel<Point>();

  auto communications_task = point_channel.open_communications(scheduler, barrier, sequencer, buffer, application_is_running);
  std::thread task_thread([&] { cppcoro::sync_wait(std::move(communications_task)); });

  while (messages_sent.load() < TOTAL_MESSAGES) {}
  flow::logging::info("Tested channel: Sent {} messages and cancelled operation.", TOTAL_MESSAGES);
  application_is_running.exchange(false);
  task_thread.join();
  return 0;
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

#include <flow/callback_handle.hpp>
#include <flow/channel.hpp>
#include <flow/channel_registry.hpp>
#include <flow/data_structures/timeout_function.hpp>
#include <flow/logging.hpp>

#include <cppcoro/schedule_on.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all_ready.hpp>
#include <flow/configuration.hpp>

using callback_handle_t = flow::callback_handle<flow::configuration>;
using callback_handles_t = std::vector<callback_handle_t>;
/**
 * This test will create a single publisher and subscriber, send 10 message_registry and then quit.
 */
callback_handles_t make_subscribers(flow::channel_registry<flow::configuration>& channels, std::atomic_size_t& counter);
callback_handles_t make_publishers(flow::channel_registry<flow::configuration>& channels);

namespace {
struct Point {
  double x;
  double y;
};

static constexpr std::size_t TOTAL_MESSAGES = 5000;
static constexpr std::array CHANNEL_NAMES = { "small_points", "large_points" };

cppcoro::static_thread_pool scheduler;
}// namespace

int main()
{
  std::atomic_size_t num_messages_received{ 0 };

  auto channel_registry = flow::channel_registry<flow::configuration>{};
  auto handles = make_subscribers(channel_registry, num_messages_received);// each sub callback will increment
  auto publisher_handles = make_publishers(channel_registry);

  while (not publisher_handles.empty()) {
    handles.push_back(std::move(publisher_handles.back()));
    publisher_handles.pop_back();
  }

  if (not channel_registry.contains<Point>("small_points") or not channel_registry.contains<Point>("large_points")) {
    flow::logging::error("Expected channel_registry to contain both small_points and large_points. It was missing at least one");
    return EXIT_FAILURE;
  }

  auto small_points_comm_task = channel_registry.get_channel<Point>("small_points").open_communications(scheduler);
  auto large_points_comm_task = channel_registry.get_channel<Point>("large_points").open_communications(scheduler);

  auto promise = std::async(
    [&] {
      cppcoro::sync_wait(cppcoro::when_all_ready(std::move(small_points_comm_task), std::move(large_points_comm_task)));
    });

  while (std::atomic_load(&num_messages_received) < TOTAL_MESSAGES) {}

  for (auto& handle : handles) {
    handle.disable();
  }

  flow::logging::info("Tested channel: Sent {} message_registry and cancelled operation.", TOTAL_MESSAGES);
}

callback_handles_t make_subscribers(flow::channel_registry<flow::configuration>& channels, std::atomic_size_t& counter)
{
  auto on_message = [&](Point const& /*unused*/) {
    std::atomic_fetch_add(&counter, 1);
  };

  callback_handles_t handles{};
  for (const auto& channel_name : CHANNEL_NAMES) {
    auto callback_handle = flow::subscribe<Point>(channel_name, channels, on_message);
    handles.push_back(std::move(callback_handle));
  }
  return handles;
}

callback_handles_t make_publishers(flow::channel_registry<flow::configuration>& channels)
{
  auto on_request = [&](Point& /*unused*/) {};

  callback_handles_t handles{};
  for (const auto& channel_name : CHANNEL_NAMES) {
    auto callback_handle = flow::publish<Point>(channel_name, channels, on_request);
    handles.push_back(std::move(callback_handle));
  }
  return handles;
}

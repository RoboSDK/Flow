#include <flow/callback_handle.hpp>
#include <flow/channel.hpp>
#include <flow/data_structures/timeout_function.hpp>
#include <flow/logging.hpp>
#include <flow/metadata.hpp>
#include <flow/registry.hpp>

#include <cppcoro/schedule_on.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all_ready.hpp>
#include <flow/configuration.hpp>

using callback_handle_t = flow::callback_handle<flow::configuration>;
using callback_handles_t = std::vector<callback_handle_t>;
/**
 * This test will create a single publisher and subscriber, send 10 messages and then quit.
 */
callback_handles_t make_subscribers(flow::registry<flow::configuration>& channels, std::atomic_size_t& counter);
callback_handles_t make_publishers(flow::registry<flow::configuration>& channels);

namespace {
struct Point {
  double x;
  double y;

  flow::metadata metadata{};
};

static constexpr std::size_t TOTAL_MESSAGES = 5000;
static constexpr auto TIMEOUT_LIMIT = std::chrono::milliseconds(3000);// should be enough time to run this without timing out
static constexpr std::array CHANNEL_NAMES = { "small_points", "large_points" };

cppcoro::static_thread_pool scheduler;
}// namespace

int main()
{
  std::atomic_size_t num_messages_received{ 0 };

  auto channel_registry = flow::registry<flow::configuration>{};
  auto handles = make_subscribers(channel_registry, num_messages_received);// each sub callback will increment
  auto publisher_handles = make_publishers(channel_registry);

  while (not publisher_handles.empty()) {
    handles.push_back(std::move(publisher_handles.back()));
    publisher_handles.pop_back();
  }

  if (not channel_registry.contains<Point>("small_points") or not channel_registry.contains<Point>("large_points")) {
    flow::logging::error("Expected registry to contain both small_points and large_points. It was missing at least one");
    return EXIT_FAILURE;
  }

  std::atomic_bool timed_out{ false };
  auto [_, timeout_routine] = flow::make_timeout_function(TIMEOUT_LIMIT, [&] {
    std::atomic_store(&timed_out, std::atomic_load(&num_messages_received) < TOTAL_MESSAGES);
  });

  auto small_points_comm_task = channel_registry.get_channel<Point>("small_points").open_communications(scheduler);
  auto large_points_comm_task = channel_registry.get_channel<Point>("large_points").open_communications(scheduler);

  cppcoro::static_thread_pool timeout_scheduler;
  auto timeout_task = cppcoro::schedule_on(timeout_scheduler, timeout_routine());

  auto promise = std::async(
    [&] {
      cppcoro::sync_wait(cppcoro::when_all_ready(std::move(small_points_comm_task), std::move(large_points_comm_task), std::move(timeout_task)));
    });

  while (std::atomic_load(&num_messages_received) < TOTAL_MESSAGES and not std::atomic_load(&timed_out)) {}

  for (auto& handle : handles) {
    handle.disable();
  }

  promise.get();

  flow::logging::info("Tested channel: Sent {} messages and cancelled operation.", TOTAL_MESSAGES);

  if (timed_out) {
    flow::logging::error("Test timed out! Time limit is {} milliseconds", TIMEOUT_LIMIT.count());
  }
  return timed_out;
}

callback_handles_t make_subscribers(flow::registry<flow::configuration>& channels, std::atomic_size_t& counter)
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

callback_handles_t make_publishers(flow::registry<flow::configuration>& channels)
{
  auto on_request = [&](Point& /*unused*/) {};

  callback_handles_t handles{};
  for (const auto& channel_name : CHANNEL_NAMES) {
    auto callback_handle = flow::publish<Point>(channel_name, channels, on_request);
    handles.push_back(std::move(callback_handle));
  }
  return handles;
}

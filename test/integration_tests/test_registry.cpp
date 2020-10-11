#include <flow/callback_handle.hpp>
#include <flow/channel.hpp>
#include <flow/logging.hpp>
#include <flow/metadata.hpp>
#include <flow/registry.hpp>

#include <cppcoro/sync_wait.hpp>
#include <flow/configuration.hpp>

/**
 * This test will create a single publisher and subscriber, send 10 messages and then quit.
 */
void make_subscribers(flow::registry<flow::configuration>& channels);
void make_publishers(flow::registry<flow::configuration>& channels);

namespace {
struct Point {
  double x;
  double y;

  flow::metadata metadata{};
};

static constexpr std::size_t TOTAL_MESSAGES = 5000;
static constexpr std::array CHANNEL_NAMES = { "small_points", "large_points" };

/**
 * Takes a bit of time to spin thread and coroutines up, need to sleep for 100 ms before checking
 */
static constexpr auto time_wait_for_coroutines = std::chrono::milliseconds (100);
cppcoro::static_thread_pool scheduler;

volatile flow::configuration::atomic_bitset_t application_is_running{};
std::atomic_size_t messages_received = 0;
std::atomic_size_t messages_sent = 0;
}// namespace

int main()
{
  auto channel_registry = flow::registry<flow::configuration>(&application_is_running);
  make_subscribers(channel_registry);
  make_publishers(channel_registry);

  if (not channel_registry.contains<Point>("small_points") or not channel_registry.contains<Point>("large_points")) {
    flow::logging::error("Expected registry to contain both small_points and large_points. It was missing at least one");
    return EXIT_FAILURE;
  }

  std::vector<cppcoro::task<void>> tasks{};
  for (auto const& name : CHANNEL_NAMES) {
    auto& point_channel = channel_registry.get_channel<Point>(name);
    auto communications_task = point_channel.open_communications(scheduler, application_is_running);
    tasks.push_back(std::move(communications_task));
  }

  std::thread task_thread([&] { cppcoro::sync_wait(cppcoro::when_all(std::move(tasks))); });

  int EXIT_CODE = EXIT_SUCCESS;
  std::size_t prev_num_messages = messages_received.load(std::memory_order_relaxed);
  while (messages_received.load(std::memory_order_relaxed) < TOTAL_MESSAGES) {

    // if test fails maybe it  because of this? We wait 10 ms for coroutines to start
    std::this_thread::sleep_for(time_wait_for_coroutines);
    if (prev_num_messages == messages_received.load(std::memory_order_relaxed)) {
      application_is_running.exchange(false);
      flow::logging::error("Expect 5000 messages to be received, but only received {} and sent {}", prev_num_messages, messages_sent.load(std::memory_order_relaxed));
      EXIT_CODE = EXIT_FAILURE;
      break;
    }
    prev_num_messages = messages_received.load(std::memory_order_relaxed);
  }

  const bool is_running = application_is_running.load(std::memory_order_relaxed).any();
  if (is_running) {
    flow::logging::info("Tested channel: Sent {} messages and cancelled operation.", TOTAL_MESSAGES);
    application_is_running.exchange(false);
  }

  flow::logging::info("Waiting for task thread to join....");
  if (task_thread.joinable()) { task_thread.join(); }
  flow::logging::info("Joined!");
  return EXIT_CODE;
}

void make_subscribers(flow::registry<flow::configuration>& channels)
{
  auto on_message = [](Point const& /*unused*/) {
    messages_received.fetch_add(1, std::memory_order_relaxed);
  };

  for (const auto& channel_name : CHANNEL_NAMES) {
    flow::subscribe<Point>(channel_name, channels, on_message);
  }
}

void make_publishers(flow::registry<flow::configuration>& channels)
{
  auto on_request = [&](Point& /*unused*/) {
    messages_sent.fetch_add(1, std::memory_order_relaxed);
  };
  for (const auto& channel_name : CHANNEL_NAMES) {
    flow::publish<Point>(channel_name, channels, on_request);
  }
}

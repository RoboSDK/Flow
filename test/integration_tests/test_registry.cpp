#include <flow/callback_handle.hpp>
#include <flow/channel.hpp>
#include <flow/logging.hpp>
#include <flow/registry.hpp>
#include <flow/metadata.hpp>

#include <cppcoro/sync_wait.hpp>

/**
 * This test will create a single publisher and subscriber, send 10 messages and then quit.
 */
void make_subscribers(flow::registry& channels);
void make_publishers(flow::registry& channels);

namespace {
struct Point {
  double x;
  double y;

  flow::metadata metadata{};
};

static constexpr std::size_t TOTAL_MESSAGES = 5000;
static constexpr std::array CHANNEL_NAMES = { "small_points", "large_points" };
cppcoro::static_thread_pool scheduler;

volatile std::atomic_bool application_is_running = true;
std::atomic_size_t messages_sent = 0;
}// namespace

int main()
{
  flow::registry channel_registry(&application_is_running);
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
  std::size_t prev_num_messages = messages_sent.load(std::memory_order_relaxed);
  while (messages_sent.load(std::memory_order_relaxed) < TOTAL_MESSAGES) {

    // if test fails maybe it  because of this? We wait 10 ms for coroutines to start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

void make_subscribers(flow::registry& channels)
{
  auto on_message = [](Point const& /*unused*/) {
    messages_sent.fetch_add(1, std::memory_order_relaxed);
  };

  for (const auto& channel_name : CHANNEL_NAMES) {
    flow::subscribe<Point>(channel_name, channels, on_message);
  }
}

void make_publishers(flow::registry& channels)
{
  auto on_request = [](Point& /*unused*/) {};
  for (const auto& channel_name : CHANNEL_NAMES) {
    flow::publish<Point>(channel_name, channels, on_request);
  }
}

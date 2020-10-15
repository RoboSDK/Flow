#include <cppcoro/schedule_on.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all_ready.hpp>
#include <cppitertools/range.hpp>

#include <flow/cancellation.hpp>
#include <flow/channel.hpp>
#include <flow/data_structures/tick_function.hpp>
#include <flow/data_structures/timeout_function.hpp>
#include <flow/logging.hpp>
#include <flow/configuration.hpp>

/**
 * This test will create a single publisher and subscriber, send 10 message_registry and then quit.
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
static constexpr auto TIMEOUT_LIMIT = std::chrono::milliseconds(3000); // should be enough time to run this without timing out
cppcoro::static_thread_pool scheduler;
}// namespace

int main()
{
  auto small_points_channel = flow::channel<Point, flow::configuration>("small_points");
  auto large_points_channel = flow::channel<Point, flow::configuration>("large_points");

  if (not confirm_name(small_points_channel, "small_points") or not confirm_name(large_points_channel, "large_points")) {
    return 1;
  }

  auto cancellation_sources = std::vector<cppcoro::cancellation_source>(4);
  const auto cancel_tasks = [&] {
    for (auto& cancel_source : cancellation_sources) {
      cancel_source.request_cancellation();
    }
  };

  std::atomic_bool success = false;
  auto tick = flow::tick_function(TOTAL_MESSAGES, [&] {
    std::atomic_store(&success, true);
    cancel_tasks();
  });

  using publisher_callback_t = flow::channel<Point, flow::configuration>::publisher_callback_t;
  using subscriber_callback_t = flow::channel<Point, flow::configuration>::subscriber_callback_t;

  auto large_points_pub_callback = publisher_callback_t{
    cancellation_sources[0].token(), [](Point& msg) {
      msg.x = 5.0;
      msg.y = 4.0;
    }
  };

  auto small_points_pub_callback = publisher_callback_t{
    cancellation_sources[1].token(), [](Point& msg) {
      msg.x = 1.0;
      msg.y = 1.0;
    }
  };

  auto large_points_sub_callback = subscriber_callback_t{
    cancellation_sources[2].token(), [&](Point const& msg) {
      tick();
      if (msg.x < 5.0 or msg.y < 4.0) {
        flow::logging::error("Got: {} Expected: {}", to_string(msg), to_string(Point{ 5.0, 4.0 }));
      }
    }
  };

  auto small_points_sub_callback = subscriber_callback_t{
    cancellation_sources[3].token(),
    [&](Point const& msg) {
      tick();
      if (msg.x < 1.0 or msg.y < 1.0) {
        flow::logging::error("Got: {} Expected: {}", to_string(msg), to_string(Point{ 1.0, 1.0 }));
      }
    }
  };

  large_points_channel.push_publisher(std::move(large_points_pub_callback));
  large_points_channel.push_subscription(std::move(large_points_sub_callback));
  small_points_channel.push_publisher(std::move(small_points_pub_callback));
  small_points_channel.push_subscription(std::move(small_points_sub_callback));

  auto small_points_task = small_points_channel.open_communications(scheduler);
  auto large_points_task = large_points_channel.open_communications(scheduler);

  cppcoro::sync_wait(cppcoro::when_all_ready(std::move(small_points_task), std::move(large_points_task)));
}

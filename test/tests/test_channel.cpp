#include <catch2/catch.hpp>
#include <flow/channel.hpp>

namespace {
struct Point {
  double x;
  double y;
};
}// namespace

TEST_CASE("Test channel", "[make_array]")
{
  //  flow::channel<Point> point_channel("points");
  //
  //  std::size_t total_requests = 5;

  //  // given by the publisher
  //  const auto on_request = [](Point& msg) {
  //    msg.x = 5.0;
  //    msg.y = 4.0;
  //  };
  //
  //  const auto pub_cb_handle = point_channel.push_publisher(std::move(on_request));
  //
  //  // given by the subscriber
  //  const auto on_message = [](Point const& msg) {
  //    REQUIRE(msg.x == 5.0);
  //    REQUIRE(msg.y == 4.0);
  //  };
  //
  //  const auto sub_cb_handle = point_channel.push_subscription(std::move(on_message));
  //
  //  cppcoro::static_thread_pool tp;
  //  cppcoro::io_service io;
  //  cppcoro::sequence_barrier<std::size_t> barrier;
  //  cppcoro::multi_producer_sequencer<std::size_t> sequencer{barrier, 64};
}

#include <catch2/catch.hpp>
#include <flow/configuration.hpp>
#include <flow/data_structures/channel_set.hpp>
#include <flow/deprecated/channel.hpp>

namespace {
struct Point {
  double x;
  double y;
};
}// namespace

SCENARIO("channel set can store and retrieve channels correctly", "[channel_set]")
{

  GIVEN("An empty channel set")
  {
    flow::channel_set<flow::configuration> channels;

    REQUIRE_FALSE(channels.contains<int>("num"));
    REQUIRE_FALSE(channels.contains<Point>("point"));
    REQUIRE_FALSE(channels.contains<void>("void"));

    WHEN("a channel is inserted")
    {
      flow::channel<Point, flow::configuration> large_points_channel("large_points");
      channels.put(std::move(large_points_channel));

      THEN("channels set is not empty")
      {
        REQUIRE(channels.contains<Point>("large_points"));
        REQUIRE_FALSE(channels.contains<Point>("small_points"));

        auto& channel = channels.at<Point>("large_points");
        REQUIRE(channel.name() == "large_points");
      }
    }

    WHEN("a two channels are inserted")
    {
      flow::channel<Point, flow::configuration> large_points_channel("large_points");
      flow::channel<Point, flow::configuration> small_points_channel("small_points");
      channels.put(std::move(large_points_channel));
      channels.put(std::move(small_points_channel));

      THEN("channels set must contain both")
      {
        REQUIRE(channels.contains<Point>("small_points"));
        REQUIRE(channels.contains<Point>("large_points"));
      }
    }
  }
}
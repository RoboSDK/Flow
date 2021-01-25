#include <catch2/catch.hpp>
#include <cppcoro/sync_wait.hpp>
#include <flow/detail/timeout_routine.hpp>

TEST_CASE("Test timeout_routine behavior", "[timeout_routine]")
{
  using namespace std::chrono;
  using namespace std::chrono_literals;

  static constexpr auto time_limit = 1000ms;

  bool called = false;
  auto timeout_routine = std::make_shared<flow::detail::timeout_routine>(time_limit, [&] {
    called = true;
  });

  REQUIRE_FALSE(called);
  cppcoro::sync_wait(timeout_routine->spin());
  REQUIRE(called);
}

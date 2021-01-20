#include <catch2/catch.hpp>
#include <cppcoro/sync_wait.hpp>
#include <flow/logging.hpp>
#include <flow/timeout_function.hpp>

TEST_CASE("Test timeout_function behavior", "[timeout_function]")
{
  using namespace std::chrono;
  using namespace std::chrono_literals;

  static constexpr auto time_limit = 1000ms;

  bool called = false;
  auto timeout_routine_ptr = flow::make_timeout_function(time_limit, [&] {
    called = true;
  });

  auto& timeout_routine = *timeout_routine_ptr;

  REQUIRE_FALSE(called);
  cppcoro::sync_wait(timeout_routine());
  REQUIRE(called);
}

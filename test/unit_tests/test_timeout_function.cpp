#include <catch2/catch.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppitertools/range.hpp>

#include <flow/data_structures/timeout_function.hpp>
#include <flow/logging.hpp>

TEST_CASE("Test timeout_function behavior", "[timeout_function]")
{
  using namespace std::chrono;
  using namespace std::chrono_literals;

  static constexpr auto time_limit = 1000us;
  static constexpr uint16_t times_to_poll = 1000;// should check every 1us

  bool called = false;
  auto [cancellation_handle, timeout_routine] = flow::make_timeout_function(
    time_limit, [&] {
      called = true;
    },
    times_to_poll);

  auto start = steady_clock::now();

  REQUIRE_FALSE(called);
  const bool timed_out = cppcoro::sync_wait(timeout_routine());
  const auto duration = duration_cast<microseconds>(steady_clock::now() - start);
  REQUIRE(duration >= time_limit);
  REQUIRE(timed_out);
  REQUIRE(called);
}

TEST_CASE("Test cancellation", "[timeout_function]")
{
  using namespace std::chrono;
  using namespace std::chrono_literals;

  static constexpr auto time_limit = 1000us;
  static constexpr uint16_t times_to_poll = 1000;// should check every 1us

  bool called = false;
  auto [cancellation_handle, timeout_routine] = flow::make_timeout_function(
    time_limit, [&] {
      called = true;
    },
    times_to_poll);

  auto start = steady_clock::now();

  REQUIRE_FALSE(called);
  auto promise = std::async([timeout_routine=std::ref(timeout_routine)] { return cppcoro::sync_wait(timeout_routine()); });
  cancellation_handle.request_cancellation();
  const bool timed_out = promise.get();

  const auto duration = duration_cast<microseconds>(steady_clock::now() - start);
  REQUIRE_FALSE(timed_out);
  REQUIRE_FALSE(called);
}

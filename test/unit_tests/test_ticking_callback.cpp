#include <catch2/catch.hpp>
#include <flow/data_structures/ticking_callback.hpp>
#include <cppitertools/range.hpp>

TEST_CASE("Test ticking_callback behavior", "[ticking_callback]") {
  static constexpr std::size_t TICK_THIS_OFTEN = 3;

  bool called = false;
  std::size_t times_called = 0;
  auto ticker = flow::ticking_callback<void()>(TICK_THIS_OFTEN, [&]{
   called = true;
   times_called++;
  });

  REQUIRE_FALSE(ticker());
  REQUIRE_FALSE(ticker());
  REQUIRE(ticker()); // ticked!
  REQUIRE(called);
  REQUIRE(times_called == 1);

  static constexpr auto CYCLES = 99;
  constexpr auto TIMES_TO_CALL = TICK_THIS_OFTEN * CYCLES;
  for ([[maybe_unused]] auto _ : iter::range(TIMES_TO_CALL)) {
    ticker();
  }

  REQUIRE(times_called == 100);
}

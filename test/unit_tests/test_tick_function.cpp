#include <catch2/catch.hpp>
#include <cppitertools/range.hpp>
#include <flow/data_structures/tick_function.hpp>

TEST_CASE("Test tick_function behavior", "[tick_function]") {
  static constexpr std::size_t TICK_THIS_OFTEN = 3;

  bool called = false;
  std::size_t times_called = 0;
  auto tick = flow::tick_function<void()>(TICK_THIS_OFTEN, [&]{
   called = true;
   times_called++;
  });

  REQUIRE_FALSE(tick());
  REQUIRE_FALSE(tick());
  REQUIRE(tick()); // ticked!
  REQUIRE(called);
  REQUIRE(times_called == 1);

  static constexpr auto CYCLES = 99;
  constexpr auto TIMES_TO_CALL = TICK_THIS_OFTEN * CYCLES;
  for ([[maybe_unused]] auto _ : iter::range(TIMES_TO_CALL)) {
    tick();
  }

  REQUIRE(times_called == 100);
}

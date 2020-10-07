#include <catch2/catch.hpp>
#include <cppitertools/range.hpp>
#include <flow/data_structures/tick_function.hpp>
#include <thread>

TEST_CASE("Test tick_function behavior", "[tick_function]")
{
  static constexpr std::size_t TICK_THIS_OFTEN = 3;

  bool called = false;
  std::size_t times_called = 0;
  auto tick = flow::tick_function<void()>(TICK_THIS_OFTEN, [&] {
    called = true;
    times_called++;
  });

  REQUIRE_FALSE(tick());
  REQUIRE_FALSE(tick());
  REQUIRE(tick());// ticked!
  REQUIRE(called);
  REQUIRE(times_called == 1);

  static constexpr auto CYCLES = 99;
  constexpr auto TIMES_TO_CALL = TICK_THIS_OFTEN * CYCLES;
  for ([[maybe_unused]] auto _ : iter::range(TIMES_TO_CALL)) {
    tick();
  }

  REQUIRE(times_called == 100);
}

TEST_CASE("Test concurrent", "[tick_function]")
{
  constexpr auto TIMES_TO_CALL = 100000;
  constexpr auto TICKS_PER_THREAD = 1000;
  constexpr auto TICK_THIS_OFTEN = 1;// always tick

  std::atomic_size_t num_ticks = 0;
  auto tick = flow::tick_function<void()>(TICK_THIS_OFTEN, [&] {
    ++num_ticks;
  });

  constexpr auto NUM_THREADS = TIMES_TO_CALL / TICKS_PER_THREAD;
  std::vector<std::thread> threads;

  std::generate_n(std::back_inserter(threads), NUM_THREADS, [&] {
    return std::thread([&] {
      for ([[maybe_unused]] auto _ : iter::range(TICKS_PER_THREAD)) {
        tick();
      }
    });
  });

  while(not threads.empty()) {
    threads.back().join();
    threads.pop_back();
  }

  REQUIRE(num_ticks == TIMES_TO_CALL);
}

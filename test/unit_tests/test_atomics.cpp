#include <catch2/catch.hpp>
#include <cppitertools/range.hpp>
#include <flow/atomic.hpp>
#include <thread>

TEST_CASE("Test atomic increment", "[flow::atomic_increment]")
{
  SECTION("single threaded")
  {
    auto incremented = flow::atomic_increment(0);
    REQUIRE(incremented == 1);

    const auto is_two = flow::atomic_increment(incremented);
    REQUIRE(is_two == 2);
    REQUIRE(incremented == 2);
    for (std::size_t i = 0; i < 1'000; ++i) {
      auto new_val = flow::atomic_increment(incremented);
      REQUIRE(new_val == i + 3);
    }
    REQUIRE(incremented == 1002);

    auto atomic_incremented = std::atomic_int(incremented);
    flow::atomic_increment(atomic_incremented);
    REQUIRE(std::atomic_load(&atomic_incremented) == 1003);
  }

  SECTION("multi threaded")
  {
    constexpr auto TIMES_TO_CALL = 100000;
    constexpr auto TICKS_PER_THREAD = 1000;

    std::size_t i = 0;

    constexpr auto NUM_THREADS = TIMES_TO_CALL / TICKS_PER_THREAD;
    std::vector<std::thread> threads;

    std::generate_n(std::back_inserter(threads), NUM_THREADS, [&] {
      return std::thread([&] {
        for ([[maybe_unused]] auto _ : iter::range(TICKS_PER_THREAD)) {
          flow::atomic_increment(i);
        }
      });
    });

    while (not threads.empty()) {
      threads.back().join();
      threads.pop_back();
    }

    REQUIRE(i == TIMES_TO_CALL);
  }
}

TEST_CASE("Test atomic post_increment", "[flow::atomic_post_increment]")
{
  SECTION("single threaded")
  {
    auto post_incremented = flow::atomic_post_increment(0);
    REQUIRE(post_incremented == 0);

    const auto zero = flow::atomic_post_increment(post_incremented);
    REQUIRE(zero == 0);
    REQUIRE(post_incremented == 1);
    for (std::size_t i = 0; i < 1'000; ++i) {
      auto old = flow::atomic_post_increment(post_incremented);
      REQUIRE(old == post_incremented - 1);
    }
    REQUIRE(post_incremented == 1001);

    auto atomic_post_incremented = std::atomic_int(post_incremented);
    flow::atomic_post_increment(atomic_post_incremented);
    REQUIRE(std::atomic_load(&atomic_post_incremented) == 1002);
  }

  SECTION("multi threaded")
  {
    constexpr auto TIMES_TO_CALL = 100000;
    constexpr auto TICKS_PER_THREAD = 1000;

    std::size_t i = 0;

    constexpr auto NUM_THREADS = TIMES_TO_CALL / TICKS_PER_THREAD;
    std::vector<std::thread> threads;

    std::generate_n(std::back_inserter(threads), NUM_THREADS, [&] {
      return std::thread([&] {
        for ([[maybe_unused]] auto _ : iter::range(TICKS_PER_THREAD)) {
          flow::atomic_post_increment(i);
        }
      });
    });

    while (not threads.empty()) {
      threads.back().join();
      threads.pop_back();
    }

    REQUIRE(i == TIMES_TO_CALL);
  }
}

TEST_CASE("Test atomic decrement", "[flow::atomic_decrement]")
{
  SECTION("single threaded")
  {
    auto decremented = flow::atomic_decrement(1'003);
    REQUIRE(decremented == 1'002);

    const auto thoussand_one = flow::atomic_decrement(decremented);
    REQUIRE(thoussand_one == 1'001);
    REQUIRE(decremented == 1'001);
    for (std::size_t i = 0; i < 1'000; ++i) {
      auto new_val = flow::atomic_decrement(decremented);
      REQUIRE(new_val == decremented);
    }
    REQUIRE(decremented == 1);

    auto atomic_decremented = std::atomic_int(decremented);
    flow::atomic_decrement(atomic_decremented);
    REQUIRE(std::atomic_load(&atomic_decremented) == 0);
  }

  SECTION("multi threaded")
  {
    constexpr auto TIMES_TO_CALL = 100000;
    constexpr auto TICKS_PER_THREAD = 1000;

    std::size_t i = TIMES_TO_CALL;

    constexpr auto NUM_THREADS = TIMES_TO_CALL / TICKS_PER_THREAD;
    std::vector<std::thread> threads;

    std::generate_n(std::back_inserter(threads), NUM_THREADS, [&] {
      return std::thread([&] {
        for ([[maybe_unused]] auto _ : iter::range(TICKS_PER_THREAD)) {
          flow::atomic_decrement(i);
        }
      });
    });

    while (not threads.empty()) {
      threads.back().join();
      threads.pop_back();
    }

    REQUIRE(i == 0);
  }
}
TEST_CASE("Test multiple threads performing multiple operations", "[atomic]")
{
  constexpr auto TIMES_TO_CALL = 100000;
  constexpr auto TICKS_PER_THREAD = 1000;

  std::size_t i = 0;

  constexpr auto NUM_THREADS = TIMES_TO_CALL / TICKS_PER_THREAD;
  std::vector<std::thread> threads;

  std::generate_n(std::back_inserter(threads), NUM_THREADS, [&] {
    return std::thread([&] {
      for ([[maybe_unused]] auto _ : iter::range(TICKS_PER_THREAD)) {
        flow::atomic_increment(i);
        flow::atomic_decrement(i);
      }
    });
  });

  while (not threads.empty()) {
    threads.back().join();
    threads.pop_back();
  }

  REQUIRE(i == 0);
}

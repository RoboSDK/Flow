#include <ranges>
#include <unordered_set>

#include <catch2/catch.hpp>
#include <cppcoro/async_mutex.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all_ready.hpp>

#include <flow/data_structures/id_generator.hpp>
#include <flow/logging.hpp>

TEST_CASE("Test async id generator", "[id_generator]")
{
  SECTION("a single id")
  {
    std::unordered_set<std::size_t> ids;

    auto id_generator = flow::id_generator{};
    auto const new_id = id_generator();

    ids.emplace(new_id);
    REQUIRE(ids.size() == 1);
    REQUIRE(new_id == 0);
  }

  SECTION("many ids single thread")
  {
    static constexpr std::size_t num_ids = 1'000;
    std::unordered_set<std::size_t> ids;

    auto id_generator = flow::id_generator{};
    std::generate_n(std::inserter(ids, std::begin(ids)), num_ids, id_generator);

    REQUIRE(ids.size() == num_ids);
    REQUIRE(*std::ranges::max_element(ids) == num_ids - 1);
    REQUIRE(*std::ranges::min_element(ids) == 0);
  }
}

TEST_CASE("Test multi thread", "[id_generator]")
{
  static constexpr std::size_t num_threads = 100;

  auto generate_id = flow::id_generator{};
  auto ids = std::unordered_set<std::size_t>{};

  auto threads = std::vector<std::thread>{};
  threads.reserve(num_threads);
  std::generate_n(std::back_inserter(threads), num_threads, [&] {
    return std::thread([&] {
      ids.emplace(generate_id());
    });
  });

  for (auto& thread : threads) {
    thread.join();
  }

  REQUIRE(ids.size() == num_threads);
  REQUIRE(*std::ranges::max_element(ids) == num_threads - 1);
  REQUIRE(*std::ranges::min_element(ids) == 0);
}

#include <catch2/catch.hpp>
#include <coroutine>
#include <cppcoro/sync_wait.hpp>
#include <cppitertools/range.hpp>
#include <flow/data_structures/async_id_generator.hpp>
#include <thread>
#include <unordered_set>

TEST_CASE("Test async id generator", "[async_id_generator]")
{
  auto id_generator = flow::async_id_generator{};
  [[maybe_unused]] const std::size_t new_id = cppcoro::sync_wait(id_generator());

  std::unordered_set<std::size_t> ids;
  ids.emplace(new_id);
  REQUIRE(ids.size() == 1);
}

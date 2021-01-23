#include <catch2/catch.hpp>
#include <flow/detail/make_array.hpp>


TEST_CASE("Make the array", "[make_array]")
{
  using namespace flow;
  [[maybe_unused]] constexpr auto val = flow::make_array<int, 50>(4);
  for (const auto& item : val) {
    REQUIRE(item == 4);
  }
}

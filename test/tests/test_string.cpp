#include <catch2/catch.hpp>
#include <flow/data_structures/string.hpp>

TEST_CASE("Test turning a list to a string", "[to_string]")
{
  SECTION("integers"){
    constexpr std::array vals = {1, 2, 3};
    REQUIRE(flow::to_string(std::begin(vals), std::end(vals)) == "{1, 2, 3}");
  }
  SECTION("doubles"){
    constexpr std::array vals = {1.0, 2.0, 3.0};
    REQUIRE(flow::to_string(std::begin(vals), std::end(vals)) == "{1.000000, 2.000000, 3.000000}");
  }
}

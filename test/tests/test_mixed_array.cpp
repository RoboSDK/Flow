#include <catch2/catch.hpp>
#include <flow/data_structures/MixedArray.hpp>
#include <iostream>
#include <sstream>

TEST_CASE("vectors can be sized and resized", "[vector]") {
  constexpr auto arr = flow::make_mixed_array(1, 2, 3.2, "dog", 100UL);

  REQUIRE(arr.size() == 5);

  std::stringstream ss;
  std::for_each(std::begin(arr), std::end(arr), flow::make_visitor([&ss](auto& item) {
    ss << item << " ";
  }));

  using namespace std::string_literals;
  REQUIRE(ss.str() == "1 2 3.2 dog 100 "s);
}

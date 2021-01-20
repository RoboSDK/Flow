#include <catch2/catch.hpp>
#include <flow/mixed_array.hpp>
#include <iostream>
#include <sstream>

void verify(auto& arr) {
  REQUIRE(arr.size() == 5);

  std::stringstream ss;
  std::for_each(std::begin(arr), std::end(arr), flow::make_visitor([&ss](auto& item) {
         ss << item << " ";
  }));

  using namespace std::string_literals;
  REQUIRE(ss.str() == "1 2 3.2 dog 100 "s);
}

TEST_CASE("Mixed array has values it is constructed with", "[mixed_array]")
{
  constexpr auto arr = flow::make_mixed_array(1, 2, 3.2, "dog", 100UL);
  verify(arr);
}

TEST_CASE("Test constructors and assignment operations", "[mixed_array]")
{
  SECTION("Test copy operator=")
  {
    constexpr auto arr = flow::make_mixed_array(1, 2, 3.2, "dog", 100UL);
    constexpr auto copy = arr;
    verify(copy);
  }

  SECTION("Test copy constructor")
  {
    constexpr auto arr = flow::make_mixed_array(1, 2, 3.2, "dog", 100UL);
    constexpr decltype(arr) copy{arr};
    verify(copy);
  }

  SECTION("Test move operator=")
  {
    constexpr auto arr = flow::make_mixed_array(1, 2, 3.2, "dog", 100UL);
    constexpr auto moved = std::move(arr);
    verify(moved);
  }

  SECTION("Test move constructor")
  {
    constexpr auto moved(flow::make_mixed_array(1, 2, 3.2, "dog", 100UL));
    verify(moved);
  }
}

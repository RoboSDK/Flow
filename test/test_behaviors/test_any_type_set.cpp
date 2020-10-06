#include <catch2/catch.hpp>
#include <flow/data_structures/any_type_set.hpp>


TEST_CASE("Test storing an int", "[any_type_set]")
{
  using namespace flow;
  any_type_set my_set;

  my_set.put(4);
  REQUIRE(my_set.contains<int>());
  REQUIRE(my_set.at<int>() == 4);

  my_set.put(5);
  REQUIRE_FALSE(my_set.at<int>() == 4);
  REQUIRE(my_set.at<int>() == 5);
}
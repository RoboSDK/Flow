#include <catch2/catch.hpp>

#include <flow/network.hpp>
#include <flow/routine_concepts.hpp>

namespace {
int produce_int() { return 0; }
std::string produce_string() { return ""; }
}// namespace

TEST_CASE("Test raw producer function pointer", "[producer_function_pointer]")
{
  SECTION("produce int")
  {
    using producer_t = decltype(produce_int);
    STATIC_REQUIRE(flow::producer_function<producer_t>);
    STATIC_REQUIRE(flow::function<producer_t>);
    STATIC_REQUIRE(not flow::consumer_function<producer_t>);
    STATIC_REQUIRE(not flow::transformer_function<producer_t>);
    STATIC_REQUIRE(not flow::spinner_function<producer_t>);

    STATIC_REQUIRE(not flow::is_user_routine<producer_t>);
    STATIC_REQUIRE(not flow::routine<producer_t>);
    STATIC_REQUIRE(not flow::is_network<producer_t>);
  }

  SECTION("produce string")
  {
    using producer_t = decltype(produce_string);
    STATIC_REQUIRE(flow::producer_function<producer_t>);
    STATIC_REQUIRE(flow::function<producer_t>);
    STATIC_REQUIRE(not flow::consumer_function<producer_t>);
    STATIC_REQUIRE(not flow::transformer_function<producer_t>);
    STATIC_REQUIRE(not flow::spinner_function<producer_t>);

    STATIC_REQUIRE(not flow::is_user_routine<producer_t>);
    STATIC_REQUIRE(not flow::routine<producer_t>);
    STATIC_REQUIRE(not flow::is_network<producer_t>);
  }
}

namespace {
void consume_int(int&& /*unused*/) {}
void consume_string(std::string&& /*unused*/) {}
}// namespace

TEST_CASE("Test raw producer function pointer", "[producer_function_pointer]")
{
  SECTION("consume int")
  {
    using consumer_t = decltype(consume_int);
    STATIC_REQUIRE(flow::consumer_function<consumer_t>);
    STATIC_REQUIRE(flow::function<consumer_t>);
    STATIC_REQUIRE(not flow::producer_function<consumer_t>);
    STATIC_REQUIRE(not flow::transformer_function<consumer_t>);
    STATIC_REQUIRE(not flow::spinner_function<consumer_t>);

    STATIC_REQUIRE(not flow::is_user_routine<consumer_t>);
    STATIC_REQUIRE(not flow::routine<consumer_t>);
    STATIC_REQUIRE(not flow::is_network<consumer_t>);
  }

  SECTION("consume string")
  {
    using consumer_t = decltype(consume_string);
    STATIC_REQUIRE(flow::consumer_function<consumer_t>);
    STATIC_REQUIRE(flow::function<consumer_t>);
    STATIC_REQUIRE(not flow::producer_function<consumer_t>);
    STATIC_REQUIRE(not flow::transformer_function<consumer_t>);
    STATIC_REQUIRE(not flow::spinner_function<consumer_t>);

    STATIC_REQUIRE(not flow::is_user_routine<consumer_t>);
    STATIC_REQUIRE(not flow::routine<consumer_t>);
    STATIC_REQUIRE(not flow::is_network<consumer_t>);
  }
}
namespace {
std::string transform_int(int&& /*unused*/) { return ""; }
int transform_string(std::string&& /*unused*/) { return 0; }
}// namespace

TEST_CASE("Test raw producer function pointer", "[producer_function_pointer]")
{
  SECTION("transform int")
  {
    using transformer_t = decltype(transform_int);
    STATIC_REQUIRE(flow::transformer_function<transformer_t>);
    STATIC_REQUIRE(flow::function<transformer_t>);
    STATIC_REQUIRE(not flow::producer_function<transformer_t>);
    STATIC_REQUIRE(not flow::consumer_function<transformer_t>);
    STATIC_REQUIRE(not flow::spinner_function<transformer_t>);

    STATIC_REQUIRE(not flow::is_user_routine<transformer_t>);
    STATIC_REQUIRE(not flow::routine<transformer_t>);
    STATIC_REQUIRE(not flow::is_network<transformer_t>);
  }

  SECTION("transform string")
  {
    using transformer_t = decltype(transform_string);
    STATIC_REQUIRE(flow::transformer_function<transformer_t>);
    STATIC_REQUIRE(flow::function<transformer_t>);
    STATIC_REQUIRE(not flow::producer_function<transformer_t>);
    STATIC_REQUIRE(not flow::consumer_function<transformer_t>);
    STATIC_REQUIRE(not flow::spinner_function<transformer_t>);

    STATIC_REQUIRE(not flow::is_user_routine<transformer_t>);
    STATIC_REQUIRE(not flow::routine<transformer_t>);
    STATIC_REQUIRE(not flow::is_network<transformer_t>);
  }
}
namespace {
void spinny() {}
}// namespace

TEST_CASE("Test raw producer function pointer", "[producer_function_pointer]")
{
    using transformer_t = decltype(spinny);
    STATIC_REQUIRE(flow::spinner_function<transformer_t>);
    STATIC_REQUIRE(flow::function<transformer_t>);
    STATIC_REQUIRE(not flow::producer_function<transformer_t>);
    STATIC_REQUIRE(not flow::consumer_function<transformer_t>);
    STATIC_REQUIRE(not flow::transformer_function<transformer_t>);

    STATIC_REQUIRE(not flow::is_user_routine<transformer_t>);
    STATIC_REQUIRE(not flow::routine<transformer_t>);
    STATIC_REQUIRE(not flow::is_network<transformer_t>);
}

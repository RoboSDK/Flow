#include <catch2/catch.hpp>

#include <flow/network.hpp>
#include <flow/routine_concepts.hpp>

namespace {
template <typename function_t>
void not_routine_or_network() {
  STATIC_REQUIRE(not flow::is_user_routine<function_t>);
  STATIC_REQUIRE(not flow::routine<function_t>);
  STATIC_REQUIRE(not flow::is_network<function_t>);
}


template<typename producer_t>
void test_producer()
{
  STATIC_REQUIRE(flow::producer_function<producer_t>);
  STATIC_REQUIRE(flow::function<producer_t>);
  STATIC_REQUIRE(not flow::consumer_function<producer_t>);
  STATIC_REQUIRE(not flow::transformer_function<producer_t>);
  STATIC_REQUIRE(not flow::spinner_function<producer_t>);

  not_routine_or_network<producer_t>();
}

template<typename consumer_t>
void test_consumer()
{
  STATIC_REQUIRE(flow::consumer_function<consumer_t>);
  STATIC_REQUIRE(flow::function<consumer_t>);
  STATIC_REQUIRE(not flow::producer_function<consumer_t>);
  STATIC_REQUIRE(not flow::transformer_function<consumer_t>);
  STATIC_REQUIRE(not flow::spinner_function<consumer_t>);

  not_routine_or_network<consumer_t>();
}

template<typename transformer_t>
void test_transformer()
{
  STATIC_REQUIRE(flow::transformer_function<transformer_t>);
  STATIC_REQUIRE(flow::function<transformer_t>);
  STATIC_REQUIRE(not flow::producer_function<transformer_t>);
  STATIC_REQUIRE(not flow::consumer_function<transformer_t>);
  STATIC_REQUIRE(not flow::spinner_function<transformer_t>);

  not_routine_or_network<transformer_t>();
}

template<typename spinner_t>
void test_spinner()
{
  STATIC_REQUIRE(flow::spinner_function<spinner_t>);
  STATIC_REQUIRE(flow::function<spinner_t>);
  STATIC_REQUIRE(not flow::producer_function<spinner_t>);
  STATIC_REQUIRE(not flow::consumer_function<spinner_t>);
  STATIC_REQUIRE(not flow::transformer_function<spinner_t>);

  not_routine_or_network<spinner_t>();
}

}// namespace

namespace {
int produce_int() { return 0; }
std::string produce_string() { return ""; }
}// namespace

TEST_CASE("Test raw producer function pointer", "[producer_function_pointer]")
{
  SECTION("produce int")
  {
    using producer_t = decltype(produce_int);
    test_producer<producer_t>();
  }

  SECTION("produce string")
  {
    using producer_t = decltype(produce_string);
    test_producer<producer_t>();
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
    test_consumer<consumer_t>();
  }

  SECTION("consume string")
  {
    using consumer_t = decltype(consume_string);
    test_consumer<consumer_t>();
  }
}

namespace {
std::string transform_int(int&& /*unused*/) { return ""; }
int transform_string(std::string&& /*unused*/) { return 0; }
}// namespace

TEST_CASE("test transformer", "[transformer]")
{
  SECTION("transform int")
  {
    using transformer_t = decltype(transform_int);
    test_transformer<transformer_t>();
  }

  SECTION("transform string")
  {
    using transformer_t = decltype(transform_string);
    test_transformer<transformer_t>();
  }
}
namespace {
void spinny() {}
}// namespace

TEST_CASE("Test spinner", "[spinner]")
{
  SECTION("raw function")
  {
    using spinner_t = decltype(spinny);
    test_spinner<spinner_t>();
  }
}

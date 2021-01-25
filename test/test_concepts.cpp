#include <catch2/catch.hpp>

#include <flow/concepts.hpp>
#include <flow/network.hpp>

namespace {
template<typename function_t>
void _or_routine_network()
{
  STATIC_REQUIRE(not flow::is_routine<function_t>);
  STATIC_REQUIRE(not flow::is_network<function_t>);
}

template<typename function_t>
void _or_network_or_function()
{
  STATIC_REQUIRE(not flow::is_network<function_t>);
  STATIC_REQUIRE(not flow::is_function<function_t>);
}

template<typename routine_t>
void test_producer_routine()
{
  STATIC_REQUIRE(flow::is_routine<routine_t>);
  STATIC_REQUIRE(flow::is_producer_routine<routine_t>);

  STATIC_REQUIRE(not flow::is_consumer_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_transformer_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_spinner_routine<routine_t>);

  _or_network_or_function<routine_t>();
}

template<typename routine_t>
void test_consumer_routine()
{
  STATIC_REQUIRE(flow::is_routine<routine_t>);
  STATIC_REQUIRE(flow::is_consumer_routine<routine_t>);

  STATIC_REQUIRE(not flow::is_producer_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_transformer_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_spinner_routine<routine_t>);

  _or_network_or_function<routine_t>();
}

template<typename routine_t>
void test_transformer_routine()
{
  STATIC_REQUIRE(flow::is_routine<routine_t>);
  STATIC_REQUIRE(flow::is_transformer_routine<routine_t>);

  STATIC_REQUIRE(not flow::is_producer_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_consumer_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_spinner_routine<routine_t>);

  _or_network_or_function<routine_t>();
}

template<typename routine_t>
void test_spinner_routine()
{
  STATIC_REQUIRE(flow::is_routine<routine_t>);
  STATIC_REQUIRE(flow::is_spinner_routine<routine_t>);

  STATIC_REQUIRE(not flow::is_producer_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_consumer_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_transformer_routine<routine_t>);

  _or_network_or_function<routine_t>();
}

template<typename producer_t>
void test_producer()
{
  STATIC_REQUIRE(flow::is_producer_function<producer_t>);
  STATIC_REQUIRE(flow::is_function<producer_t>);
  STATIC_REQUIRE(not flow::is_consumer_function<producer_t>);
  STATIC_REQUIRE(not flow::is_transformer_function<producer_t>);
  STATIC_REQUIRE(not flow::is_spinner_function<producer_t>);

  _or_routine_network<producer_t>();
}

template<typename consumer_t>
void test_consumer()
{
  STATIC_REQUIRE(flow::is_consumer_function<consumer_t>);
  STATIC_REQUIRE(flow::is_function<consumer_t>);
  STATIC_REQUIRE(not flow::is_producer_function<consumer_t>);
  STATIC_REQUIRE(not flow::is_transformer_function<consumer_t>);
  STATIC_REQUIRE(not flow::is_spinner_function<consumer_t>);

  _or_routine_network<consumer_t>();
}

template<typename transformer_t>
void test_transformer()
{
  STATIC_REQUIRE(flow::is_transformer_function<transformer_t>);
  STATIC_REQUIRE(flow::is_function<transformer_t>);
  STATIC_REQUIRE(not flow::is_producer_function<transformer_t>);
  STATIC_REQUIRE(not flow::is_consumer_function<transformer_t>);
  STATIC_REQUIRE(not flow::is_spinner_function<transformer_t>);

  _or_routine_network<transformer_t>();
}

template<typename spinner_t>
void test_spinner()
{
  STATIC_REQUIRE(flow::is_spinner_function<spinner_t>);
  STATIC_REQUIRE(flow::is_function<spinner_t>);
  STATIC_REQUIRE(not flow::is_producer_function<spinner_t>);
  STATIC_REQUIRE(not flow::is_consumer_function<spinner_t>);
  STATIC_REQUIRE(not flow::is_transformer_function<spinner_t>);

  _or_routine_network<spinner_t>();
}

}// namespace

namespace {
int produce_int() { return 0; }
std::string produce_string() { return ""; }

static int static_produce_int() { return 0; }
static std::string static_produce_string() { return ""; }

template<typename T>
class producer_functor {
public:
  T operator()() { return data; }

private:
  T data{};
};
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

  SECTION("static produce int")
  {
    using producer_t = decltype(static_produce_int);
    test_producer<producer_t>();
  }

  SECTION("static produce string")
  {
    using producer_t = decltype(static_produce_string);
    test_producer<producer_t>();
  }

  SECTION("produce functor int")
  {
    [[maybe_unused]] auto producer = producer_functor<int>{};
    using producer_t = decltype(producer);
    test_producer<producer_t>();
  }

  SECTION("produce functor string")
  {
    [[maybe_unused]] auto producer = producer_functor<std::string>{};
    using producer_t = decltype(producer);
    test_producer<producer_t>();
  }

  SECTION("lambdas")
  {
    auto int_producer_lambda = [] { return 0; };
    test_producer<decltype(int_producer_lambda)>();

    auto test_lambda = [](auto&& lambda) {
      using lambda_producer_t = decltype(lambda);
      test_producer<lambda_producer_t>();
      return 0;
    };

    test_lambda([] { return 0; });
  }
}

namespace {
void consume_int(int&& /*unused*/) {}
void consume_string(std::string&& /*unused*/) {}

template<typename T>
class consumer_functor {
public:
  void operator()(T&& data) { m_data = std::move(data); }

private:
  T m_data{};
};
}// namespace

TEST_CASE("Test raw consumer function pointer", "[consumer_function_pointer]")
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

  SECTION("test consumer functor")
  {
    auto int_consumer_functor = consumer_functor<int>{};
    test_consumer<decltype(int_consumer_functor)>();

    auto string_consumer_functor = consumer_functor<std::string>{};
    test_consumer<decltype(string_consumer_functor)>();
  }
}

namespace {
std::string transform_int(int&& /*unused*/) { return ""; }
int transform_string(std::string&& /*unused*/) { return 0; }
}// namespace

TEST_CASE("test consumer", "[consumer]")
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

TEST_CASE("Test producer routine", "[producer_routine]")
{
  SECTION("raw function")
  {
    auto producer_routine = flow::producer(produce_int, "int");
    STATIC_REQUIRE(flow::is_producer_routine<decltype(producer_routine)>);
  }

  SECTION("static function")
  {
    auto producer_routine = flow::producer(static_produce_int, "int");
    STATIC_REQUIRE(flow::is_producer_routine<decltype(producer_routine)>);
  }

  SECTION("functor")
  {
    auto producer_routine = flow::producer(producer_functor<int>{}, "int");
    STATIC_REQUIRE(flow::is_producer_routine<decltype(producer_routine)>);
  }

  SECTION("lambda")
  {
    auto producer_routine = flow::producer([] { return 0; }, "int");
    STATIC_REQUIRE(flow::is_producer_routine<decltype(producer_routine)>);
  }
}

TEST_CASE("Test consumer routine", "[consumer_routine]")
{
  SECTION("raw function")
  {
    auto consumer_routine = flow::consumer(consume_int, "int");
    STATIC_REQUIRE(flow::is_consumer_routine<decltype(consumer_routine)>);
  }

  SECTION("functor")
  {
    auto consumer_routine = flow::consumer(consumer_functor<int>{}, "int");
    STATIC_REQUIRE(flow::is_consumer_routine<decltype(consumer_routine)>);
  }

  SECTION("lambda")
  {
    auto consumer_routine = flow::consumer([](int&& /*unused*/) {}, "int");
    STATIC_REQUIRE(flow::is_consumer_routine<decltype(consumer_routine)>);
  }
}

TEST_CASE("Test consumer routine", "[transformer_routine]")
{
  SECTION("raw function")
  {
    auto transformer_routine = flow::transformer(transform_int, "int", "bar");
    STATIC_REQUIRE(flow::is_transformer_routine<decltype(transformer_routine)>);
  }
}

TEST_CASE("Test spinner routine", "[spinner_routine]")
{
  SECTION("raw function")
  {
    auto spinner_routine = flow::spinner(spinny);
    STATIC_REQUIRE(flow::is_spinner_routine<decltype(spinner_routine)>);
  }
}

TEST_CASE("Test network", "[network]")
{
  auto network = flow::network([] {});
  using network_t = decltype(network);
  STATIC_REQUIRE(flow::is_network<network_t>);
  STATIC_REQUIRE(not flow::is_routine<network_t>);
  STATIC_REQUIRE(not flow::is_function<network_t>);
}

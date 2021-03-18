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
void test_publisher_routine()
{
  STATIC_REQUIRE(flow::is_routine<routine_t>);
  STATIC_REQUIRE(flow::is_publisher_routine<routine_t>);

  STATIC_REQUIRE(not flow::is_subscriber_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_transformer_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_spinner_routine<routine_t>);

  _or_network_or_function<routine_t>();
}

template<typename routine_t>
void test_subscriber_routine()
{
  STATIC_REQUIRE(flow::is_routine<routine_t>);
  STATIC_REQUIRE(flow::is_subscriber_routine<routine_t>);

  STATIC_REQUIRE(not flow::is_publisher_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_transformer_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_spinner_routine<routine_t>);

  _or_network_or_function<routine_t>();
}

template<typename routine_t>
void test_transformer_routine()
{
  STATIC_REQUIRE(flow::is_routine<routine_t>);
  STATIC_REQUIRE(flow::is_transformer_routine<routine_t>);

  STATIC_REQUIRE(not flow::is_publisher_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_subscriber_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_spinner_routine<routine_t>);

  _or_network_or_function<routine_t>();
}

template<typename routine_t>
void test_spinner_routine()
{
  STATIC_REQUIRE(flow::is_routine<routine_t>);
  STATIC_REQUIRE(flow::is_spinner_routine<routine_t>);

  STATIC_REQUIRE(not flow::is_publisher_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_subscriber_routine<routine_t>);
  STATIC_REQUIRE(not flow::is_transformer_routine<routine_t>);

  _or_network_or_function<routine_t>();
}

template<typename publisher_t>
void test_publisher()
{
  STATIC_REQUIRE(flow::is_publisher_function<publisher_t>);
  STATIC_REQUIRE(flow::is_function<publisher_t>);
  STATIC_REQUIRE(not flow::is_subscriber_function<publisher_t>);
  STATIC_REQUIRE(not flow::is_transformer_function<publisher_t>);
  STATIC_REQUIRE(not flow::is_spinner_function<publisher_t>);

  _or_routine_network<publisher_t>();
}

template<typename subscriber_t>
void test_subscriber()
{
  STATIC_REQUIRE(flow::is_subscriber_function<subscriber_t>);
  STATIC_REQUIRE(flow::is_function<subscriber_t>);
  STATIC_REQUIRE(not flow::is_publisher_function<subscriber_t>);
  STATIC_REQUIRE(not flow::is_transformer_function<subscriber_t>);
  STATIC_REQUIRE(not flow::is_spinner_function<subscriber_t>);

  _or_routine_network<subscriber_t>();
}

template<typename transformer_t>
void test_transformer()
{
  STATIC_REQUIRE(flow::is_transformer_function<transformer_t>);
  STATIC_REQUIRE(flow::is_function<transformer_t>);
  STATIC_REQUIRE(not flow::is_publisher_function<transformer_t>);
  STATIC_REQUIRE(not flow::is_subscriber_function<transformer_t>);
  STATIC_REQUIRE(not flow::is_spinner_function<transformer_t>);

  _or_routine_network<transformer_t>();
}

template<typename spinner_t>
void test_spinner()
{
  STATIC_REQUIRE(flow::is_spinner_function<spinner_t>);
  STATIC_REQUIRE(flow::is_function<spinner_t>);
  STATIC_REQUIRE(not flow::is_publisher_function<spinner_t>);
  STATIC_REQUIRE(not flow::is_subscriber_function<spinner_t>);
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
class publisher_functor {
public:
  T operator()() { return data; }

private:
  T data{};
};
}// namespace

TEST_CASE("Test raw publish function pointer", "[publisher_function_pointer]")
{
  SECTION("publish int")
  {
    using publisher_t = decltype(produce_int);
    test_publisher<publisher_t>();
  }

  SECTION("publish string")
  {
    using publisher_t = decltype(produce_string);
    test_publisher<publisher_t>();
  }

  SECTION("static publish int")
  {
    using publisher_t = decltype(static_produce_int);
    test_publisher<publisher_t>();
  }

  SECTION("static publish string")
  {
    using publisher_t = decltype(static_produce_string);
    test_publisher<publisher_t>();
  }

  SECTION("publish functor int")
  {
    [[maybe_unused]] auto publisher = publisher_functor<int>{};
    using publisher_t = decltype(publisher);
    test_publisher<publisher_t>();
  }

  SECTION("publish functor string")
  {
    [[maybe_unused]] auto publisher = publisher_functor<std::string>{};
    using publisher_t = decltype(publisher);
    test_publisher<publisher_t>();
  }

  SECTION("lambdas")
  {
    auto int_publisher_lambda = [] { return 0; };
    test_publisher<decltype(int_publisher_lambda)>();

    auto test_lambda = [](auto&& lambda) {
      using lambda_publisher_t = decltype(lambda);
      test_publisher<lambda_publisher_t>();
      return 0;
    };

    test_lambda([] { return 0; });
  }
}

namespace {
void consume_int(int&& /*unused*/) {}
void consume_string(std::string&& /*unused*/) {}

template<typename T>
class subscriber_functor {
public:
  void operator()(T&& data) { m_data = std::move(data); }

private:
  T m_data{};
};
}// namespace

TEST_CASE("Test raw subscribe function pointer", "[subscriber_function_pointer]")
{
  SECTION("consume int")
  {
    using subscriber_t = decltype(consume_int);
    test_subscriber<subscriber_t>();
  }

  SECTION("consume string")
  {
    using subscriber_t = decltype(consume_string);
    test_subscriber<subscriber_t>();
  }

  SECTION("test subscribe functor")
  {
    auto int_subscriber_functor = subscriber_functor<int>{};
    test_subscriber<decltype(int_subscriber_functor)>();

    auto string_subscriber_functor = subscriber_functor<std::string>{};
    test_subscriber<decltype(string_subscriber_functor)>();
  }
}

namespace {
std::string transform_int(int&& /*unused*/) { return ""; }
int transform_string(std::string&& /*unused*/) { return 0; }
}// namespace

TEST_CASE("test subscribe", "[subscribe]")
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

TEST_CASE("Test publish routine", "[string_publisher]")
{
  SECTION("raw function")
  {
    auto publisher_routine = flow::publish(produce_int, "int");
    STATIC_REQUIRE(flow::is_publisher_routine<decltype(publisher_routine)>);
  }

  SECTION("static function")
  {
    auto publisher_routine = flow::publish(static_produce_int, "int");
    STATIC_REQUIRE(flow::is_publisher_routine<decltype(publisher_routine)>);
  }

  SECTION("functor")
  {
    auto publisher_routine = flow::publish(publisher_functor<int>{}, "int");
    STATIC_REQUIRE(flow::is_publisher_routine<decltype(publisher_routine)>);
  }

  SECTION("lambda")
  {
    auto publisher_routine = flow::publish([] { return 0; }, "int");
    STATIC_REQUIRE(flow::is_publisher_routine<decltype(publisher_routine)>);
  }
}

TEST_CASE("Test subscribe routine", "[string_subscriber]")
{
  SECTION("raw function")
  {
    auto subscriber_routine = flow::subscribe(consume_int, "int");
    STATIC_REQUIRE(flow::is_subscriber_routine<decltype(subscriber_routine)>);
  }

  SECTION("functor")
  {
    auto subscriber_routine = flow::subscribe(subscriber_functor<int>{}, "int");
    STATIC_REQUIRE(flow::is_subscriber_routine<decltype(subscriber_routine)>);
  }

  SECTION("lambda")
  {
    auto subscriber_routine = flow::subscribe([](int&& /*unused*/) {}, "int");
    STATIC_REQUIRE(flow::is_subscriber_routine<decltype(subscriber_routine)>);
  }
}

TEST_CASE("Test transformer routine", "[transformer_routine]")
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

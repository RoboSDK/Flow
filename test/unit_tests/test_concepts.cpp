#include <catch2/catch.hpp>

#include <flow/make_routine.hpp>
#include <flow/network.hpp>
#include <flow/routine_concepts.hpp>

namespace {
template<typename function_t>
void not_user_routine_or_routine_network()
{
  STATIC_REQUIRE(not flow::is_user_routine<function_t>);
  STATIC_REQUIRE(not flow::routine<function_t>);
  STATIC_REQUIRE(not flow::is_network<function_t>);
}

template<typename function_t>
void not_user_routine_or_network_or_function()
{
  STATIC_REQUIRE(not flow::is_user_routine<function_t>);
  STATIC_REQUIRE(not flow::is_network<function_t>);
  STATIC_REQUIRE(not flow::function<function_t>);
}

template<typename routine_t>
void test_producer_routine()
{
  STATIC_REQUIRE(flow::routine<routine_t>);
  STATIC_REQUIRE(flow::producer_routine<routine_t>);

  STATIC_REQUIRE(not flow::consumer_routine<routine_t>);
  STATIC_REQUIRE(not flow::transformer_routine<routine_t>);
  STATIC_REQUIRE(not flow::spinner_routine<routine_t>);

  not_user_routine_or_network_or_function<routine_t>();
}

template<typename routine_t>
void test_consumer_routine()
{
  STATIC_REQUIRE(flow::routine<routine_t>);
  STATIC_REQUIRE(flow::consumer_routine<routine_t>);

  STATIC_REQUIRE(not flow::producer_routine<routine_t>);
  STATIC_REQUIRE(not flow::transformer_routine<routine_t>);
  STATIC_REQUIRE(not flow::spinner_routine<routine_t>);

  not_user_routine_or_network_or_function<routine_t>();
}

template<typename routine_t>
void test_transformer_routine()
{
  STATIC_REQUIRE(flow::routine<routine_t>);
  STATIC_REQUIRE(flow::transformer_routine<routine_t>);

  STATIC_REQUIRE(not flow::producer_routine<routine_t>);
  STATIC_REQUIRE(not flow::consumer_routine<routine_t>);
  STATIC_REQUIRE(not flow::spinner_routine<routine_t>);

  not_user_routine_or_network_or_function<routine_t>();
}

template<typename routine_t>
void test_spinner_routine()
{
  STATIC_REQUIRE(flow::routine<routine_t>);
  STATIC_REQUIRE(flow::spinner_routine<routine_t>);

  STATIC_REQUIRE(not flow::producer_routine<routine_t>);
  STATIC_REQUIRE(not flow::consumer_routine<routine_t>);
  STATIC_REQUIRE(not flow::transformer_routine<routine_t>);

  not_user_routine_or_network_or_function<routine_t>();
}

template<typename producer_t>
void test_producer()
{
  STATIC_REQUIRE(flow::producer_function<producer_t>);
  STATIC_REQUIRE(flow::function<producer_t>);
  STATIC_REQUIRE(not flow::consumer_function<producer_t>);
  STATIC_REQUIRE(not flow::transformer_function<producer_t>);
  STATIC_REQUIRE(not flow::spinner_function<producer_t>);

  not_user_routine_or_routine_network<producer_t>();
}

template<typename consumer_t>
void test_consumer()
{
  STATIC_REQUIRE(flow::consumer_function<consumer_t>);
  STATIC_REQUIRE(flow::function<consumer_t>);
  STATIC_REQUIRE(not flow::producer_function<consumer_t>);
  STATIC_REQUIRE(not flow::transformer_function<consumer_t>);
  STATIC_REQUIRE(not flow::spinner_function<consumer_t>);

  not_user_routine_or_routine_network<consumer_t>();
}

template<typename transformer_t>
void test_transformer()
{
  STATIC_REQUIRE(flow::transformer_function<transformer_t>);
  STATIC_REQUIRE(flow::function<transformer_t>);
  STATIC_REQUIRE(not flow::producer_function<transformer_t>);
  STATIC_REQUIRE(not flow::consumer_function<transformer_t>);
  STATIC_REQUIRE(not flow::spinner_function<transformer_t>);

  not_user_routine_or_routine_network<transformer_t>();
}

template<typename spinner_t>
void test_spinner()
{
  STATIC_REQUIRE(flow::spinner_function<spinner_t>);
  STATIC_REQUIRE(flow::function<spinner_t>);
  STATIC_REQUIRE(not flow::producer_function<spinner_t>);
  STATIC_REQUIRE(not flow::consumer_function<spinner_t>);
  STATIC_REQUIRE(not flow::transformer_function<spinner_t>);

  not_user_routine_or_routine_network<spinner_t>();
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

TEST_CASE("Test producer routine", "[producer_routine]")
{
  SECTION("raw function")
  {
    auto producer_routine = flow::make_routine<flow::producer>(produce_int, "int");
    STATIC_REQUIRE(flow::producer_routine<decltype(producer_routine)>);
  }

  SECTION("static function")
  {
    auto producer_routine = flow::make_routine<flow::producer>(static_produce_int, "int");
    STATIC_REQUIRE(flow::producer_routine<decltype(producer_routine)>);
  }

  SECTION("functor")
  {
    auto producer_routine = flow::make_routine<flow::producer>(producer_functor<int>{}, "int");
    STATIC_REQUIRE(flow::producer_routine<decltype(producer_routine)>);
  }

  SECTION("lambda")
  {
    auto producer_routine = flow::make_routine<flow::producer>([] { return 0; }, "int");
    STATIC_REQUIRE(flow::producer_routine<decltype(producer_routine)>);
  }
}

TEST_CASE("Test consumer routine", "[consumer_routine]")
{
  SECTION("raw function")
  {
    auto consumer_routine = flow::make_routine<flow::consumer>(consume_int, "int");
    STATIC_REQUIRE(flow::consumer_routine<decltype(consumer_routine)>);
  }

  SECTION("functor")
  {
    auto consumer_routine = flow::make_routine<flow::consumer>(consumer_functor<int>{}, "int");
    STATIC_REQUIRE(flow::consumer_routine<decltype(consumer_routine)>);
  }

  SECTION("lambda")
  {
    auto consumer_routine = flow::make_routine<flow::consumer>([](int&& /*unused*/) {}, "int");
    STATIC_REQUIRE(flow::consumer_routine<decltype(consumer_routine)>);
  }
}

TEST_CASE("Test transformer routine", "[transformer_routine]")
{
  SECTION("raw function")
  {
    auto transformer_routine = flow::make_routine<flow::transformer>(transform_int, "int", "bar");
    STATIC_REQUIRE(flow::transformer_routine<decltype(transformer_routine)>);
  }
}

TEST_CASE("Test spinner routine", "[spinner_routine]")
{
  SECTION("raw function")
  {
    auto spinner_routine = flow::make_routine<flow::spinner>(spinny);
    STATIC_REQUIRE(flow::spinner_routine<decltype(spinner_routine)>);
  }
}

namespace {
struct user_routine_impl : flow::user_routine {};
}

TEST_CASE("Test user routine", "[user_routine]")
{
  STATIC_REQUIRE(flow::is_user_routine<user_routine_impl>);
  STATIC_REQUIRE(not flow::routine<user_routine_impl>);
  STATIC_REQUIRE(not flow::is_network<user_routine_impl>);
  STATIC_REQUIRE(not flow::function<user_routine_impl>);
}

TEST_CASE("Test network", "[network]")
{
  auto network = flow::make_network([]{});
  using network_t = decltype(network);
  STATIC_REQUIRE(flow::is_network<network_t>);
  STATIC_REQUIRE(not flow::routine<network_t>);
  STATIC_REQUIRE(not flow::function<network_t>);
  STATIC_REQUIRE(not flow::is_user_routine<network_t>);
}

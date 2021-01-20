#include <catch2/catch.hpp>
#include <cppcoro/sync_wait.hpp>

#include <flow/configuration.hpp>
#include <flow/network.hpp>

using namespace std::literals;

TEST_CASE("Test network behavior using lambdas", "[network behavior lambdas]")
{
  using context_t = flow::context<flow::configuration>;
  using chain_t = flow::network<flow::configuration>;

  auto context = std::make_unique<context_t>();
  chain_t chain{ context.get() };

  int test_value = 0;

  chain.push([&] {
    flow::logging::info("producer lambda");
    return test_value++;
  },
    "producer");

  chain.push([](int&& val) {
    flow::logging::info("transformer lambda");
    return 2 * val;
  },
    "producer",
    "doubler");

  std::vector<int> doubled_values{};

  chain.push([&](int&& doubled_value) {
    flow::logging::info("transformer lambda");
    doubled_values.push_back(std::move(doubled_value));
  },
    "doubler");

  chain.cancel_after(0ms);
  flow::logging::error("foo");
  cppcoro::sync_wait(chain.spin());

  for (int i = 0; i < test_value; ++i) {
    const int doubled_value = (i + 1) * 2;
    REQUIRE(doubled_values[static_cast<unsigned long>(i)] == doubled_value);
  }
}

/*
 * Raw functions for testing
 */

int producer()
{
  static int val = 0;
  return val++;
}

int doubler(int&& val)
{
  return val * 2;
}

void consumer(int&& /*unused*/)
{
}

TEST_CASE("Test network behavior using raw functions", "[network behavior function pointers]")
{
  using context_t = flow::context<flow::configuration>;
  using chain_t = flow::network<flow::configuration>;

  auto context = std::make_unique<context_t>();
  chain_t chain{ context.get() };

  chain.push(producer, "producer");
  chain.push(doubler, "producer", "doubler");
  chain.push(consumer, "doubler");

  chain.cancel_after(0ms);
  cppcoro::sync_wait(chain.spin());
}

TEST_CASE("Test network behavior using function pointers and lambdas ", "[network behavior function pointers]")
{
  using context_t = flow::context<flow::configuration>;
  using chain_t = flow::network<flow::configuration>;

  auto context = std::make_unique<context_t>();
  chain_t chain{ context.get() };

  chain.push(producer, "producer");
  chain.push(doubler, "producer", "doubler");
  chain.push([](int&& /*unused*/) {}, "doubler");

  chain.cancel_after(0ms);
  cppcoro::sync_wait(chain.spin());
}

TEST_CASE("Test network state machine", "[network state machine]")
{
  using context_t = flow::context<flow::configuration>;
  using chain_t = flow::network<flow::configuration>;

  SECTION("happy path producer-transformer-consumer")
  {
    auto context = std::make_unique<context_t>();
    chain_t chain{ context.get() };
    REQUIRE(chain.state() == chain_t::state::empty);

    chain.push(producer, "1");
    REQUIRE(chain.state() == chain_t::state::open);

    chain.push(doubler, "1", "2");
    REQUIRE(chain.state() == chain_t::state::open);

    chain.push(doubler, "2", "3");
    REQUIRE(chain.state() == chain_t::state::open);

    chain.push(consumer, "3");
    REQUIRE(chain.state() == chain_t::state::closed);
  }

  SECTION("happy path spinner")
  {
    auto context = std::make_unique<context_t>();
    chain_t chain{ context.get() };
    chain.push([] {});

    REQUIRE(chain.state() == chain_t::state::closed);
  }

  SECTION("exception producer-producer")
  {
    auto context = std::make_unique<context_t>();
    chain_t chain{ context.get() };
    chain.push(producer, "1");
    REQUIRE(chain.state() == chain_t::state::open);

    REQUIRE_THROWS(chain.push(producer, "1"));
  }

  SECTION("exception consumer")
  {
    auto context = std::make_unique<context_t>();
    chain_t chain{ context.get() };
    REQUIRE_THROWS(chain.push(consumer, "1"));
  }

  SECTION("exception transformer")
  {
    auto context = std::make_unique<context_t>();
    chain_t chain{ context.get() };
    REQUIRE_THROWS(chain.push(doubler, "1", "2"));
  }

  SECTION("exception producer-consumer-transformer")
  {
    auto context = std::make_unique<context_t>();
    chain_t chain{ context.get() };
    chain.push(producer, "1");
    chain.push(consumer, "3");
    REQUIRE_THROWS(chain.push(doubler, "2", "3"));
  }
}

#include <catch2/catch.hpp>
#include <flow/configuration.hpp>
#include <flow/forest.hpp>

using namespace std::literals;

int producer()
{
  static int val = 0;
//  flow::logging::info("producer_routine: {}", val);
  return val++;
}

int doubler(int&& val)
{
//  flow::logging::info("transformer_routine: {}", val * 2);
  return val * 2;
}

void consumer(int&& /*unused*/)
{
//  flow::logging::info("consumer_routine: {}", val);
}

TEST_CASE("Test forest behavior", "[forest behavior]")
{
  using forest_t = flow::forest<flow::configuration>;
  forest_t forest{};

  forest.push(producer, "1");
  forest.push(doubler, "1", "2");
  forest.push(consumer, "2");
  forest.cancel_after(0ms);
  cppcoro::sync_wait(forest.spin());
}

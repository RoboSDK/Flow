#include <catch2/catch.hpp>
#include <flow/configuration.hpp>
#include <flow/forest.hpp>

using namespace std::literals;

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

TEST_CASE("Test forest behavior", "[forest behavior]")
{
  using forest_t = flow::forest<flow::configuration>;
  forest_t forest{};

  forest.push(producer);
  forest.push(consumer);
  forest.cancel_after(10ms);
  cppcoro::sync_wait(forest.spin());
}


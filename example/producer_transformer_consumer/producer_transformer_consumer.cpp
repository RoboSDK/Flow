#include <flow/flow.hpp>

#include "consumer_routine.hpp"
#include "producer_routine.hpp"
#include "string_hasher.hpp"
#include "string_reverser.hpp"

int main()
{
  using namespace example;
  using namespace std::literals;

  // order doesn't matter
  auto network = flow::network(consumer_routine{}, string_reverser{}, string_hasher{}, producer_routine{});
  network.cancel_after(1ms);
  flow::spin(std::move(network));
}
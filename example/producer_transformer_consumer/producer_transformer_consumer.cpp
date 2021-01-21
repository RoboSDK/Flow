#include <flow/flow.hpp>

#include "consumer_routine.hpp"
#include "producer_routine.hpp"
#include "string_hasher.hpp"
#include "string_reverser.hpp"

int main()
{
  using namespace example;
  using namespace std::literals;

  auto network = flow::make_network(consumer_routine{}, string_reverser{}, string_hasher{}, producer_routine{});
  network.cancel_after(1s);
  flow::spin(std::move(network));
}
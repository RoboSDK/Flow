#include <flow/flow.hpp>

#include "consumer_routine.hpp"
#include "producer_routine.hpp"

int main()
{
  using namespace example;
  using namespace std::literals;

  auto network = flow::make_network(consumer_routine{}, producer_routine{});
  network.cancel_after(2s);

  flow::spin(std::move(network));
}
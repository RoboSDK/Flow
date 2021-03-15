#include <flow/flow.hpp>

#include "consumer_routine.hpp"
#include "producer_routine.hpp"

int main()
{
  using namespace example;
  using namespace std::literals;

  auto network = flow::network(flow::chain() | hashed_string_subscriber{} | string_publisher{});
  network.cancel_after(1ms);

  flow::spin(std::move(network));
}
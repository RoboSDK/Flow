#include <flow/flow.hpp>

#include "subscriber_routine.hpp"
#include "publisher_routine.hpp"

int main()
{
  using namespace example;
  using namespace std::literals;

  auto network = flow::network(flow::chain() |  string_publisher{} | string_subscriber{});
  network.cancel_after(100ms);
  flow::spin(std::move(network));
}
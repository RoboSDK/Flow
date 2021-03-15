#include <flow/flow.hpp>

#include "hashed_string_subscriber.hpp"
#include "string_hasher.hpp"
#include "string_publisher.hpp"
#include "string_reverser.hpp"

int main()
{
  using namespace example;
  using namespace std::literals;

  // order doesn't matter
  auto network = flow::network(flow::chain() | string_publisher{}| string_reverser{} | string_hasher{} | hashed_string_subscriber{});
  network.cancel_after(1ms);
  flow::spin(std::move(network));
}
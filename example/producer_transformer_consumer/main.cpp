#include <flow/flow.hpp>

#include "consumer_routine.hpp"
#include "producer_routine.hpp"
#include "string_hasher.hpp"
#include "string_reverser.hpp"

int main()
{
  using namespace example;

  flow::spin(
    consumer_routine{},
    string_reverser{},
    string_hasher{},
    producer_routine{}
 );
}
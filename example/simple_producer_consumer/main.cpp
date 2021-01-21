#include <flow/flow.hpp>

#include "consumer_routine.hpp"
#include "producer_routine.hpp"

int main()
{
  using namespace example;
  flow::spin(consumer_routine{}, producer_routine{});
}
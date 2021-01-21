#include <flow/spin.hpp>

#include "consumer_routine.hpp"
#include "producer_routine.hpp"

int main()
{
  flow::spin(consumer_routine{}, producer_rotine{});
}
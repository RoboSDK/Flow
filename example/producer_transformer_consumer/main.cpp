#include <flow/spin.hpp>

#include "consumer_routine.hpp"
#include "producer_routine.hpp"
#include "string_hasher.hpp"
#include "string_reverser.hpp"

int main()
{
  flow::spin(consumer_routine{},
    string_reverser{},
    string_hasher{},
    producer_rotine{});
}
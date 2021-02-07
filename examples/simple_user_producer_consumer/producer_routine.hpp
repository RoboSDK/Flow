#pragma once

#include <flow/producer.hpp>

namespace example {
struct producer_routine {
  std::string operator()()
  {
    return "Hello World!";
  }

  std::string publish_to() { return "hello_world"; }
};
}// namespace example

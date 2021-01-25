#pragma once

#include <flow/logging.hpp>
#include <flow/producer.hpp>

namespace example {
class producer_routine {
public:
  std::string operator()()
  {
    flow::logging::info("Producing Hello World string!");
    return "Hello World!";
  }

  std::string publish_to() { return "hello_world"; };
};
}// namespace example

#pragma once

#include <flow/producer.hpp>

#include <spdlog/spdlog.h>

namespace example {
class producer_routine {
public:
  std::string operator()()
  {
    spdlog::info("Producing Hello World string!");
    return "Hello World!";
  }

  std::string publish_to() { return "hello_world"; };
};
}// namespace example

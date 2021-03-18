#pragma once

#include <flow/publisher.hpp>

#include <spdlog/spdlog.h>

namespace example {
class string_publisher {
public:
  std::string operator()()
  {
    spdlog::info("Producing Hello World string!");
    return "Hello World!";
  }

  std::string publish_to() { return "hello_world"; };
};
}// namespace example

#pragma once

#include <flow/publisher.hpp>

namespace example {
struct string_publisher {
  std::string operator()()
  {
    return "Hello World!";
  }

  std::string publish_to() { return "hello_world"; }
};
}// namespace example

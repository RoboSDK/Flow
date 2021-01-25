#pragma once

#include <flow/logging.hpp>
#include <flow/transformer.hpp>

namespace example {
class string_reverser {
public:
  std::string operator()(std::string&& message)
  {
    std::reverse(std::begin(message), std::end(message));
    flow::logging::info("Received Hello World Message! Reversed: {}", message);
    return std::move(message);
  }

  std::string subscribe_to() { return "hello_world"; }
  std::string publish_to() { return "hello_world_reversed"; }
};
}// namespace example

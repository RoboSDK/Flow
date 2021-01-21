#pragma once

#include <flow/logging.hpp>
#include <flow/transformer.hpp>

namespace example {
class string_reverser {
public:
  void initialize(flow::network& network)
  {
    flow::register(string_reverse, network);
  }

private:
  std::string reverse_message(std::string&& message)
  {
    std::reverse(std::begin(message), std::end(message));
    return message;
  }

  flow::transformer<std::string(std::string)> string_reverser{
    reverse_message,
    "hello_world",
    "hello_world_reversed"
  };
};
}// namespace example

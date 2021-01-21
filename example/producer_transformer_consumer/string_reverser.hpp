#pragma once

#include <flow/logging.hpp>
#include <flow/transformer_handle.hpp>

namespace example {
class string_reverser {
public:
  void initialize(flow::network& network)
  {
    flow::register(string_reverse_handle, network);
  }

private:
  std::string reverse_message(std::string&& message)
  {
    std::reverse(std::begin(message), std::end(message));
    return message;
  }

  flow::transformer_handle<std::string(std::string)> string_reverser_handle{
    reverse_message,
    "hello_world",
    "hello_world_reversed"
  };
};
}// namespace example

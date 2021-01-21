#pragma once

#include <flow/logging.hpp>
#include <flow/transformer.hpp>
#include <flow/user_routine.hpp>

namespace example {
class string_reverser : flow::user_routine {
public:
  void initialize(auto& network)
  {
    network.push(std::move(string_reverser));
  }

private:
  std::string reverse_message(std::string&& message)
  {
    std::reverse(std::begin(message), std::end(message));
    flow::logging::info("Received Hello World Message! Reversed: {}", message);
    return std::move(message);
  }

  flow::transformer<std::string(std::string)> string_reverser{
    [this](std::string&& msg) { return reverse_message(std::move(msg)); },
    "hello_world",
    "hello_world_reversed"
  };
};
}// namespace example

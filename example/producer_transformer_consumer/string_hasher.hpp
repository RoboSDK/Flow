#pragma once

#include <flow/logging.hpp>
#include <flow/transformer.hpp>
#include <flow/user_routine.hpp>

namespace example {
class string_hasher : flow::user_routine {
public:
  void initialize(auto& network)
  {
    network.push(std::move(string_hasher));
  }

private:
  std::size_t hash_message(std::string&& message)
  {
    const auto hashed =  std::hash<std::string>{}(std::move(message));
    flow::logging::info("Received Reversed String: {} Hashed: {}", message, hashed);
    return hashed;
  }

  flow::detail::transformer_impl<std::size_t(std::string)> string_hasher{
    [this](std::string&& msg) { return hash_message(std::move(msg)); },
    "hello_world_reversed",
    "hashed"
  };
};
}// namespace example

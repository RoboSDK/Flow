#pragma once

#include <flow/transformer.hpp>

#include <spdlog/spdlog.h>

namespace example {
class string_hasher {
public:
  std::size_t operator()(std::string&& message)
  {
    const auto hashed =  std::hash<std::string>{}(std::move(message));
    spdlog::info("Received Reversed String: {} Hashed: {}", message, hashed);
    return hashed;
  }

  std::string subscribe_to() { return "hello_world_reversed"; }
  std::string publish_to() { return "hashed"; }
};
}// namespace example

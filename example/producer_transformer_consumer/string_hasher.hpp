#pragma once

#include <flow/transformer_handle.hpp>
#include <flow/logging.hpp>

namespace example {
class string_hasher {
public:
  void initialize(flow::network &network) {
    flow::register(string_hasher_handle, network);
  }

private:
  std::size_t hash_message(std::string&& message) {
    return std::hash<std::string>{}(std::move(message));
  }

  flow::transformer_handle<std::string> string_hasher_handle{ hash_message, "hello_world_reversed", "hashed"};
};
}

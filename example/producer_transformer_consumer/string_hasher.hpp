#pragma once

#include <flow/logging.hpp>
#include <flow/transformer.hpp>

namespace example {
class string_hasher {
public:
  void initialize(flow::network& network)
  {
    flow::register(string_hasher, network);
  }

private:
  std::size_t hash_message(std::string&& message)
  {
    return std::hash<std::string>{}(std::move(message));
  }

  flow::transformer<std::size_t(std::string)> string_hasher{
    hash_message,
    "hello_world_reversed",
    "hashed"
  };
};
}// namespace example

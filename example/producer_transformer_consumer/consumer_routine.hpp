#pragma once

#include <flow/consumer.hpp>

#include <spdlog/spdlog.h>

namespace example {
class consumer_routine {
public:
  void operator()(std::size_t&& message)
  {
    spdlog::info("Received Hashed Message: {}", message);
  }

  std::string subscribe_to() { return "hashed"; }
};
}// namespace example

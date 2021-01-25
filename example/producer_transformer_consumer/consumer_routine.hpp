#pragma once

#include <flow/consumer.hpp>
#include <flow/logging.hpp>

namespace example {
class consumer_routine {
public:
  void operator()(std::size_t&& message)
  {
    flow::logging::info("Received Hashed Message: {}", message);
  }

  std::string subscribe_to() { return "hashed"; }
};
}// namespace example

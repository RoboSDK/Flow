#pragma once

#include <flow/consumer.hpp>
#include <flow/logging.hpp>

namespace example {
struct consumer_routine {
  void operator()(std::string&& message)
  {
    flow::logging::info("Received Message: {}", message);
  }

  std::string subscribe_to() { return "hello_world"; }
};
}// namespace example

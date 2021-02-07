#pragma once

#include <flow/consumer.hpp>
#include <spdlog/spdlog.h>

namespace example {
struct consumer_routine {
  void operator()(std::string&& message)
  {
    spdlog::info("Received Message: {}", message);
  }

  std::string subscribe_to() { return "hello_world"; }
};
}// namespace example

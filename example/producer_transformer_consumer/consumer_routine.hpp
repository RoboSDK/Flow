#pragma once

#include <flow/consumer_handle.hpp>
#include <flow/logging.hpp>

namespace example {
class consumer_routine {
public:
  void initialize(flow::network &network) {
    flow::register(consumer_handle, network);
  }

private:
  void receive_hashed_message(std::size_t&& message) {
    flow::logging::info("Received Hashed Message: {}", message);
  }

  flow::consumer_handle<std::string> consumer_handle{ receive_hashed_message, "hashed" };
};
}

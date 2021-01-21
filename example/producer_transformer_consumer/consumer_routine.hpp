#pragma once

#include <flow/consumer.hpp>
#include <flow/logging.hpp>

namespace example {
class consumer_routine {
public:
  void initialize(flow::network &network) {
    flow::register(consumer, network);
  }

private:
  void receive_hashed_message(std::size_t&& message) {
    flow::logging::info("Received Hashed Message: {}", message);
  }

  flow::consumer<std::string> consumer{ receive_hashed_message, "hashed" };
};
}

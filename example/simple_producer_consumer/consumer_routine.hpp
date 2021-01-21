#pragma once

#include <flow/consumer.hpp>
#include <flow/logging.hpp>

namespace example {
class consumer_routine {
public:
  void initialize(flow::network& network)
  {
    flow::register(hello_world_consumer, network);
  }

private:
  void message_handler(std::string&& message)
  {
    flow::logging::info("Received Message: {}", message);
  }

  flow::consumer<std::string> hello_world_consumer{
    message_handler,
    "hello_world"
  };
};
}// namespace example

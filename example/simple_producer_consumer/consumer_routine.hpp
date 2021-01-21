#pragma once

#include <flow/consumer.hpp>
#include <flow/logging.hpp>
#include <flow/user_routine.hpp>

namespace example {
class consumer_routine : flow::user_routine {
public:
  void initialize(auto& network)
  {
    network.push(std::move(hello_world_consumer));
  }

private:
  void message_handler(std::string&& message)
  {
    flow::logging::info("Received Message: {}", message);
  }

  flow::consumer<std::string> hello_world_consumer{
    [this](std::string&& msg) { message_handler(std::move(msg)); },
    "hello_world"
  };
};
}// namespace example

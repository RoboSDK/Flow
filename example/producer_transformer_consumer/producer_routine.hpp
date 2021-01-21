#pragma once

#include <flow/logging.hpp>
#include <flow/producer.hpp>
#include <flow/user_routine.hpp>

namespace example {
class producer_routine : flow::user_routine {
public:
  void initialize(auto& network)
  {
    network.push(std::move(hello_world_producer));
  }

private:
  std::string message_handler()
  {
    flow::logging::info("Producing Hello World string!");
    return "Hello World!";
  }

  flow::producer<std::string> hello_world_producer{
    [this] { return message_handler(); },
    "hello_world"
  };
};
}// namespace example

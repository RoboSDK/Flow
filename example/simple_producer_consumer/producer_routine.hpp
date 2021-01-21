#pragma once

#include <flow/logging.hpp>
#include <flow/producer_handle.hpp>

namespace example {
class producer_routine {
public:
  void initialize(flow::network& network)
  {
    flow::register(hello_world_producer, network);
  }

private:
  std::string message_handler()
  {
    return "Hello World!";
  }

  flow::producer_handle<std::string> hello_world_producer{
    message_handler,
    "hello_world"
  };
};
}// namespace example

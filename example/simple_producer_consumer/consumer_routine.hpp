#pragma once

#include <flow/consumer.hpp>
#include <flow/logging.hpp>
#include <flow/user_routine.hpp>

namespace example {
class consumer_routine : flow::user_routine {
public:
  void initialize(auto& network)
  {
    network.push(flow::make_consumer(receive_hello, "hello_world"));
  }

private:
  static void receive_hello(std::string&& message)
  {
    flow::logging::info("Received Message: {}", message);
  }
};
}// namespace example

#pragma once

#include <flow/logging.hpp>
#include <flow/producer.hpp>

namespace example {
class producer_routine {
public:
  void initialize(auto& network)
  {
    network.push(flow::make_producer(hello_world, "hello_world"));
  }

private:
  static std::string hello_world()
  {
    return "Hello World!";
  }
};
}// namespace example

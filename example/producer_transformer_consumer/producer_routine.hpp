#pragma once

#include <flow/logging.hpp>
#include <flow/producer.hpp>

namespace example {
class producer_routine {
public:
  void initialize(auto& network)
  {
    network.push(std::move(hello_world_producer));
  }

private:
  std::string hello_world()
  {
    flow::logging::info("Producing Hello World string!");
    return "Hello World!";
  }

  flow::detail::producer_impl<std::string> hello_world_producer{
    [this] { return hello_world(); },
    "hello_world"
  };
};
}// namespace example

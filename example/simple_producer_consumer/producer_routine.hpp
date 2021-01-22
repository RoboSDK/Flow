#pragma once

#include <flow/logging.hpp>
#include <flow/producer.hpp>
#include <flow/user_routine.hpp>

namespace example {
class producer_routine : flow::user_routine {
public:
  void initialize(auto& network)
  {
    using namespace flow;

    auto say_hello = make_producer(
      hello_world,
      options{ .publish_to{ "hello_world" } });

    network.push(std::move(say_hello));
  }

private:
  static std::string hello_world()
  {
    return "Hello World!";
  }
};
}// namespace example

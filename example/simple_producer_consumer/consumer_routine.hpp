#pragma once

#include <flow/consumer.hpp>
#include <flow/logging.hpp>
#include <flow/user_routine.hpp>

namespace example {
class consumer_routine : flow::user_routine {
public:
  void initialize(auto& network)
  {
    using namespace flow;
    auto receiver = make_consumer(receive_hello,
      options{ .subscribe_to = "hello_world" });

    network.push(std::move(receiver));
  }

private:
  static void receive_hello(std::string&& message)
  {
    flow::logging::info("Received Message: {}", message);
  }
};
}// namespace example

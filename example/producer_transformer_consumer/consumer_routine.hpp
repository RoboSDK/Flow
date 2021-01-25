#pragma once

#include <flow/consumer.hpp>
#include <flow/logging.hpp>

namespace example {
class consumer_routine {
public:
  void initialize(flow::is_network auto& network)
  {
    network.push(std::move(consumer));
  }

private:
  void receive_hashed_message(std::size_t&& message)
  {
    flow::logging::info("Received Hashed Message: {}", message);
  }

  flow::detail::consumer_impl<std::size_t> consumer{
    [this](std::size_t&& msg) { receive_hashed_message(std::move(msg)); },
    "hashed"
  };
};
}// namespace example

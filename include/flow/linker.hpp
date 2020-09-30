#pragma once

#include <functional>
#include <variant>

#include "flow/subscription.hpp"
#include "flow/data_structures/static_vector.hpp"

namespace flow {

template <std::size_t subcription_buffer_size, std::size_t publisher_buffer_size, typename... messages_ts>
class linker {
  using subscription_t = std::variant<subscription<messages_ts> ...>;
  static_vector<subscription_t , subcription_buffer_size> subscriptions;

  using publisher_t = std::variant<publisher<messages_ts> ...>;
  static_vector<publisher_t , subcription_buffer_size> publishers;
};
}


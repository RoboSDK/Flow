#pragma once

#include "flow/metaprogramming.hpp"
#include "flow/message.hpp"

namespace flow {

template<typename callback_t>
struct message_callback {
  constexpr message_callback(callback_t cb) : callback(std::move(cb))
  {
    using traits = flow::metaprogramming::function_traits<decltype(cb)>;
    using message_t = std::decay_t<typename traits::template argument<0>::type>;
    static_assert(std::is_base_of_v<flow::message<message_t>, message_t>, "flow::message_callback: First argument of message callback must derive from flow::message");
  }

  constexpr callback_t get() const { return callback; }
  callback_t callback;
};
}// namespace flow
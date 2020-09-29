#pragma once

#include <NamedType/named_type.hpp>
#include "flow/metaprogramming.hpp"
#include "flow/request.hpp"

namespace flow {


template<typename callback_t>
struct request_callback {
  constexpr request_callback(callback_t cb) : callback(std::move(cb))
  {
    using traits = flow::metaprogramming::function_traits<decltype(cb)>;
    using request_t = std::decay_t<typename traits::template argument<0>::type>;
    static_assert(std::is_base_of_v<flow::request<request_t>, request_t>, "flow::request_callback: First argument of request callback must derive from flow::request");
  }

  constexpr callback_t get() const { return callback; }
  callback_t callback;
};
}// namespace flow

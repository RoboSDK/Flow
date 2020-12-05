#pragma once

#include <flow/metaprogramming.hpp>
#include <concepts>

namespace flow {
template <typename function_t>
using traits = flow::metaprogramming::function_traits<function_t>;

template <typename function_t>
concept spinner = requires (function_t f) {
  traits<function_t>::arity == 0;
  std::is_void_v<typename traits<function_t>::return_type>;
};

template <typename function_t>
concept producer = requires (function_t f) {
  traits<std::decay_t<function_t>>::arity == 0;
  not std::is_void_v<typename traits<std::decay_t<function_t>>::return_type>;
};

template <typename function_t>
concept consumer = requires (function_t f) {
  traits<function_t>::arity > 0;
  std::is_void_v<typename traits<function_t>::return_type>;
};

template <typename function_t>
concept transformer = requires (function_t f) {
  traits<function_t>::arity > 0;
  not std::is_void_v<typename traits<function_t>::return_type>;
};
}

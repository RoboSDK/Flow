#pragma once

#include <concepts>
#include <flow/metaprogramming.hpp>

namespace flow {
template<typename function_t>
using traits = flow::metaprogramming::function_traits<std::decay_t<function_t>>;

template<typename function_t>
concept spinner = traits<function_t>::arity == 0 and std::is_void_v<typename traits<function_t>::return_type>;

template<typename function_t>
concept producer = traits<function_t>::arity == 0 and not std::is_void_v<typename traits<function_t>::return_type>;

template<typename function_t>
concept consumer = traits<function_t>::arity > 0 and std::is_void_v<typename traits<function_t>::return_type>;

template<typename function_t>
concept transformer = traits<function_t>::arity > 0 and not std::is_void_v<typename traits<function_t>::return_type>;

template<typename function_t>
concept task = spinner<function_t> or producer<function_t> or consumer<function_t> or transformer<function_t>;
}// namespace flow

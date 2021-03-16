#pragma once

#include "flow/detail/forward.hpp"
#include "flow/detail/routine.hpp"

#include "flow/chain.hpp"

namespace flow {

template<typename routine_t>
concept is_begin_routine = is_transformer_routine<routine_t> or is_producer_routine<routine_t>;

template<typename routine_t>
concept is_end_routine = is_transformer_routine<routine_t> or is_consumer_routine<routine_t>;

constexpr auto operator|(is_chain auto&& current_chain, is_begin_routine auto&& routine)
{
  using chain_state = typename decltype(current_chain.state())::type;
  static_assert(is_init<chain_state>(),
    "Can only pass a flow::producer or flow::transformer at the beginning of a chain.");

  return chain<open_chain>(concat(forward(current_chain.routines), forward(routine)));
}

constexpr auto operator|(is_chain auto&& current_chain, is_end_routine auto&& routine)
{
  using chain_state = typename decltype(current_chain.state())::type;
  static_assert(is_open<chain_state>,
    "Can only pass a flow::consumer or flow::transformer at to an open chain.");

  return chain<closed_chain>(concat(forward(current_chain.routines), forward(routine)));
}

template<typename function_t>
concept is_chain_function = is_consumer_function<function_t> or is_producer_function<function_t> or is_transformer_function<function_t>;

constexpr auto operator|(is_chain auto&& current_chain, is_chain_function auto&& function)
{
  using function_t = decltype(function);
  using chain_state = typename decltype(current_chain.state())::type;

  static_assert(not is_closed<chain_state>(),
    "Can only pass a flow::consumer or flow::transformer at to an open chain.");

  if constexpr (is_init<chain_state>()) {
    static_assert(is_producer_function<function_t> or is_transformer_function<function_t>,
      "Chain can only be initialized with a producer of transformer function.");

    auto routine = detail::to_routine(forward(function));
    return chain<open_chain>(concat(forward(current_chain.routines), std::move(routine)));
  }
  else if constexpr (is_open<chain_state>()) {
    static_assert(is_transformer_function<function_t> or is_consumer_function<function_t>,
      "Chain can only be appended to by transformer functions or a consumer function.");

    if constexpr (is_transformer_function<function_t>) {
      return chain<open_chain>(concat(forward(current_chain.routines), forward(function)));
    }
    else {
      auto routine = detail::to_routine(forward(function));
      return chain<closed_chain>(concat(forward(current_chain.routines), std::move(routine)));
    }
  }
}

template<typename spinner_t>
concept is_chain_spinner = is_spinner_function<spinner_t> or is_spinner_routine<spinner_t>;

constexpr auto operator|(is_chain auto&& current_chain, is_chain_spinner auto&& spinner)
{
  static_assert(is_init<decltype(current_chain)::state>(), "Spinners may only be pushed into an empty chain.");

  auto routine = detail::to_routine(forward(spinner));
  return chain<closed_chain>(concat(forward(current_chain.routines), std::move(routine)));
}

constexpr auto operator|(is_chain auto&& chain_so_far, is_routine auto&& f)
{
  return chain(concat(forward(chain_so_far.functions), forward(f)));
}
}// namespace flow
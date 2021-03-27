#pragma once

#include "flow/detail/forward.hpp"
#include "flow/detail/routine.hpp"

#include "flow/chain.hpp"

namespace flow {

constexpr auto operator|(is_chain auto&& current_chain, is_transformer_routine auto&& routine)
{
  using chain_state = typename decltype(current_chain.state())::type;
  static_assert(is_init<chain_state>() or is_open<chain_state>(),
                "flow::transform goes at the beginning of a chain or any open chain.");

  return detail::make_appended_chain<open_chain>(std::move(current_chain), forward(routine));
}

constexpr auto operator|(is_chain auto&& current_chain, is_publisher_routine auto&& routine)
{
  using chain_state = typename decltype(current_chain.state())::type;
  static_assert(is_init<chain_state>(),
    "Can only pass a flow::publish or flow::transform at the beginning of a chain.");

  return detail::make_appended_chain<open_chain>(std::move(current_chain), forward(routine));
}

constexpr auto operator|(is_chain auto&& current_chain, is_subscriber_routine auto&& routine)
{
  using chain_state = typename decltype(current_chain.state())::type;
  static_assert(is_open<chain_state>,
    "Can only pass a flow::subscribe or flow::transform at to an open chain.");

  return detail::make_appended_chain<closed_chain>(std::move(current_chain), forward(routine));
}

template<typename function_t>
concept is_chain_function = is_subscriber_function<function_t> or is_publisher_function<function_t> or is_transformer_function<function_t>;

constexpr auto operator|(is_chain auto&& current_chain, is_chain_function auto&& function)
{
  using function_t = decltype(function);
  using chain_state = typename decltype(current_chain.state())::type;

  static_assert(not is_closed<chain_state>(),
    "Can only pass a flow::subscribe or flow::transform at to an open chain.");

  if constexpr (is_init<chain_state>()) {
    static_assert(is_publisher_function<function_t> or is_transformer_function<function_t>,
      "Chain can only be initialized with a publish of transform function.");

    auto routine = detail::to_routine(forward(function));
    return detail::make_appended_chain<open_chain>(std::move(current_chain), forward(routine));
  }
  else if constexpr (is_open<chain_state>()) {
    static_assert(is_transformer_function<function_t> or is_subscriber_function<function_t>,
      "Chain can only be appended to by transform functions or a subscribe function.");

    if constexpr (is_transformer_function<function_t>) {
      return detail::make_appended_chain<open_chain>(std::move(current_chain), forward(function));
    }
    else {
      auto routine = detail::to_routine(forward(function));
      return detail::make_appended_chain<closed_chain>(std::move(current_chain), std::move(routine));
    }
  }
}

template<typename spinner_t>
concept is_chain_spinner = is_spinner_function<spinner_t> or is_spinner_routine<spinner_t>;

constexpr auto operator|(is_chain auto&& current_chain, is_chain_spinner auto&& spinner)
{
  using chain_t = decltype(current_chain);
  static_assert(is_init<chain_t::state>(), "Spinners may only be pushed into an empty chain.");

  auto routine = detail::to_routine(forward(spinner));
  return detail::make_appended_chain<closed_chain>(std::move(current_chain), std::move(routine));
}
}// namespace flow
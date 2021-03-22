#pragma once

#include <utility>

#include "flow/detail/forward.hpp"
#include "flow/detail/metaprogramming.hpp"

#include "flow/concepts.hpp"
#include "flow/literals.hpp"

namespace flow {

struct open_chain {
};
struct init_chain {
};
struct closed_chain {
};

struct chain_tag {
};

template<typename chain_t>
concept is_chain = std::is_base_of_v<chain_tag, std::decay_t<chain_t>>;

template<typename chain_state_t>
concept is_chain_state = std::is_same_v<chain_state_t, open_chain> or std::is_same_v<chain_state_t, init_chain> or std::is_same_v<chain_state_t, closed_chain>;

namespace detail {
  template<is_chain_state current_state, units::Unit Unit, units::ScalableNumber Rep, typename... routines_t>
  struct chain_impl : chain_tag {
    std::tuple<routines_t...> routines{};
    units::physical::si::frequency<Unit, Rep> frequency{};

    constexpr chain_impl(
      units::physical::si::frequency<Unit, Rep> freq,
      std::tuple<routines_t...>&& rs) : routines(std::move(rs)), frequency{ freq } {}

    // TODO: Clean this up
    constexpr metaprogramming::type_container<current_state> state()
    {
      return metaprogramming::type_container<current_state>{};
    }

    /*
  TODO: Need to use this to make function traits happy. Remove this at some point once
       I am smart enough to figure out how not to make this crash with function traits when
       checking for operator()
   */
    void operator()() {}
  };
}// namespace detail

template<is_chain_state state = init_chain, units::Unit U, units::ScalableNumber Rep , typename... routines_t > constexpr auto
    chain(units::physical::si::frequency<U, Rep> freq, std::tuple<routines_t...> && routines = std::tuple<>{})
{
  return detail::chain_impl<state, U, Rep, routines_t...>(freq, std::move(routines));
}

template<typename routine_t>
concept is_valid_chain_item = is_routine<routine_t> or is_function<routine_t>;

template<typename... routines_t>
constexpr auto concat(std::tuple<routines_t...>&& routines, is_valid_chain_item auto&& new_routine)
{
  return std::tuple_cat(std::move(routines), std::make_tuple(forward(new_routine)));
}

template<is_chain_state state>
constexpr bool is_open()
{
  return std::is_same_v<state, open_chain>;
}

template<is_chain_state state>
constexpr bool is_init()
{
  return std::is_same_v<state, init_chain>;
}

template<is_chain_state state>
constexpr bool is_closed()
{
  return std::is_same_v<state, closed_chain>;
}
}// namespace flow
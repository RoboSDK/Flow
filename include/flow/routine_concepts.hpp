#pragma once

#include "flow/detail/metaprogramming.hpp"
#include "flow/user_routine.hpp"
#include <concepts>

/**
 * A callable_routine is a core concept of this framework; a building block of a network.
 *
 * A callable_routine is a callable where it's dependencies (arguments) and return type define its category
 */

namespace flow {

template<typename callable_t>
using traits = detail::metaprogramming::function_traits<std::decay_t<callable_t>>;

template<typename routine_t>
concept is_user_routine = std::is_base_of_v<user_routine, routine_t>;

template<typename... routines_t>
concept are_user_routines = (is_user_routine<routines_t> and ...);

/**
 * A spinner_function is a callable which has a void return type and requires no arguments
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept spinner_function = not is_user_routine<callable_t> and traits<callable_t>::arity == 0 and std::is_void_v<typename traits<callable_t>::return_type>;

/**
 * A producer_function is a callable which has a return type and requires no arguments
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept producer_function = not is_user_routine<callable_t> and traits<callable_t>::arity == 0 and not std::is_void_v<typename traits<callable_t>::return_type>;

/**
 * A consumer_function is a callable which has no return type and requires at least one argument
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept consumer_function = not is_user_routine<callable_t> and traits<callable_t>::arity >= 1 and std::is_void_v<typename traits<callable_t>::return_type>;

/**
 * A transformer_function is a callable which has a return type and requires at least one argument
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept transformer_function = traits<callable_t>::arity >= 1 and not std::is_void_v<typename traits<callable_t>::return_type>;

template<typename callable_t>
concept callable_routine = spinner_function<callable_t> or producer_function<callable_t> or consumer_function<callable_t> or transformer_function<callable_t>;

template<typename... callables_t>
concept callable_routines = (callable_routine<callables_t> and ...);
}// namespace flow

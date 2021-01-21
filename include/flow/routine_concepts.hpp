#pragma once

#include <concepts>
#include "flow/metaprogramming.hpp"
#include "flow/user_routine.hpp"

/**
 * A routine is a core concept of this framework; a building block of a network.
 *
 * A routine is a callable where it's dependencies (arguments) and return type define its category
 */

namespace flow {

template<typename callable_t>
using traits = flow::metaprogramming::function_traits<std::decay_t<callable_t>>;

template<typename routine_t>
concept user_routine_concept = std::is_base_of_v<user_routine, routine_t>;

/**
 * A spinner_routine is a callable which has a void return type and requires no arguments
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept spinner_routine = traits<callable_t>::arity == 0 and std::is_void_v<typename traits<callable_t>::return_type>;

/**
 * A producer_routine is a callable which has a return type and requires no arguments
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept producer_routine = traits<callable_t>::arity == 0 and not std::is_void_v<typename traits<callable_t>::return_type>;

/**
 * A consumer_routine is a callable which has no return type and requires at least one argument
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept consumer_routine = traits<callable_t>::arity >= 1 and std::is_void_v<typename traits<callable_t>::return_type>;

/**
 * A transformer_routine is a callable which has a return type and requires at least one argument
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept transformer_routine = traits<callable_t>::arity >= 1 and not std::is_void_v<typename traits<callable_t>::return_type>;

template<typename callable_t>
concept routine = spinner_routine<callable_t> or producer_routine<callable_t> or consumer_routine<callable_t> or transformer_routine<callable_t>;
}// namespace flow

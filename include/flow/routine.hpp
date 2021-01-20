#pragma once

#include <concepts>
#include <flow/metaprogramming.hpp>

/**
 * A routine is a core concept of this framework; a building block of a network.
 *
 * A routine is a callable where it's dependencies (arguments) and return type define its category
 */

namespace flow {

template<typename callable_t>
using traits = flow::metaprogramming::function_traits<std::decay_t<callable_t>>;

/**
 * A spinner is a callable which has a void return type and requires no arguments
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept spinner = traits<callable_t>::arity == 0 and std::is_void_v<typename traits<callable_t>::return_type>;

/**
 * A producer is a callable which has a return type and requires no arguments
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept producer = traits<callable_t>::arity == 0 and not std::is_void_v<typename traits<callable_t>::return_type>;

/**
 * A consumer is a callable which has no return type and requires at least one argument
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept consumer = traits<callable_t>::arity >= 1 and std::is_void_v<typename traits<callable_t>::return_type>;

/**
 * A transformer is a callable which has a return type and requires at least one argument
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept transformer = traits<callable_t>::arity >= 1 and not std::is_void_v<typename traits<callable_t>::return_type>;

template<typename callable_t>
concept routine = spinner<callable_t> or producer<callable_t> or consumer<callable_t> or transformer<callable_t>;
}// namespace flow

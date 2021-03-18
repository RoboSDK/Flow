#pragma once

#include "flow/detail/metaprogramming.hpp"
#include "flow/configuration.hpp"

#include <concepts>

/**
 * A routine is a core concept of this framework; a building block of a network.
 *
 * A routine is a callable where it's dependencies (arguments) and return type define its category
 */

namespace flow {

/// shortcut to function traits to be used locally here
namespace detail {
template<typename callable_t>
using traits = detail::metaprogramming::function_traits<std::decay_t<callable_t>>;
}

/**
 * Used to determine if a user made routine has a name they specified to publish to
 * @tparam routine_t The user made routine
 */
template<typename routine_t>
concept has_publisher_channel = requires(routine_t routine){
  routine.publish_to();
};

/**
 * Used to determine if a user made routine has a name they specified to subscribe to
 * @tparam routine_t The user made routine
 */
template<typename routine_t>
concept has_subscription_channel = requires(routine_t routine){
  routine.subscribe_to();
};


/**
 * These concepts are used to determine if a routine is an implementation of a routine
 *
 * TODO: There has to be a better way than to have the using declarations at the top of each implementation for a routine
 */
template<typename transformer_t>
concept is_transformer_routine = std::is_same_v<typename transformer_t::is_transformer, std::true_type>;

template<typename spinner_t>
concept is_spinner_routine = std::is_same_v<typename spinner_t::is_spinner, std::true_type>;

template<typename publisher_t>
concept is_publisher_routine = std::is_same_v<typename publisher_t::is_publisher, std::true_type>;

template<typename consumer_t>
concept is_consumer_routine = std::is_same_v<typename consumer_t::is_consumer, std::true_type>;

/// all routines made by make_routine have a callback function
template <typename routine_t>
concept has_callback_function = requires(routine_t routine) {
  routine.callback();
};

template<typename routine_t>
concept is_routine = has_callback_function<routine_t> and (is_spinner_routine<routine_t> or is_publisher_routine<routine_t> or is_consumer_routine<routine_t> or is_transformer_routine<routine_t>);

template <typename network_t>
concept is_network = std::is_same_v<typename network_t::is_network, std::true_type>;

/**
 * A spinner_function is a callable which has a void return type and requires no arguments
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept is_spinner_function =  not has_callback_function<callable_t> and not is_network<callable_t> and not is_routine<callable_t> and detail::traits<callable_t>::arity == 0 and std::is_void_v<typename detail::traits<callable_t>::return_type>;

/**
 * A publisher_function is a callable which has a return type and requires no arguments
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept is_publisher_function =  not has_callback_function<callable_t> and not is_network<callable_t> and not is_routine<callable_t> and detail::traits<callable_t>::arity == 0 and not std::is_void_v<typename detail::traits<callable_t>::return_type>;

/**
 * A consumer_function is a callable which has no return type and requires at least one argument
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept is_consumer_function =  not has_callback_function<callable_t> and not is_network<callable_t> and not is_routine<callable_t> and detail::traits<callable_t>::arity >= 1 and std::is_void_v<typename detail::traits<callable_t>::return_type>;

/**
 * A transformer_function is a callable which has a return type and requires at least one argument
 * @tparam callable_t Any callable type
 */
template<typename callable_t>
concept is_transformer_function =  (not has_callback_function<callable_t> and not is_network<callable_t> and not is_routine<callable_t> and detail::traits<callable_t>::arity >= 1 and not std::is_void_v<typename detail::traits<callable_t>::return_type>);

template<typename callable_t>
concept is_function = is_spinner_function<callable_t> or is_publisher_function<callable_t> or is_consumer_function<callable_t> or is_transformer_function<callable_t>;

template <typename... functions_t>
concept are_functions = (is_function<functions_t> and ...);

template <typename... routines_t>
concept are_routines = (is_routine<routines_t> and ...);
}// namespace flow

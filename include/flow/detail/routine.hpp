#pragma once

#include "flow/concepts.hpp"
#include "flow/detail/forward.hpp"

#include "flow/producer.hpp"
#include "flow/consumer.hpp"
#include "flow/transformer.hpp"
#include "flow/spinner.hpp"

namespace flow::detail {

template<flow::is_function function_t>
std::string get_publish_to(function_t& function)
{
  if constexpr (flow::has_publisher_channel<function_t>) {
    return function.publish_to();
  }
  else {
    return "";
  }
}

template<flow::is_function function_t>
std::string get_subscribe_to(function_t& function)
{
  if constexpr (flow::has_subscription_channel<function_t>) {
    return function.subscribe_to();
  }
  else {
    return "";
  }
}

constexpr auto to_routine(is_function auto&& function)
{
  using function_t = decltype(function);

  if constexpr (is_transformer_function<function_t>) {
    return transformer(forward(function), get_subscribe_to(function), get_publish_to(function));
  }
  else if constexpr (is_consumer_function<function_t>) {
    return flow::consumer(function, get_subscribe_to(function));
  }
  else if constexpr (is_producer_function<function_t>) {
    return flow::producer(function, get_publish_to(function));
  }
  /**
         * If you change this please be careful. The constexpr check for a spinner function seems to
         * be broken and I'm not sure why. I need to explicitly check if it's not a routine, and remove the
         * requirement from the spinner_impl constructor because otherwise the routines generates by make_routine
         * will fail because they do not implement operator(). The operator() check is in metaprogramming.hpp
         * with the function traits section at the bottom of the header.
         */
  else if constexpr (not is_routine<function_t> and is_spinner_function<function_t>) {
    return flow::spinner(function);
  }
}

constexpr auto to_routine(is_routine auto&& routine) {
  return forward(routine);
}

}// namespace flow::detail
#pragma once

#include "flow/concepts.hpp"
#include "flow/detail/forward.hpp"

#include "flow/publisher.hpp"
#include "flow/spinner.hpp"
#include "flow/subscriber.hpp"
#include "flow/transformer.hpp"

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
    auto subscription_channel = get_subscribe_to(function);
    auto publish_channel = get_publish_to(function);
    return transform(__forward(function), subscription_channel, publish_channel);
  }
  else if constexpr (is_subscriber_function<function_t>) {
    auto subscription_channel = get_subscribe_to(function);
    return subscribe(__forward(function), subscription_channel);
  }
  else if constexpr (is_publisher_function<function_t>) {
    auto publish_channel = get_publish_to(function);
    return publish(__forward(function), publish_channel);
  }
  /**
         * If you change this please be careful. The constexpr check for a spinner function seems to
         * be broken and I'm not sure why. I need to explicitly check if it's not a routine, and remove the
         * requirement from the spinner_impl constructor because otherwise the routines generates by make_routine
         * will fail because they do not implement operator(). The operator() check is in metaprogramming.hpp
         * with the function traits section at the bottom of the header.
         */
  else if constexpr (not is_routine<function_t> and is_spinner_function<function_t>) {
    return flow::spinner(__forward(function));
  }
}

constexpr auto to_routine(is_routine auto&& routine)
{
  return __forward(routine);
}

}// namespace flow::detail
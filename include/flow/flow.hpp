#pragma once

#include "flow/consumer.hpp"
#include "flow/network.hpp"
#include "flow/producer.hpp"
#include "flow/spinner.hpp"
#include "flow/transformer.hpp"

namespace flow {
/**
 * Creates a global network using the routines and functions and spins it up.
 *
 * The constraint on routines_t is that they are not a global network. This will trigger
 * the function dedicated to spinning up a network directly.
 *
 * This function should be used in the context where there is no need to ever quit.
 * Passing in the routines directly means no handle is available to request cancellation.
 *
 * @tparam configuration_t A compile time configuration file. It may or may not be from the user.
 * @tparam routines_t A list of routines that will be linked to a global network
 */
template<typename configuration_t, flow::not_is_network... routines_t>
auto spin(routines_t&&... routines)
{
  using network_t = flow::network<configuration_t>;
  network_t network{};

  auto routines_array = detail::make_mixed_array(std::forward<decltype(routines)>(routines)...);
  std::for_each(std::begin(routines_array), std::end(routines_array), detail::make_visitor([&](auto& r) {
    using routine_t = decltype(r);

    if constexpr (transformer_function<routine_t>) {
      network.push(make_transformer(r));
    }
    else if constexpr (consumer_function<routine_t>) {
      network.push(flow::make_consumer(r));
    }
    else if constexpr (producer_function<routine_t>) {
      network.push(flow::make_producer(r));
    }
    else if constexpr (spinner_function<routine_t>) {
      network.push(flow::make_spinner(r));
    }
    else if constexpr (routine<routine_t>) {
      network.push(std::move(r));
    }
  }));

  return cppcoro::sync_wait(network.spin());
}

/**
 * This function is called when user routines and functions are passed in directly
 * to the spin function without a compile time configuration passed in by the user.
 *
 * The default library configuration will be used.
 *See above for more
 * @param routines
 */
auto spin(flow::not_is_network auto&&... routines)
{
  return spin<flow::configuration>(std::forward<decltype(routines)>(routines)...);
}

/**
 * When spinning up a global network this function will be called.
 * @param global_network  The network where user name channel names are used
 */
auto spin(flow::is_network auto&& global_network)
{
  return cppcoro::sync_wait(global_network.spin());
}
}// namespace flow

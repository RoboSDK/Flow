#pragma once

#include "flow/consumer.hpp"
#include "flow/producer.hpp"
#include "flow/spinner.hpp"
#include "flow/transformer.hpp"

#include "flow/network.hpp"

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
template<typename configuration_t = flow::configuration, typename... routines_t>
auto spin(routines_t&&... routines)
{
  auto network = flow::network(std::forward<routines_t>(routines)...);;
  return cppcoro::sync_wait(network.spin());
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

#ifndef MODULES_CORE_FLOW_HPP
#define MODULES_CORE_FLOW_HPP

#include "flow/AutonomousSystem.hpp"

namespace flow {
/**
 * Parse arguments passed in to application execute the options selected
 * @param argc The argc in main
 * @param argv The argv in main
 */
void begin(int argc, const char **argv);

template <typename ...Layers>
constexpr decltype(auto) make_system()
{
  // TODO: Check for repeats and add constraints
  return System<Layers...>{};
}
}// namespace flow

#endif//MODULES_CORE_FLOW_HPP

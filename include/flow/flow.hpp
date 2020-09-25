#ifndef MODULES_CORE_FLOW_HPP
#define MODULES_CORE_FLOW_HPP

#include "flow/AutonomousSystem.hpp"
#include "flow/utility/metaprogramming.hpp"

namespace flow {
/**
 * Parse arguments passed in to application execute the options selected
 * @param argc The argc in main
 * @param argv The argv in main
 */
void begin(int argc, const char **argv);

template<typename... Layers>
constexpr decltype(auto) make_system() {
  static_assert(std::is_same_v<std::tuple<Layers...>, decltype(metaprogramming::make_type_set<Layers...>(std::tuple<>()))>,
    "A unique set of layers must be passed in to flow::make_system");

  return System<Layers...>{};
}
}// namespace flow

#endif//MODULES_CORE_FLOW_HPP

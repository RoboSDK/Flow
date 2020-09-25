//
// Created by manuelmeraz on 9/17/20.
//

#ifndef FLOW_AUTONOMOUSSYSTEM_HPP
#define FLOW_AUTONOMOUSSYSTEM_HPP

#include "flow/perception/PerceptionSystem.hpp"
#include "flow/executive/ExecutiveSystem.hpp"
#include "flow/utility/MixedArray.hpp"
#include "flow/System.hpp"


namespace flow {

template <typename PerceptionSystem, typename ExecutiveSystem>
class AutonomousSystem {
public:
  template <typename... PerceptionLayers, typename ...ExecutiveLayers>
  constexpr AutonomousSystem(System<PerceptionLayers...> perception, System<ExecutiveLayers...> executive) : m_perception(flow::make_perception_system(perception)),
                                                                                                   m_executive(flow::make_executive_system(executive)) {}
private:
  PerceptionSystem m_perception{};
  ExecutiveSystem m_executive{};
};

template <typename... PerceptionLayers, typename... ExecutiveLayers>
constexpr decltype(auto) make_autonomous_system(System<PerceptionLayers...> perception_system, System<ExecutiveLayers...> executive_system)
{
  return AutonomousSystem<PerceptionSystem<PerceptionLayers...>, ExecutiveSystem<ExecutiveLayers...>>(perception_system, executive_system);
}
}

#endif//FLOW_AUTONOMOUSSYSTEM_HPP

//
// Created by manuelmeraz on 9/17/20.
//

#ifndef FLOW_PERCEPTIONSYSTEM_HPP
#define FLOW_PERCEPTIONSYSTEM_HPP

#include "flow/utility/MixedArray.hpp"
#include "flow/System.hpp"

namespace flow {

template <typename... Layers>
class PerceptionSystem
{
private:
  constexpr static MixedArray<Layers...> m_perception_layers = flow::make_mixed_array(Layers{} ...);
};

template <typename... Layers>
constexpr PerceptionSystem<Layers...> make_perception_system(System<Layers...> /*unused*/)
{
  return PerceptionSystem<Layers...>{};
}
}// namespace flow::autonomy

#endif//FLOW_PERCEPTIONSYSTEM_HPP

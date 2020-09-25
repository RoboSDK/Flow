#ifndef FLOW_EXECUTIVESYSTEM_HPP
#define FLOW_EXECUTIVESYSTEM_HPP

#include "flow/System.hpp"
#include "flow/utility/MixedArray.hpp"

namespace flow {
template <typename... Layers>
class ExecutiveSystem
{
private:
  static constexpr std::size_t N = sizeof...(Layers);
  constexpr static MixedArray<N, Layers...> m_executive_layers = flow::make_mixed_array(Layers{} ...);
};

template <typename... Layers>
constexpr ExecutiveSystem<Layers...> make_executive_system(System<Layers...> /*unused*/)
{
  return ExecutiveSystem<Layers...>{};
}
}

#endif//FLOW_EXECUTIVESYSTEM_HPP

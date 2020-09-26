#ifndef FLOW_SYSTEM_HPP
#define FLOW_SYSTEM_HPP

#include <type_traits>
#include "flow/utility/MixedArray.hpp"

namespace flow {
template<typename... Layers>
class System {
private:
  static constexpr std::size_t N = sizeof...(Layers);
  static constexpr MixedArray<N, Layers...> m_perception_layers = flow::make_mixed_array(Layers{}...);
};

template<typename... Layers>
constexpr decltype(auto) make_system() {
  static_assert(std::is_same_v<std::tuple<Layers...>, decltype(metaprogramming::make_type_set<Layers...>(std::tuple<>()))>,
    "A unique set of layers must be passed in to flow::make_system");

  return System<Layers...>{};
}
}// namespace flow
#endif//FLOW_SYSTEM_HPP

#ifndef FLOW_SYSTEM_HPP
#define FLOW_SYSTEM_HPP

#include <type_traits>
#include "flow/data_structures/MixedArray.hpp"
#include "flow/options.hpp"
#include "flow/linker.hpp"

namespace flow {
template<typename options_t, typename... Layers>
class System {
private:
  static constexpr std::size_t N = sizeof...(Layers);
  static constexpr MixedArray<N, Layers...> m_perception_layers = flow::make_mixed_array(Layers{}...);

  static constexpr options_t options{};
  static constexpr Linker<options.linker_buffer_size> linker;
};

template<typename... Layers>
constexpr decltype(auto) make_system()
{
  static_assert(std::is_same_v<std::tuple<Layers...>, decltype(metaprogramming::make_type_set<Layers...>(std::tuple<>()))>,
    "A unique set of layers must be passed in to flow::make_system");

  [[maybe_unused]] constexpr auto options = make_options(flow::linker_buffer_size{});
  return System<decltype(options), Layers...>{};
}

template<typename... Layers>
constexpr decltype(auto) make_system(auto&& options)
{
  static_assert(std::is_same_v<std::tuple<Layers...>, decltype(metaprogramming::make_type_set<Layers...>(std::tuple<>()))>,
    "A unique set of layers must be passed in to flow::make_system");

  return System<decltype(options), Layers...>{};
}
}// namespace flow
#endif//FLOW_SYSTEM_HPP

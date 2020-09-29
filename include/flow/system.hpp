#ifndef FLOW_SYSTEM_HPP
#define FLOW_SYSTEM_HPP

#include <type_traits>
#include "flow/data_structures/MixedArray.hpp"
#include "flow/options.hpp"
#include "flow/linker.hpp"

namespace flow {
template <typename... Layers>
concept no_repeated_layers = std::is_same_v<std::tuple<Layers...>, decltype(metaprogramming::make_type_set<Layers...>(std::tuple<>()))>;

template<options_concept options_t, typename... Layers>
requires no_repeated_layers<Layers...>
class System {
private:
  static constexpr std::size_t N = sizeof...(Layers);
  static constexpr MixedArray<N, Layers...> m_perception_layers = flow::make_mixed_array(Layers{}...);

  static constexpr options_t options{};
  static constexpr Linker<options.linker_buffer_size> linker;
};

template<typename... Layers>
requires no_repeated_layers<Layers...>
constexpr decltype(auto) make_system()
{
  [[maybe_unused]] constexpr auto options = make_options(flow::linker_buffer_size{});
  return System<decltype(options), Layers...>{};
}

template<typename... Layers>
requires no_repeated_layers<Layers...>
constexpr decltype(auto) make_system(auto&& options = flow::make_options(flow::linker_buffer_size{}))
{
  return System<decltype(options), Layers...>{};
}
}// namespace flow
#endif//FLOW_SYSTEM_HPP

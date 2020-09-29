#ifndef FLOW_SYSTEM_HPP
#define FLOW_SYSTEM_HPP

#include <type_traits>
#include "flow/data_structures/MixedArray.hpp"
#include "flow/options.hpp"
#include "flow/scheduler.hpp"

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
  static constexpr scheduler<options.scheduler_subscriber_buffer_size, options.scheduler_publisher_buffer_size> scheduler;
};

template<typename... Layers>
requires no_repeated_layers<Layers...>
consteval decltype(auto) make_system()
{
  [[maybe_unused]] constexpr auto options = flow::make_options();
  return System<decltype(options), Layers...>{};
}

template<typename... Layers>
requires no_repeated_layers<Layers...>
consteval decltype(auto) make_system(options_concept auto&& options)
{
  return System<decltype(options), Layers...>{};
}
}// namespace flow
#endif//FLOW_SYSTEM_HPP

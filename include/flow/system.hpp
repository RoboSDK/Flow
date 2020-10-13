#ifndef FLOW_SYSTEM_HPP
#define FLOW_SYSTEM_HPP

#include "flow/channel.hpp"
#include "flow/channel_registry.hpp"
#include "flow/configuration.hpp"
#include "flow/data_structures/mixed_array.hpp"
#include "flow/layer.hpp"
#include "flow/message_registry.hpp"
#include "flow/metaprogramming.hpp"

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all.hpp>
#include <frozen/unordered_map.h>

#include <bitset>
#include <ranges>
#include <type_traits>

namespace flow {
template<typename... Layers>
concept no_repeated_layers =
  std::is_same_v<std::tuple<Layers...>, decltype(metaprogramming::make_type_set<Layers...>(std::tuple<>()))>;

template<typename... Layers> requires no_repeated_layers<Layers...> class system {
};

template<typename... Layers> requires no_repeated_layers<Layers...> auto make_layers(system<Layers...> /*unused*/)

{
  return flow::make_mixed_array(Layers{}...);
}

template<typename... Layers> requires no_repeated_layers<Layers...> auto make_system() { return system<Layers...>{}; }

template<typename... message_ts>
std::vector<cppcoro::task<void>> make_communication_tasks(auto& scheduler,
  auto& channel_registry,
  message_registry<message_ts...> /*unused*/)
{
  using namespace flow::metaprogramming;
  std::vector<cppcoro::task<void>> communication_tasks{};
  for_each<message_ts...>([&]<typename message_t>(type_container<message_t> /*unused*/) {
    auto channel_refs = channel_registry.template get_channels<message_t>();
    for (auto& channel : channel_refs) {
      communication_tasks.push_back(channel.get().open_communications(scheduler));
    }
  });

  return communication_tasks;
}

template<typename config_t> void spin(auto system, auto message_registry)
{
  flow::channel_registry<config_t> channel_registry{};

  auto layers = make_layers(system);
  std::ranges::for_each(layers, flow::make_visitor([&](auto& layer) {
    // TODO: resolve issue with layer not inheriting from base layer correctly
    //    flow::begin(layer, channel_registry);
    layer.begin(channel_registry);
  }));

  cppcoro::static_thread_pool scheduler;

  auto communication_tasks = make_communication_tasks(scheduler, channel_registry, message_registry);
  cppcoro::sync_wait(when_all_ready(std::move(communication_tasks)));

  std::ranges::for_each(layers, flow::make_visitor([&](auto& layer) {
    //    flow::end(layer);
    layer.end();
  }));
}

void spin(auto system, auto message_registry) { spin<flow::configuration>(system, message_registry); }
}// namespace flow
#endif// FLOW_SYSTEM_HPP

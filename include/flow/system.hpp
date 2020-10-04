#ifndef FLOW_SYSTEM_HPP
#define FLOW_SYSTEM_HPP

#include "flow/data_structures/mixed_array.hpp"
#include "flow/data_structures/static_vector.hpp"
#include "flow/metaprogramming.hpp"
#include "flow/options.hpp"
#include "flow/registry.hpp"
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all.hpp>
#include <frozen/unordered_map.h>
#include <type_traits>

namespace flow {
template<typename... Layers>
concept no_repeated_layers = std::is_same_v<std::tuple<Layers...>, decltype(metaprogramming::make_type_set<Layers...>(std::tuple<>()))>;

template<options_concept options_t, typename... Layers>
requires no_repeated_layers<Layers...> class System {
  static constexpr std::size_t N = sizeof...(Layers);

public:
  using options = options_t;
  cppcoro::task<void> begin([[maybe_unused]] auto& registry)

  {
    [[maybe_unused]] mixed_array<N, Layers...> layers = flow::make_mixed_array(Layers{}...);
    for (auto& layer : layers) {
      std::visit([&](auto& l) { l.begin(registry); }, layer);
    }

    cppcoro::static_thread_pool tp;
    cppcoro::io_service io;
    cppcoro::sequence_barrier<std::size_t> barrier;
    cppcoro::multi_producer_sequencer<std::size_t> sequencer{barrier, 64};

    std::vector<cppcoro::task<void>> tasks{};
    for (auto& [_, channel] : registry.m_channels) {
      auto task = std::visit([&](auto& c1) -> cppcoro::task<void> { return c1.open_communications(tp, io, barrier, sequencer); }, channel);
      tasks.push_back(std::move(task));
    }

//    co_await cppcoro::when_all(channel_tasks);
//    co_await new_channel.open_communications();
//    co_return;
    return cppcoro::task<void>{};
  }

private:
//  [[maybe_unused]] static constexpr options_t m_options{};
};

template<typename... Layers>
requires no_repeated_layers<Layers...> auto make_system()
{
  [[maybe_unused]] constexpr auto options = flow::make_options();
  return System<decltype(options), Layers...>{};
}

template<typename... Layers>
requires no_repeated_layers<Layers...> auto make_system(options_concept auto& options)
{
  return System<decltype(options), Layers...>{};
}

template<typename system_t, typename message_registry_t>
void spin(system_t& system, message_registry_t& messages)
{
  const auto make_registry = [&]<typename... message_ts>(flow::message_registry<message_ts...> & /*unused*/)
  {
    return flow::registry<typename system_t::options, message_ts...>{};
  };
  auto registry = make_registry(messages);

  cppcoro::static_thread_pool tp;
  cppcoro::io_service io;
  cppcoro::sequence_barrier<std::size_t> barrier;
  cppcoro::multi_producer_sequencer<std::size_t> sequencer{barrier, 64};

  std::vector<cppcoro::task<void>> tasks{};
  for (auto& [_, channel] : registry.m_channels) {
    auto task = std::visit([&](auto& c1) -> cppcoro::task<void> { return c1.open_communications(tp, io, barrier, sequencer); }, channel);
    tasks.push_back(std::move(task));
  }

  cppcoro::sync_wait(system.begin(registry));
}
}// namespace flow
#endif//FLOW_SYSTEM_HPP

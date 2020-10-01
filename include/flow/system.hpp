#ifndef FLOW_SYSTEM_HPP
#define FLOW_SYSTEM_HPP

#include "flow/data_structures/MixedArray.hpp"
#include "flow/data_structures/static_vector.hpp"
#include "flow/options.hpp"
#include "flow/registry.hpp"
#include "flow/metaprogramming.hpp"
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
  cppcoro::task<void> begin([[maybe_unused]] auto& registry, auto& channel_container)
  {
    for (auto& layer : m_layers) {
      std::visit([&](auto& l) { l.begin(registry); }, layer);
    }

    static constexpr std::size_t callback_buffer_size = 64;
    static constexpr std::size_t message_buffer_size = 64;

    std::vector<cppcoro::task<void>> tasks{};
      auto new_channel = std::visit([&](auto& callback)
      {
             using traits = flow::metaprogramming::function_traits<std::decay_t<decltype(callback)>>;
             using request_t = std::decay_t<typename traits::template argument<0>::type>;
             return flow::channel<request_t, callback_buffer_size, message_buffer_size>(registry.sub_info[0].channel_name);
      }, registry.sub_info[0].on_message);

    std::visit([&] (auto& cb) { new_channel.m_on_message_callbacks.push_back(cb); }, registry.sub_info[0].on_message);
    std::visit([&] (auto& cb) { new_channel.m_on_request_callbacks.push_back(cb); }, registry.pub_info[0].on_request);

//    std::vector<cppcoro::task<void>> tasks{};
//    for (auto& sub : registry.sub_info) {
//        std::visit([&](auto& callback) {
//           using traits = flow::metaprogramming::function_traits<std::decay_t<decltype(callback)>>;
//           using request_t = std::decay_t<typename traits::template argument<0>::type>;
//           auto new_channel = flow::channel<request_t, callback_buffer_size, message_buffer_size>(sub.channel_name);
//           channel_container.push_back(std::move(new_channel));
//        }, sub.on_message);
//      }
//
//      auto& channel = map.at(sub.channel_name);
//      std::visit([&](auto& c, auto& handle) { c.m_on_message_callbacks.push_back(handle); }, channel, sub.on_message);

//    for (auto& pub : registry.pub_info) {
//      if (map.find(pub.channel_name) == std::end(map)) {
//        auto channel = channel_t(pub.channel_name);
//        map.emplace(std::make_pair(pub.channel_name, std::move(channel)));
//      }
//
//      auto& channel = map.at(pub.channel_name);
//      std::visit([&](auto& c, auto& handle) { c.m_on_request_callbacks.push_back(handle); }, channel, pub.on_request);
//    }
//
//    std::vector<cppcoro::task<void>> channel_tasks{};

//    for (auto& [_, channel] : map) {
//      co_await std::visit([](auto& c1) -> cppcoro::task<void> { co_return c1.open_communications(); }, channel);
//    }

//    co_await cppcoro::when_all(channel_tasks);
    co_await new_channel.open_communications();
  }

private:
  [[maybe_unused]] static constexpr options_t m_options{};
  MixedArray<N, Layers...> m_layers = flow::make_mixed_array(Layers{}...);
};

template<typename... Layers>
requires no_repeated_layers<Layers...> consteval auto make_system()
{
  [[maybe_unused]] constexpr auto options = flow::make_options();
  return System<decltype(options), Layers...>{};
}

template<typename... Layers>
requires no_repeated_layers<Layers...> consteval auto make_system(options_concept auto& options)
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

  const auto make_channel_container = [&]<typename... message_ts>(flow::message_registry<message_ts...>  /*unused*/)
  {
         return std::vector<std::variant<flow::channel<message_ts, 64, 64> ...>>{};
  };

  auto channel_container = make_channel_container(messages);
  cppcoro::sync_wait(system.begin(registry, channel_container));
}
}// namespace flow
#endif//FLOW_SYSTEM_HPP

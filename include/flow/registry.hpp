#pragma once

#include <variant>

#include "flow/data_structures/string.hpp"
#include "flow/data_structures/static_vector.hpp"
#include "flow/channel.hpp"
#include "flow/metaprogramming.hpp"

namespace flow {
template <typename... message_ts>
class registry {
  using subscription_callback_t = std::variant<std::function<void(message_ts const&)> ...>;
  using publisher_callback_t = std::variant<std::function<void(message_ts &)> ...>;


  struct subscription_info {
    std::string channel_name;
    subscription_callback_t on_message;
  };

  struct publisher_info {
    std::string channel_name;
    publisher_callback_t on_request;
  };

public:
  using channel_t = std::variant<flow::channel<message_ts> ...>;

  template <typename message_t>
  void register_subscription(std::string&& channel_name, std::function<void(message_t const&)>&& on_message)
  {
    if (m_channels.find(channel_name) == m_channels.end()) {
      auto channel = flow::channel<message_t>{channel_name};
      channel.m_on_message_callbacks.push_back(on_message);
      m_channels.emplace(channel_name, channel);
    } else {
      auto channel = m_channels.at(channel_name);
      std::visit([&](auto& chan) { chan.m_on_message_callbacks.push_back(on_message); }, channel);
    }
  }

  template <typename message_t>
  void register_publisher(std::string&& channel_name, std::function<void(message_t&)>&& on_request)
  {
    if (m_channels.find(channel_name) == m_channels.end()) {
      auto channel = flow::channel<message_t>{channel_name};
      channel.m_on_request_callbacks.push_back(on_request);
      m_channels.emplace(channel_name, channel);
    } else {
      auto channel = m_channels.at(channel_name);
      std::visit([&](auto& chan) { chan.m_on_request_callbacks.push_back(on_request); }, channel);
    }
  }

  std::vector<subscription_info> sub_info;
  std::vector<publisher_info> pub_info;

  std::unordered_map<std::string, channel_t> m_channels;
};

template <typename message_t>
void subscribe_to(std::string channel_name, std::function<void(message_t const&)> on_message, auto& registry)
{
  registry.register_subscription(std::move(channel_name), std::move(on_message));
}

template <typename message_t>
void publish_to(std::string channel_name, std::function<void(message_t&)> on_request, auto& registry)
{
  registry.register_publisher(std::move(channel_name), std::move(on_request));
}

template <typename... message_ts>
constexpr auto make_registry()
{
  return message_registry<message_ts...>{};
}
}
#pragma once

#include <string>

#include "flow/callback_handle.hpp"
#include "flow/channel.hpp"
#include "flow/data_structures/channel_set.hpp"
#include "flow/hash.hpp"

#include <bitset>

namespace flow {

template<typename config_t>
class channel_registry {
public:
  template<typename message_t>
  flow::callback_handle<config_t> register_subscription(std::string&& channel_name, std::function<void(message_t const&)>&& on_message)
  {
    if (m_current_callback_id >= config_t::global::max_callbacks) {
      flow::logging::critical_throw("Current callback id is {}. Max callbacks is {}. Increase the value in configuration::global::max_callbacks.", m_current_callback_id, config_t::global::max_callbacks);
    }

    if (not m_channels.template contains<message_t>(channel_name)) {
      m_channels.put(channel<message_t, config_t>(channel_name));
      m_channel_names[hash<message_t>()].push_back(channel_name);
    }

    auto cancellation_handle = flow::cancellation_handle{};
    auto subscription = flow::cancellable_function<void(message_t const&)>(cancellation_handle.token(), std::move(on_message));

    auto& ch = m_channels.template at<message_t>(channel_name);
    ch.push_subscription(std::move(subscription));

    auto info = callback_info{
      .id = m_current_callback_id,
      .type = callback_type::subscription,
      .channel_name = channel_name,
      .message_type = typeid(message_t)
    };

    ++m_current_callback_id;
    return callback_handle<config_t>(std::move(info), std::move(cancellation_handle));
  }

  template<typename message_t>
  flow::callback_handle<config_t> register_publisher(std::string&& channel_name, std::function<void(message_t&)>&& on_request)
  {
    if (m_current_callback_id >= config_t::global::max_callbacks) {
      flow::logging::critical_throw("Current callback id is {}. Max callbacks is {}. Increase the value in configuration::global::max_callbacks.", m_current_callback_id, config_t::global::max_callbacks);
    }

    if (not m_channels.template contains<message_t>(channel_name)) {
      m_channels.put(channel<message_t, config_t>(channel_name));
      m_channel_names[hash<message_t>()].push_back(channel_name);
    }

    auto cancellation_handle = flow::cancellation_handle{};
    auto publisher = flow::cancellable_function<void(message_t&)>(cancellation_handle.token(), std::move(on_request));

    auto& ch = m_channels.template at<message_t>(channel_name);
    ch.push_publisher(std::move(publisher));

    auto info = callback_info{
      .id = m_current_callback_id,
      .type = callback_type::publisher,
      .channel_name = channel_name,
      .message_type = typeid(message_t)
    };

    ++m_current_callback_id;
    return callback_handle<config_t>(std::move(info), std::move(cancellation_handle));
  }

  template<typename message_t>
  bool contains(std::string const& channel_name)
  {
    return m_channels.template contains<message_t>(channel_name);
  }

  template<typename message_t>
  channel<message_t, config_t>& get_channel(std::string const& channel_name)
  {
    return m_channels.template at<message_t>(channel_name);
  }

  template<typename message_t>
  auto get_channels()
  {
    auto& channel_names = m_channel_names[hash<message_t>()];

    std::vector<std::reference_wrapper<channel<message_t, config_t>>> channels;
    for (const auto& name : channel_names) {
      channels.push_back(m_channels.template at<message_t>(name));
    }

    return channels;
  }

private:
  /// the message type will be used to map into the channel
  channel_set<config_t> m_channels;
  std::size_t m_current_callback_id{};

  /// Maps a type hash to all channel names associated with it
  std::unordered_map<std::size_t, std::vector<std::string>> m_channel_names;
};

template<typename message_t>
auto subscribe(std::string channel_name, auto& registry, std::function<void(const message_t&)> on_message)
{
  return registry.register_subscription(std::move(channel_name), std::move(on_message));
}

template<typename message_t>
auto publish(std::string channel_name, auto& registry, std::function<void(message_t&)> on_request)
{
  return registry.register_publisher(std::move(channel_name), std::move(on_request));
}
}// namespace flow
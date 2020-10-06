#pragma once

#include <string>

#include "flow/callback_handle.hpp"
#include "flow/channel.hpp"
#include "flow/data_structures/channel_set.hpp"
#include "flow/hash.hpp"

namespace flow {
class registry {
public:
  registry(volatile std::atomic_bool* program_is_running) : m_program_is_running(program_is_running) {}

  template<typename message_t>
  flow::callback_handle register_subscription(std::string&& channel_name, std::function<void(message_t const&)>&& on_message)
  {
    if (not m_channels.contains<message_t>(channel_name)) {
      m_channels.put(channel<message_t>(channel_name));
      m_channel_names[hash<message_t>()].push_back(channel_name);
    }

    auto cancellation_handle = flow::cancellation_handle{};
    auto subscription = flow::cancellable_callback<void, message_t const&>(cancellation_handle.token(), std::move(on_message));

    auto& ch = m_channels.at<message_t>(channel_name);
    ch.push_subscription(std::move(subscription));

    return callback_handle(std::move(cancellation_handle), m_program_is_running);
  }

  template<typename message_t>
  flow::callback_handle register_publisher(std::string&& channel_name, std::function<void(message_t&)>&& on_request)
  {
    if (not m_channels.contains<message_t>(channel_name)) {
      m_channels.put(channel<message_t>(channel_name));
      m_channel_names[hash<message_t>()].push_back(channel_name);
    }

    auto cancellation_handle = flow::cancellation_handle{};
    auto publisher = flow::cancellable_callback<void, message_t&>(cancellation_handle.token(), std::move(on_request));

    auto& ch = m_channels.at<message_t>(channel_name);
    ch.push_publisher(std::move(publisher));

    return callback_handle(std::move(cancellation_handle), m_program_is_running);
  }

  template <typename message_t>
  bool contains(std::string const& channel_name) {
    return m_channels.contains<message_t>(channel_name);
  }

  template<typename message_t>
  channel<message_t>& get_channel(std::string const& channel_name)
  {
    return m_channels.at<message_t>(channel_name);
  }

  template<typename message_t>
  auto get_channels()
  {
    auto& channel_names = m_channel_names[hash<message_t>()];

    std::vector<std::reference_wrapper<channel<message_t>>> channels;
    for (const auto& name : channel_names) {
       channels.push_back(m_channels.at<message_t>(name));
    }

    return channels;
  }

private:
  /// the message type will be used to map into the channel
  channel_set m_channels;

  /// Maps a type hash to all channel names associated with it
  std::unordered_map<std::size_t, std::vector<std::string>> m_channel_names;
  volatile std::atomic_bool* m_program_is_running{nullptr};
};

template<typename message_t>
flow::callback_handle subscribe(std::string channel_name, auto& registry, std::function<void(const message_t&)> on_message)
{
  return registry.register_subscription(std::move(channel_name), std::move(on_message));
}

template<typename message_t>
flow::callback_handle publish(std::string channel_name, auto& registry, std::function<void(message_t&)> on_request)
{
  return registry.register_publisher(std::move(channel_name), std::move(on_request));
}

}// namespace flow
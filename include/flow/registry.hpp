#pragma once

#include <string>

#include "flow/callback_handle.hpp"
#include "flow/channel.hpp"
#include "flow/data_structures/any_type_set.hpp"

namespace flow {
class registry {
public:
  registry(volatile std::atomic_bool* program_is_running) : m_program_is_running(program_is_running) {}

  template<typename message_t>
  flow::callback_handle register_subscription(std::string&& channel_name, std::function<void(message_t const&)>&& on_message)
  {
    if (not m_channels.contains<channel<message_t>>()) {
      m_channels.put(channel<message_t>(channel_name));
    }

    auto cancellation_handle = flow::cancellation_handle{};
    auto subscription = flow::cancellable_callback<void, message_t const&>(cancellation_handle.token(), std::move(on_message));

    auto& ch = m_channels.at<channel<message_t>>();
    ch.push_subscription(std::move(subscription));

    return callback_handle(std::move(cancellation_handle), m_program_is_running);
  }

  template<typename message_t>
  flow::callback_handle register_publisher(std::string&& channel_name, std::function<void(message_t&)>&& on_request)
  {
    if (not m_channels.contains<channel<message_t>>()) {
      m_channels.put(channel<message_t>(channel_name));
    }

    auto cancellation_handle = flow::cancellation_handle{};
    auto publisher = flow::cancellable_callback<void, message_t&>(cancellation_handle.token(), std::move(on_request));

    auto& ch = m_channels.at<channel<message_t>>();
    ch.push_publisher(std::move(publisher));

    return callback_handle(std::move(cancellation_handle), m_program_is_running);
  }

  template<typename message_t>
  channel<message_t>& get_channel()
  {
    return m_channels.at<channel<message_t>>();
  }

private:
  /// the message type will be used to map into the channel
  any_type_set m_channels;
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
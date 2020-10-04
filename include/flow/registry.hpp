#pragma once

#include <string>

#include "flow/channel.hpp"
#include "flow/data_structures/any_type_set.hpp"

namespace flow {
class registry {
public:
  template<typename message_t>
  void register_subscription(std::string&& channel_name, std::function<void(message_t const&)>&& on_message)
  {
    if (not channels.contains<channel<message_t>>()) {
      channels.put(channel<message_t>(channel_name));
    }

    auto& ch = channels.at<channel<message_t>>();
    ch.push_publisher(on_message);
  }

  template<typename message_t>
  void register_publisher(std::string&& channel_name, std::function<void(message_t&)>&& on_request)
  {
    if (not channels.contains<channel<message_t>>()) {
      channels.put(channel<message_t>(channel_name));
    }

    auto& ch = channels.at<channel<message_t>>();
    ch.push_publisher(on_request);
  }

  template<typename message_t>
  channel<message_t>& get_channel()
  {
    return channels.at<message_t>();
  }

  template<typename message_t>
  bool has_channel() const
  {
    return channels.contains<message_t>();
  }

private:
  /// the message type will be used to map intp the channel
  any_type_set channels;
};

template<typename message_t>
void subscribe(std::string channel_name, auto& registry, std::function<void(const message_t&)> on_message)
{
  registry.register_subscription(std::move(channel_name), std::move(on_message));
}

template<typename message_t>
void publish_to(std::string channel_name, std::function<void(message_t&)> on_request, auto& registry)
{
  registry.register_publisher(std::move(channel_name), std::move(on_request));
}

}// namespace flow
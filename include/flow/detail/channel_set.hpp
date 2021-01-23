#pragma once

#include <any>

#include "channel.hpp"

/**
 * A channel set will contain a unique channels only
 */
namespace flow::detail {

template <typename config_t>
class channel_set {
public:

  /**
   * Determine if the channel exists in the set
   * @tparam message_t The message type of the channel
   * @param channel_name The name of the channel
   * @return Whether or not the channele exists
   */
  template<typename message_t>
  bool contains(std::string const& channel_name = "")const
  {
    return m_channels.find(hash<message_t>(channel_name)) != m_channels.end();
  }

  /**
   * Give ownership of a channel to the channel set
   * @param channel Any channel
   */
  void put(auto&& channel)
  {
    m_channels[channel.hash()] = std::forward<decltype(channel)>(channel);
  }

  /**
   * Retrieve a channel
   * @tparam message_t The message type of the channel
   * @param channel_name The name of the channel
   * @return A reference to the channel
   */
  template<typename message_t>
  auto& at(std::string const& channel_name = "")
  {
    return std::any_cast<detail::channel<message_t, config_t>&>(m_channels.at(hash<message_t>(channel_name)));
  }

private:
  /**
   * Helper function used to hash the channel information to retrieve from the map
   * @tparam message_t The message type of the channel
   * @param channel_name  The channel name
   * @return The channel name and message type hashed
   */
  template<typename message_t>
  std::size_t hash(std::string const& channel_name) const
  {
    return typeid(message_t).hash_code() ^ std::hash<std::string>{}(channel_name);
  }

  std::unordered_map<std::size_t, std::any> m_channels{};
};
}// namespace flow

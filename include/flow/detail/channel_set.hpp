#pragma once

#include <any>

#include "multi_channel.hpp"

/**
 * A multi_channel set will contain a unique m_channels only
 */
namespace flow::detail {

template <typename config_t>
class channel_set {
public:

  /**
   * Determine if the multi_channel exists in the set
   * @tparam message_t The message type of the multi_channel
   * @param channel_name The name of the multi_channel
   * @return Whether or not the channele exists
   */
  template<typename message_t>
  bool contains(std::string const& channel_name = "")const
  {
    return m_channels.find(hash<message_t>(channel_name)) != m_channels.end();
  }

  /**
   * Give ownership of a multi_channel to the multi_channel set
   * @param channel Any multi_channel
   */
  void put(auto&& channel)
  {
    m_channels[channel.hash()] = std::forward<decltype(channel)>(channel);
  }

  /**
   * Retrieve a multi_channel
   * @tparam message_t The message type of the multi_channel
   * @param channel_name The name of the multi_channel
   * @return A reference to the multi_channel
   */
  template<typename message_t>
  auto& at(std::string const& channel_name = "")
  {
    return std::any_cast<detail::multi_channel<message_t, config_t>&>(m_channels.at(hash<message_t>(channel_name)));
  }

private:
  /**
   * Helper function used to hash the multi_channel information to retrieve from the map
   * @tparam message_t The message type of the multi_channel
   * @param channel_name  The multi_channel name
   * @return The multi_channel name and message type hashed
   */
  template<typename message_t>
  std::size_t hash(std::string const& channel_name) const
  {
    return typeid(message_t).hash_code() ^ std::hash<std::string>{}(channel_name);
  }

  std::unordered_map<std::size_t, std::any> m_channels{};
};
}// namespace flow

#pragma once

#include <any>

namespace flow {

template <typename config_t>
class channel_set {
public:
  template<typename message_t>
  bool contains(std::string const& channel_name = "")const
  {
    return m_items.find(hash<message_t>(channel_name)) != m_items.end();
  }

  void put(auto&& channel)
  {
    m_items[channel.hash()] = std::forward<decltype(channel)>(channel);
  }

  template<typename message_t>
  auto& at(std::string const& channel_name)
  {
    return std::any_cast<channel<message_t, config_t>&>(m_items.at(hash<message_t>(channel_name)));
  }

private:
  template<typename message_t>
  std::size_t hash(std::string const& channel_name) const
  {
    return typeid(message_t).hash_code() ^ std::hash<std::string>{}(channel_name);
  }

  std::unordered_map<std::size_t, std::any> m_items{};
};
}// namespace flow

#pragma once
#include <typeinfo>

namespace flow::detail {

template<typename T>
std::size_t hash()
{
  return typeid(T).hash_code();
}

template<typename message_t>
std::size_t hash(std::string channel_name)
{
  return typeid(message_t).hash_code() ^ std::hash<std::string>{}(channel_name);
}
}// namespace flow

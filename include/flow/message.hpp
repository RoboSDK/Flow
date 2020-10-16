#pragma once

#include <sstream>

namespace flow {
template<typename message_t>
struct message {
  using type = message_t;
  struct metadata_t {
    std::size_t sequence{};
    bool last_message{};
  } metadata{};

  message_t message{};
};

template<typename wrapper_t>
concept wrapped_message = requires
{
  typename wrapper_t::type;
};

template<typename message_t>
requires wrapped_message<message_t>
constexpr auto is_wrapped() -> bool
{
  return true;
}

template<typename message_t>
constexpr auto is_wrapped() -> bool
{
  return false;
}

template <typename message_t>
std::string to_string(message<message_t> const& msg)
{
  std::stringstream ss;

  ss << "message {";
  ss << "sequence : " << msg.metadata.sequence;
  ss << ", last_message : " << std::boolalpha << msg.metadata.last_message;
  ss << ", message_type: " << typeid(message_t).name();
  ss << "}";
  return ss.str();
}

}// namespace flow
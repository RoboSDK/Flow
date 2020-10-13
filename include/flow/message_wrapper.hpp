#pragma once

namespace flow {
template<typename message_t>
struct message_wrapper {
  using type = message_t;
  struct metadata_t {
    std::size_t sequence{};
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
}// namespace flow
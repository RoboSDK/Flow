#pragma once

namespace flow {
template<typename... ts>
struct message_registry {
};

template<typename... message_ts>
constexpr auto make_registry()
{
  return message_registry<message_ts...>{};
}
}// namespace flow

#pragma once

namespace flow {
template<typename... ts>
struct messages {
};

template<typename... message_ts>
constexpr auto make_messages()
{
  return messages<message_ts...>{};
}
}// namespace flow

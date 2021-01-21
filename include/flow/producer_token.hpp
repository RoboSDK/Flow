#pragma once

#include <stack>
#include <cppcoro/sequence_range.hpp>

namespace flow {

template <typename message_t>
class producer_token {
public:
  auto& messages() { return m_messages; }
  auto& sequences() { return m_sequences; }
private:
  std::stack<message_t> m_messages{};
  cppcoro::sequence_range<std::size_t> m_sequences{};
};
}
#pragma once

#include <stack>
#include <cppcoro/sequence_range.hpp>

namespace flow::detail {

template <typename message_t>
struct producer_token {
  std::stack<message_t> messages{};
  cppcoro::sequence_range<std::size_t> sequences{};
};
}
#pragma once

#include <queue>
#include <cppcoro/sequence_range.hpp>

namespace flow::detail {

template <typename message_t>
struct publisher_token {
  std::queue<message_t> messages{};
  cppcoro::sequence_range<std::size_t> sequences{};

  std::size_t sequence{};
};}
#pragma once

#include <stack>
#include <cppcoro/sequence_range.hpp>

namespace flow {

template <typename message_t>
struct consumer_token {
  std::size_t end_sequence{};
  std::size_t sequence{};
  std::size_t last_sequence_published{};
};
}
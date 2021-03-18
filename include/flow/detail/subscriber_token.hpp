#pragma once

namespace flow::detail {

template <typename message_t>
struct subscriber_token {
  std::size_t end_sequence{};
  std::size_t sequence{};
  std::size_t last_sequence_published{};
};
}
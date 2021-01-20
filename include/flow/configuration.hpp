#pragma once

namespace flow {
struct configuration {
  static constexpr std::size_t max_resources = 256;
  static constexpr std::size_t message_buffer_size = 256;
};
}// namespace flow
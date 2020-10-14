#pragma once

namespace flow {
struct configuration {
  struct global {
    static constexpr std::size_t max_callbacks = 256;
  };

  struct channel {
    static constexpr std::size_t message_buffer_size = 1024;
  };
};
}
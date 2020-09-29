#pragma once

namespace flow {

template <std::size_t subscriber_buffer_size, std::size_t publisher_buffer_size>
class scheduler {
  static constexpr std::size_t channel_buffer_size = std::max(subscriber_buffer_size, publisher_buffer_size);
};
}


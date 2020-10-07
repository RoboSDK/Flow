#pragma once

#include "../configuration.hpp"
#include <flow/metadata.hpp>

namespace mock {
struct lidar_message {
  static constexpr std::size_t capacity = mock::defaults::message_capacity;
  std::vector<double> points;

  flow::metadata metadata{};
};
}

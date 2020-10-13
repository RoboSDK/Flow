#pragma once

namespace mock {
struct lidar_message {
  static constexpr std::size_t capacity = 100;
  std::vector<double> points;
};
}

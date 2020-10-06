#pragma once

#include "../configuration.hpp"

namespace app {
struct lidar_data {
  static constexpr std::size_t capacity = mock::lidar::data_capacity;

  std::size_t id{};
  std::vector<double> points;
};
}

#pragma once

#include "configuration.hpp"

namespace app {
struct LidarData
{
  static constexpr std::size_t capacity = mock::lidar_data_capacity;
  std::vector<double> points;
};
}

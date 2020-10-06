#pragma once

#include "lidar_data.hpp"
#include <numeric>
#include <random>

namespace mock {
struct lidar_driver {
  lidar_data drive()
  {
    lidar_data data{};
    std::generate_n(std::back_inserter(data.points), lidar_data::capacity, [this] { return dist(rd); });
    return data;
  }
  std::random_device rd;
  std::uniform_real_distribution<double> dist{ mock::lidar::driver_distribution_range.first, mock::lidar::driver_distribution_range.second };
};
}// namespace mock

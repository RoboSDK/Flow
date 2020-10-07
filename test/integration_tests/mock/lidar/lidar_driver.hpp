#pragma once

#include "lidar_message.hpp"
#include <numeric>
#include <random>

namespace mock {
struct lidar_driver {
  lidar_message drive()
  {
    lidar_message data{};
    std::generate_n(std::back_inserter(data.points), lidar_message::capacity, [this] { return dist(rd); });
    return data;
  }
  std::random_device rd;
  std::uniform_real_distribution<double> dist{ 0, 10 };
};
}// namespace mock

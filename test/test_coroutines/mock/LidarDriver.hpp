#pragma once

#include "LidarData.hpp"
#include <numeric>
#include <random>

namespace app {
struct LidarDriver {
  LidarData drive()
  {
    LidarData data{};
    std::generate_n(std::back_inserter(data.points), LidarData::capacity, [this] { return dist(rd); });
    return data;
  }
  std::random_device rd;
  std::uniform_real_distribution<double> dist{ 0, 9 };
};
}// namespace app

#pragma once

namespace app {
struct LidarData
{
  static constexpr std::size_t capacity = 100;
  std::vector<double> points;
};
}

#pragma once

#include "flow/data_structures/static_vector.hpp"

namespace app {
struct LidarData
{
  static constexpr std::size_t capacity = 100;
  std::vector<double> points;
};
}

#pragma once

#include "lidar_message.hpp"
#include <numeric>
#include <random>

namespace mock {
struct lidar_driver {
  lidar_message drive()
  {
    return lidar_message{.magic_number=42};
  }
};
}// namespace mock

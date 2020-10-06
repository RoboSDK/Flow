#pragma once

#include "../configuration.hpp"
#include <flow/metadata.hpp>

namespace mock {
struct lidar_data {
  static constexpr std::size_t capacity = mock::lidar::data_capacity;
  std::vector<double> points;

  flow::metadata metadata{};
};
}

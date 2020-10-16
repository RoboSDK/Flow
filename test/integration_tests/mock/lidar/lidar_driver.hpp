#pragma once

#include "lidar_message.hpp"
#include <numeric>
#include <random>

namespace mock {
struct lidar_driver {
  static constexpr std::size_t NUM_PRECOMPUTED_MESSAGES = 1'000;
  lidar_driver() = default;

  lidar_message make_message()
  {
    lidar_message data{};
    std::generate_n(std::back_inserter(data.points), lidar_message::capacity, [this] { return dist(rd); });
    return data;
  }

  lidar_message drive()
  {
    current_message %= NUM_PRECOMPUTED_MESSAGES;
    return messages[current_message++];
  }


  std::random_device rd;
  std::uniform_real_distribution<double> dist{ 0, 10 };

  std::size_t current_message{};
  std::array<lidar_message, NUM_PRECOMPUTED_MESSAGES> messages{};
};
}// namespace mock

#pragma once

#include <flow/task.hpp>
#include <random>

#include "lidar_driver.hpp"
#include <chrono>
#include <flow/data_structures/static_vector.hpp>
#include <flow/data_structures/string.hpp>
#include <flow/logging.hpp>
#include <flow/registry.hpp>
#include <vector>

namespace mock {
template <typename config_t>
class lidar_transform_task final : public flow::task<lidar_transform_task<config_t>> {
public:
  void begin(flow::registry& channel_registry)
  {
    const auto on_message = [this](lidar_message const& message) {
      [[maybe_unused]] const auto transformed = std::reduce(std::begin(message.points), std::end(message.points), 0);
    };

    for (std::size_t i = 0; i < config_t::num_subscriptions; ++i) {
        flow::subscribe<lidar_message>("lidar_points", channel_registry, on_message);
    }
  }
};
}// namespace mock

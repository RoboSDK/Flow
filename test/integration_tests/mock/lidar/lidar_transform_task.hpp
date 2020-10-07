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
class lidar_transform_task final : public flow::task<lidar_transform_task> {
public:
  void begin(flow::registry& channel_registry)
  {
    const auto on_message = [this](lidar_message const& message) {
      ++num_messages;
      [[maybe_unused]] const auto transformed = std::reduce(std::begin(message.points), std::end(message.points), 0);
    };

    [[maybe_unused]] flow::callback_handle handle = flow::subscribe<lidar_message>(mock::defaults::channel_name, channel_registry, on_message);
  }
  std::size_t num_messages = 0;
};
}// namespace mock

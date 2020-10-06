#pragma once

#include <flow/task.hpp>
#include <random>

#include "LidarDriver.hpp"
#include <chrono>
#include <flow/data_structures/static_vector.hpp>
#include <flow/data_structures/string.hpp>
#include <flow/registry.hpp>
#include <flow/logging.hpp>
#include <vector>

namespace app {
class LidarTransformTask final : public flow::task<LidarTransformTask> {
public:
  void begin(flow::registry& channel_registry)
  {
    const auto on_message = [this](LidarData const& message) {
      ++num_messages;
      [[maybe_unused]] const auto transformed = std::reduce(std::begin(message.points), std::end(message.points), 0);
    };

    [[maybe_unused]] flow::callback_handle handle = flow::subscribe<LidarData>(mock::lidar_channel_name, channel_registry, on_message);
  }
  std::size_t num_messages = 0;
};
}// namespace app

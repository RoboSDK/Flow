#pragma once

#include <flow/task.hpp>
#include <random>

#include "LidarDriver.hpp"
#include <chrono>
#include <flow/data_structures/static_vector.hpp>
#include <flow/algorithms/to_string.hpp>
#include <flow/registry.hpp>
#include <flow/logging.hpp>
#include <vector>

namespace app {
class LidarTransformTask final : public flow::task<LidarTransformTask> {
public:
  void begin(auto& registry)
  {
    const auto on_message = [this](LidarData const& message) {
      [[maybe_unused]] const auto transformed = std::reduce(std::begin(message.points), std::end(message.points), 0);
      flow::logging::info("Transformed!");
    };

    flow::publish_to<LidarData>("lidar_data", on_message, registry);
  }
};
}// namespace app

#pragma once

#include <flow/task.hpp>
#include <random>

#include "LidarDriver.hpp"
#include <chrono>
#include <flow/data_structures/static_vector.hpp>
#include <flow/registry.hpp>
#include <vector>

namespace {
app::LidarDriver g_driver{};
}

namespace app {
class LidarTask final : public flow::task<LidarTask> {
public:
  void begin(auto& registry){
    const auto on_request = [this](LidarData& message) {
      message = g_driver.drive();
    };

    flow::publish<LidarData>("lidar_data", registry, on_request);
  }
};
}// namespace app

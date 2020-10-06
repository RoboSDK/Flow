#pragma once

#include "lidar/lidar_task.hpp"
#include <flow/layer.hpp>
#include <flow/task.hpp>

namespace mock {
class sensor_layer : flow::layer<sensor_layer> {
public:
  void register_channels(auto& registry)
  {
    flow::begin(m_task, registry);
  }
private:
  lidar_task m_task;
};
}// namespace mock
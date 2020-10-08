#pragma once

#include "lidar/lidar_drive_task.hpp"
#include <flow/layer.hpp>
#include <flow/task.hpp>
#include "configuration.hpp"

namespace mock {
class sensor_layer : flow::layer<sensor_layer> {
public:
  void register_channels(auto& registry)
  {
    flow::begin(m_task, registry);
  }
private:
  mock::lidar_drive_task<configuration::defaults> m_task;
};
}// namespace mock
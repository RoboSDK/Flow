#pragma once

#include <flow/layer.hpp>
#include <flow/task.hpp>

#include "lidar/lidar_transform_task.hpp"

namespace app {
class transform_layer : flow::layer<transform_layer> {
public:
  void register_channels(auto& registry)
  {
    flow::begin(m_task, registry);
  }
private:
  lidar_transform_task m_task;
};
}// namespace app

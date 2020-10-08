#pragma once

#include <flow/layer.hpp>
#include <flow/task.hpp>

#include "lidar/lidar_transform_task.hpp"

namespace mock {
class transform_layer : flow::layer<transform_layer> {
public:
  void register_channels(auto& registry)
  {
    flow::begin(m_task, registry);
  }
private:
  lidar_transform_task<configuration::defaults> m_task;
};
}// namespace mock

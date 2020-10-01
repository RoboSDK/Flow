#pragma once

#include "LidarTask.hpp"
#include <flow/layer.hpp>
#include <flow/task.hpp>

namespace app {
class SensorLayer : flow::layer<SensorLayer> {
public:
  void begin(auto& registry)
  {
    flow::begin(m_task, registry);
  }
private:
  LidarTask m_task;
};
}// namespace app
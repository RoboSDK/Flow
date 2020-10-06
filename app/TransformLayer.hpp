#pragma once

#include <flow/layer.hpp>
#include <flow/task.hpp>

#include "LidarTransformTask.hpp"

namespace app {
class TransformLayer : flow::layer<TransformLayer> {
public:
  void register_channels(auto& registry)
  {
    flow::begin(m_task, registry);
  }
private:
  LidarTransformTask m_task;
};
}// namespace app

#include <flow/system.hpp>
#include <flow/message_registry.hpp>

#include "mock/LidarData.hpp"
#include "mock/TransformLayer.hpp"
#include "mock/SensorLayer.hpp"

int main()
{
  using namespace app;
  auto messages = flow::make_registry<LidarData>();
  auto system = flow::make_system<TransformLayer, SensorLayer>();
  flow::spin(system, messages);
}

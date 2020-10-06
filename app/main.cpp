#include <flow/system.hpp>
#include <flow/message_registry.hpp>

#include "LidarData.hpp"
#include "TransformLayer.hpp"
#include "SensorLayer.hpp"

int main()
{
  using namespace app;
  auto messages = flow::make_registry<LidarData>();
  auto system = flow::make_system<TransformLayer, SensorLayer>();
  flow::spin(system, messages);
}

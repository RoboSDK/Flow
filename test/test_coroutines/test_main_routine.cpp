#include <flow/messages.hpp>
#include <flow/system.hpp>

#include "mock/LidarData.hpp"
#include "mock/TransformLayer.hpp"
#include "mock/SensorLayer.hpp"

int main()
{
  using namespace app;
  auto messages = flow::make_messages<LidarData>();
  auto system = flow::make_system<TransformLayer, SensorLayer>();
  flow::spin(system, messages);
}

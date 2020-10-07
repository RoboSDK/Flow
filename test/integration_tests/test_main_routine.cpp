#include <flow/flow.hpp>

#include "mock/lidar/lidar_message.hpp"
#include "mock/sensor_layer.hpp"
#include "mock/transform_layer.hpp"

int main()
{
  using namespace mock;
  auto messages = flow::make_messages<lidar_message>();
  auto system = flow::make_system<transform_layer, sensor_layer>();
  flow::spin(system, messages);
}

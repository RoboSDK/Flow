#include <flow/flow.hpp>

#include "mock/lidar/lidar_message.hpp"
#include "mock/testing_layer.hpp"
#include "mock/lidar/lidar_transform_task.hpp"
#include "mock/lidar/lidar_drive_task.hpp"
#include "mock/configuration.hpp"

int main()
{
  using namespace mock;

  using drive_task_t = lidar_drive_task<configuration::defaults>;
  using transform_task_t = lidar_transform_task<configuration::defaults>;

  using sensor_layer_t = testing_layer<drive_task_t>;
  using transform_layer_t = testing_layer<transform_task_t>;

  auto messages = flow::make_messages<lidar_message>();
  auto system = flow::make_system<sensor_layer_t , transform_layer_t>();
  flow::spin(system, messages);
}

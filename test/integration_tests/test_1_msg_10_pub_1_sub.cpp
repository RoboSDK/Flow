#include <flow/flow.hpp>
#include <flow/configuration.hpp>

#include "mock/drive_task.hpp"
#include "mock/lidar/lidar_driver.hpp"
#include "mock/lidar/lidar_message.hpp"
#include "mock/testing_layer.hpp"
#include "mock/transform_task.hpp"

struct config_t {
  using driver_t = mock::lidar_driver;
  using message_t = flow::message<mock::lidar_message>;

  static constexpr auto channel_name = "lidar_points";

  static constexpr std::size_t total_messages = 1;
  static constexpr std::size_t num_publishers = 10;
  static constexpr std::size_t num_subscriptions = 1;
  static constexpr std::size_t num_sequences = num_publishers * total_messages;
  static constexpr std::size_t receive_messages = num_sequences * num_subscriptions;

  using default_config_t = flow::configuration;
};

int main()
{
  using namespace mock;

  using drive_task_t = drive_task<config_t>;
  using transform_task_t = transform_task<config_t>;

  using sensor_layer_t = testing_layer<drive_task_t>;
  using transform_layer_t = testing_layer<transform_task_t>;

  auto messages = flow::make_messages<config_t::message_t>();
  auto system = flow::make_system<sensor_layer_t, transform_layer_t>();
  flow::spin<config_t::default_config_t>(system, messages);
}

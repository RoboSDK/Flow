#pragma once


#include <flow/data_structures/static_vector.hpp>
#include <flow/data_structures/tick_function.hpp>
#include <flow/registry.hpp>
#include <flow/task.hpp>

#include <chrono>
#include <random>
#include <vector>

#include "lidar_driver.hpp"

namespace mock {
template<typename config_t>
class lidar_drive_task final : public flow::task<lidar_drive_task<config_t>> {
public:
  void begin(auto& channel_registry)
  {
    // every publisher will tick concurrently
    static mock::lidar_driver g_driver{};
    const auto on_request = [this](lidar_message& message) {
      m_tick();
      message = g_driver.drive();
    };

    std::generate_n(std::back_inserter(m_callback_handles), config_t::num_publishers, [&] {
      return flow::publish<lidar_message>("lidar_points", channel_registry, on_request);
    });

    constexpr auto tick_cycle = config_t::total_messages;
    m_tick = flow::tick_function(tick_cycle, [this] {
      flow::logging::info("Test complete: {} messages have been processed.", config_t::total_messages);
      m_callback_handles.front().stop_everything();// choose front arbitrarily
    });
  }

  std::vector<flow::callback_handle> m_callback_handles{};
  flow::tick_function m_tick{};
};
}// namespace mock

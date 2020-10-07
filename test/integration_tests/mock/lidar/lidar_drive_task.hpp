#pragma once


#include <flow/data_structures/static_vector.hpp>
#include <flow/data_structures/tick_function.hpp>
#include <flow/registry.hpp>
#include <flow/task.hpp>

#include <chrono>
#include <random>
#include <vector>

#include "lidar_driver.hpp"

namespace {
static constexpr std::size_t TOTAL_MESSAGES = mock::defaults::total_messages;
mock::lidar_driver g_driver{};
}// namespace

namespace mock {
template<std::size_t total_messages_t = mock::defaults::total_messages, std::size_t num_tasks = mock::defaults::num_publishers>
class lidar_drive_task final : public flow::task<lidar_drive_task<total_messages_t, num_tasks>> {
public:
  void begin(auto& channel_registry)
  {
    const auto on_request = [this](lidar_message& message) {
      m_tick();
      message = g_driver.drive();
    };

    m_cb_handle = flow::publish<lidar_message>("lidar_points", channel_registry, on_request);

    constexpr auto tick_cycle = total_messages_t;
    m_tick = flow::tick_function<void()>(tick_cycle, [this] {
      flow::logging::info("Test complete: {} messages have been processed.", total_messages_t);
      m_cb_handle.stop_everything();
    });
  }

  flow::callback_handle m_cb_handle{};
  flow::tick_function<void()> m_tick{};
};
}// namespace mock

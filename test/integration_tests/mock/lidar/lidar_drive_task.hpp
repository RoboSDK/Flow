#pragma once

#include <flow/task.hpp>
#include <random>

#include "lidar_driver.hpp"
#include <chrono>
#include <flow/data_structures/static_vector.hpp>
#include <flow/registry.hpp>
#include <vector>

namespace {
static constexpr std::size_t TOTAL_MESSAGES = mock::defaults::total_messages;
mock::lidar_driver g_driver{};
}

namespace mock {
template <std::size_t total_messages_t = mock::defaults::total_messages, std::size_t num_tasks = mock::defaults::num_publishers>
class lidar_drive_task final : public flow::task<lidar_drive_task<total_messages_t, num_tasks>> {
public:
  void begin(auto& channel_registry){
    const auto on_request = [this](lidar_message& message) {
      static bool done = false;
       if (++m_num_messages >= TOTAL_MESSAGES and not done) {
         flow::logging::info("Test complete: {} messages have been processed.", TOTAL_MESSAGES);
         m_cb_handle.stop_everything();
         done = true;
       }
      message = g_driver.drive();
    };

    m_cb_handle = flow::publish<lidar_message>("lidar_message", channel_registry, on_request);
  }

  flow::callback_handle m_cb_handle;

  std::size_t m_num_messages = 0;
};
}// namespace mock

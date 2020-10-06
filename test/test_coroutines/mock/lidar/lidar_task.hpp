#pragma once

#include <flow/task.hpp>
#include <random>

#include "lidar_driver.hpp"
#include <chrono>
#include <flow/data_structures/static_vector.hpp>
#include <flow/registry.hpp>
#include <vector>

namespace {
static constexpr std::size_t TOTAL_MESSAGES = mock::lidar::total_messages;
app::lidar_driver g_driver{};
}

namespace app {
class lidar_task final : public flow::task<lidar_task> {
public:
  void begin(auto& channel_registry){
    const auto on_request = [this](lidar_data& message) {
      static bool done = false;
       if (++m_num_messages >= TOTAL_MESSAGES and not done) {
         flow::logging::info("Test complete: {} messages have been processed.", TOTAL_MESSAGES);
         m_cb_handle.stop_everything();
         done = true;
       }
      message = g_driver.drive();
    };

    m_cb_handle = flow::publish<lidar_data>("lidar_data", channel_registry, on_request);
  }

  flow::callback_handle m_cb_handle;

  std::size_t m_num_messages = 0;
};
}// namespace app

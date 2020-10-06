#pragma once

#include <flow/task.hpp>
#include <random>

#include "LidarDriver.hpp"
#include <chrono>
#include <flow/data_structures/static_vector.hpp>
#include <flow/registry.hpp>
#include <vector>

namespace {
static constexpr std::size_t TOTAL_MESSAGES = 1'000;
app::LidarDriver g_driver{};
}

namespace app {
class LidarTask final : public flow::task<LidarTask> {
public:
  void begin(auto& channel_registry){
    const auto on_request = [this](LidarData& message) {
      static bool done = false;
       if (++m_num_messages >= TOTAL_MESSAGES and not done) {
         flow::logging::info("Test complete: {} messages have been processed.", TOTAL_MESSAGES);
         m_cb_handle.stop_everything();
         done = true;
       }
      message = g_driver.drive();
    };

    m_cb_handle = flow::publish<LidarData>("lidar_data", channel_registry, on_request);
  }

  flow::callback_handle m_cb_handle;

  std::size_t m_num_messages = 0;
};
}// namespace app

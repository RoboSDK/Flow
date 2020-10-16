#pragma once


#include <flow/channel_registry.hpp>
#include <flow/data_structures/static_vector.hpp>
#include <flow/data_structures/tick_function.hpp>
#include <flow/message.hpp>
#include <flow/task.hpp>

#include <chrono>
#include <random>
#include <vector>

namespace mock {
template<typename config_t>
class drive_task final : public flow::task<drive_task<config_t>> {
public:
  void begin(auto& channel_registry)
  {
    using message_t = typename config_t::message_t;
    using driver_t = typename config_t::driver_t;

    // every publisher will tick concurrently
    static driver_t driver{};
    const auto on_request = [this](message_t& wrapped_msg) {
      wrapped_msg.message = driver.drive();
      m_tick();
    };

    std::generate_n(std::back_inserter(m_callback_handles), config_t::num_publishers, [&] {
      return flow::publish<message_t>(config_t::channel_name, channel_registry, on_request);
    });

    constexpr auto tick_cycle = config_t::num_publishers * config_t::num_messages;
    m_tick = flow::tick_function(tick_cycle, [this] {
      std::ranges::for_each(m_callback_handles, [](auto& handle) {
        flow::logging::info("Disabling callback. {}", flow::to_string(handle));
        handle.disable();
      });
    });
  }

  void end() {}

private:
  using default_config_t = typename config_t::default_config_t;
  std::vector<flow::callback_handle<default_config_t>> m_callback_handles{};
  flow::tick_function m_tick{};
};
}// namespace mock

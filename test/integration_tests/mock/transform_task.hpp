#pragma once

#include <bitset>
#include <flow/registry.hpp>
#include <flow/task.hpp>

namespace mock {
template<typename config_t>
class transform_task final : public flow::task<transform_task<config_t>> {
public:
  transform_task() = default;
  transform_task(transform_task const& other) noexcept = default;
  transform_task(transform_task&& other) noexcept = default;
  transform_task& operator=(transform_task const& other) = default;
  transform_task& operator=(transform_task&& other) noexcept = default;

  void begin(auto& channel_registry)
  {
    const auto on_message = [this](typename config_t::message_t const& message) {
      [[maybe_unused]] const auto transformed = std::reduce(std::begin(message.points), std::end(message.points), 0);

      auto seq = message.metadata.sequence;
      if (m_seq_tracker.test(seq)) {
        flow::logging::critical_throw("Received message was previously received. seq: ", seq);
      }
      m_tick();
      m_seq_tracker.set(message.metadata.sequence);
    };

    std::generate_n(std::back_inserter(m_callback_handles), config_t::num_subscriptions, [&] {
      return flow::subscribe<typename config_t::message_t>(config_t::channel_name, channel_registry, on_message);
    });

    constexpr auto tick_cycle = config_t::total_messages;// publisher sends one extra message at the end
    m_tick = flow::tick_function(tick_cycle, [this] {
      flow::logging::info("sub tick func called");
      std::ranges::for_each(m_callback_handles, [](auto& handle) {
        flow::logging::info("Disabling callback: {}", flow::to_string(handle));
        handle.disable();
      });
    });
  }

  void end()
  {
    if (not m_seq_tracker.all()) {
      flow::logging::critical_throw("Expected all sequence numbers to be sent by channel. At least one was not sent. Result: {}/{}", m_seq_tracker.count(), m_seq_tracker.size());
    }
  }

private:
  std::bitset<config_t::num_sequences> m_seq_tracker{};
  std::vector<flow::callback_handle<typename config_t::default_config_t>> m_callback_handles{};
  flow::tick_function m_tick{};
};
}// namespace mock

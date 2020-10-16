#pragma once

#include <flow/channel_registry.hpp>
#include <flow/message.hpp>
#include <flow/task.hpp>
#include <mutex>
#include <numeric>

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
    using message_t = typename config_t::message_t;

    const auto on_message = [&]([[maybe_unused]] message_t const& wrapped_msg) {
      flow::atomic_increment(m_message_count);
    };

    std::generate_n(std::back_inserter(m_callback_handles), config_t::num_subscriptions, [&] {
      return flow::subscribe<message_t>(config_t::channel_name, channel_registry, on_message);
    });
  }

  void end()
  {
    if (m_message_count == 0) {
      flow::logging::critical_throw("Expected the number of messages received to be received to be at least {}. Received {}", m_message_count);
    }
  }

private:
  std::size_t m_message_count{};
  std::vector<flow::callback_handle<typename config_t::default_config_t>> m_callback_handles{};
};
}// namespace mock

#pragma once

#include <flow/channel_registry.hpp>
#include <flow/message.hpp>
#include <flow/task.hpp>
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

    const auto on_message = [this](message_t const& wrapped_msg) {
      [[maybe_unused]] const auto transformed = std::reduce(std::begin(wrapped_msg.message.points), std::end(wrapped_msg.message.points), 0);

      //      const auto sequence_count = flow::atomic_read(m_seq_tracker[wrapped_msg.metadata.sequence]);
      //      if (sequence_count > config_t::num_subscriptions) {
      //        flow::logging::critical("Received the same sequence more times than the number of subscriptions. seq: {} count: {}", wrapped_msg.metadata.sequence, sequence_count);
      //      }

      //      flow::atomic_increment(m_seq_tracker[wrapped_msg.metadata.sequence]);
      m_tick();
    };

    std::generate_n(std::back_inserter(m_callback_handles), config_t::num_subscriptions, [&] {
      return flow::subscribe<message_t>(config_t::channel_name, channel_registry, on_message);
    });

    constexpr auto tick_cycle = config_t::receive_messages;
    m_tick = flow::tick_function(tick_cycle, [this] {
      std::ranges::for_each(m_callback_handles, [](auto& handle) {
        handle.disable();
      });
      flow::logging::info("pub tick count: {}", m_tick.count());
      flow::logging::info("being called");
    });
  }

  void end()
  {
    //    for (std::size_t i = 0; i < config_t::num_sequences; ++i) {
    //      assert(m_seq_tracker[i] >= config_t::num_subscriptions);
    //    }
    //
    //    auto total_messages = std::reduce(std::begin(m_seq_tracker), std::end(m_seq_tracker), config_t::num_subscriptions);
    //    constexpr std::size_t expected_messages = config_t::receive_messages + config_t::num_subscriptions;// each sub will receive 1 extra message to cancel
    //
    //    if (total_messages < expected_messages) {
    //      flow::logging::critical_throw("Expected the number of message_registry to be received to be at least {}. Received {}", expected_messages, total_messages);
    //    }
  }

private:
  //  static constexpr auto seq_buffer = 10;// extra message_registry sent by pub before stopping
  //  std::array<std::size_t, config_t::num_sequences + seq_buffer> m_seq_tracker{};
  std::vector<flow::callback_handle<typename config_t::default_config_t>> m_callback_handles{};
  flow::tick_function m_tick{};
};
}// namespace mock

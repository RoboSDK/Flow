#pragma once

#include <bitset>
#include <flow/registry.hpp>
#include <flow/task.hpp>

namespace mock {
template<typename config_t>
class transform_task final : public flow::task<transform_task<config_t>> {
public:
  void begin(flow::registry& channel_registry)
  {
    const auto on_message = [this](typename config_t::message_t const& message) {
      [[maybe_unused]] const auto transformed = std::reduce(std::begin(message.points), std::end(message.points), 0);

      auto seq = message.metadata.sequence;
      if (m_seq_tracker.test(seq)) {
        std::stringstream ss;
        ss << "Received message was previously received. seq: " << seq << "\n";
        throw std::runtime_error(ss.str());
      }
      m_seq_tracker.set(message.metadata.sequence);
    };

    std::generate_n(std::back_inserter(m_callback_handles), config_t::num_subscriptions, [&] {
      return flow::subscribe<typename config_t::message_t>(config_t::channel_name, channel_registry, on_message);
    });
  }

  void end()
  {
    if (not m_seq_tracker.all()) {
      throw std::runtime_error("Expected all sequence numbers to be sent by channel. At least one was not sent.");
    }
  }

private:
  std::bitset<config_t::num_sequences> m_seq_tracker{};
  std::vector<flow::callback_handle> m_callback_handles{};
};
}// namespace mock

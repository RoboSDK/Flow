#pragma once

#include <bitset>
#include <flow/registry.hpp>
#include <flow/task.hpp>

namespace mock {
template<typename config_t>
class transform_task final : public flow::task<transform_task<config_t>> {
public:
  transform_task() = default;
  transform_task(transform_task const& other) noexcept : m_num_messages_received(other.m_num_messages_received.load(std::memory_order_relaxed)),
                                                         m_seq_tracker(other.m_seq_tracker),
                                                         m_callback_handles(other.m_callback_handles)
  {
  }

  transform_task(transform_task&& other) noexcept : m_num_messages_received(other.m_num_messages_received.load(std::memory_order_relaxed)),
                                                    m_seq_tracker(std::move(other.m_seq_tracker)),
                                                    m_callback_handles(std::move(other.m_callback_handles))
  {
    other.m_count = 0;
    other.m_seq_tracker.reset();
    other.m_callback_handles.clear();
  }

  transform_task& operator=(transform_task const& other)
  {
    m_num_messages_received = other.m_num_messages_received.load(std::memory_order_relaxed);
    m_seq_tracker = other.m_seq_tracker;
    m_callback_handles = other.m_callback_handles;
    return *this;
  }

  transform_task& operator=(transform_task&& other) noexcept
  {
    m_num_messages_received = other.m_num_messages_received.load(std::memory_order_relaxed);
    m_seq_tracker = std::move(other.m_seq_tracker);
    m_callback_handles = std::move(other.m_callback_handles);

    other.m_count = 0;
    other.m_seq_tracker.reset();
    other.m_callback_handles.clear();
    return *this;
  }

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
      ++m_num_messages_received;
      m_seq_tracker.set(message.metadata.sequence);
    };

    std::generate_n(std::back_inserter(m_callback_handles), config_t::num_subscriptions, [&] {
      return flow::subscribe<typename config_t::message_t>(config_t::channel_name, channel_registry, on_message);
    });
  }

  void end()
  {
    // publisher will send one extra message out at the end to trigger subscription to quit
    if (config_t::receive_messages - m_num_messages_received > 1) {
      std::stringstream ss;
      ss << "Expected " << config_t::receive_messages << " or " << config_t::receive_messages + 1
         << " messages to be processed. Got: " << m_num_messages_received << std::endl;
      throw std::runtime_error(ss.str());
    }

    if (not m_seq_tracker.all()) {
      std::stringstream ss;
      ss << "Expected all sequence numbers to be sent by channel. At least one was not sent. Result: " << m_seq_tracker.count() << "/" << m_seq_tracker.size();
      throw std::runtime_error(ss.str());
    }
  }

private:
  std::atomic_size_t m_num_messages_received{};
  std::bitset<config_t::num_sequences> m_seq_tracker{};
  std::vector<flow::callback_handle> m_callback_handles{};
};
}// namespace mock

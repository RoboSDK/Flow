#pragma once

#include <functional>

#include "flow/data_structures/static_vector.hpp"
#include "flow/data_structures/string.hpp"

#include <cppcoro/io_service.hpp>
#include <cppcoro/multi_producer_sequencer.hpp>
#include <cppcoro/sequence_barrier.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all.hpp>

namespace flow {

template<typename message_t, std::size_t message_buffer_size, std::size_t callback_buffer_size>
class channel {
public:
  channel(std::string name) : m_name(std::move(name)) {}
  channel() = default;

  void push_callback(std::function<void(message_t&)> callback) { m_on_request_callbacks.push_back(std::move(callback)); }
  void push_callback(std::function<void(const message_t&)> callback) { m_on_message_callbacks.push_back(std::move(callback)); }


  cppcoro::task<void> open_communications(
    cppcoro::static_thread_pool& tp,
    cppcoro::io_service& io,
    cppcoro::sequence_barrier<std::size_t>& barrier,
    cppcoro::multi_producer_sequencer<std::size_t>& sequencer)
  {
    using tasks_t = std::vector<cppcoro::task<void>>;
    tasks_t to_be_completed{};

    std::transform(std::begin(m_on_request_callbacks), std::end(m_on_request_callbacks), std::back_inserter(to_be_completed), [&](const auto& handle_request) -> cppcoro::task<void> {
      while (true) {
        size_t seq = co_await sequencer.claim_one(io);
        auto& msg = m_buffer[seq & index_mask];
        handle_request(msg);
        sequencer.publish(seq);
      }
    });

    std::transform(std::begin(m_on_message_callbacks), std::end(m_on_message_callbacks), std::back_inserter(to_be_completed), [&](const auto& handle_message) -> cppcoro::task<void> {
      size_t nextToRead = 0;
      while (true) {
        // Wait until the next message is available
        // There may be more than one available.
        const size_t available = co_await sequencer.wait_until_published(nextToRead, available, tp);
        do {
          auto& msg = m_buffer[nextToRead & index_mask];
          handle_message(msg);
        } while (nextToRead++ != available);

        barrier.publish(available);
      }
    });

    co_await cppcoro::when_all(std::move(to_be_completed));
  }

  using publisher_callbacks_t = std::vector<std::function<void(message_t&)>>;
  publisher_callbacks_t m_on_request_callbacks{};

  using subscriber_callbacks_t = std::vector<std::function<void(const message_t&)>>;
  subscriber_callbacks_t m_on_message_callbacks{};

  using message_buffer_t = std::array<message_t, message_buffer_size>;
  message_buffer_t m_buffer{};

  std::string m_name;


  static constexpr std::size_t index_mask = message_buffer_size - 1;
};

}// namespace flow
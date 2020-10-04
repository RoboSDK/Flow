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
/**
 * A channel represents the central location where all communication happens between different tasks
 * sharing information through messages
 *
 * It gets built up as publishers and subscribes are created by tasks. Once the begin phase is over, the open communication coroutine
 * begins and triggers all tasks to begin communication asynchronously.
 *
 * @tparam message_t The information being transmitted through this channel
 */
template<typename message_t>
class channel {
public:
  // TODO: Add this ias a template param
  static constexpr auto message_buffer_size = 64;

  channel(std::string name) : m_name(std::move(name)) {}

  /**
   * Called by registry when handing over ownership of the callback registered by a task
   * @param callback The request or message call back from a task
   */
  void push_publisher(std::function<void(message_t&)> && callback) { m_on_request_callbacks.push_back(std::move(callback)); }
  void push_subscription(std::function<void(message_t const&)> && callback) { m_on_message_callbacks.push_back(std::move(callback)); }


  /**
   * Called as part of the main routine through flow::spin
   *
   * @param tp The threadpool created by flow::spin
   * @param io A scheduler for asynchronous io
   * @param barrier Handles signals for publishing/subscribing between tasks
   * @param sequencer Handles buffer organization of messages
   * @return A coroutine that will be executed by flow::spin
   */
  auto open_communications(
    cppcoro::static_thread_pool& tp,
    cppcoro::io_service& io,
    cppcoro::sequence_barrier<std::size_t>& barrier,
    cppcoro::multi_producer_sequencer<std::size_t>& sequencer)
  {
    using tasks_t = std::vector<cppcoro::task<void>>;
    tasks_t to_be_completed{};

    const auto from_handle_request = [&](auto& handle_request) -> cppcoro::task<void> {
      while (true) {
        size_t seq = co_await sequencer.claim_one(io);
        auto& msg = m_buffer[seq & index_mask];
        handle_request(msg);
        sequencer.publish(seq);
      } };

    for (auto& handle_request : m_on_request_callbacks) {
      to_be_completed.push_back(from_handle_request(handle_request));
    };

    for (auto& handle_message : m_on_message_callbacks) {
      to_be_completed.push_back([&](const auto& handler) -> cppcoro::task<void> {
        size_t nextToRead = 0;
        while (true) {
          // Wait until the next message is available
          // There may be more than one available.
          const size_t available = co_await sequencer.wait_until_published(nextToRead, available, tp);
          do {
            auto& msg = m_buffer[nextToRead & index_mask];
            handler(msg);
          } while (nextToRead++ != available);

          barrier.publish(available);
        }
      }(handle_message));
    }

    return cppcoro::when_all(std::move(to_be_completed));
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
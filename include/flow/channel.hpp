#pragma once

#include <functional>
#include <mutex>

#include <array>
#include <cppcoro/multi_producer_sequencer.hpp>
#include <cppcoro/sequence_barrier.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all.hpp>
#include <cppcoro/when_all_ready.hpp>

#include "flow/cancellation.hpp"
#include "flow/message_wrapper.hpp"

namespace flow {
/**
 * A channel represents the central location where all communication happens between different tasks
 * sharing information through messages
 *
 * It gets built up as publishers and subscribes are created by tasks. Once the begin phase is over, the open
 * communication coroutine begins and triggers all tasks to begin communication asynchronously.
 *
 * @tparam message_t The information being transmitted through this channel
 */
template<typename message_t>
class channel {
  using task_t = cppcoro::task<void>;
  using tasks_t = std::vector<task_t>;
  static constexpr std::size_t BUFFER_SIZE = 64;

public:
  using publisher_callback_t = flow::cancellable_function<void(message_t&)>;
  using subscriber_callback_t = flow::cancellable_function<void(message_t const&)>;
  using publisher_callbacks_t = std::vector<publisher_callback_t>;
  using subscriber_callbacks_t = std::vector<subscriber_callback_t>;

  channel(std::string name) : m_name(std::move(name)) {}

  /**
   * Called by registry when handing over ownership of the callback registered by a task
   * @param callback The request or message call back from a task
   */
  void push_publisher(publisher_callback_t&& callback)
  {
    m_on_request_callbacks.push_back(std::move(callback));
  }
  void push_subscription(subscriber_callback_t&& callback)
  {
    m_on_message_callbacks.push_back(std::move(callback));
  }

  std::string_view name() const { return m_name; }

  std::size_t hash() { return typeid(message_t).hash_code() ^ std::hash<std::string>{}(m_name); }

  /**
   * Called as part of the main routine through flow::spin
   *
   * @param tp The threadpool created by flow::spin
   * @param io A scheduler for asynchronous io
   * @param barrier Handles signals for publishing/subscribing between tasks
   * @param sequencer Handles buffer organization of messages
   * @return A coroutine that will be executed by flow::spin
   */
  task_t open_communications(auto& sched)
  {
    using namespace cppcoro;

    const auto message_buffer = [] {
      if constexpr (is_wrapped<message_t>()) {
        return std::array<message_t, BUFFER_SIZE>{};
      }
      else {
        return std::array<message_wrapper<message_t>, BUFFER_SIZE>{};
      }
    };

    using buffer_t = decltype(message_buffer());
    using scheduler_t = decltype(sched);

    struct context_t {
      scheduler_t scheduler;
      sequence_barrier<std::size_t> barrier{};// notifies publishers that it can publish more
      multi_producer_sequencer<std::size_t> sequencer{ barrier, BUFFER_SIZE };// controls sequence values for the message buffer
      std::size_t index_mask = BUFFER_SIZE - 1;// Used to mask the sequence number and get the index to access the buffer
      buffer_t buffer{};// the message buffer
    } context{ sched };

    tasks_t on_request_tasks = make_publisher_tasks(context);
    tasks_t on_message_tasks = make_subscriber_tasks(context);

    auto on_messages = cppcoro::when_all_ready(std::move(on_message_tasks));
    auto on_requests = cppcoro::when_all_ready(std::move(on_request_tasks));

    co_await cppcoro::when_all_ready(std::move(on_messages), std::move(on_requests));
  }

private:
  void invoke_handler(auto& handler, auto& message)
  {
    if constexpr (is_wrapped<message_t>()) {
      std::invoke(handler, message);
    }
    else {
      std::invoke(handler, message.message);
    }
  }

  tasks_t make_publisher_tasks(auto& context)
  {
    tasks_t on_request_tasks{};
    for (auto&& handle_request : m_on_request_callbacks) {
      on_request_tasks.push_back(make_publisher_task(std::move(handle_request), context));
    };

    return on_request_tasks;
  }

  task_t make_publisher_task(publisher_callback_t&& cancellable_handler, auto& context)
  {
    auto message_sequence = std::atomic_ref<std::size_t>{ m_sequence };

    const auto handle_message = [&]() -> task_t {
      size_t buffer_sequence = co_await context.sequencer.claim_one(context.scheduler);
      auto& message_wrapper = context.buffer[buffer_sequence & context.index_mask];
      message_wrapper.metadata.sequence = message_sequence.fetch_add(1);
      invoke_handler(cancellable_handler, message_wrapper);
      context.sequencer.publish(buffer_sequence);
    };

    while (not cancellable_handler.is_cancellation_requested()) {
      co_await handle_message();
    }

    co_await handle_message();
  }

  tasks_t make_subscriber_tasks(auto& context)
  {
    tasks_t on_message_tasks{};
    for (auto&& handle_message : m_on_message_callbacks) {
      on_message_tasks.push_back(make_subscriber_task(std::move(handle_message), context));
    };

    return on_message_tasks;
  }

  task_t make_subscriber_task(subscriber_callback_t&& handler, auto& context)
  {
    size_t next_to_read = 0;
    size_t last_known_published = 0;

    while (not handler.is_cancellation_requested()) {
      // Wait until the next message is available
      // There may be more than one available.
      const size_t available = co_await context.sequencer.wait_until_published(next_to_read, context.sequencer.last_published_after(last_known_published), context.scheduler);
      do {
        auto& message_wrapper = context.buffer[next_to_read & context.index_mask];
        invoke_handler(handler, message_wrapper);
      } while (next_to_read++ != available);

      context.barrier.publish(available);
      last_known_published = available;
    }
  }

  publisher_callbacks_t m_on_request_callbacks{};
  subscriber_callbacks_t m_on_message_callbacks{};

  std::string m_name;
  std::size_t m_sequence{};
};

}// namespace flow
#pragma once

#include <functional>

#include <array>
#include <cppcoro/sequence_barrier.hpp>
#include <cppcoro/single_producer_sequencer.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all.hpp>
#include <cppcoro/when_all_ready.hpp>

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
  using task_t = cppcoro::task<void>;
  using tasks_t = std::vector<task_t>;
  static constexpr std::size_t BUFFER_SIZE = 4096;

public:
  channel(std::string name) : m_name(std::move(name)) {}

  /**
   * Called by registry when handing over ownership of the callback registered by a task
   * @param callback The request or message call back from a task
   */
  void push_publisher(std::function<void(message_t&)>&& callback) { m_on_request_callbacks.push_back(std::move(callback)); }
  void push_subscription(std::function<void(message_t const&)>&& callback) { m_on_message_callbacks.push_back(std::move(callback)); }

  std::string_view name() const
  {
    return m_name;
  }

  std::size_t hash()
  {
    return typeid(message_t).hash_code() ^ std::hash<std::string>{}(m_name);
  }

  /**
   * Called as part of the main routine through flow::spin
   *
   * @param tp The threadpool created by flow::spin
   * @param io A scheduler for asynchronous io
   * @param barrier Handles signals for publishing/subscribing between tasks
   * @param sequencer Handles buffer organization of messages
   * @return A coroutine that will be executed by flow::spin
   */
  task_t open_communications(auto& sched, volatile std::atomic_bool& app_is_running)
  {
    using namespace cppcoro;

    struct context_t {
      decltype(sched) scheduler;
      volatile std::atomic_bool& application_is_running;// flag that keeps the coroutines spinning
      sequence_barrier<std::size_t> barrier{};// notifies publishers that it can publish more
      single_producer_sequencer<std::size_t> sequencer{ barrier, BUFFER_SIZE };// controls sequence values for the message buffer
      std::array<message_t, BUFFER_SIZE> buffer{};// the message buffer
      std::size_t index_mask = BUFFER_SIZE - 1;// Used to mask the sequence number and get the index to access the buffer
    } context{sched, app_is_running};

    tasks_t on_request_tasks = make_publisher_tasks(context);
    tasks_t on_message_tasks = make_subscriber_tasks(context);

    auto on_messages = cppcoro::when_all_ready(std::move(on_message_tasks));
    auto on_requests = cppcoro::when_all_ready(std::move(on_request_tasks));

    co_await cppcoro::when_all_ready(std::move(on_messages), std::move(on_requests));
  }

private:
  tasks_t make_publisher_tasks(auto& context)
  {
    tasks_t on_request_tasks{};
    for (auto&& handle_request : m_on_request_callbacks) {
      on_request_tasks.push_back(make_publisher_task(std::move(handle_request), context));
    };

    return on_request_tasks;
  }

  task_t make_publisher_task(
    std::function<void(message_t&)>&& handler,
    auto& context)
  {
    while (context.application_is_running.load(std::memory_order_relaxed)) {
      size_t seq = co_await context.sequencer.claim_one(context.scheduler);
      auto& msg = context.buffer[seq & context.index_mask];
      handler(msg);
      context.sequencer.publish(seq);
    }

    // send one last final message to allow the consumers to quit
    size_t seq = co_await context.sequencer.claim_one(context.scheduler);
    auto& msg = context.buffer[seq & context.index_mask];
    handler(msg);
    msg.metadata.sequence = m_sequence++;
    context.sequencer.publish(seq);
  }

  tasks_t make_subscriber_tasks(auto& context)
  {
    tasks_t on_message_tasks{};
    for (auto&& handle_message : m_on_message_callbacks) {
      on_message_tasks.push_back(make_subscriber_task(std::move(handle_message), context));
    };

    return on_message_tasks;
  }

  task_t make_subscriber_task(std::function<void(message_t const&)>&& handler, auto& context)
  {
    // Theoretically we should start reading at the 0th index, but for some reason
    // the first message sent is empty, so the first message will be skipped.
    size_t nextToRead = sizeof(message_t);

    while (context.application_is_running.load(std::memory_order_relaxed)) {
      // Wait until the next message is available
      // There may be more than one available.
      const size_t available = co_await context.sequencer.wait_until_published(nextToRead, context.scheduler);
      do {
        auto& msg = context.buffer[nextToRead & context.index_mask];
        handler(msg);
      } while (nextToRead++ != available);

      context.barrier.publish(available);
    }
  }

  using publisher_callbacks_t = std::vector<std::function<void(message_t&)>>;
  publisher_callbacks_t m_on_request_callbacks{};

  using subscriber_callbacks_t = std::vector<std::function<void(message_t const&)>>;
  subscriber_callbacks_t m_on_message_callbacks{};

  std::string m_name;
  std::size_t m_sequence{};
};

}// namespace flow
#ifndef FLOW_CHANNEL_HPP
#define FLOW_CHANNEL_HPP

#include <algorithm>
#include <array>
#include <functional>
#include <thread>

#include <cppcoro/multi_producer_sequencer.hpp>
#include <cppcoro/sequence_barrier.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all_ready.hpp>

#include <unordered_set>

#include "flow/atomic.hpp"
#include "flow/cancellation.hpp"
#include "flow/channel_status.hpp"
#include "flow/logging.hpp"
#include "flow/message.hpp"
#include "flow/spin_wait.hpp"

namespace flow {
/**
 * A channel represents the central location where all communication happens between different tasks
 * sharing information through message_registry
 *
 * It gets built up as publishers and subscribes are created by tasks. Once the begin phase is over, the open
 * communication coroutine begins and triggers all tasks to begin communication asynchronously.
 *
 * @tparam message_t The information being transmitted through this channel
 */
template<typename message_t, typename config_t>
class channel {
  using task_t = cppcoro::task<void>;
  using tasks_t = std::vector<task_t>;
  static constexpr std::size_t BUFFER_SIZE = config_t::channel::message_buffer_size;

public:
  using publisher_callback_t = flow::cancellable_function<void(message_t&)>;
  using subscriber_callback_t = flow::cancellable_function<void(message_t const&)>;
  using publisher_callbacks_t = std::vector<publisher_callback_t>;
  using subscriber_callbacks_t = std::vector<subscriber_callback_t>;

  channel(std::string name) : m_name(std::move(name)) {}

  /**
   * Called by channel_registry when handing over ownership of the callback registered by a task
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

  /**
   * All messages are wrapped with flow::message<message_t> that contains metadata.
   *
   * If the user wants to use a flow::message<> wrapper, this will allow them to receive the
   * wrapped message type through a callback and look at the metadata
   *
   * This is used at compile time to get the buffer type
   * @return A buffer that contains the message buffer type
   */
  constexpr auto select_message_type()
  {
    if constexpr (is_wrapped<message_t>()) {
      return std::array<message_t, BUFFER_SIZE>{};
    }
    else {
      return std::array<message<message_t>, BUFFER_SIZE>{};
    }
  }

  std::size_t hash() { return typeid(message_t).hash_code() ^ std::hash<std::string>{}(m_name); }

  /**
   * Called as part of the main routine through flow::spin
   *
   * @param tp The threadpool created by flow::spin
   * @param io A scheduler for asynchronous io
   * @param barrier Handles signals for publishing/subscribing between tasks
   * @param sequencer Handles buffer organization of message_registry
   * @return A coroutine that will be executed by flow::spin
   */
  task_t open_communications(cppcoro::static_thread_pool& thread_pool_scheduler)
  {
    using namespace cppcoro;
    using buffer_t = decltype(select_message_type());

    const std::size_t num_publishers = m_on_request_callbacks.size();
    const std::size_t num_subscribers = m_on_message_callbacks.size();


    struct context_t {
      static_thread_pool& scheduler;
      channel_status status;

      sequence_barrier<std::size_t> barrier{};
      multi_producer_sequencer<std::size_t> sequencer{ barrier, BUFFER_SIZE };

      std::size_t index_mask = BUFFER_SIZE - 1;
      buffer_t buffer{};
    } context{ thread_pool_scheduler, channel_status{ num_publishers, num_subscribers } };


    const auto merge_tasks = [](tasks_t&& lhs, tasks_t&& rhs) {
      tasks_t tasks(lhs.size() + rhs.size());
      std::ranges::move(lhs, std::begin(tasks));
      std::ranges::move(rhs, std::next(std::begin(tasks), static_cast<long int>(lhs.size())));
      return tasks;
    };

    // When coroutines begin, they begin in order
    // We purposefully put subscriber tasks first because they need to request messages
    tasks_t merged = merge_tasks(make_subscriber_tasks(context), make_publisher_tasks(context));

    co_await cppcoro::when_all_ready(std::move(merged));
  }

private:
  void invoke(cancellable_callback auto& callback, auto& wrapped_message)
  {
    if constexpr (is_wrapped<message_t>()) {
      std::invoke(callback, wrapped_message);
    }
    else {
      std::invoke(callback, wrapped_message.message);
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

  task_t make_publisher_task(publisher_callback_t&& callback, auto& context)
  {
    // TODO: Move this to config
    // This is how many messages a publish routine should request from the sequencer before asking or more
    static constexpr std::size_t STRIDE_LENGTH = 1;

    auto& channel_status = context.status;
    auto coroutine_id = channel_status.generate_coroutine_id(); // used for logging purposes


    std::size_t last_buffer_sequence = 0;
    const auto spin_publisher = [&]() -> task_t {
      flow::logging::trace("[pub:{}] spin: last_buffer_sequencer {} {}", coroutine_id, last_buffer_sequence, flow::to_string(channel_status));

      const auto buffer_sequences = co_await context.sequencer.claim_up_to(STRIDE_LENGTH, context.scheduler);

      for (const auto& buffer_sequence : buffer_sequences) {
        last_buffer_sequence = buffer_sequence;
        flow::logging::trace("[pub:{}] buffer_sequence {}", coroutine_id, buffer_sequence);

        auto& message_wrapper = context.buffer[buffer_sequence & context.index_mask];
        message_wrapper.metadata.sequence = ++std::atomic_ref(m_sequence);

        const auto callback_routine = [&]() -> task_t {
          invoke(callback, message_wrapper);
          co_return;
        };

        co_await callback_routine();
      }

      context.sequencer.publish(buffer_sequences);
    };


    while (not callback.is_cancellation_requested() and channel_status.num_subscribers() > 0) {
      cppcoro::sync_wait(spin_publisher());
    }
    --channel_status.num_publishers();

    // if any subscribers waiting, flush them out TODO:: clean up this boolean logic
    while (channel_status.num_publishers() == 0 and channel_status.num_subscribers() > 0 and (last_buffer_sequence <= (context.barrier.last_published() + config_t::channel::message_buffer_size))) {
      flow::logging::trace("[pub:{}] flush: last_buffer_sequence {} <= context.barrier.last_published(): {}", coroutine_id, last_buffer_sequence, context.barrier.last_published());
      co_await spin_publisher();
    }

    flow::logging::trace("[pub:{}] done: {}", coroutine_id, flow::to_string(channel_status));
    co_return;
  }

  tasks_t make_subscriber_tasks(auto& context)
  {
    tasks_t on_message_tasks{};
    for (auto&& handle_message : m_on_message_callbacks) {
      on_message_tasks.push_back(make_subscriber_task(std::move(handle_message), context));
    };

    return on_message_tasks;
  }

  task_t make_subscriber_task(subscriber_callback_t&& callback, auto& context)
  {
    auto& status = context.status;
    auto coroutine_id = status.generate_coroutine_id();

    std::atomic_size_t next_to_read = 1;
    std::atomic_size_t last_published = 0;


    const auto spin_subscriber = [&]() -> task_t {
      flow::logging::trace("[sub:{}] spin: {}", coroutine_id, flow::to_string(status));

      flow::logging::trace("[sub:{}] WAIT: next to read {} last published {} last published after: {}", coroutine_id, next_to_read, last_published, context.sequencer.last_published_after(last_published));
      const size_t available = co_await context.sequencer.wait_until_published(next_to_read, last_published, context.scheduler);
      flow::logging::trace("[sub:{}] GOT: available: {}", coroutine_id, available);

      while (flow::atomic_post_increment(next_to_read) < available) {

        const std::size_t current_sequence = next_to_read;
        auto& message_wrapper = context.buffer[current_sequence & context.index_mask];

        const auto callback_routine = [&]() -> task_t {
          invoke(callback, message_wrapper);
          co_return;
        };

        co_await callback_routine();
      }

      context.barrier.publish(available);
      last_published = available;
    };


    while (not callback.is_cancellation_requested() and status.num_publishers() > 0) {
      co_await spin_subscriber();
    }

    --status.num_subscribers();

    while (status.num_publishers() > 0 and status.num_subscribers() == 0 and next_to_read < context.sequencer.last_published_after(last_published)) {
      flow::logging::debug("[sub:{}] flush: available: {}", coroutine_id, last_published);
      co_await spin_subscriber();
    }

    flow::logging::trace("[sub:{}] done: {}", coroutine_id, flow::to_string(status));
    co_return;
  }

  publisher_callbacks_t m_on_request_callbacks{};
  subscriber_callbacks_t m_on_message_callbacks{};

  std::string m_name;
  std::size_t m_sequence{};
};

}// namespace flow

#endif
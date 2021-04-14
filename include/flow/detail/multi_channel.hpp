#pragma once

#include "channel_resource.hpp"
#include "publisher_token.hpp"
#include "subscriber_token.hpp"

#include <cppcoro/async_generator.hpp>
#include <cppcoro/multi_producer_sequencer.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>

/**
 * The link between routines in a network are m_channels.
 *
 * A multi multi_channel in this framework is a multi publisher_impl multi subscribe multi_channel that are linked
 * to two corresponding neighbors in a network.
 */

namespace flow::detail {

/**
 * A multi publisher_impl multi subscribe channel
 *
 * This means that because the assumption is that multiple publishers and subscribers will be used to
 * communicate through this single channel, there will be a performance cost of atomics for synchronization
 *
 * @tparam raw_message_t The raw message type is the message type with references potentially attached
 * @tparam configuration_t The global compile time configuration
 */
template<typename raw_message_t, is_configuration configuration_t>
class multi_channel {
public:
  using message_t = std::decay_t<raw_message_t>;/// Remove references
  using resource_t = channel_resource<configuration_t, cppcoro::multi_producer_sequencer<std::size_t>>;
  using scheduler_t = cppcoro::static_thread_pool;/// The static thread pool is used to schedule threads
  using configuration = configuration_t;

  constexpr metaprogramming::type_container<message_t> message_type()
  {
    return metaprogramming::type_container<message_t>{};
  }

  enum class termination_state {
    uninitialised,
    subscriber_initialized,
    publisher_received,
    subscriber_finalized
  };

  /**
   * @param name Name of the multi_channel
   * @param resource A generated multi_channel channel_resource
   * @param scheduler The global scheduler
   */
  multi_channel(std::string name, resource_t* resource, scheduler_t* scheduler)
    : m_resource{ resource },
      m_scheduler{ scheduler }
  {
    if (not name.empty()) {
      m_name = std::move(name);
    }
    else {
      m_name = typeid(message_t).name();
    }
  }

  multi_channel& operator=(multi_channel const& other)
  {
    m_resource = other.m_resource;
    m_scheduler = other.m_scheduler;
    std::copy(std::begin(other.buffer), std::end(other.buffer), std::begin(m_buffer));
    return *this;
  }

  multi_channel(multi_channel const& other)
  {
    m_resource = other.m_resource;
    m_scheduler = other.m_scheduler;
    std::copy(std::begin(other.m_buffer), std::end(other.m_buffer), std::begin(m_buffer));
  }

  multi_channel& operator=(multi_channel&& other) noexcept
  {
    m_resource = other.m_resource;
    m_scheduler = other.m_scheduler;
    std::move(std::begin(other.m_buffer), std::end(other.m_buffer), std::begin(m_buffer));
    return *this;
  }

  multi_channel(multi_channel&& other) noexcept : m_resource(other.m_resource)
  {
    *this = std::move(other);
  }

  /**
   * Used to store the multi_channel into a multi_channel set
   * @return The hash of the message and multi_channel name
   */
  std::size_t hash() { return typeid(message_t).hash_code() ^ std::hash<std::string>{}(m_name); }

  std::string name() const
  {
    if (m_name.empty()) {
      return typeid(message_t).name();
    }

    return m_name;
  }

  /*******************************************************
   ****************** publish INTERFACE *****************
   ******************************************************/

  /**
   * Request permission to publish the next message
   * @return
   */
  cppcoro::task<bool> request_permission_to_publish(publisher_token<message_t>& token)
  {
    if (m_state > termination_state::uninitialised) co_return false;

    static constexpr std::size_t STRIDE_LENGTH = configuration_t::stride_length;

    ++std::atomic_ref(m_num_publishers_waiting);
    cppcoro::sequence_range<std::size_t> sequences = co_await m_resource->sequencer.claim_up_to(STRIDE_LENGTH, *m_scheduler);
    --std::atomic_ref(m_num_publishers_waiting);

    token.sequences = std::move(sequences);
    co_return true;
  }

  cppcoro::task<void> request_permission_to_publish_one(publisher_token<message_t>& token)
  {
    ++std::atomic_ref(m_num_publishers_waiting);
    token.sequence = co_await m_resource->sequencer.claim_one(*m_scheduler);
    --std::atomic_ref(m_num_publishers_waiting);
  }

  /**
   * Publish the produced message
   * @param message The message type the multi_channel communicates
   */
  void publish_messages(publisher_token<message_t>& token)
  {
    for (auto& sequence_number : token.sequences) {
      m_buffer[sequence_number & m_index_mask] = std::move(token.messages.front());
      token.messages.pop();
    }

    m_resource->sequencer.publish(std::move(token.sequences));
  }

  void publish_one(publisher_token<message_t>& token)
  {
    m_buffer[token.sequence & m_index_mask] = std::move(token.messages.front());
    token.messages.pop();

    m_resource->sequencer.publish(token.sequence);
  }

  void confirm_termination()
  {
    std::atomic_ref(m_state).store(std::max(termination_state::publisher_received, m_state));
  }

  /*******************************************************
   ****************** subscribe INTERFACE *****************
   ******************************************************/

  /**
   * Retrieve an iterable message generator. Will generate all messages that
   * have already been published by a publisher_function
   * @return a message generator
   */
  cppcoro::async_generator<message_t> message_generator(subscriber_token<message_t>& token)
  {
    token.end_sequence = co_await m_resource->sequencer.wait_until_published(
      token.sequence, token.last_sequence_published, *m_scheduler);

    while (token.sequence <= token.end_sequence) {
      co_yield m_buffer[std::atomic_ref(token.sequence)++ & m_index_mask];
    }
  }

  /**
   * Notify the publisher_function to publish the next messages
   */
  bool notify_message_consumed(subscriber_token<message_t>& token)
  {
    m_resource->barrier.publish(token.sequence);
    token.last_sequence_published = token.sequence;
    return true;
  }

  void initialize_termination()
  {
    std::atomic_ref(m_state).store(std::max(termination_state::subscriber_initialized, m_state));
  }

  void finalize_termination()
  {
    std::atomic_ref(m_state).store(std::max(termination_state::subscriber_finalized, m_state));
  }

  /**
   * @return if any publisher_function m_channels are currently waiting for permission
   */
  bool is_waiting()
  {
    return std::atomic_ref(m_num_publishers_waiting).load() > 0;
  }

  std::size_t num_waiters()
  {
    return std::atomic_ref(m_num_publishers_waiting).load();
  }

  bool is_being_flushed() {
    return std::atomic_ref(m_flushing).load() > 0;
  }

  std::size_t num_flushers() {
    return std::atomic_ref(m_flushing).load();
  }

  void flush() {
    std::atomic_ref(m_flushing)++;
  }

  void end_flush() {
    std::atomic_ref(m_flushing)--;
  }

  /*******************************************************
   ****************** END subscribe INTERFACE *****************
   ******************************************************/

  termination_state state()
  {
    return std::atomic_ref(m_state).load();
  }


private:
  std::size_t m_flushing{};

  termination_state m_state{ termination_state::uninitialised };

  std::size_t m_num_publishers_waiting{};

  /// The message buffer size determines how many messages can communicated at once
  std::array<message_t, configuration_t::message_buffer_size> m_buffer{};
  std::size_t m_index_mask = configuration_t::message_buffer_size - 1;

  std::string m_name;

  /// Non owning
  resource_t* m_resource{ nullptr };
  scheduler_t* m_scheduler{ nullptr };
};
}// namespace flow::detail

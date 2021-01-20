#pragma once

#include <array>

#include <cppcoro/async_generator.hpp>
#include <cppcoro/sequence_barrier.hpp>
#include <cppcoro/single_producer_sequencer.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>

/**
 * The link between routines in a chain are channels.
 *
 * A channel in this framework is a single producer single consumer channel that are linked
 * to two corresponding neighbors in a chain.
 */

namespace flow {

/**
 * This is a resource that each channel will own
 * @tparam configuration_t The compile time global configuration for this project
 */
template<typename configuration_t>
struct resource {
  using sequence_barrier = cppcoro::sequence_barrier<std::size_t>;
  using single_producer_sequencer = cppcoro::single_producer_sequencer<std::size_t>;

  /*
   * The sequence barrier is used to communicate from the consumer end that it has
   * received and consumed the message to the producer end of the channel
   */
  sequence_barrier barrier{};

  /*
   * The producer sequencer generated sequence numbers that the producer end of the channel
   * uses to publish to the consumer end
   */
  single_producer_sequencer sequencer{ barrier, configuration_t::message_buffer_size };
};

/**
 * Has an array of contiguous channel resources. Channel resources will be
 * retrieved from this generator
 * @tparam configuration_t The global compile time configuration for the project
 */
template<typename configuration_t>
struct channel_resource_generator {
  using resource_t = resource<configuration_t>;

  std::array<resource_t, configuration_t::max_resources> channel_resources{};
  std::size_t current_resource{};
};

/**
 * Get the next channel resource available from the generator
 * @param generator a channel resource generator
 * @return A pointer to the resource
 */
auto* get_channel_resource(auto& generator)
{
  return &generator.channel_resources[std::atomic_ref(generator.current_resource)++];
}

/**
 * A single producer single consumer channel
 * @tparam raw_message_t The raw message type is the message type with references potentially attached
 * @tparam configuration_t The global compile time configuration
 */
template<typename raw_message_t, typename configuration_t>
class channel {
  using message_t = std::decay_t<raw_message_t>;/// Remove references
  using resource_t = resource<configuration_t>;
  using scheduler_t = cppcoro::static_thread_pool;/// The static thread pool is used to schedule threads

public:

  /**
   * @param name Name of the channel
   * @param resource A generated channel resource
   * @param scheduler The global scheduler
   */
  channel(std::string name, resource_t* resource, scheduler_t* scheduler)
    : m_name{ std::move(name) },
      m_resource{ resource },
      m_scheduler{ scheduler }
  {
  }

  channel& operator=(channel const& other)
  {
    m_resource = other.m_resource;
    m_scheduler = other.m_scheduler;
    std::copy(std::begin(other.buffer), std::end(other.buffer), std::begin(m_buffer));
    return *this;
  }

  channel(channel const& other)
  {
    m_resource = other.m_resource;
    m_scheduler = other.m_scheduler;
    std::copy(std::begin(other.m_buffer), std::end(other.m_buffer), std::begin(m_buffer));
  }

  channel& operator=(channel&& other) noexcept
  {
    m_resource = other.m_resource;
    m_scheduler = other.m_scheduler;
    std::move(std::begin(other.m_buffer), std::end(other.m_buffer), std::begin(m_buffer));
    std::ranges::fill(other.m_buffer, 0);
    return *this;
  }

  channel(channel&& other) noexcept : m_resource(other.m_resource)
  {
    *this = std::move(other);
  }

  /**
   * Used to store the channel into a channel set
   * @return The hash of the message and channel name
   */
  std::size_t hash() { return typeid(message_t).hash_code() ^ std::hash<std::string>{}(m_name); }

  /*******************************************************
   ****************** PRODUCER INTERFACE *****************
   ******************************************************/

  /**
   * Request permission to publish the next message
   * @return
   */
  cppcoro::task<void> request_permission_to_publish()
  {
    std::atomic_ref(m_is_waiting).store(true);
    m_producer_sequence = co_await m_resource->sequencer.claim_one(*m_scheduler);
  }

  /**
   * Publish the produced message
   * @param message The message type the channel communicates
   */
  void publish_message(message_t&& message)
  {
    m_buffer[m_producer_sequence & m_index_mask] = std::move(message);
    m_resource->sequencer.publish(m_producer_sequence);
  }

  /*******************************************************
   ****************** CONSUMER INTERFACE *****************
   ******************************************************/

  /**
   * Retrieve an iterable message generator. Will generate all messages that
   * have already been published by a producer
   * @return a message generator
   */
  cppcoro::async_generator<message_t> message_generator()
  {
    std::atomic_ref(m_is_waiting).store(false);
    m_end_consumer_sequence = co_await m_resource->sequencer.wait_until_published(
      m_consumer_sequence, *m_scheduler);

    do {
      co_yield m_buffer[m_consumer_sequence & m_index_mask];
    } while (m_consumer_sequence <= m_end_consumer_sequence);
  }


  /**
   * Notify the producer to produce the next messages
   */
  void notify_message_consumed()
  {
    ++m_consumer_sequence;

    if (m_consumer_sequence == m_end_consumer_sequence) {
      m_resource->barrier.publish(m_end_consumer_sequence);
    }
  }

  /**
   * Disable the channel
   *
   * When cancelling a chain the beginning of the cancellation happens
   * with the consumer end of the chain. This trickles down all the way to the
   * beginning end of the chain with the first producer.
   *
   * The consumer cancels itself, terminates the channel, and then flushes out any
   * producers waiting for permission to publish.
   */
  void terminate()
  {
    std::atomic_ref(m_is_terminated).store(true);
  }

  /**
   * Used by the producer ends of the channels to keep looping or not
   * @return if the channel has been cancelled by the consumer end of the channel
   */
  bool is_terminated()
  {
    return std::atomic_ref(m_is_terminated).load();
  }

  /**
   * @return if any producer channels are currently waiting for permission
   */
  bool is_waiting()
  {
    return std::atomic_ref(m_is_waiting).load();
  }

private:
  bool m_is_terminated{};
  bool m_is_waiting{};

  /// The message buffer size determines how many messages can communicated at once
  std::array<message_t, configuration_t::message_buffer_size> m_buffer{};
  std::size_t m_index_mask = configuration_t::message_buffer_size - 1;

  std::string m_name;

  std::size_t m_producer_sequence{};
  std::size_t m_consumer_sequence{};

  /// The last sequence number before waiting to consume more messages
  std::size_t m_end_consumer_sequence{};

  /// Non owning
  resource_t* m_resource{ nullptr };
  scheduler_t* m_scheduler{ nullptr };
};
}// namespace flow

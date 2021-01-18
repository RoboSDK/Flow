#pragma once

#include <array>

#include <cppcoro/async_generator.hpp>
#include <cppcoro/sequence_barrier.hpp>
#include <cppcoro/single_producer_sequencer.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>

namespace flow {
template<typename configuration_t>
struct resource {
  using sequence_barrier = cppcoro::sequence_barrier<std::size_t>;
  using single_producer_sequencer = cppcoro::single_producer_sequencer<std::size_t>;

  sequence_barrier barrier{};
  single_producer_sequencer sequencer{ barrier, configuration_t::message_buffer_size };
};

template<typename configuration_t>
struct channel_resource_generator {
  using resource_t = resource<configuration_t>;

  std::array<resource_t, configuration_t::max_resources> channel_resources{};
  std::size_t current_resource{};
};

auto* get_channel_resource(auto& generator)
{
  return &generator.channel_resources[std::atomic_ref(generator.current_resource)++];
}

template<typename raw_message_t, typename configuration_t>
class channel {
  using message_t = std::decay_t<raw_message_t>;
  using resource_t = resource<configuration_t>;
  using scheduler_t = cppcoro::static_thread_pool;
  using task_t = cppcoro::task<void>;

public:
  channel(std::string name, resource_t* resource, scheduler_t* scheduler)
    : m_name{ std::move(name) }, m_resource{ resource }, m_scheduler{ scheduler } {}

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

  std::size_t hash() { return typeid(message_t).hash_code() ^ std::hash<std::string>{}(m_name); }

  task_t request()
  {
    m_producer_sequence = co_await m_resource->sequencer.claim_one(*m_scheduler);
  }

  cppcoro::async_generator<message_t> message_generator()
  {
    m_available = co_await m_resource->sequencer.wait_until_published(
      m_consumer_sequence, *m_scheduler);

    do {
      co_yield m_buffer[m_consumer_sequence & m_index_mask];
    } while (m_consumer_sequence <= m_available);
  }


  void publish_message(message_t&& message)
  {
    m_buffer[m_producer_sequence & m_index_mask] = std::move(message);
    m_resource->sequencer.publish(m_producer_sequence);
  }

  void make_request()
  {
    ++m_consumer_sequence;

    if (m_consumer_sequence == m_available) {
      m_resource->barrier.publish(m_available);
    }
  }

  bool more_to_publish()
  {
    return m_resource->barrier.last_published() > m_resource->sequencer.last_published();
  }

private:
  std::array<message_t, configuration_t::message_buffer_size> m_buffer{};
  std::size_t m_index_mask = configuration_t::message_buffer_size - 1;

  std::string m_name;

  //producer
  std::size_t m_producer_sequence{};

  // consumer
  std::size_t m_consumer_sequence{};
  std::size_t m_available{};

  resource_t* m_resource{nullptr};
  scheduler_t* m_scheduler{nullptr};
};
}// namespace flow

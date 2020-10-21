#pragma once

#include <array>
#include <execution>

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

auto make_channel_resources(auto& generator)
{
  return std::ref(generator.channel_resources[std::atomic_ref(generator.current_resource)++]);
}

template<typename message_t, typename configuration_t>
class channel {
  using resource_t = resource<configuration_t>;

  template<typename T>
  using ref = std::reference_wrapper<T>;

public:
  channel(std::string name, std::reference_wrapper<resource_t> resource)
    : m_name{ std::move(name) }, m_resource{ resource } {}

  channel& operator=(channel const& other)
  {
    m_resource = other.m_resource;
    std::copy(std::execution::par, std::begin(other.buffer), std::end(other.buffer), std::begin(m_buffer));
    return *this;
  }

  channel(channel const& other)
  {
    *this = other;
  }

  channel& operator=(channel&& other) noexcept
  {
    m_resource = other.m_resource;
    std::move(std::execution::par, std::begin(other.buffer), std::end(other.buffer), std::begin(m_buffer));
    std::ranges::fill(other.buffer, 0);
    return *this;
  }

  channel(channel&& other) noexcept : m_resource(other.m_resource)
  {
    *this = std::move(other);
  }

  std::size_t hash() { return typeid(message_t).hash_code() ^ std::hash<std::string>{}(m_name); }

  cppcoro::async_generator<message_t&> request_generator()
  {
    auto& resource = m_resource.get();
    auto& scheduler = m_scheduler.get();

    const auto buffer_sequence = co_await resource.sequencer.claim_one(scheduler);
    co_yield m_buffer[buffer_sequence & INDEX_MASK];
  }

  cppcoro::async_generator<message_t&> message_generator()
  {
    auto& resource = m_resource.get();
    auto& scheduler = m_scheduler.get();

    m_available = co_await resource.sequencer.wait_until_published(
      m_consumer_sequence, scheduler);

    do {
      co_yield m_buffer[m_consumer_sequence & INDEX_MASK];
    } while (m_consumer_sequence < m_available);
  }


  void fulfill_request()
  {
    m_resource.get().sequencer.publish(m_producer_sequence);
  }

  void make_request()
  {
    ++m_consumer_sequence;

    if (m_consumer_sequence == m_available) {
      m_resource.get().barrier.publish(m_available);
    }
  }

private:
  static constexpr std::size_t INDEX_MASK = configuration_t::message_buffer_size - 1;
  std::array<message_t, configuration_t::message_buffer_size> m_buffer{};

  std::string m_name;

  //producer
  std::size_t m_producer_sequence;

  // consumer
  std::size_t m_consumer_sequence{};
  std::size_t m_available{};

  ref<resource_t> m_resource;
  ref<cppcoro::static_thread_pool> m_scheduler;
};
}// namespace flow

#pragma once

#include <cppcoro/sequence_barrier.hpp>

namespace flow::detail {
/**
 * This is a channel_resource that each multi_channel will own
 * @tparam configuration_t The compile time global configuration for this project
 */
template<typename configuration_t, typename sequencer_t>
struct channel_resource {
  using sequence_barrier = cppcoro::sequence_barrier<std::size_t>;

  /*
   * The sequence barrier is used to communicate from the consumer_function end that it has
   * received and consumed the message to the producer_function end of the multi_channel
   */
  sequence_barrier barrier{};

  /*
   * The producer_function sequencer generated sequence numbers that the producer_function end of the multi_channel
   * uses to publish to the consumer_function end
   */
  sequencer_t sequencer{ barrier, configuration_t::message_buffer_size };
};

/**
 * Has an array of contiguous multi_channel resources. Channel resources will be
 * retrieved from this generator
 * @tparam configuration_t The global compile time configuration for the project
 */
template<typename configuration_t, typename sequencer_t>
class channel_resource_generator {
  using resource_t = channel_resource<configuration_t, sequencer_t>;

public:
  /**
   * Return a non owning raw pointer that will be used by a multi_channel as a
   * communication buffer between at least two routines
   * @return A resource that will be used to construct a multi_channel
   */
  resource_t* operator()()
  {
    return &channel_resources[std::atomic_ref(current_resource)++];
  }

private:
  std::array<resource_t, configuration_t::max_resources> channel_resources{};
  std::size_t current_resource{};
};

}// namespace flow::detail

#pragma once

#include <cppcoro/on_scope_exit.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all_ready.hpp>

#include "channel_set.hpp"
#include "flow/cancellable_function.hpp"
#include "flow/channel.hpp"
#include "flow/context.hpp"
#include "flow/network_handle.hpp"
#include "flow/routine_concepts.hpp"
#include "flow/spin.hpp"
#include "flow/timeout_routine.hpp"

#include "flow/consumer.hpp"
#include "flow/producer.hpp"
#include "flow/spinner.hpp"
#include "flow/transformer.hpp"

/**
 * A network is a sequence of routines connected by single producer_routine single consumer_routine channels.
 * The end of the network depends on the data flow from the beginning of the network. The beginning
 * of the network has no dependencies.
 *
 * An empty network is a network which has no routines and can take a spinner_routine or a producer_routine.
 *
 * The minimal network s a network with a spinner_routine, because it depends on nothing and nothing depends on it.
 *
 * When a network is begun with a producer_routine, transformers may be inserted into the network until it is capped
 * with a consumer_routine.
 *
 * Each network is considered independent from another network and may not communicate with each other.
 *
 * producer_routine -> transfomer -> ... -> consumer_routine
 *
 * Each channel in the network uses contiguous memory to pass data to the channel waiting on the other way. All
 * data must flow from producer_routine to consumer_routine; no cyclical dependencies.
 *
 * Cancellation
 * Cancelling the network of coroutines is a bit tricky because if you stop them all at once, some of them will hang
 * with no way to have them leave the awaiting state.
 *
 * When starting the network reaction all routines will begin to wait and the first routine that is given priority is the
 * producer_routine at the beginning of the network, and the last will be the end of the network, or consumer_routine.
 *
 * The consumer_routine then has to be the one that initializes the cancellation. The algorithm is as follows:
 *
 * Consumer receives cancellation request from the cancellation handle
 * consumer_routine terminates the channel it is communicating with
 * consumer_routine flushes out any awaiting producers/transformers on the producing end of the channel
 * end routine
 *
 * The transformer_routine or producer_routine that is next in the network will then receiving channel termination notification
 * from the consumer_routine at the end of the network and break out of its loop
 * It well then notify terminate the producer_routine channel it receives data from and flush it out
 *
 * rinse repeat until the beginning of the network, which is a producer_routine
 * The producer_routine simply breaks out of its loop and exits the scope
 */

namespace flow {
template<typename configuration_t>
class network {
public:
  enum class state {
    empty,/// Initial state
    open,/// Has producers and transformers with no consumer_routine
    closed/// The network is complete and is capped by a consumer_routine
  };

  /*
   * @param context The context contains all raw resources used to create the network
   */
  explicit network(auto* context) : m_context(context) {}

  /**
   * Makes a channel if it doesn't exist and returns a reference to it
   * @tparam message_t The message type the channel will communicate
   * @param channel_name The name of the channel (optional)
   * @return A reference to the channel
   */
  template<typename message_t>
  auto& make_channel_if_not_exists(std::string channel_name)
  {
    if (m_context->channels.template contains<message_t>(channel_name)) {
      return m_context->channels.template at<message_t>(channel_name);
    }

    using channel_t = channel<message_t, configuration_t>;

    channel_t channel{
      channel_name,
      get_channel_resource(m_context->resource_generator),
      &m_context->thread_pool
    };

    m_context->channels.put(std::move(channel));
    return m_context->channels.template at<message_t>(channel_name);
  }

  /**
   * Pushes a routine into the network
   * @param spinner A routine with no dependencies and nothing depends on it
   */
  template<typename routine_t>
  requires std::is_same_v<flow::spinner, routine_t>
  void push(routine_t&& routine)
  {
    m_handle.push(routine.callback().handle());
    m_context->tasks.push_back(spin_spinner(m_context->thread_pool, routine.callback()));
  }

  /**
   * Pushes a producer_routine into the network
   * @param producer The producer_routine routine
   * @param channel_name The channel name the producer_routine will publish to
   */

  template<typename message_t>
  void push(flow::producer<message_t>&& routine)
  {
    auto& channel = make_channel_if_not_exists<message_t>(routine.channel_name());
    m_context->tasks.push_back(spin_producer<message_t>(channel, routine.callback()));
  }

  /**
   *  Pushes a transformer_routine into the network and creates any necessary channels it requires
   * @param transformer A routine that depends on another routine and is depended on by a consumer_routine or transformer_routine
   * @param producer_channel_name The channel it depends on
   * @param consumer_channel_name The channel that it will publish to
   */
  template<typename return_t, typename... args_t>
  void push(flow::transformer<return_t(args_t...)>&& routine)
  {
    auto& producer_channel = make_channel_if_not_exists<args_t...>(routine.producer_channel_name());
    auto& consumer_channel = make_channel_if_not_exists<args_t...>(routine.consumer_channel_name());

    m_context->tasks.push_back(spin_transformer<return_t, args_t...>(producer_channel, consumer_channel, routine.callback()));
  }

  /**
   * Pushes a consumer_routine into the network
   * @param callback A routine no other routine depends on and depends on at least a single routine
   * @param channel_name The channel it will consume from
   */
  template<typename message_t>
  void push(flow::consumer<message_t>&& routine)
  {
    auto& channel = make_channel_if_not_exists<message_t>(routine.channel_name());

    m_handle.push(routine.callback().handle());
    m_context->tasks.push_back(spin_consumer<message_t>(channel, routine.callback()));
  }

  /**
   * Joins all the routines into a single coroutine
   * @return a coroutine
   */
  cppcoro::task<void> spin()
  {
    co_await cppcoro::when_all_ready(std::move(m_context->tasks));
  }

  /**
   * Makes a handle to this network that will allow whoever holds the handle to cancel
   * the network
   *
   * The cancellation handle will trigger the consumer_routine to cancel and trickel down all the way to the producer_routine
   * @return A cancellation handle
   */
  network_handle handle()
  {
    return m_handle;
  }

  /**
   * Cancel the network after the specified time
   *
   * This does not mean the network will be stopped after this amount of time! It takes a non-deterministic
   * amount of time to fully shut the network down.
   * @param time in milliseconds
   */
  void cancel_after(std::chrono::milliseconds time)
  {
    auto timeout_function_ptr = make_timeout_routine(time, [&] {
      handle().request_cancellation();
    });

    auto& timeout_function = *timeout_function_ptr;

    m_context->tasks.push_back(timeout_function());
    m_heap_storage.push_back(std::move(timeout_function_ptr));
  }

private:
  context<configuration_t>* m_context;
  std::vector<std::any> m_heap_storage;

  network_handle m_handle{};
};
}// namespace flow
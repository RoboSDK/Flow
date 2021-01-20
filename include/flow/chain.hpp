#pragma once

#include <cppcoro/on_scope_exit.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all_ready.hpp>

#include "flow/cancellable_function.hpp"
#include "flow/channel.hpp"
#include "flow/context.hpp"
#include "flow/data_structures/channel_set.hpp"
#include "flow/routine.hpp"
#include "flow/spin.hpp"
#include "flow/timeout_routine.hpp"

/**
 * A chain is a sequence of routines connected by single producer single consumer channels.
 * The end of the chain depends on the data flow from the beginning of the chain. The beginning
 * of the chain has no dependencies.
 *
 * An empty chain is a chain which has no routines and can take a spinner or a producer.
 *
 * The minimal chain s a chain with a spinner, because it depends on nothing and nothing depends on it.
 *
 * When a chain is begun with a producer, transformers may be inserted into the chain until it is capped
 * with a consumer.
 *
 * Each chain is considered independent from another chain and may not communicate with each other.
 *
 * producer -> transfomer -> ... -> consumer
 *
 * Each channel in the chain uses contiguous memory to pass data to the channel waiting on the other way. All
 * data must flow from producer to consumer; no cyclical dependencies.
 */

namespace flow {
template<typename configuration_t>
class chain {
public:

  enum class state {
    empty, /// Initial state
    open, /// Has producers and transformers with no consumer
    closed /// The chain is complete and is capped by a consumer
  };

  /// A task coroutine
  using task_t = cppcoro::task<void>;

  /*
   * @param context The context contains all raw resources used to create the chain
   */
  explicit chain(auto* context) : m_context(context) {}

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
   * Pushes a routine into the chain
   * @param spinner A routine with no dependencies and nothing depends on it
   */
  void push(flow::spinner auto&& spinner)
  {
    if (m_state not_eq state::empty) {
      flow::logging::critical_throw(
        "Attempted to push a spinner into chain while the chain is not empty.\n"
        "The current state of the chain is: {}",
        m_state);
    }

    m_state = state::closed;

    using spinner_t = decltype(spinner);
    auto cancellable = flow::make_cancellable_function(std::forward<spinner_t>(spinner));

    m_handle = cancellable->handle();
    m_context->tasks.push_back(spin_spinner(m_context->thread_pool, *cancellable));
    m_callbacks.push_back(cancellable);
  }

  /**
   * Pushes a producer into the chain
   * @param producer The producer routine
   * @param channel_name The channel name the producer will publish to
   */
  void push(flow::producer auto&& producer, std::string channel_name = "")
  {
    if (m_state not_eq state::empty) {
      flow::logging::critical_throw(
        "Attempted to push a producer into chain while the chain is not empty.\n"
        "The current state of the chain is: {}",
        m_state);
    }

    m_state = state::open;

    using producer_t = decltype(producer);
    using return_t = std::decay_t<typename flow::metaprogramming::function_traits<producer_t>::return_type>;

    auto& channel = make_channel_if_not_exists<return_t>(channel_name);
    auto cancellable = flow::make_cancellable_function(std::forward<producer_t>(producer));

    m_context->tasks.push_back(spin_producer<return_t>(channel, *cancellable));
    m_callbacks.push_back(cancellable);
  }

  /**
   *  Pushes a transformer into the chain and creates any necessary channels it requires
   * @param transformer A routine that depends on another routine and is depended on by a consumer or transformer
   * @param producer_channel_name The channel it depends on
   * @param consumer_channel_name The channel that it will publish to
   */
  void push(flow::transformer auto&& transformer, std::string producer_channel_name = "", std::string consumer_channel_name = "")
  {
    if (m_state not_eq state::open) {
      flow::logging::critical_throw(
        "Attempted to push a transformer into chain while the chain is not open.\n"
        "The current state of the chain is: {}",
        m_state);
    }

    using transformer_t = decltype(transformer);
    using argument_t = std::decay_t<typename flow::metaprogramming::function_traits<transformer_t>::template args<0>::type>;
    using return_t = std::decay_t<typename flow::metaprogramming::function_traits<transformer_t>::return_type>;

    auto& producer_channel = make_channel_if_not_exists<argument_t>(producer_channel_name);
    auto& consumer_channel = make_channel_if_not_exists<argument_t>(consumer_channel_name);
    auto cancellable = flow::make_cancellable_function(std::forward<transformer_t>(transformer));

    m_context->tasks.push_back(spin_transformer<return_t, argument_t>(producer_channel, consumer_channel, *cancellable));
    m_callbacks.push_back(cancellable);
  }

  /**
   * Pushes a consumer into the chain
   * @param consumer A routine no other routine depends on and depends on at least a single routine
   * @param channel_name The channel it will consume from
   */
  void push(flow::consumer auto&& consumer, std::string channel_name = "")
  {
    if (m_state not_eq state::open) {
      flow::logging::critical_throw(
        "Attempted to push a consumer into chain while the chain is not open.\n"
        "The current state of the chain is: {}",
        m_state);
    }

    m_state = state::closed;

    using consumer_t = decltype(consumer);
    using argument_t = std::decay_t<typename flow::metaprogramming::function_traits<consumer_t>::template args<0>::type>;

    auto& channel = make_channel_if_not_exists<argument_t>(channel_name);
    auto cancellable = flow::make_cancellable_function(std::forward<consumer_t>(consumer));

    m_handle = cancellable->handle();
    m_context->tasks.push_back(spin_consumer<argument_t>(channel, *cancellable));
    m_callbacks.push_back(cancellable);
  }

  /**
   * Joins all the routines into a single coroutine
   * @return a coroutine
   */
  task_t spin()
  {
    co_await cppcoro::when_all_ready(std::move(m_context->tasks));
  }

  /**
   * Makes a handle to this chain that will allow whoever holds the handle to cancel
   * the chain
   *
   * The cancellation handle will trigger the consumer to cancel and trickel down all the way to the producer
   * @return A cancellation handle
   */
  cancellation_handle handle()
  {
    if (m_state not_eq state::closed) {
      flow::logging::critical_throw(
        "Attempted to acquire a chain handle without completing the chain first.\n"
        "The current state of the chain is: {}",
        m_state);
    }

    return m_handle;
  }


  /**
   * The current state of the chain
   * @return chain state
   */
  chain::state state()
  {
    return m_state;
  }

  /**
   * Cancel the chain after the specified time
   *
   * This does not mean the chain will be stopped after this amount of time! It takes a non-deterministic
   * amount of time to fully shut the chain down.
   * @param time in milliseconds
   */
  void cancel_after(std::chrono::milliseconds time)
  {
    auto timeout_function_ptr = make_timeout_routine(time, [&] {
      handle().request_cancellation();
    });

    auto& timeout_function = *timeout_function_ptr;

    m_context->tasks.push_back(timeout_function());
    m_callbacks.push_back(std::move(timeout_function_ptr));
  }

private:
  //TODO:: why doesn't chain::state work?
  decltype(chain::state::empty) m_state{ chain::state::empty };

  context<configuration_t>* m_context;
  std::vector<std::any> m_callbacks;
  cancellation_handle m_handle;
};
}// namespace flow
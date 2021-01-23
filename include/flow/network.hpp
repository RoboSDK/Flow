#pragma once

#include <cppcoro/on_scope_exit.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all_ready.hpp>

#include "flow/configuration.hpp"
#include "flow/detail/cancellable_routine.hpp"
#include "flow/detail/channel.hpp"
#include "flow/detail/channel_set.hpp"
#include "flow/detail/mixed_array.hpp"
#include "flow/detail/spin.hpp"
#include "flow/detail/timeout_routine.hpp"
#include "flow/network_handle.hpp"
#include "flow/routine_concepts.hpp"

#include "flow/consumer.hpp"
#include "flow/producer.hpp"
#include "flow/spinner.hpp"
#include "flow/transformer.hpp"

/**
 * A network is a sequence of routines connected by single callable_producer single callable_consumer channels.
 * The end of the network depends on the data flow from the beginning of the network. The beginning
 * of the network has no dependencies.
 *
 * An empty network is a network which has no routines and can take a callable_spinner or a callable_producer.
 *
 * The minimal network s a network with a callable_spinner, because it depends on nothing and nothing depends on it.
 *
 * When a network is begun with a callable_producer, transformers may be inserted into the network until it is capped
 * with a callable_consumer.
 *
 * Each network is considered independent from another network and may not communicate with each other.
 *
 * callable_producer -> transfomer -> ... -> callable_consumer
 *
 * Each channel in the network uses contiguous memory to pass data to the channel waiting on the other way. All
 * data must flow from callable_producer to callable_consumer; no cyclical dependencies.
 *
 * Cancellation
 * Cancelling the network of coroutines is a bit tricky because if you stop them all at once, some of them will hang
 * with no way to have them leave the awaiting state.
 *
 * When starting the network reaction all routines will begin to wait and the first callable_routine that is given priority is the
 * callable_producer at the beginning of the network, and the last will be the end of the network, or callable_consumer.
 *
 * The callable_consumer then has to be the one that initializes the cancellation. The algorithm is as follows:
 *
 * Consumer receives cancellation request from the cancellation handle
 * callable_consumer terminates the channel it is communicating with
 * callable_consumer flushes out any awaiting producers/transformers on the producing end of the channel
 * end callable_routine
 *
 * The callable_transformer or callable_producer that is next in the network will then receiving channel termination notification
 * from the callable_consumer at the end of the network and break out of its loop
 * It well then notify terminate the callable_producer channel it receives data from and flush it out
 *
 * rinse repeat until the beginning of the network, which is a callable_producer
 * The callable_producer simply breaks out of its loop and exits the scope
 */

namespace flow {
template<typename configuration_t>
class network {
public:
  using is_network = std::true_type;

  enum class state {
    empty,/// Initial state
    open,/// Has producers and transformers with no callable_consumer
    closed/// The network is complete and is capped by a callable_consumer
  };

  /**
   * Makes a channel if it doesn't exist and returns a reference to it
   * @tparam message_t The message type the channel will communicate
   * @param channel_name The name of the channel (optional)
   * @return A reference to the channel
   */
  template<typename message_t>
  auto& make_channel_if_not_exists(std::string channel_name)
  {
    if (channels.template contains<message_t>(channel_name)) {
      return channels.template at<message_t>(channel_name);
    }

    using channel_t = detail::channel<message_t, configuration_t>;

    channel_t channel{
      channel_name,
      std::invoke(*resource_generator),
      thread_pool.get()
    };

    channels.put(std::move(channel));
    return channels.template at<message_t>(channel_name);
  }

  /**
   * Pushes a callable_routine into the network
   * @param spinner A callable_routine with no dependencies and nothing depends on it
   */
  template<typename routine_t>
  requires std::is_same_v<flow::spinner, routine_t> void push(routine_t&& routine)
  {
    m_handle.push(routine.callback().handle());
    tasks.push_back(detail::spin_spinner(thread_pool, routine.callback()));
    m_heap_storage.push_back(std::move(routine));
  }

  /**
   * Pushes a callable_producer into the network
   * @param producer The callable_producer callable_routine
   * @param channel_name The channel name the callable_producer will publish to
   */

  template<typename message_t>
  void push(flow::producer<message_t>&& routine)
  {
    auto& channel = make_channel_if_not_exists<message_t>(routine.channel_name());
    tasks.push_back(detail::spin_producer<message_t>(channel, routine.callback()));
    m_heap_storage.push_back(std::move(routine));
  }

  /**
   *  Pushes a callable_transformer into the network and creates any necessary channels it requires
   * @param transformer A callable_routine that depends on another callable_routine and is depended on by a callable_consumer or callable_transformer
   * @param producer_channel_name The channel it depends on
   * @param consumer_channel_name The channel that it will publish to
   */
  template<typename return_t, typename... args_t>
  void push(flow::transformer<return_t(args_t...)>&& routine)
  {
    auto& producer_channel = make_channel_if_not_exists<args_t...>(routine.producer_channel_name());
    auto& consumer_channel = make_channel_if_not_exists<return_t>(routine.consumer_channel_name());

    tasks.push_back(detail::spin_transformer<return_t, args_t...>(producer_channel, consumer_channel, routine.callback()));
    m_heap_storage.push_back(std::move(routine));
  }

  /**
   * Pushes a callable_consumer into the network
   * @param callback A callable_routine no other callable_routine depends on and depends on at least a single callable_routine
   * @param channel_name The channel it will consume from
   */
  template<typename message_t>
  void push(flow::consumer<message_t>&& routine)
  {
    auto& channel = make_channel_if_not_exists<message_t>(routine.channel_name());

    m_handle.push(routine.callback().handle());
    tasks.push_back(detail::spin_consumer<message_t>(channel, routine.callback()));
    m_heap_storage.push_back(std::move(routine));
  }

  /**
   * Joins all the routines into a single coroutine
   * @return a coroutine
   */
  cppcoro::task<void> spin()
  {
    co_await cppcoro::when_all_ready(std::move(tasks));
  }

  /**
   * Makes a handle to this network that will allow whoever holds the handle to cancel
   * the network
   *
   * The cancellation handle will trigger the callable_consumer to cancel and trickel down all the way to the callable_producer
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
    auto timeout_routine = detail::make_shared_timeout_routine(time, [&] {
      handle().request_cancellation();
    });

    tasks.push_back(timeout_routine->spin());
    m_heap_storage.push_back(std::move(timeout_routine));
  }

private:
  using thread_pool_t = cppcoro::static_thread_pool;
  using resource_generator_t = detail::channel_resource_generator<configuration_t>;

  std::unique_ptr<thread_pool_t> thread_pool = std::make_unique<cppcoro::static_thread_pool>();
  std::unique_ptr<resource_generator_t> resource_generator = std::make_unique<resource_generator_t>();
  detail::channel_set<configuration_t> channels{};

  std::vector<cppcoro::task<void>> tasks{};
  std::vector<std::any> m_heap_storage{};

  network_handle m_handle{};
};



template <typename network_t>
concept is_network = std::is_same_v<typename network_t::is_network, std::true_type>;

template<typename routine_t>
concept routine = spinner_concept<routine_t> or producer_concept<routine_t> or consumer_concept<routine_t> or transformer_concept<routine_t>;

template<typename... routines_t>
concept routines = (routine<routines_t> and ...);

template <typename callable_t>
concept not_network_or_routine = not flow::is_network<callable_t> and not flow::are_user_routines<callable_t> and not flow::routine<callable_t>;

template<typename... callables_t>
concept not_network_or_user_routines = (not_network_or_routine<callables_t> and ...);

template<typename configuration_t>
auto make_network(flow::routines auto&&... routines)
{
  using network_t = flow::network<configuration_t>;

  network_t network{};

  auto routines_array = detail::make_mixed_array(std::forward<decltype(routines)>(routines)...);
  std::for_each(std::begin(routines_array), std::end(routines_array), detail::make_visitor([&](auto& routine) {
    network.push(std::move(routine));
  }));

  return network;
}

auto make_network(flow::routines auto&&... routines)
{
  return make_network<flow::configuration>(std::forward<decltype(routines)>(routines)...);
}

template<typename configuration_t>
auto make_network(flow::not_network_or_user_routines auto&&... callables)
{
  using network_t = flow::network<configuration_t>;
  network_t network{};

  auto callables_array = detail::make_mixed_array(std::forward<decltype(callables)>(callables)...);
  std::for_each(std::begin(callables_array), std::end(callables_array), detail::make_visitor([&](auto& callable) {
         using callable_t = decltype(callable);

         if constexpr (flow::callable_transformer<callable_t>) {
           network.push(flow::make_transformer(callable));
         } else if constexpr(flow::callable_consumer<callable_t>) {
           network.push(flow::make_consumer(callable));
         } else if constexpr(flow::callable_producer<callable_t>) {
           network.push(flow::make_producer(callable));
         } else {
           network.push(flow::make_spinner(callable));
         }
  }));

  return network;
}

auto make_network(flow::not_network_or_user_routines auto&&... callables)
{
  return make_network<flow::configuration>(std::forward<decltype(callables)>(callables)...);
}

template<typename configuration_t>
auto make_network(flow::are_user_routines auto&&... routines)
{
  using network_t = flow::network<configuration_t>;

  network_t network{};

  auto routines_array = detail::make_mixed_array(std::forward<decltype(routines)>(routines)...);
  std::for_each(std::begin(routines_array), std::end(routines_array), detail::make_visitor([&](auto& routine) {
         routine.initialize(network);
  }));

  return network;
}

auto make_network(flow::are_user_routines auto&&... routines)
{
  return make_network<flow::configuration>(std::forward<decltype(routines)>(routines)...);
}

}// namespace flow
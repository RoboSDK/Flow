#pragma once

#include <cppcoro/on_scope_exit.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all_ready.hpp>

#include "flow/configuration.hpp"
#include "flow/detail/cancellable_function.hpp"
#include "flow/detail/channel_set.hpp"
#include "flow/detail/mixed_array.hpp"
#include "flow/detail/multi_channel.hpp"
#include "flow/detail/spin.hpp"
#include "flow/detail/timeout_routine.hpp"
#include "flow/network_handle.hpp"
#include "flow/routine_concepts.hpp"

#include "flow/consumer.hpp"
#include "flow/producer.hpp"
#include "flow/spinner.hpp"
#include "flow/transformer.hpp"

/**
 * A network is a sequence of routines connected by single producer_function single consumer_function m_channels.
 * The end of the network depends on the data flow from the beginning of the network. The beginning
 * of the network has no dependencies.
 *
 * An empty network is a network which has no routines and can take a spinner_function or a producer_function.
 *
 * The minimal network s a network with a spinner_function, because it depends on nothing and nothing depends on it.
 *
 * When a network is begun with a producer_function, transformers may be inserted into the network until it is capped
 * with a consumer_function.
 *
 * Each network is considered independent from another network and may not communicate with each other.
 *
 * producer_function -> transfomer -> ... -> consumer_function
 *
 * Each multi_channel in the network uses contiguous memory to pass data to the multi_channel waiting on the other way. All
 * data must flow from producer_function to consumer_function; no cyclical dependencies.
 *
 * Cancellation
 * Cancelling the network of coroutines is a bit tricky because if you stop them all at once, some of them will hang
 * with no way to have them leave the awaiting state.
 *
 * When starting the network reaction all routines will begin to wait and the first callable_routine that is given priority is the
 * producer_function at the beginning of the network, and the last will be the end of the network, or consumer_function.
 *
 * The consumer_function then has to be the one that initializes the cancellation. The algorithm is as follows:
 *
 * Consumer receives cancellation request from the cancellation handle
 * consumer_function terminates the multi_channel it is communicating with
 * consumer_function flushes out any awaiting producers/transformers on the producing end of the multi_channel
 * end callable_routine
 *
 * The transformer_function or producer_function that is next in the network will then receiving multi_channel termination notification
 * from the consumer_function at the end of the network and break out of its loop
 * It well then notify terminate the producer_function multi_channel it receives data from and flush it out
 *
 * rinse repeat until the beginning of the network, which is a producer_function
 * The producer_function simply breaks out of its loop and exits the scope
 */

namespace flow {
template<typename configuration_t>
class network {
public:
  using is_network = std::true_type;

  /**
   * Makes a multi_channel if it doesn't exist and returns a reference to it
   * @tparam message_t The message type the multi_channel will communicate
   * @param channel_name The name of the multi_channel (optional)
   * @return A reference to the multi_channel
   */
  template<typename message_t>
  auto& make_channel_if_not_exists(std::string channel_name)
  {
    if (m_channels.template contains<message_t>(channel_name)) {
      return m_channels.template at<message_t>(channel_name);
    }

    using channel_t = detail::multi_channel<message_t, configuration_t>;

    channel_t channel{
      channel_name,
      std::invoke(*m_resource_generator),
      m_thread_pool.get()
    };

    m_channels.put(std::move(channel));
    return m_channels.template at<message_t>(channel_name);
  }

  /**
   * Pushes a callable_routine into the network
   * @param spinner A callable_routine with no dependencies and nothing depends on it
   */
  void push(flow::spinner_routine auto&& routine)
  {
    m_handle.push(routine.callback().handle());
    m_routines_to_spin.push_back(detail::spin_spinner(m_thread_pool, routine.callback()));
    m_heap_storage.push_back(std::move(routine));
  }

  /**
   * Pushes a producer_function into the network
   * @param producer The producer_function callable_routine
   * @param channel_name The multi_channel name the producer_function will publish to
   */
  template<typename message_t>
  void push(flow::detail::producer_impl<message_t>&& routine)
  {
    auto& channel = make_channel_if_not_exists<message_t>(routine.channel_name());
    m_routines_to_spin.push_back(detail::spin_producer<message_t>(channel, routine.callback()));
    m_heap_storage.push_back(std::move(routine));
  }

  /**
   *  Pushes a transformer_function into the network and creates any necessary m_channels it requires
   * @param transformer A callable_routine that depends on another callable_routine and is depended on by a consumer_function or transformer_function
   * @param producer_channel_name The multi_channel it depends on
   * @param consumer_channel_name The multi_channel that it will publish to
   */
  template<typename return_t, typename... args_t>
  void push(flow::detail::transformer_impl<return_t(args_t...)>&& routine)
  {
    auto& producer_channel = make_channel_if_not_exists<args_t...>(routine.producer_channel_name());
    auto& consumer_channel = make_channel_if_not_exists<return_t>(routine.consumer_channel_name());

    m_routines_to_spin.push_back(detail::spin_transformer<return_t, args_t...>(producer_channel, consumer_channel, routine.callback()));
    m_heap_storage.push_back(std::move(routine));
  }

  /**
   * Pushes a consumer_function into the network
   * @param callback A callable_routine no other callable_routine depends on and depends on at least a single callable_routine
   * @param channel_name The multi_channel it will consume from
   */
  template<typename message_t>
  void push(detail::consumer_impl<message_t>&& routine)
  {
    auto& channel = make_channel_if_not_exists<message_t>(routine.channel_name());

    m_handle.push(routine.callback().handle());
    m_routines_to_spin.push_back(detail::spin_consumer<message_t>(channel, routine.callback()));
    m_heap_storage.push_back(std::move(routine));
  }

  /**
   * Joins all the routines into a single coroutine
   * @return a coroutine
   */
  cppcoro::task<void> spin()
  {
    co_await cppcoro::when_all_ready(std::move(m_routines_to_spin));
  }

  /**
   * Makes a handle to this network that will allow whoever holds the handle to cancel
   * the network
   *
   * The cancellation handle will trigger the consumer_function to cancel and trickel down all the way to the producer_function
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
   * @param any chrono time
   */
  void cancel_after(std::chrono::nanoseconds time)
  {
    // Needs to be heap allocated so coroutines can access the member data out of scope
    auto timeout_routine = std::make_shared<detail::timeout_routine>(time, [&] {
      handle().request_cancellation();
    });

    m_routines_to_spin.push_back(timeout_routine->spin());
    m_heap_storage.push_back(std::move(timeout_routine));
  }

private:
  using thread_pool_t = cppcoro::static_thread_pool;
  using multi_channel_resource_generator = detail::channel_resource_generator<configuration_t, cppcoro::multi_producer_sequencer<std::size_t>>;

  std::unique_ptr<thread_pool_t> m_thread_pool = std::make_unique<cppcoro::static_thread_pool>();
  std::unique_ptr<multi_channel_resource_generator> m_resource_generator = std::make_unique<multi_channel_resource_generator>();

  detail::channel_set<configuration_t> m_channels{};

  std::vector<cppcoro::task<void>> m_routines_to_spin{};
  std::vector<std::any> m_heap_storage{};

  network_handle m_handle{};
};


template<typename configuration_t = flow::configuration>
auto make_network(auto&&... callables)
{
  using network_t = flow::network<configuration_t>;
  network_t network{};

  auto callables_array = detail::make_mixed_array(std::forward<decltype(callables)>(callables)...);
  std::for_each(std::begin(callables_array), std::end(callables_array), detail::make_visitor([&](auto& r) {
         using callable_t = decltype(r);

         // TODO: Why does this work? Add test for concepts
         if constexpr (routine<callable_t>) {
           network.push(std::move(r));
         }
         else if constexpr (is_user_routine<callable_t>) {
           r.initialize(network);
         }
//         else if constexpr (transformer_function<callable_t>) {
//           network.push(make_transformer(r));
//         }
//         else if constexpr (consumer_function<callable_t>) {
//           network.push(flow::make_consumer(r));
//         }
//         else if constexpr (producer_function<callable_t>) {
//           network.push(flow::make_producer(r));
//         }
//         else if constexpr (spinner_function<callable_t>) {
//           network.push(flow::make_spinner(r));
//         }
  }));

  return network;
}

}// namespace flow
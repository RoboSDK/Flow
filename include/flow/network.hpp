#pragma once

#include <iostream>

#include <cppcoro/on_scope_exit.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all_ready.hpp>

#include "flow/configuration.hpp"
#include "flow/detail/cancellable_function.hpp"
#include "flow/detail/channel_set.hpp"
#include "flow/detail/mixed_array.hpp"
#include "flow/detail/multi_channel.hpp"
#include "flow/detail/spin_routine.hpp"
#include "flow/detail/timeout_routine.hpp"

#include "flow/concepts.hpp"
#include "flow/consumer.hpp"
#include "flow/network_handle.hpp"
#include "flow/producer.hpp"
#include "flow/spinner.hpp"
#include "flow/transformer.hpp"

/**
 * A network is a sequence of routines connected by single producer single consumer channels.
 * The end of the network depends on the data flow from the beginning of the network. The beginning
 * of the network has no dependencies.
 *
 * An empty network is a network which has no routines and can take a spinner or a producer.
 *
 * The minimal network s a network with a spinner, because it depends on nothing and nothing depends on it.
 *
 * When a network is begun with a producer, transformers may be inserted into the network until it is capped
 * with a consumer.
 *
 * Each network is considered independent from another network and may not communicate with each other.
 *
 * producer -> transfomer -> ... -> consumer
 *
 * Each channel in the network uses contiguous memory to pass data to the channel waiting on the other way. All
 * data must flow from producer to consumer; no cyclical dependencies.
 *
 * Cancellation
 * Cancelling the network of coroutines is a bit tricky because if you stop them all at once, some of them will hang
 * with no way to have them leave the awaiting state.
 *
 * When starting the network reaction all routines will begin to wait and the first callable_routine that is given priority is the
 * producer at the beginning of the network, and the last will be the end of the network, or consumer.
 *
 * The consumer then has to be the one that initializes the cancellation. The algorithm is as follows:
 *
 * Consumer receives cancellation request from the cancellation handle
 * consumer terminates the channel it is communicating with
 * consumer flushes out any awaiting producers/transformers on the producing end of the channel
 * end callable_routine
 *
 * The transformer or producer that is next in the network will then receiving channel termination notification
 * from the consumer at the end of the network and break out of its loop
 * It well then notify terminate the producer channel it receives data from and flush it out
 *
 * rinse repeat until the beginning of the network, which is a producer
 * The producer simply breaks out of its loop and exits the scope
 */

namespace flow {
namespace detail {
  template<typename configuration_t>
  class network_impl;

  /// Retrieve channel name this function is publishing to
  template<flow::is_function function_t>
  std::string get_publish_to(function_t& function);

  /// Retrieve channel name this function is subscribing to to
  template<flow::is_function function_t>
  std::string get_subscribe_to(function_t& function);
}// namespace detail

/***
 * Creates a network implementation from raw functions, lambdas, std::function, functors, and routine implementations in any order
 * @tparam configuration_t The global compile time configuration for the project
 * @param routines
 * @return
 */
template<typename configuration_t = flow::configuration>
auto network(auto&&... routines)
{
  using network_t = flow::detail::network_impl<configuration_t>;
  network_t network{};

  auto routines_array = detail::make_mixed_array(std::forward<decltype(routines)>(routines)...);
  std::for_each(std::begin(routines_array), std::end(routines_array), detail::make_visitor([&](auto& r) {
    using namespace flow::detail;

    using routine_t = decltype(r);

    if constexpr (is_transformer_function<routine_t>) {
      network.push(transformer(r, get_subscribe_to(r), get_publish_to(r)));
    }
    else if constexpr (is_consumer_function<routine_t>) {
      network.push(flow::consumer(r, get_subscribe_to(r)));
    }
    else if constexpr (is_producer_function<routine_t>) {
      network.push(flow::producer(r, get_publish_to(r)));
    }
    /**
       * If you change this please be careful. The constexpr check for a spinner function seems to
       * be broken and I'm not sure why. I need to explicitly check if it's not a routine, and remove the
       * requirement from the spinner_impl constructor because otherwise the routines generates by make_routine
       * will fail because they do not implement operator(). The operator() check is in metaprogramming.hpp
       * with the function traits section at the bottom of the header.
       */
    else if constexpr (not is_routine<routine_t> and is_spinner_function<routine_t>) {
      network.push(flow::spinner(r));
    }
    else {
      network.push(std::move(r));
    }
  }));

  if (network.size() < routines_array.size()) {
    std::cerr << "Network size: " << network.size() << "\n";
    std::cerr << "Callables array size: " << routines_array.size() << "\n";
    throw std::runtime_error("Network size is less than functions array. This is a developer error. Please submit an issue.");
  }

  return network;
}

namespace detail {
  template<flow::is_function function_t>
  std::string get_publish_to(function_t& function)
  {
    if constexpr (flow::has_publisher_channel<function_t>) {
      return function.publish_to();
    }
    else {
      return "";
    }
  }

  template<flow::is_function function_t>
  std::string get_subscribe_to(function_t& function)
  {
    if constexpr (flow::has_subscription_channel<function_t>) {
      return function.subscribe_to();
    }
    else {
      return "";
    }
  }

  template<typename configuration_t>
  class network_impl {
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
    void push(flow::is_spinner_routine auto&& routine)
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
      auto& channel = make_channel_if_not_exists<message_t>(routine.publish_to());
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
      auto& producer_channel = make_channel_if_not_exists<args_t...>(routine.subscribe_to());
      auto& consumer_channel = make_channel_if_not_exists<return_t>(routine.publish_to());

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
      auto& channel = make_channel_if_not_exists<message_t>(routine.subscribing_to());

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

    bool empty() const
    {
      return m_routines_to_spin.empty();
    }

    std::size_t size() const
    {
      return m_routines_to_spin.size();
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
}// namespace detail
}// namespace flow
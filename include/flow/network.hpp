#pragma once

#include <future>

#include <cppcoro/on_scope_exit.hpp>
#include <cppcoro/schedule_on.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all.hpp>

#include "flow/configuration.hpp"
#include "flow/detail/cancellable_function.hpp"
#include "flow/detail/channel_set.hpp"
#include "flow/detail/mixed_array.hpp"
#include "flow/detail/multi_channel.hpp"
#include "flow/detail/routine.hpp"
#include "flow/detail/single_channel.hpp"
#include "flow/detail/spin_routine.hpp"
#include "flow/detail/timeout_routine.hpp"

#include "flow/concepts.hpp"
#include "flow/consumer.hpp"
#include "flow/network_handle.hpp"
#include "flow/producer.hpp"
#include "flow/spinner.hpp"
#include "flow/transformer.hpp"

#include "flow/chain.hpp"

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

  namespace channel {
    enum class policy {
      SINGLE,
      MULTI
    };
  }

  template<typename configuration_t>
  class network_impl;

  /// Retrieve channel name this function is publishing to
  template<flow::is_function function_t>
  std::string get_publish_to(function_t& function);

  /// Retrieve channel name this function is subscribing to to
  template<flow::is_function function_t>
  std::string get_subscribe_to(function_t& function);
}// namespace detail

auto push_routine(is_network auto& network, auto&& routine)
{
  using namespace flow::detail;

  using routine_t = decltype(routine);
  if constexpr (is_transformer_function<routine_t>) {
    network.push(transformer(routine, get_subscribe_to(routine), get_publish_to(routine)));
  }
  else if constexpr (is_consumer_function<routine_t>) {
    network.push(flow::consumer(routine, get_subscribe_to(routine)));
  }
  else if constexpr (is_producer_function<routine_t>) {
    network.push(flow::producer(routine, get_publish_to(routine)));
  }
  /**
       * If you change this please be careful. The constexpr check for a spinner function seems to
       * be broken and I'm not sure why. I need to explicitly check if it's not a routine, and remove the
       * requirement from the spinner_impl constructor because otherwise the routines generates by make_routine
       * will fail because they do not implement operator(). The operator() check is in metaprogramming.hpp
       * with the function traits section at the bottom of the header.
       */
  else if constexpr (not is_routine<routine_t> and is_spinner_function<routine_t>) {
    network.push(flow::spinner(routine));
  }
  else {
    network.push(forward(routine));
  }
}

auto push_routine_or_chain(auto& network, auto&& routine)
{
  using routine_t = decltype(routine);

  if constexpr (is_chain<routine_t>) {
    network.push_chain(forward(routine));
  }
  else if constexpr (not is_chain<routine_t>) {
    push_routine(network, forward(routine));
  }
}

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

  (push_routine_or_chain(network, forward(routines)), ...);

  constexpr std::size_t num_routines = sizeof...(routines);
  if (network.size() < num_routines) {
    std::cerr << "Network size: " << network.size() << "\n";
    std::cerr << "Callables array size: " << num_routines << "\n";
    throw std::runtime_error("Network size is less than functions array. This is a developer error. Please submit an issue.");
  }

  return network;
}

namespace detail {
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
    template<typename message_t, detail::channel::policy policy = detail::channel::policy::MULTI>
    auto& make_channel(std::string channel_name = "")
    {

      if constexpr (policy == detail::channel::policy::MULTI) {
        if (m_channels.template contains<message_t>(channel_name)) {
          return m_channels.template at<message_t>(channel_name);
        }

        using channel_t = detail::multi_channel<message_t, configuration_t>;

        channel_t channel{
          channel_name,
          std::invoke(*m_multi_channel_resource_generator),
          m_thread_pool.get()
        };

        m_channels.put(std::move(channel));
        return m_channels.template at<message_t>(channel_name);
      }
      else if constexpr (policy == detail::channel::policy::SINGLE) {
        using channel_t = detail::single_channel<message_t, configuration_t>;

        channel_t channel{
          channel_name,
          std::invoke(*m_single_channel_resource_generator),
          m_thread_pool.get()
        };

        m_heap_storage.push_back(std::move(channel));
        return std::any_cast<channel_t&>(m_heap_storage.back());
      }
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
    template<
      detail::channel::policy producer_channel_policy = detail::channel::policy::MULTI,
      typename message_t>
    auto& push(flow::detail::producer_impl<message_t>&& routine)
    {
      auto& channel = make_channel<message_t, producer_channel_policy>(routine.publish_to());
      m_routines_to_spin.push_back(detail::spin_producer<message_t>(channel, routine.callback()));
      m_heap_storage.push_back(std::move(routine));
      return channel;
    }

    /**
   *  Pushes a transformer_function into the network and creates any necessary m_channels it requires
   * @param transformer A callable_routine that depends on another callable_routine and is depended on by a consumer_function or transformer_function
   * @param producer_channel_name The multi_channel it depends on
   * @param consumer_channel_name The multi_channel that it will publish to
   */
    template<
      detail::channel::policy producer_channel_policy = detail::channel::policy::MULTI,
      detail::channel::policy consumer_channel_policy = detail::channel::policy::MULTI,
      typename return_t,
      typename... args_t>
    auto push(flow::detail::transformer_impl<return_t(args_t...)>&& routine)
    {
      auto& producer_channel = make_channel<args_t..., producer_channel_policy>(routine.subscribe_to());
      auto& consumer_channel = make_channel<return_t, consumer_channel_policy>(routine.publish_to());

      m_routines_to_spin.push_back(detail::spin_transformer<return_t, args_t...>(producer_channel, consumer_channel, routine.callback()));
      m_heap_storage.push_back(std::move(routine));

      return std::make_pair(std::ref(producer_channel), std::ref(consumer_channel));
    }

    /**
   * Pushes a consumer_function into the network
   * @param callback A callable_routine no other callable_routine depends on and depends on at least a single callable_routine
   * @param channel_name The multi_channel it will consume from
   */
    template<typename message_t>
    void push(detail::consumer_impl<message_t>&& routine)
    {
      auto& channel = make_channel<message_t>(routine.subscribing_to());

      m_handle.push(routine.callback().handle());
      m_routines_to_spin.push_back(detail::spin_consumer<message_t>(channel, routine.callback()));
      m_heap_storage.push_back(std::move(routine));
    }

    template<typename begin_t>
      auto& push_chain_begin(begin_t&& begin) requires is_transformer_routine<begin_t> or is_producer_routine<begin_t>
    {
      using namespace detail::channel;

      static_assert(not is_consumer_routine<begin_t> and not is_spinner_routine<begin_t>,
        "network.hpp:push_chain_begin only takes in transformer or producer routines implementations.");

      if constexpr (is_transformer_routine<begin_t>) {
        return push<policy::MULTI, policy::SINGLE>(std::move(begin)).second;
      }
      else {
        return push<policy::SINGLE>(std::move(begin));
      }
    }

    template<typename end_t>
      void push_chain_end(end_t&& end, auto& channel) requires is_transformer_routine<end_t> or is_consumer_routine<end_t>
    {
      using namespace detail::channel;

      static_assert(not is_producer_routine<end_t> and not is_spinner_routine<end_t>,
        "network.hpp:push_chain_end only takes in transformer or consumer routines implementations.");

      if constexpr (is_transformer_routine<end_t>) {
        using arg_t = typename decltype(channel.message_type())::type;

        using return_t = typename detail::traits<decltype(end.callback())>::return_type;
        auto& next_channel = make_channel<return_t, policy::SINGLE>();

        m_routines_to_spin.push_back(detail::spin_transformer<return_t, arg_t>(channel, next_channel, end.callback()));
        m_heap_storage.push_back(std::move(end.callback()));
      }
      else {
        using message_t = typename decltype(channel.message_type())::type;

        m_handle.push(end.callback().handle());
        m_routines_to_spin.push_back(detail::spin_consumer<message_t>(channel, end.callback()));
        m_heap_storage.push_back(std::move(end));
      }
    }

    template<std::size_t tuple_index, std::size_t tuple_size>
    auto& push_tightly_linked_functions(auto& channel, auto& functions)
    {
      // up to second to last
      if constexpr (tuple_index == tuple_size - 1) return channel;
      else {
        using namespace detail::channel;

        auto next_function = std::get<tuple_index>(functions);

        using arg_t = typename decltype(channel.message_type())::type;

        using return_t = typename detail::traits<decltype(next_function)>::return_type;
        auto& next_channel = make_channel<return_t, policy::SINGLE>();

        auto next_routine = to_routine(std::move(next_function));
        m_routines_to_spin.push_back(detail::spin_transformer<return_t, arg_t>(channel, next_channel, next_routine.callback()));
        m_heap_storage.push_back(std::move(next_routine));

        return push_tightly_linked_functions<tuple_index + 1, tuple_size>(next_channel, functions);
      }
    }

    constexpr void push_chain(is_chain auto&& chain)
    {
      constexpr std::size_t tuple_size = std::tuple_size<decltype(chain.routines)>();
      constexpr std::size_t start_index = 0;

      if constexpr (tuple_size == 1) {
        push(std::move(std::get<start_index>(chain.routines)));
      }
      else if constexpr (tuple_size == 2) {
        auto& channel = push_chain_begin(std::move(std::get<start_index>(chain.routines)));
        push_chain_end(std::move(std::get<tuple_size - 1>(chain.routines)), channel);
      }
      else if constexpr (tuple_size > 2) {
        auto& channel = push_chain_begin(std::move(std::get<start_index>(chain.routines)));
        auto& last_channel = push_tightly_linked_functions<start_index + 1, tuple_size>(channel, chain.routines);
        push_chain_end(std::move(std::get<tuple_size - 1>(chain.routines)), last_channel);
      }
    }

    /**
   * Joins all the routines into a single coroutine
   * @return a coroutine
   */
    cppcoro::task<void> spin()
    {
      co_await cppcoro::when_all(std::move(m_routines_to_spin));
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
    auto cancel_after(std::chrono::nanoseconds time)
    {
      auto time_out_after = [](auto time, std::reference_wrapper<network_handle> handle) {
        spin_wait waiter{ time };
        while (not waiter.is_ready()) {
          std::this_thread::yield();
        }

        handle.get().request_cancellation();
      };

      auto thread =  std::thread( time_out_after, time, std::ref(m_handle));
      thread.detach();
    }

  private:
    using thread_pool_t = cppcoro::static_thread_pool;
    using multi_channel_resource_generator = detail::channel_resource_generator<configuration_t, cppcoro::multi_producer_sequencer<std::size_t>>;
    using single_channel_resource_generator = detail::channel_resource_generator<configuration_t, cppcoro::single_producer_sequencer<std::size_t>>;

    std::unique_ptr<thread_pool_t> m_thread_pool = std::make_unique<cppcoro::static_thread_pool>();
    std::unique_ptr<multi_channel_resource_generator> m_multi_channel_resource_generator = std::make_unique<multi_channel_resource_generator>();

    std::unique_ptr<single_channel_resource_generator> m_single_channel_resource_generator = std::make_unique<single_channel_resource_generator>();

    detail::channel_set<configuration_t> m_channels{};

    std::vector<cppcoro::task<void>> m_routines_to_spin{};
    std::vector<std::any> m_heap_storage{};

    network_handle m_handle{};
  };
}// namespace detail
}// namespace flow

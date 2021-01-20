#pragma once

#include <execution>

#include "flow/chain.hpp"
#include "flow/context.hpp"
#include "flow/hash.hpp"
#include "flow/metaprogramming.hpp"

namespace flow {

/**
 * A forest is a collection of disjoint trees. In other words, we can also say
 * that forest is a collection of an acyclic graph which is not connected.
 *
 * The acyclic disjoint graphs in this case are chains of routines that are connected
 * by channels
 */

template<typename configuration_t>
class forest {
  using context_t = flow::context<configuration_t>;
  using chain_t = flow::chain<configuration_t>;

  template<typename callable_t>
  using traits = flow::metaprogramming::function_traits<std::decay_t<callable_t>>;

public:
  /**
   * Push a spinner into the forest
   * Producers will always generate a new chain and be complete
   * @param routine the spinner
   */
  void push(flow::spinner auto&& routine)
  {
    m_chains.emplace_back(m_context.get());
    auto& chain = m_chains.back();
    chain.push(routine);
  }

  /**
   * Push a producer into the forest
   * Producers will always generate a new chain
   * @param routine the producer
   * @param channel_name the channel name the producer will publish on
   */
  void push(flow::producer auto&& routine, std::string channel_name = "")
  {
    m_chains.emplace_back(m_context.get());
    auto& chain = m_chains.back();

    chain.push(std::move(routine), std::move(channel_name));

    using message_t = typename traits<decltype(routine)>::return_type;
    std::size_t hashed_consumer = flow::hash<message_t>(channel_name);

    // Used to find the chain that needs this consumer or transformer
    m_incomplete_chains.emplace(std::make_pair(hashed_consumer, std::ref(chain)));
  }

  /**
   * Push a consumer into the forest
   *
   * @param routine the consumer
   * @param channel_name the channel name the consumer will subscribe to
   */
  void push(flow::consumer auto&& routine, std::string channel_name = "")
  {
    using message_t = typename traits<decltype(routine)>::template args<0>::type;

    std::size_t hashed_channel = flow::hash<message_t>(channel_name);

    try {
      auto chain_reference = m_incomplete_chains.at(hashed_channel);
      chain_reference.get().push(std::move(routine), std::move(channel_name));
    }
    catch (...) {
      flow::logging::critical_throw("Attempted to push a consumer into a forest with no producer to match it.");
    }

    m_incomplete_chains.erase(hashed_channel);// The chain is now complete
  }

  /**
   * Push a transformer into the forest
   *
   * @param routine the transformer
   * @param channel_name the channel name the consumer will subscribe to
   */
  void push(flow::transformer auto&& routine, std::string producer_channel_name = "", std::string consumer_channel_name = "")
  {
    using producer_message_t = typename traits<decltype(routine)>::template args<0>::type;

    const std::size_t hashed_producer_channel = flow::hash<producer_message_t>(producer_channel_name);

    std::reference_wrapper<chain_t> chain_reference{};

    try {
      auto chain = m_incomplete_chains.at(hashed_producer_channel);
      chain.get().push(std::move(routine), std::move(producer_channel_name));
      chain_reference = chain;
    }
    catch (...) {
      flow::logging::critical_throw("Attempted to push a transformer into a forest with no producer to match it.");
    }

    m_incomplete_chains.erase(hashed_producer_channel);// The chain is now complete

    using consumer_message_t = typename traits<decltype(routine)>::return_type;
    const std::size_t hashed_consumer_channel = flow::hash<consumer_message_t>(consumer_channel_name);
    m_incomplete_chains.emplace(std::make_pair(hashed_consumer_channel, chain_reference));
  }

  /**
   * Joins all the chains into a single coroutine
   * @return a coroutine
   */
  cppcoro::task<void> spin()
  {
    if (not is_ready()) {
      flow::logging::critical_throw("There is at least chain that is not complete in the forest!");
    }

    co_await cppcoro::when_all_ready(std::move(m_context->tasks));
  }

  /**
   * Checks if all chains are closed and ready to run
   */
  bool is_ready() const
  {
    const bool is_ready = std::ranges::all_of(m_chains, [](auto& chain) {
      return chain.state() == chain_t::state::closed;
    });

    return is_ready;
  }

  /**
   * Cancel the forest after the specified time
   *
   * This does not mean the forest will be stopped after this amount of time! It takes a non-deterministic
   * amount of time to fully shut the forest down.
   * @param time in milliseconds
   */
  void cancel_after(std::chrono::milliseconds time)
  {
    if (not is_ready()) {
      flow::logging::critical_throw("Attempted to set cancellation time before all chains are completed.");
    }

    for (auto& chain : m_chains) {
      m_cancellation_handles.push_back(chain.handle());
    }

    auto timeout_function_ptr = make_timeout_routine(time, [&] {
      std::for_each(std::execution::par_unseq, std::begin(m_cancellation_handles), std::end(m_cancellation_handles), [](auto& handle) {
        handle.request_cancellation();
      });
    });

    auto& timeout_function = *timeout_function_ptr;

    m_context->tasks.push_back(timeout_function());
  }

private:
  /*
   * The context needs to be dynamically allocated so that channel resources
   * will be accessible by coroutines that the chains will be spinning up
   */
  std::unique_ptr<context_t> m_context = std::make_unique<context_t>();
  std::vector<chain_t> m_chains{};

  std::unordered_map<std::size_t, std::reference_wrapper<chain_t>> m_incomplete_chains{};

  std::vector<cancellation_handle> m_cancellation_handles{};
};

}// namespace flow

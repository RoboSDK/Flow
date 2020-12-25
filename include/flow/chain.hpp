#pragma once

#include <cppcoro/task.hpp>
#include <cppcoro/when_all_ready.hpp>

#include "flow/callback_handle.hpp"
#include "flow/cancellation.hpp"
#include "flow/channel.hpp"
#include "flow/context.hpp"
#include "flow/data_structures/channel_set.hpp"
#include "flow/function_concepts.hpp"
#include "flow/spin.hpp"

namespace flow {

class chain_handle {
public:
  void request_cancellation()
  {
    for (auto& handle : m_handles) handle.request_cancellation();
  }

  void push(cancellation_handle&& handle)
  {
    m_handles.push_back(std::move(handle));
  }

private:
  std::vector<cancellation_handle> m_handles{};
};

template<typename configuration_t>
class chain {
public:
  enum class state {
    empty,
    open,
    closed
  };

  using task_t = cppcoro::task<void>;

  chain(auto* context) : m_context(context) {}

  /************************************************************************************************/
  template<typename message_t>
  auto& make_channel_if_not_exists(std::string channel_name)
  {
    if (m_context->channels.template contains<message_t>(channel_name)) {
      return m_context->channels.template at<message_t>(channel_name);
    }

    using channel_t = channel<message_t, configuration_t>;

    auto channel = channel_t{
      channel_name,
      get_channel_resource(m_context->resource_generator),
      &m_context->thread_pool
    };

    m_context->channels.put(std::move(channel));
    return m_context->channels.template at<message_t>(channel_name);
  }

  /************************************************************************************************/
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

    m_handle.push(cancellable->handle());
    m_context->tasks.push_back(spin_spinner(m_context->thread_pool, *cancellable));
    m_callbacks.push_back(cancellable);
  }

  /************************************************************************************************/
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

    m_handle.push(cancellable->handle());
    m_context->tasks.push_back(spin_producer<return_t>(channel, *cancellable));
    m_callbacks.push_back(cancellable);
  }

  /************************************************************************************************/

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

    m_handle.push(cancellable->handle());
    m_context->tasks.push_back(spin_transformer<return_t, argument_t>(producer_channel, consumer_channel, *cancellable));
    m_callbacks.push_back(cancellable);
  }

  /************************************************************************************************/

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

    m_handle.push(cancellable->handle());
    m_context->tasks.push_back(spin_consumer<argument_t>(channel, *cancellable));
    m_callbacks.push_back(cancellable);
  }

  /************************************************************************************************/

  task_t spin()
  {
    co_await cppcoro::when_all_ready(std::move(m_context->tasks));
  }

  /************************************************************************************************/

  chain_handle handle()
  {
    return m_handle;
  }

  /************************************************************************************************/

  chain::state state()
  {
    return m_state;
  }

private:
  //TODO:: why doesn't chain::state work?
  decltype(chain::state::empty) m_state{chain::state::empty};

  context<configuration_t>* m_context;
  std::vector<std::any> m_callbacks;
  chain_handle m_handle{};
};
}// namespace flow
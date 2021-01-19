#pragma once

#include <cppcoro/on_scope_exit.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all_ready.hpp>
#include <future>

#include "flow/callback_handle.hpp"
#include "flow/cancellation.hpp"
#include "flow/channel.hpp"
#include "flow/context.hpp"
#include "flow/data_structures/channel_set.hpp"
#include "flow/function_concepts.hpp"
#include "flow/spin.hpp"
#include "flow/spin_wait.hpp"

namespace flow {
template<typename configuration_t>
class chain {
public:
  enum class state {
    empty,
    open,
    closed
  };

  using task_t = cppcoro::task<void>;

  explicit chain(auto* context) : m_context(context) {}

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

    m_handle = cancellable->handle();
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

    m_handle = cancellable->handle();
    m_context->tasks.push_back(spin_consumer<argument_t>(channel, *cancellable));
    m_callbacks.push_back(cancellable);
  }

  /************************************************************************************************/

  task_t spin()
  {
    co_await cppcoro::when_all_ready(std::move(m_context->tasks));
  }

  /************************************************************************************************/

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

  /************************************************************************************************/

  chain::state state()
  {
    return m_state;
  }

  void cancel_after(std::chrono::milliseconds time)
  {
    using namespace std::chrono;

    struct allocated_task {
      cppcoro::task<void> spin()
      {
        auto cancelOnExit = cppcoro::on_scope_exit([&] {
          auto ret = std::async(std::launch::async, [&]() {
                 handle.request_cancellation();
          });
        });
        while (time_elapsed < threshold) {
          auto time_delta = std::chrono::steady_clock::now() - last_timestamp;
          time_elapsed += duration_cast<milliseconds>(time_delta);
        }
        co_return;
      }

      cancellation_handle handle;
      std::chrono::milliseconds threshold{};
      milliseconds time_elapsed{ 0 };
      decltype(steady_clock::now()) last_timestamp{ steady_clock::now() };
    };

    auto waiting_ask = std::make_shared<allocated_task>(m_handle, time);
    m_context->tasks.push_back(waiting_ask->spin());
    m_callbacks.push_back(std::move(waiting_ask));
  }

private:
  //TODO:: why doesn't chain::state work?
  decltype(chain::state::empty) m_state{ chain::state::empty };

  context<configuration_t>* m_context;
  std::vector<std::any> m_callbacks;
  cancellation_handle m_handle;
};
}// namespace flow
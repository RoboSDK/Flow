#pragma once

#include <cppcoro/task.hpp>
#include <cppcoro/when_all_ready.hpp>

#include "flow/callback_handle.hpp"
#include "flow/cancellation.hpp"
#include "flow/channel.hpp"
#include "flow/data_structures/channel_set.hpp"
#include "flow/function_concepts.hpp"
#include "flow/spin.hpp"
#include "flow/context.hpp"

namespace flow {

template<typename configuration_t>
struct chain {
  using task_t = cppcoro::task<void>;

  chain(auto* context) :  m_context(context) {}

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
  cancellation_handle push_producer(flow::producer auto&& producer, std::string channel_name = "")
  {
    using producer_t = decltype(producer);
    using return_t = std::decay_t<typename flow::metaprogramming::function_traits<producer_t>::return_type>;

    auto& channel = make_channel_if_not_exists<return_t>(channel_name);
    auto cancellable = flow::make_cancellable_function(std::forward<producer_t>(producer));

    m_context->tasks.push_back(spin_producer<return_t>(channel, *cancellable));
    m_callbacks.push_back(cancellable);
    return cancellable->handle();
  }

  /************************************************************************************************/

  cancellation_handle push_transformer(
    flow::transformer auto&& transformer,
    std::string producer_channel_name = "",
    std::string consumer_channel_name = "")
  {
    using transformer_t = decltype(transformer);
    using argument_t = std::decay_t<typename flow::metaprogramming::function_traits<transformer_t>::template args<0>::type>;
    using return_t = std::decay_t<typename flow::metaprogramming::function_traits<transformer_t >::return_type>;

    auto& producer_channel = make_channel_if_not_exists<argument_t>(producer_channel_name);
    auto& consumer_channel = make_channel_if_not_exists<argument_t>(consumer_channel_name);

    auto cancellable = flow::make_cancellable_function(std::forward<transformer_t>(transformer));
    m_context->tasks.push_back(spin_transformer<return_t, argument_t>(producer_channel, consumer_channel, *cancellable));
    m_callbacks.push_back(cancellable);
    return cancellable->handle();
  }

  /************************************************************************************************/

  cancellation_handle push_consumer(flow::consumer auto&& consumer, std::string channel_name = "")
  {
    using consumer_t = decltype(consumer);
    using argument_t = std::decay_t<typename flow::metaprogramming::function_traits<consumer_t>::template args<0>::type>;

    auto& channel = make_channel_if_not_exists<argument_t>(channel_name);
    auto cancellable = flow::make_cancellable_function(std::forward<consumer_t>(consumer));
    m_context->tasks.push_back(spin_consumer<argument_t>(channel, *cancellable));
    m_callbacks.push_back(cancellable);
    return cancellable->handle();
  }

  /************************************************************************************************/

  task_t spin()
  {
    flow::logging::info("m_context->tasks.size(): {}", m_context->tasks.size());
    co_await cppcoro::when_all_ready(std::move(m_context->tasks));
  }

  /************************************************************************************************/

  context<configuration_t>* m_context;
  std::vector<std::any> m_callbacks;
};
}// namespace flow
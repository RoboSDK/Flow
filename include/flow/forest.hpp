#pragma once

namespace flow {
class forest {
public:
  cppcoro::task<void> spin()
  {
    std::vector<cppcoro::task<void>> tasks{};
    for (auto& chain : m_chains) {
      for (auto& task : chain) {
        tasks.push_back(std::move(task));
      }
    }
    co_return cppcoro::when_all_ready(std::move(tasks));
  }

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

private:
  std::vector<chain> m_chains;
  std::unordered_set<size_t> m_channel_hashes;
};
}// namespace flow
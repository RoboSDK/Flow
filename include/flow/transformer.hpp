#pragma once

#include "flow/options.hpp"

namespace flow {

template<typename T>
class transformer;

template<typename return_t, typename... args_t>
class transformer<return_t(args_t...)> {
public:
  using is_transformer = std::true_type;
  using is_routine = std::true_type;

  transformer() = default;
  ~transformer() = default;

  transformer(transformer&&) noexcept = default;
  transformer(transformer const&) = default;
  transformer& operator=(transformer&&) noexcept = default;
  transformer& operator=(transformer const&) = default;

  transformer(flow::callable_transformer auto&& callback, std::string producer_channel_name, std::string consumer_channel_name)
    : m_callback(detail::make_cancellable_routine(std::forward<decltype(callback)>(callback))),
      m_producer_channel_name(std::move(producer_channel_name)),
      m_consumer_channel_name(std::move(consumer_channel_name))
  {}

  auto producer_channel_name() { return m_producer_channel_name; }
  auto consumer_channel_name() { return m_consumer_channel_name; }

  auto& callback() { return *m_callback; }

private:
  typename detail::cancellable_routine<return_t(args_t&&...)>::sPtr m_callback{ nullptr };

  std::string m_producer_channel_name{};
  std::string m_consumer_channel_name{};
};

template<typename return_t, typename argument_t>
auto make_transformer(std::function<return_t(argument_t&&)>&& callback, flow::options options = flow::options{})
{
  using callback_t = decltype(callback);
  return transformer<return_t(argument_t)>(std::forward<callback_t>(callback), std::move(options.subscribe_to), std::move(options.publish_to));
}

template<typename return_t, typename argument_t>
auto make_transformer(return_t (*callback)(argument_t&&), flow::options options = flow::options{})
{
  using callback_t = decltype(callback);
  return transformer<return_t(argument_t)>(std::forward<callback_t>(callback), std::move(options.subscribe_to), std::move(options.publish_to));
}

auto make_transformer(auto&& lambda, flow::options options = flow::options{})
{
  using callback_t = decltype(lambda);
  return make_transformer(detail::metaprogramming::to_function(std::forward<callback_t>(lambda)), std::move(options.subscribe_to), std::move(options.publish_to));
}

template<typename return_t, typename argument_t>
auto make_transformer(std::function<return_t(argument_t&&)>&& callback, std::string publish_to, std::string subscribe_to)
{
  using callback_t = decltype(callback);
  return transformer<return_t(argument_t)>(std::forward<callback_t>(callback), std::move(subscribe_to), std::move(publish_to));
}

template<typename return_t, typename argument_t>
auto make_transformer(return_t (*callback)(argument_t&&), std::string publish_to, std::string subscribe_to)
{
  using callback_t = decltype(callback);
  return transformer<return_t(argument_t)>(std::forward<callback_t>(callback), std::move(subscribe_to), std::move(publish_to));
}

auto make_transformer(auto&& lambda, std::string publish_to, std::string subscribe_to)
{
  using callback_t = decltype(lambda);
  return make_transformer(detail::metaprogramming::to_function(std::forward<callback_t>(lambda)), std::move(subscribe_to), std::move(publish_to));
}


template<typename transformer_t>
concept transformer_concept = std::is_same_v<typename transformer_t::is_transformer, std::true_type>;
}// namespace flow
#pragma once

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
    : m_callback(flow::make_cancellable_function(std::forward<decltype(callback)>(callback))),
      m_producer_channel_name(std::move(producer_channel_name)),
      m_consumer_channel_name(std::move(consumer_channel_name))
  {}

  auto producer_channel_name() { return m_producer_channel_name; }
  auto consumer_channel_name() { return m_consumer_channel_name; }

  auto& callback() { return *m_callback; }

private:
  typename flow::cancellable_function<return_t(args_t&&...)>::sPtr m_callback{ nullptr };

  std::string m_producer_channel_name{};
  std::string m_consumer_channel_name{};
};

template<typename return_t, typename argument_t>
auto make_transformer(std::function<return_t(argument_t&&)>&& callback, std::string channel_name = "")
{
  using callback_t = decltype(callback);
  return transformer<return_t(argument_t)>(std::forward<callback_t>(callback), std::move(channel_name));
}

template<typename return_t, typename argument_t>
auto make_transformer(return_t (*callback)(argument_t&&), std::string producer_channel_name = "", std::string consumer_channel_name = "")
{
  using callback_t = decltype(callback);
  return transformer<return_t(argument_t)>(std::forward<callback_t>(callback), std::move(producer_channel_name), std::move(consumer_channel_name));
}

auto make_transformer(auto&& lambda, std::string producer_channel_name = "", std::string consumer_channel_name = "")
{
  using callback_t = decltype(lambda);
  return make_transformer(flow::metaprogramming::to_function(std::forward<callback_t>(lambda)), std::move(producer_channel_name), std::move(consumer_channel_name));
}


template<typename transformer_t>
concept transformer_concept = std::is_same_v<typename transformer_t::is_transformer, std::true_type>;
}// namespace flow
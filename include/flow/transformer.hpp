#pragma once

namespace flow {

template<typename T>
class transformer;

template<typename return_t, typename... args_t>
class transformer<return_t(args_t...)> {
public:

  transformer() = default;
  ~transformer() = default;

  transformer(transformer&&) noexcept = default;
  transformer(transformer const&) = default;
  transformer& operator=(transformer&&) noexcept = default;
  transformer& operator=(transformer const&) = default;

  transformer(flow::transformer_routine auto&& callback, std::string producer_channel_name, std::string consumer_channel_name)
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


auto make_transformer(flow::transformer_routine auto&& callback, std::string producer_channel_name = "", std::string consumer_channel_name = "")
{
  using callback_t = decltype(callback);
  using argument_t = std::decay_t<typename flow::metaprogramming::function_traits<callback_t>::template args<0>::type>;
  using return_t = std::decay_t<typename flow::metaprogramming::function_traits<callback_t>::return_type>;

  return transformer<return_t(argument_t)>(std::forward<callback_t>(callback), std::move(producer_channel_name), std::move(consumer_channel_name));
}
}// namespace flow
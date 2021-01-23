#pragma once

#include "flow/detail/cancellable_function.hpp"
#include "flow/detail/metaprogramming.hpp"
#include "flow/options.hpp"
#include "flow/routine_concepts.hpp"
#include <string>

namespace flow {

template<typename message_t>
class consumer {
public:
  using is_consumer = std::true_type;
  using is_routine = std::true_type;

  consumer() = default;
  ~consumer() = default;

  consumer(consumer&&) noexcept = default;
  consumer(consumer const&) = default;
  consumer& operator=(consumer&&) noexcept = default;
  consumer& operator=(consumer const&) = default;

  consumer(flow::callable_consumer auto&& callback, std::string channel_name)
    : m_callback(detail::make_cancellable_function(std::forward<decltype(callback)>(callback))),
      m_channel_name(std::move(channel_name)) {}

  auto channel_name() { return m_channel_name; }
  auto& callback() { return *m_callback; }

private:
  typename detail::cancellable_function<void(message_t&&)>::sPtr m_callback{ nullptr };
  std::string m_channel_name{};
};

template<typename argument_t>
auto make_consumer(std::function<void(argument_t&&)>&& callback, flow::options options = flow::options{})
{
  using callback_t = decltype(callback);
  return consumer<argument_t>(std::forward<callback_t>(callback), std::move(options.subscribe_to));
}

template<typename argument_t>
auto make_consumer(void (*callback)(argument_t&&), flow::options options = flow::options{})
{
  using callback_t = decltype(callback);
  return consumer<argument_t>(std::forward<callback_t>(callback), std::move(options.subscribe_to));
}

auto make_consumer(auto&& lambda, flow::options options= flow::options{})
{
  using callback_t = decltype(lambda);
  return make_consumer(flow::metaprogramming::to_function(std::forward<callback_t>(lambda)), std::move(options.subscribe_to));
}

template<typename argument_t>
auto make_consumer(std::function<void(argument_t&&)>&& callback, std::string channel_name)
{
  using callback_t = decltype(callback);
  return consumer<argument_t>(std::forward<callback_t>(callback), std::move(channel_name));
}

template<typename argument_t>
auto make_consumer(void (*callback)(argument_t&&), std::string channel_name)
{
  using callback_t = decltype(callback);
  return consumer<argument_t>(std::forward<callback_t>(callback), std::move(channel_name));
}

auto make_consumer(auto&& lambda, std::string channel_name)
{
  using callback_t = decltype(lambda);
  return make_consumer(flow::metaprogramming::to_function(std::forward<callback_t>(lambda)), std::move(channel_name));
}
template<typename consumer_t>
concept consumer_concept = std::is_same_v<typename consumer_t::is_consumer, std::true_type>;
}// namespace flow
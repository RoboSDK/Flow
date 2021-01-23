#pragma once

#include "flow/detail/cancellable_function.hpp"
#include "flow/detail/metaprogramming.hpp"
#include "flow/options.hpp"
#include "flow/routine_concepts.hpp"
#include <string>

namespace flow {

struct consumer {};

namespace detail {
  template<typename message_t>
  class consumer_impl {
  public:
    using is_consumer = std::true_type;
    using is_routine = std::true_type;

    consumer_impl() = default;
    ~consumer_impl() = default;

    consumer_impl(consumer_impl&&) noexcept = default;
    consumer_impl(consumer_impl const&) = default;
    consumer_impl& operator=(consumer_impl&&) noexcept = default;
    consumer_impl& operator=(consumer_impl const&) = default;

    consumer_impl(flow::consumer_function auto&& callback, std::string channel_name)
      : m_callback(detail::make_shared_cancellable_function(std::forward<decltype(callback)>(callback))),
        m_channel_name(std::move(channel_name)) {}

    auto channel_name() { return m_channel_name; }
    auto& callback() { return *m_callback; }

  private:
    using function_ptr = typename detail::cancellable_function<void(message_t&&)>::sPtr;

    function_ptr m_callback{ nullptr };
    std::string m_channel_name{};
  };
}// namespace detail

template<typename argument_t>
auto make_consumer(std::function<void(argument_t&&)>&& callback, flow::options options = flow::options{})
{
  using callback_t = decltype(callback);
  return detail::consumer_impl<argument_t>(std::forward<callback_t>(callback), std::move(options.subscribe_to));
}

template<typename argument_t>
auto make_consumer(void (*callback)(argument_t&&), flow::options options = flow::options{})
{
  using callback_t = decltype(callback);
  return detail::consumer_impl<argument_t>(std::forward<callback_t>(callback), std::move(options.subscribe_to));
}

auto make_consumer(auto&& lambda, flow::options options = flow::options{})
{
  using callback_t = decltype(lambda);
  return make_consumer(detail::metaprogramming::to_function(std::forward<callback_t>(lambda)), std::move(options.subscribe_to));
}

template<typename argument_t>
auto make_consumer(std::function<void(argument_t&&)>&& callback, std::string channel_name)
{
  using callback_t = decltype(callback);
  return detail::consumer_impl<argument_t>(std::forward<callback_t>(callback), std::move(channel_name));
}

template<typename argument_t>
auto make_consumer(void (*callback)(argument_t&&), std::string channel_name)
{
  using callback_t = decltype(callback);
  return detail::consumer_impl<argument_t>(std::forward<callback_t>(callback), std::move(channel_name));
}

auto make_consumer(auto&& lambda, std::string channel_name)
{
  using callback_t = decltype(lambda);
  return make_consumer(detail::metaprogramming::to_function(std::forward<callback_t>(lambda)), std::move(channel_name));
}
template<typename consumer_t>
concept consumer_routine = std::is_same_v<typename consumer_t::is_consumer, std::true_type> or std::is_same_v<consumer_t, flow::consumer>;
}// namespace flow
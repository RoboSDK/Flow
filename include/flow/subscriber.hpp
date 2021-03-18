#pragma once

#include "flow/concepts.hpp"
#include "flow/detail/cancellable_function.hpp"
#include "flow/detail/metaprogramming.hpp"
#include <string>

namespace flow {
namespace detail {
  template<typename message_t>
  class subscriber_impl;
}

/**
 * Create a subscriber to be used by a flow::network
 *
 * These objects created are passed in to the network to spin up the routines
 * @tparam argument_t The subscriber tag
 * @param callback A subscriber function
 * @param channel_name The channel to subscribe to
 * @return A subscriber object used to retrieve data by the network
 */
template<typename argument_t>
auto subscriber(std::function<void(argument_t&&)>&& callback, std::string channel_name)
{
  using callback_t = decltype(callback);
  return detail::subscriber_impl<argument_t>(std::forward<callback_t>(callback), std::move(channel_name));
}

template<typename argument_t>
auto subscriber(void (*callback)(argument_t&&), std::string channel_name)
{
  //  return detail::subscriber_impl<argument_t>([callback=std::move(callback)](argument_t&& msg) { return callback(std::move(msg)); }, std::move(channel_name));
  return detail::subscriber_impl<argument_t>(std::move(callback), std::move(channel_name));
}

auto subscriber(auto&& lambda, std::string channel_name)
{
  using callback_t = decltype(lambda);
  return subscriber(detail::metaprogramming::to_function(std::forward<callback_t>(lambda)), std::move(channel_name));
}

namespace detail {
  template<typename message_t>
  class subscriber_impl {
  public:
    using is_subscriber = std::true_type;
    using is_routine = std::true_type;

    subscriber_impl() = default;
    ~subscriber_impl() = default;

    subscriber_impl(subscriber_impl&&) noexcept = default;
    subscriber_impl(subscriber_impl const&) = default;
    subscriber_impl& operator=(subscriber_impl&&) noexcept = default;
    subscriber_impl& operator=(subscriber_impl const&) = default;

    subscriber_impl(flow::is_subscriber_function auto&& callback, std::string channel_name)
      : m_callback(detail::make_shared_cancellable_function(std::forward<decltype(callback)>(callback))),
        m_channel_name(std::move(channel_name)) {}

    auto subscribing_to() { return m_channel_name; }
    auto& callback() { return *m_callback; }

  private:
    using function_ptr = typename detail::cancellable_function<void(message_t&&)>::sPtr;

    function_ptr m_callback{ nullptr };
    std::string m_channel_name{};
  };
}// namespace detail
}// namespace flow
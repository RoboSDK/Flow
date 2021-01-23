#pragma once

#include "flow/options.hpp"

namespace flow {

/**
 * Tag to dispatch to the make_producer function from make_routine<routine_t>
 */
struct producer {};

namespace detail {
  template<typename message_t>
  class producer_impl {
  public:
    using is_producer = std::true_type;
    using is_routine = std::true_type;

    producer_impl() = default;
    ~producer_impl() = default;

    producer_impl(producer_impl&&) noexcept = default;
    producer_impl(producer_impl const&) = default;
    producer_impl& operator=(producer_impl&&) noexcept = default;
    producer_impl& operator=(producer_impl const&) = default;

    producer_impl(flow::producer_function auto&& callback, std::string channel_name)
      : m_callback(detail::make_shared_cancellable_function(std::forward<decltype(callback)>(callback))),
        m_channel_name(std::move(channel_name)) {}

    auto channel_name() { return m_channel_name; }
    auto& callback() { return *m_callback; }

  private:
    using function_ptr = typename detail::cancellable_function<message_t()>::sPtr;

    function_ptr m_callback{ nullptr };
    std::string m_channel_name{};
  };
}// namespace detail

template<typename return_t>
auto make_producer(std::function<return_t()>&& callback, flow::options options = flow::options{})
{
  using callback_t = decltype(callback);
  return detail::producer_impl<return_t>(std::forward<callback_t>(callback), std::move(options.publish_to));
}

template<typename return_t>
auto make_producer(return_t (*callback)(), flow::options options = flow::options{})
{
  using callback_t = decltype(callback);
  return detail::producer_impl<return_t>(std::forward<callback_t>(callback), std::move(options.publish_to));
}

auto make_producer(auto&& lambda, flow::options&& options = flow::options{})
{
  using callback_t = decltype(lambda);
  return make_producer(detail::metaprogramming::to_function(std::forward<callback_t>(lambda)), std::move(options.publish_to));
}

template<typename return_t>
auto make_producer(std::function<return_t()>&& callback, std::string channel_name)
{
  using callback_t = decltype(callback);
  return detail::producer_impl<return_t>(std::forward<callback_t>(callback), std::move(channel_name));
}

template<typename return_t>
auto make_producer(return_t (*callback)(), std::string channel_name)
{
  using callback_t = decltype(callback);
  return detail::producer_impl<return_t>(std::forward<callback_t>(callback), std::move(channel_name));
}

auto make_producer(auto&& lambda, std::string channel_name)
{
  using callback_t = decltype(lambda);
  return make_producer(detail::metaprogramming::to_function(std::forward<callback_t>(lambda)), std::move(channel_name));
}

template<typename producer_t>
concept producer_routine = std::is_same_v<typename producer_t::is_producer, std::true_type> or std::is_same_v<producer_t, flow::producer>;

}// namespace flow
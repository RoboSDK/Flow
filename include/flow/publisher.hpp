#pragma once

namespace flow {

namespace detail {
  template<typename message_t>
  class publisher_impl;
}

/**
 * Create a publisher from a callback and channel name to publish on
 *
 * These objects created are passed in to the network to spin up the routines
 *
 * @tparam argument_t The publisher tag
 * @param callback A publisher function
 * @param publish_to The channel to publish to
 * @return A publisher object used to retrieve data by the network
 */
template<typename return_t>
auto publisher(std::function<return_t()>&& callback, std::string publish_to)
{
  using callback_t = decltype(callback);
  return detail::publisher_impl<return_t>(std::forward<callback_t>(callback), std::move(publish_to));
}

template<typename return_t>
auto publisher(return_t (*callback)(), std::string publish_to)
{
  return detail::publisher_impl<return_t>([callback = std::move(callback)]() -> return_t { return callback(); }, std::move(publish_to));
}

auto publisher(auto&& lambda, std::string publish_to)
{
  using callback_t = decltype(lambda);
  return publisher(detail::metaprogramming::to_function(std::forward<callback_t>(lambda)), std::move(publish_to));
}


namespace detail {
  template<typename message_t>
  class publisher_impl {
  public:
    using is_publisher = std::true_type;
    using is_routine = std::true_type;

    publisher_impl() = default;
    ~publisher_impl() = default;

    publisher_impl(publisher_impl&&) noexcept = default;
    publisher_impl(publisher_impl const&) = default;
    publisher_impl& operator=(publisher_impl&&) noexcept = default;
    publisher_impl& operator=(publisher_impl const&) = default;

    publisher_impl(flow::is_publisher_function auto&& callback, std::string channel_name)
      : m_callback(detail::make_shared_cancellable_function(std::forward<decltype(callback)>(callback))),
        m_channel_name(std::move(channel_name)) {}

    auto publish_to() { return m_channel_name; }
    auto& callback() { return *m_callback; }

  private:
    using function_ptr = typename detail::cancellable_function<message_t()>::sPtr;

    function_ptr m_callback{ nullptr };
    std::string m_channel_name{};
  };
}// namespace detail
}// namespace flow
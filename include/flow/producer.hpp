#pragma once

namespace flow {

namespace detail {
  template<typename message_t>
  class producer_impl;
}

/**
 * May be called directly instead of make_routine<flow::producer>(args);
 *
 * These objects created are passed in to the network to spin up the routines
 *
 * @tparam argument_t The producer tag
 * @param callback A producer function
 * @param publish_to The channel to publish to
 * @return A producer object used to retrieve data by the network
 */
template<typename return_t>
auto make_producer(std::function<return_t()>&& callback, std::string publish_to)
{
  using callback_t = decltype(callback);
  return detail::producer_impl<return_t>(std::forward<callback_t>(callback), std::move(publish_to));
}

template<typename return_t>
auto make_producer(return_t (*callback)(), std::string publish_to)
{
  return detail::producer_impl<return_t>([callback = std::move(callback)]() -> return_t { return callback(); }, std::move(publish_to));
}

auto make_producer(auto&& lambda, std::string publish_to)
{
  using callback_t = decltype(lambda);
  return make_producer(detail::metaprogramming::to_function(std::forward<callback_t>(lambda)), std::move(publish_to));
}


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

    producer_impl(flow::is_producer_function auto&& callback, std::string channel_name)
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
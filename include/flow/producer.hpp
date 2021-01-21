#pragma once

namespace flow {

template<typename T>
class producer {
public:
  using message_t = T;

  producer() = default;
  ~producer() = default;

  producer(producer&&) noexcept = default;
  producer(producer const&) = default;
  producer& operator=(producer&&) noexcept = default;
  producer& operator=(producer const&) = default;

  producer(flow::producer_routine auto&& callback, std::string channel_name)
    : m_callback(flow::make_cancellable_function(std::forward<decltype(callback)>(callback))),
      m_channel_name(std::move(channel_name)) {}

  auto channel_name() { return m_channel_name; }
  auto& callback() { return *m_callback; }

private:
  typename flow::cancellable_function<message_t()>::sPtr m_callback{ nullptr };
  std::string m_channel_name{};
};

auto make_producer(flow::producer_routine auto&& callback, std::string channel_name = "")
{
  using callback_t = decltype(callback);
  using return_t = std::decay_t<typename flow::metaprogramming::function_traits<callback_t>::return_type>;
  return producer<return_t>(std::forward<callback_t>(callback), std::move(channel_name));
}

}// namespace flow
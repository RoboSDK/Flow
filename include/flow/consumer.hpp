#pragma once

#include <string>
#include "flow/metaprogramming.hpp"
#include "flow/cancellable_function.hpp"
#include "flow/routine_concepts.hpp"

namespace flow {

template<typename message_t>
class consumer {
public:

  consumer() = default;
  ~consumer() = default;

  consumer(consumer&&) noexcept = default;
  consumer(consumer const&) = default;
  consumer& operator=(consumer&&) noexcept = default;
  consumer& operator=(consumer const&) = default;

  consumer(flow::consumer_routine auto&& callback, std::string channel_name)
    : m_callback(flow::make_cancellable_function(std::forward<decltype(callback)>(callback))),
      m_channel_name(std::move(channel_name)) {}

  auto channel_name() { return m_channel_name; }
  auto& callback() { return *m_callback; }

private:
  typename flow::cancellable_function<void(message_t&&)>::sPtr m_callback{ nullptr };
  std::string m_channel_name{};
};

auto make_consumer(flow::consumer_routine auto&& callback, std::string channel_name = "'")
{
  using callback_t = decltype(callback);

  // TODO: handle multiple arguments
  using argument_t = std::decay_t<typename flow::metaprogramming::function_traits<callback_t>::template args<0>::type>;

  return consumer<argument_t>(std::forward<callback_t>(callback), std::move(channel_name));
}
}// namespace flow
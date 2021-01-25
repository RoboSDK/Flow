#pragma once

namespace flow {

namespace detail {
class spinner_impl {
public:
  using is_spinner = std::true_type;
  using is_routine = std::true_type;

  spinner_impl() = default;
  ~spinner_impl() = default;

  spinner_impl(spinner_impl&&) noexcept = default;
  spinner_impl(spinner_impl const&) = default;
  spinner_impl& operator=(spinner_impl&&) noexcept = default;
  spinner_impl& operator=(spinner_impl const&) = default;

  spinner_impl(auto&& callback)
    : m_callback(detail::make_shared_cancellable_function(std::forward<decltype(callback)>(callback))) {}

  auto& callback() { return *m_callback; }

private:
  detail::cancellable_function<void()>::sPtr m_callback{ nullptr };
};
}

auto make_spinner(std::function<void()>&& callback)
{
  using callback_t = decltype(callback);
  return detail::spinner_impl{std::forward<callback_t>(callback)};
}

auto make_spinner(void (*callback)())
{
  using callback_t = decltype(callback);
  return detail::spinner_impl{std::forward<callback_t>(callback)};
}

auto make_spinner(auto&& lambda)
{
  using callback_t = decltype(lambda);
  return make_spinner(detail::metaprogramming::to_function(std::forward<callback_t>(lambda)));
}
}// namespace flow
#pragma once

namespace flow {
class spinner {
public:
  spinner() = default;
  ~spinner() = default;

  spinner(spinner&&) noexcept = default;
  spinner(spinner const&) = default;
  spinner& operator=(spinner&&) noexcept = default;
  spinner& operator=(spinner const&) = default;

  spinner(flow::spinner_routine auto&& callback)
    : m_callback(flow::make_cancellable_function(std::forward<decltype(callback)>(callback))) {}

  auto& callback() { return *m_callback; }

private:
  flow::cancellable_function<void()>::sPtr m_callback{ nullptr };
};

spinner make_spinner(flow::spinner_routine auto&& callback)
{
  return spinner{ std::forward<decltype(callback)>(callback) };
}

}// namespace flow
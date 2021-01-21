#pragma once

namespace flow {
class spinner {
public:
  using is_spinner = std::true_type;

  spinner() = default;
  ~spinner() = default;

  spinner(spinner&&) noexcept = default;
  spinner(spinner const&) = default;
  spinner& operator=(spinner&&) noexcept = default;
  spinner& operator=(spinner const&) = default;

  spinner(flow::callable_spinner auto&& callback)
    : m_callback(flow::make_cancellable_function(std::forward<decltype(callback)>(callback))) {}

  auto& callback() { return *m_callback; }

private:
  flow::cancellable_function<void()>::sPtr m_callback{ nullptr };
};

spinner make_spinner(flow::callable_spinner auto&& callback)
{
  return spinner{ std::forward<decltype(callback)>(callback) };
}

template<typename spinner_t>
concept spinner_concept = std::is_same_v<typename spinner_t::is_spinner, std::true_type>;
}// namespace flow
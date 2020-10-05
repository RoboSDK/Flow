#include <cppcoro/cancellation_registration.hpp>
#include <cppcoro/cancellation_source.hpp>
#include <cppcoro/cancellation_token.hpp>
#include <cppcoro/operation_cancelled.hpp>

#include <iostream>

namespace flow {
class cancellation_handle {
public:
  void request_cancellation()
  {
    m_cancel_source.request_cancellation();
  }

  cppcoro::cancellation_token token() {
    return m_cancel_source.token();
  }

private:
  cppcoro::cancellation_source m_cancel_source;
};

template<typename R, typename... Args>
struct cancellable_callback {
public:
  using callback_t = std::function<R(Args...)>;
  cancellable_callback(cppcoro::cancellation_token token, callback_t && callback)
    : m_cancel_token(token), m_current_callback(std::move(callback))
  {}

  R operator()(Args&&... args)
  {
    if (m_cancel_token.is_cancellation_requested() != m_change_callbacks) {
      m_change_callbacks = m_cancel_token.is_cancellation_requested();
      std::swap(m_current_callback, m_next_callback);
    }

    return m_current_callback(std::forward<Args>(args)...);
  }

  void throw_if_cancellation_requested () {
    m_cancel_token.throw_if_cancellation_requested();
  }

  bool is_cancellation_requested() {
    return m_cancel_token.is_cancellation_requested();
  }


private:
  bool m_change_callbacks{false};

  cppcoro::cancellation_token m_cancel_token;
  callback_t m_current_callback;
  callback_t m_next_callback = [](Args... /*unused*/) -> R {};
};
}// namespace flow
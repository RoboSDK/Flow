#include <cppcoro/cancellation_registration.hpp>
#include <cppcoro/cancellation_source.hpp>
#include <cppcoro/cancellation_token.hpp>
#include <cppcoro/operation_cancelled.hpp>

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
  cancellable_callback(cppcoro::cancellation_token token, std::function<R(Args...)>&& callback)
    : m_cancel_token(token), m_callback(std::move(callback))
  {}

  R operator()(Args... args)
  {
    return m_callback(std::forward<Args>(args)...);
  }

  void throw_if_cancellation_requested () {
    m_cancel_token.throw_if_cancellation_requested();
  }

  bool is_cancellation_requested() {
    return m_cancel_token.is_cancellation_requested();
  }


private:
  cppcoro::cancellation_token m_cancel_token;
  std::function<R(Args...)> m_callback;
};
}// namespace flow
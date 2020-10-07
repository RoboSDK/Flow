#include <cppcoro/cancellation_registration.hpp>
#include <cppcoro/cancellation_source.hpp>
#include <cppcoro/cancellation_token.hpp>
#include <cppcoro/operation_cancelled.hpp>

#include <iostream>

/**
 * This module is dedicated to the cancellation_handle and cancellation_callback.
 *
 * For every publisher or subscriber there exists a cancellation handle and callback pair.
 *
 * The callback handle (which owns a cancellable handle) is given to the task that subscriber
 * or publishers to the channel, which gives control of the callback to the task owner.
 *
 * The cancellable callback is linked to this callback handle, which may be disabled by the callback handle.
 */

namespace flow {

/**
 * Allows the owner of this handle to cancel a callback asynchronously
 */
class cancellation_handle {
public:
  void request_cancellation()
  {
    m_cancel_source.request_cancellation();
  }

  cppcoro::cancellation_token token()
  {
    return m_cancel_source.token();
  }

private:
  cppcoro::cancellation_source m_cancel_source;
};

/**
 * Takes in a function and manually passed parameters, along with a cancellation token that allows the callback
 * to be cancelled by the owner of the cancellation handle.
 * @tparam R The return type of the function
 * @tparam Args The arguments of the function
 */
template<typename T>
class cancellable_function;

template<typename return_t, typename... args_t>
class cancellable_function<return_t(args_t...)> {
public:
  using callback_t = std::function<return_t(args_t...)>;
  cancellable_function(cppcoro::cancellation_token token, callback_t&& callback)
    : m_cancel_token(token), m_current_callback(std::move(callback))
  {
  }

  return_t operator()(args_t&&... args)
  {
    if (m_cancel_token.is_cancellation_requested() != m_change_callbacks) {
      m_change_callbacks = m_cancel_token.is_cancellation_requested();
      std::swap(m_current_callback, m_next_callback);
    }

    return m_current_callback(std::forward<args_t>(args)...);
  }

  void throw_if_cancellation_requested()
  {
    m_cancel_token.throw_if_cancellation_requested();
  }

  bool is_cancellation_requested()
  {
    return m_cancel_token.is_cancellation_requested();
  }


private:
  bool m_change_callbacks{ false };

  cppcoro::cancellation_token m_cancel_token;
  callback_t m_current_callback;
  callback_t m_next_callback = [](args_t... /*unused*/) -> return_t {};
};
}// namespace flow
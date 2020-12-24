#pragma once

#include <cassert>
#include <iostream>

#include <cppcoro/cancellation_registration.hpp>
#include <cppcoro/cancellation_source.hpp>
#include <cppcoro/cancellation_token.hpp>
#include <cppcoro/operation_cancelled.hpp>

#include "flow/metaprogramming.hpp"
#include "logging.hpp"

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
  cancellation_handle(cppcoro::cancellation_source* cancel_source) : m_cancel_source(cancel_source) {}

  void request_cancellation()
  {
    m_cancel_source->request_cancellation();
  }

  cppcoro::cancellation_token token()
  {
    return m_cancel_source->token();
  }

private:
  cppcoro::cancellation_source* m_cancel_source{ nullptr };
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
  using function_ptr_t = return_t (*)(args_t...);
  using is_cancellable = std::true_type;

  cancellable_function(callback_t&& callback): m_callback(std::move(callback)) {}
  cancellable_function(function_ptr_t callback) : m_callback(callback) {}

  return_t operator()(args_t&&... args)
  {
    return m_callback(std::forward<args_t>(args)...);
  }

  void throw_if_cancellation_requested()
  {
    m_cancel_token.throw_if_cancellation_requested();
  }

  bool is_cancellation_requested()
  {
    return m_cancel_token.is_cancellation_requested();
  }

  cancellation_handle handle()
  {
    return cancellation_handle{&m_cancellation_source};
  }


private:
  cppcoro::cancellation_source m_cancellation_source{};
  cppcoro::cancellation_token m_cancel_token{m_cancellation_source.token()};

  callback_t m_callback;
};

/**
 * Return a pair of a cancellation handle and function
 *
 * The cancellation handle will control the function and may cancel it
 * @tparam return_t The return type of the callback
 * @tparam args_t The arguments for the callback
 * @param callback The callback itself
 * @return A function handle and callback pair
 */
template<typename return_t, typename... args_t>
auto make_cancellable_function(std::function<return_t(args_t...)>&& callback)
{
  using cancellable_function_t = cancellable_function<return_t(args_t...)>;
  return std::make_shared<cancellable_function_t>(std::forward<decltype(callback)>(callback));
}

template<typename return_t, typename... args_t>
auto make_cancellable_function(return_t (*callback)(args_t...))
{
  using cancellable_function_t = cancellable_function<return_t(args_t...)>;
  return std::make_unique<cancellable_function_t>(std::forward<decltype(callback)>(callback));
}

auto make_cancellable_function(auto&& lambda)
{
  return make_cancellable_function(flow::metaprogramming::to_function(lambda));
}

template<typename T>
concept cancellable_callback = requires
{
  typename T::is_cancellable;
};


}// namespace flow
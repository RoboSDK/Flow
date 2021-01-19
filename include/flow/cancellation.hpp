#pragma once

#include <iostream>

#include <cppcoro/cancellation_source.hpp>
#include <cppcoro/cancellation_token.hpp>
#include <cppcoro/operation_cancelled.hpp>

#include "flow/metaprogramming.hpp"
#include "logging.hpp"

/**
 * This module is dedicated to the cancellation_handle and cancellable_function.
 *
 * Any std::function, function pointer, or lambda may be used to make a cancellable_function
 *
 * A cancellable function can generate a cancellation_handle via the handle() method. The handle is
 * the object whereby cancellation is transmitted to the cancellable_function.
 *
 * The nominal use case is as follows:
 *   void do_foo() {  // do foo here }
 *
 *   auto cancellable = flow::make_cancellable_function(do_foo);
 *   auto handle = cancellable.handle(); // may be copied
 *
 *   while (not cancellable.is_cancellation_requested()) { // do work }
 *
 * These cancellable functions are used by the chain data structure to cancel the consumer
 * at the end of the chain. This triggers the full cancellation of the chain itself.
 */

namespace flow {

/**
 * Allows the owner of this handle to cancel a callback asynchronously
 */
class cancellation_handle {
public:
  cancellation_handle() = default;
  ~cancellation_handle() = default;

  cancellation_handle(cancellation_handle&&) noexcept = default;
  cancellation_handle(cancellation_handle const&) = default;
  cancellation_handle& operator=(cancellation_handle&&) noexcept = default;
  cancellation_handle& operator=(cancellation_handle const&) = default;

  explicit cancellation_handle(cppcoro::cancellation_source* cancel_source) : m_cancel_source(cancel_source)
  {
  }

  void request_cancellation()
  {
    m_cancel_source->request_cancellation();
  }

private:
  cppcoro::cancellation_source* m_cancel_source{ nullptr };
};

/**
 * Created from another function function and may be cancelled
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

  [[maybe_unused]] explicit cancellable_function(callback_t&& callback) : m_callback(std::move(callback)) {}
  [[maybe_unused]] explicit cancellable_function(function_ptr_t callback) : m_callback(callback) {}

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
    return cancellation_handle{ &m_cancellation_source};
  }

private:
  cppcoro::cancellation_source m_cancellation_source{};
  cppcoro::cancellation_token m_cancel_token{ m_cancellation_source.token() };

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
  return std::make_shared<cancellable_function_t>(std::forward<decltype(callback)>(callback));
}

auto make_cancellable_function(auto&& lambda)
{
  return make_cancellable_function(flow::metaprogramming::to_function(lambda));
}

}// namespace flow
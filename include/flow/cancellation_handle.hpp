#pragma once

#include <iostream>

#include <cppcoro/cancellation_source.hpp>
#include <cppcoro/cancellation_token.hpp>
#include <cppcoro/operation_cancelled.hpp>

#include "flow/metaprogramming.hpp"
#include "logging.hpp"

/**
 * This module is dedicated to the cancellation_handle
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
 * These cancellable functions are used by the network data structure to cancel the callable_consumer
 * at the end of the network. This triggers the full cancellation of the network itself.
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
}// namespace flow
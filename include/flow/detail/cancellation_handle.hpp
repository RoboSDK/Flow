#pragma once

#include <cppcoro/cancellation_source.hpp>
#include <cppcoro/cancellation_token.hpp>

#include "metaprogramming.hpp"

/**
 * A cancellation_handle is a handle to a cancellable_function. This allows
 * the owner of the handle to cancel the function being called at any time.
 *
 * Note: This does not prevent the owner of cancellable_function from being called,
 * but it does notify the user of that function that they should cancel if they are
 * calling that function within a loop.
 */

namespace flow::detail {

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
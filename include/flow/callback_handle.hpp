#pragma once

#include "flow/cancellation.hpp"

namespace flow {
class callback_handle {
public:
  callback_handle(cancellation_handle&& ch) : m_cancel_handle(std::move(ch)) {}

  void disable()
  {
    m_cancel_handle.request_cancellation();
  }

private:
  cancellation_handle m_cancel_handle;
};
}// namespace flow
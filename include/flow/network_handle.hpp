#pragma once

#include "flow/detail/cancellation_handle.hpp"

namespace flow {
class network_handle {
public:
  void push(cancellation_handle&& handle) {
    m_handles.push_back(handle);
  }

  void request_cancellation()
  {
    for (auto& handle : m_handles) handle.request_cancellation();
  }

private:
  std::vector<cancellation_handle> m_handles{};
};
}


#pragma once

#include "flow/detail/cancellation_handle.hpp"

/**
 * The network handle is used to request cancellation of the network by the owner of the handle
 */

namespace flow {
class network_handle {
public:
  void push(detail::cancellation_handle&& handle) {
    m_handles.push_back(handle);
  }

  /**
   * Requests cancellation of the network routines and will begin cancellation
   */
  void request_cancellation()
  {
    for (auto& handle : m_handles) handle.request_cancellation();
  }

private:
  std::vector<detail::cancellation_handle> m_handles{};
};
}


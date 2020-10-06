#pragma once

#include "flow/cancellation.hpp"

/**
 * A callback handle is a handle that has various controls up the communication heirarchy.
 *
 * It is given back when subscribing or publishing to a channel.
 */

namespace flow {
class callback_handle {
public:
  callback_handle() = default;
  callback_handle(callback_handle &&) = default;
  callback_handle(callback_handle const&) = default;
  callback_handle& operator=(callback_handle &&) = default;
  callback_handle& operator=(callback_handle const&) = default;

  callback_handle(cancellation_handle&& ch, volatile std::atomic_bool* program_is_running) : m_cancel_handle(std::move(ch)), m_program_is_running(program_is_running)
  {
  }

  /**
   * Disables the callback (it becomes a noop)
   */
  void disable()
  {
    m_cancel_handle.request_cancellation();
  }

  /**
   * Stops all communication between channels and ends the program
   */
  void stop_everything()
  {
    m_program_is_running->exchange(false);
  }

private:
  cancellation_handle m_cancel_handle;
  volatile std::atomic_bool* m_program_is_running{nullptr};
};
}// namespace flow
#pragma once

#include <chrono>
#include <thread>

#include <cppcoro/on_scope_exit.hpp>
#include <cppcoro/task.hpp>

#include "flow/detail/metaprogramming.hpp"
#include "flow/detail/spin_wait.hpp"

/**
 * Routine that will run until the specified time limit and call the passed in callback
 */

namespace flow::detail {
class timeout_routine {
public:
  using callback_t = std::function<void()>;
  using function_ptr_t = void (*)();

  [[maybe_unused]] timeout_routine(std::chrono::nanoseconds time_limit, callback_t&& callback)
    : m_callback(std::move(callback)),
      m_time_limit(time_limit)
  {
  }

  [[maybe_unused]] timeout_routine(std::chrono::nanoseconds time_limit, function_ptr_t callback)
    : m_callback(std::move(callback)),
      m_time_limit(time_limit)
  {
  }

  [[maybe_unused]] cppcoro::task<void> spin()
  {
    auto cancel_on_exit = cppcoro::on_scope_exit([&] {
      m_callback();
    });

    spin_wait waiter{ "timeout_routine", m_time_limit };

    while (not waiter.is_ready()) {
      std::this_thread::yield();
    }
  }

private:
  callback_t m_callback;
  std::chrono::nanoseconds m_time_limit;
};
}// namespace flow::detail

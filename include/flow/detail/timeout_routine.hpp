#pragma once

#include <chrono>
#include <thread>

#include <cppcoro/on_scope_exit.hpp>
#include <cppcoro/task.hpp>

#include "metaprogramming.hpp"

/**
 * Routine that will run until the specified time limit and call the passed in callback
 */

namespace flow::detail {
class timeout_routine {
public:
  using sPtr = std::shared_ptr<timeout_routine>;
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
    using namespace std::chrono;
    using namespace std::chrono_literals;

    m_last_timestamp = steady_clock::now();

    auto cancel_on_exit = cppcoro::on_scope_exit([&] {
      m_callback();
    });

    while (m_time_elapsed < m_time_limit) {
      auto new_timestamp = std::chrono::steady_clock::now();
      auto time_delta = new_timestamp - m_last_timestamp;
      m_last_timestamp = new_timestamp;

      m_time_elapsed += duration_cast<nanoseconds>(time_delta);
      std::this_thread::yield();
    }
    co_return;
  }

private:
  decltype(std::chrono::steady_clock::now()) m_last_timestamp{};
  callback_t m_callback;

  std::chrono::nanoseconds m_time_limit;
  std::chrono::nanoseconds m_time_elapsed{ 0 };
};
}// namespace flow

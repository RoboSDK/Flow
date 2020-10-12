#pragma once

#include <cppcoro/task.hpp>

#include <flow/cancellation.hpp>

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>

/**
 * An object that will call the function passed in after a specified time
 */

namespace flow {
class timeout_function {
public:
  using callback_t = flow::cancellable_function<void()>;

  timeout_function(std::chrono::nanoseconds time_limit, callback_t&& callback, uint16_t times_to_poll)
    : m_time_limit(time_limit),
      m_sleep_duration(time_limit/times_to_poll),
      m_callback(std::move(callback)) {}

  /**
   * @return true if it timed out
   */
  cppcoro::task<bool> operator()()
  {
    const bool timed_out = co_await countdown();
    if (timed_out) {
      m_callback();
    }

    co_return timed_out;
  }

private:
  cppcoro::task<bool> countdown()
  {
    using namespace std::chrono_literals;
    using namespace std::chrono;

    const auto start = steady_clock::now();
    while (not m_callback.is_cancellation_requested()) {
      co_await sleep();
      auto time_elapsed = steady_clock::now() - start;
      const bool timed_out = time_elapsed >= m_time_limit;
      if (timed_out) {
        co_return true;
      }
    }

    co_return false;
  }

  cppcoro::task<void> sleep()
  {
    std::this_thread::sleep_for(m_sleep_duration);
    co_return;
  }

  std::chrono::nanoseconds m_time_limit;
  std::chrono::nanoseconds m_sleep_duration;
  callback_t m_callback;
};

auto make_timeout_function(std::chrono::nanoseconds limit, std::function<void()>&& callback, uint16_t times_to_poll = 10)
{
  auto [handle, cancellable] = flow::make_cancellable_function(std::move(callback));
  return std::make_pair(handle, timeout_function(limit, std::move(cancellable), times_to_poll));
}
}// namespace flow

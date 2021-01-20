#pragma once

#include <chrono>
#include <thread>

#include <cppcoro/on_scope_exit.hpp>
#include <cppcoro/task.hpp>

#include "flow/metaprogramming.hpp"


namespace flow {
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

  cppcoro::task<void> operator()()
  {
    using namespace std::chrono;
    using namespace std::chrono_literals;

    m_last_timestamp = steady_clock::now();

    auto cancel_on_exit = cppcoro::on_scope_exit([&] {
      m_callback();
    });

    while (m_time_elapsed < m_time_limit) {
      auto time_delta = std::chrono::steady_clock::now() - m_last_timestamp;
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

/**
 * Return a function that will expire in the specified time
 *
 * @param callback The callback itself
 * @return A function handle and callback pair
 */
auto make_timeout_function(std::chrono::nanoseconds threshold, std::function<void()>&& callback)
{
  return std::make_shared<timeout_routine>(threshold, std::forward<decltype(callback)>(callback));
}

[[maybe_unused]] auto make_timeout_function(std::chrono::nanoseconds threshold, void (*callback)())
{
  return std::make_shared<timeout_routine>(threshold, std::forward<decltype(callback)>(callback));
}

auto make_timeout_function(std::chrono::nanoseconds threshold, auto&& lambda)
{
  return make_timeout_function(threshold, flow::metaprogramming::to_function(lambda));
}
}// namespace flow

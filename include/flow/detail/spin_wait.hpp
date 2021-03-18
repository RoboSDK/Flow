#pragma once

#include <chrono>

#include <cppcoro/task.hpp>

namespace flow {
class spin_wait {
public:
  spin_wait(std::chrono::nanoseconds wait_time) : m_last_timestamp(std::chrono::steady_clock::now()), m_wait_time(wait_time) {}

  bool is_ready()
  {
    using namespace std::chrono;

    auto new_timestamp = std::chrono::steady_clock::now();
    auto time_delta = new_timestamp - m_last_timestamp;
    m_last_timestamp = new_timestamp;

    m_time_elapsed += duration_cast<nanoseconds>(time_delta);

    return m_time_elapsed >= m_wait_time;
  }

  cppcoro::task<bool> async_is_ready() {
    co_return is_ready();
  }

private:
  decltype(std::chrono::steady_clock::now()) m_last_timestamp{};

  std::chrono::nanoseconds m_wait_time{};
  std::chrono::nanoseconds m_time_elapsed{ 0 };
};
}// namespace flow
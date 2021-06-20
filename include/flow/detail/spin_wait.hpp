#pragma once

#include <chrono>

#include "flow/detail/units.hpp"
#include <cppcoro/task.hpp>

namespace flow {
class spin_wait_tag {};

template <typename spin_wait_t>
concept is_spin_wait = std::is_base_of_v<spin_wait_tag, spin_wait_t>;

struct null_spin_wait : spin_wait_tag {
  bool is_ready() { true; }
  void reset() {}

  cppcoro::task<void> async_reset() { reset(); co_return; }
  cppcoro::task<bool> async_is_ready() { co_return is_ready(); }
};

class spin_wait : spin_wait_tag {
public:
  spin_wait(const char * desc, std::chrono::nanoseconds wait_time) : m_wait_time(wait_time) {
    _desc = desc;
  }

  bool is_ready()
  {
    using namespace std::chrono;
    auto new_timestamp = high_resolution_clock::now();
    auto time_delta = new_timestamp - m_last_timestamp;
    m_last_timestamp = new_timestamp;
    m_time_elapsed += duration_cast<nanoseconds>(time_delta);
    return m_time_elapsed >= m_wait_time;
  }

  void reset()
  {
    m_time_elapsed = decltype(m_time_elapsed){ 0 };
  }

  cppcoro::task<void> async_reset()
  {
    reset();
    co_return;
  }


  cppcoro::task<bool> async_is_ready()
  {
    co_return is_ready();
  }

private:
  std::string _desc{};
  std::mutex m{};
  std::chrono::high_resolution_clock::time_point m_last_timestamp{ std::chrono::high_resolution_clock::now() };

  std::chrono::nanoseconds m_wait_time{};
  std::chrono::nanoseconds m_time_elapsed{ 0 };
};
}// namespace flow
#pragma once
#include <atomic>
#include <iostream>
#include <memory>
#include <utility>
#include <mutex>

/**
 * Tick functions are function wrappers that will trigger the callback every N ticks
 */

namespace flow {
template<typename T>
class tick_function;

template<typename R, typename... Args>
class tick_function<R(Args...)> {
  using callback_t = std::function<R(Args...)>;

public:
  tick_function() = default;
  tick_function(tick_function const& other) : m_count(other.m_count.load(std::memory_order_relaxed)),
                                              m_limit(other.m_limit.load(std::memory_order_relaxed)),
                                              m_callback(other.m_callback) {}

  tick_function(tick_function&& other) noexcept : m_count(other.m_count.load(std::memory_order_relaxed)),
                                                  m_limit(other.m_limit.load(std::memory_order_relaxed)),
                                                  m_callback(std::move(other.m_callback))
  {
    other.m_count = 0;
    other.m_limit = 0;
    other.m_callback = std::function<R(Args...)>{};
  }

  tick_function& operator=(tick_function const& other)
  {
    m_count = other.m_count.load(std::memory_order_relaxed);
    m_limit = other.m_limit.load(std::memory_order_relaxed);
    m_callback = other.m_callback;
    return *this;
  }

  tick_function& operator=(tick_function&& other) noexcept
  {
    m_count = other.m_count.load(std::memory_order_relaxed);
    m_limit = other.m_limit.load(std::memory_order_relaxed);
    m_callback = std::move(other.m_callback);

    other.m_count = 0;
    other.m_limit = 0;
    other.m_callback = std::function<R(Args...)>{};
  }

  tick_function(size_t limit, callback_t&& callback) : m_limit(limit), m_callback(callback) {}

  bool operator()()
  {
    std::size_t count = ++m_count;
    std::size_t limit = m_limit;

    if (count < limit) {
      return false;
    }

    m_callback();

    m_count.compare_exchange_strong(count, 0);
    return true;
  }

private:
  std::atomic_size_t m_count{};
  std::atomic_size_t m_limit{};

  std::mutex m_callback_mutex;
  callback_t m_callback;
};
}// namespace flow
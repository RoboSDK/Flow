#pragma once
#include <atomic>
#include <memory>

/**
 * Tick functions are function wrappers that will trigger the callback every N ticks
 */

namespace flow {
class tick_function {
  using callback_t = std::function<void()>;

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
    other.m_callback = [] {};
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
    other.m_callback = [] {};
    return *this;
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

  callback_t m_callback;
};
}// namespace flow
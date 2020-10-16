#pragma once
#include <memory>
#include <mutex>

/**
 * Tick functions are function wrappers that will trigger the callback every N ticks
 */

namespace flow {
class tick_function {
  using callback_t = std::function<void()>;

public:
  tick_function() = default;
  tick_function(tick_function const& other) : m_count(other.m_count),
                                              m_limit(other.m_limit),
                                              m_callback(other.m_callback) {}

  tick_function(tick_function&& other) noexcept : m_count(other.m_count),
                                                  m_limit(other.m_limit),
                                                  m_callback(std::move(other.m_callback))
  {
    other.m_count = 0;
    other.m_limit = 0;
    other.m_callback = [] {};
  }

  tick_function& operator=(tick_function const& other)
  {
    m_count = other.m_count;
    m_limit = other.m_limit;
    m_callback = other.m_callback;
    return *this;
  }

  tick_function& operator=(tick_function&& other) noexcept
  {
    m_count = other.m_count;
    m_limit = other.m_limit;
    m_callback = std::move(other.m_callback);

    other.m_count = 0;
    other.m_limit = 0;
    other.m_callback = [] {};
    return *this;
  }

  tick_function(size_t limit, callback_t&& callback) : m_limit(limit), m_callback(callback) {}

  bool operator()()
  {
    {
      std::lock_guard lock{m_count_mutex};
      if (++m_count < m_limit) {
        return false;
      }

      m_count = 0;

      m_callback();
    }
    return true;
  }

  std::size_t count() {
    std::lock_guard lock{m_count_mutex};
    return m_count;
  }

private:
  std::mutex m_count_mutex{};
  std::mutex m_callback_mutex{};
  std::size_t m_count{};
  std::size_t m_limit{};

  callback_t m_callback;
};
}// namespace flow
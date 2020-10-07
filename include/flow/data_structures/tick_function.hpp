#pragma once
#include <iostream>
#include <memory>
#include <utility>

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
  tick_function(tick_function const& other) = default;
  tick_function(tick_function&&) noexcept = default;
  tick_function& operator=(tick_function const&) = default;
  tick_function& operator=(tick_function&&) noexcept = default;

  tick_function(size_t limit, callback_t&& callback) : m_limit(limit), m_callback(callback) {}

  bool operator()()
  {
    ++m_count;
    m_count %= m_limit;
    if (m_count > 0) {
      return false;
    }

    m_callback();
    return true;
  }

private:
  std::size_t m_count{};
  std::size_t m_limit{};
  callback_t m_callback;
};
}// namespace flow
#pragma once
#include <utility>
#include <iostream>
#include <memory>

namespace flow {
template <typename T>
class ticking_callback;

template <typename R, typename ...Args>
class ticking_callback<R (Args...)> {
  using callback_t = std::function<R(Args...)>;
public:

  ticking_callback(size_t limit, callback_t&& callback) : m_limit(limit), m_callback(callback) {}

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
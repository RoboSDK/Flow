#pragma once
#include <utility>

namespace flow {
template<std::size_t limit_t>
class ticking_callback {
  using callback_t = std::function<void()>;
public:
  ticking_callback(callback_t&& cb) : m_callback(std::move(cb)) {}
  bool operator()()
  {
    ++m_count;
    m_count %= limit_t;
    if (m_count > 0) {
      return false;
    }

    m_callback();
    return true;
  }

private:
  std::size_t m_count = 0;
  callback_t m_callback;
};
}// namespace flow
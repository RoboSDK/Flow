#pragma once
#include <cppcoro/task.hpp>

namespace flow {
class async_id_generator {
public:
  cppcoro::task<std::size_t> operator()()
  {
    co_return m_current_id++;
  }
private:
  std::size_t m_current_id;
};
}// namespace flow
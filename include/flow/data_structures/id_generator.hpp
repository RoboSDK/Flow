#pragma once
#include <cppcoro/shared_task.hpp>
#include <atomic>

namespace flow {
class id_generator {
public:
  std::size_t operator()()
  {
    return std::atomic_ref(m_current_id)++;
  }
private:
  std::size_t m_current_id;
};
}// namespace flow
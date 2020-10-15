#pragma once

#include <atomic>
#include <concepts>

namespace flow {

template<typename T>
T atomic_read(T& t)
{
  return std::atomic_ref<T>(t);
}

template<typename T>
auto atomic_increment(T& t)
{
  return ++std::atomic_ref<T>(t);
}

template<typename T1, typename T2>
T1 atomic_assignment(T1& t1, T2 t2)
{
    auto t1_ref = std::atomic_ref<T1>(t1);
    auto t1_copy = t1_ref.load();
    while (not t1_ref.compare_exchange_weak(t1_copy, t2)) {}

    return t2;
}

}// namespace flow
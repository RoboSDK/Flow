#pragma once

#include <atomic>
#include <concepts>

namespace flow {

template<typename T>
T atomic_read(T& t)
{
  const auto ref = std::atomic_ref(t);
  auto current = ref.load();
  while (not ref.compare_exchange_weak(current, current)) {}
  return current;
}

template<typename T>
T atomic_read(std::atomic<T>& t)
{
  auto current = t.load();
  while (not t.compare_exchange_weak(current, current)) {}
  return current;
}

template<typename T>
T atomic_read(std::atomic_ref<T> t)
{
  auto current = t.load();
  while (not t.compare_exchange_weak(current, current)) {}
  return current;
}

template<typename T>
auto atomic_increment(T&& t)
{
  const auto perform_increment = [](auto& val) {
    auto current_value = val.load();
    while (not val.compare_exchange_weak(current_value, current_value + 1)) {}
    return current_value + 1;
  };

  if constexpr (std::is_rvalue_reference_v<T>) {
    auto value = std::atomic<T>(std::move(t));
    return perform_increment(value);
  }
  else {
    const auto ref = std::atomic_ref(t);
    return perform_increment(ref);
  }
}

template<typename T>
auto atomic_increment(std::atomic<T>& t)
{
  auto current_value = t.load();
  while (not t.compare_exchange_weak(current_value, current_value + 1)) {}
  return current_value + 1;
}

template<typename T>
auto atomic_increment(std::atomic_ref<T> t)
{
  auto current_value = t.load();
  while (not t.compare_exchange_weak(current_value, current_value + 1)) {}
  return current_value + 1;
}

template<typename T>
auto atomic_post_increment(T&& t)
{
  const auto perform_increment = [](auto& val) {
    auto current_value = val.load();
    while (not val.compare_exchange_weak(current_value, current_value + 1)) {}
    return current_value;
  };

  if constexpr (std::is_rvalue_reference_v<T>) {
    auto value = std::atomic<T>(std::move(t));
    return perform_increment(value);
  }
  else {
    const auto ref = std::atomic_ref(t);
    return perform_increment(ref);
  }
}

template<typename T>
auto atomic_post_increment(std::atomic<T>& t)
{
  auto current_value = t.load();
  while (not t.compare_exchange_weak(current_value, current_value + 1)) {}
  return current_value;
}

template<typename T>
auto atomic_post_increment(std::atomic_ref<T> t)
{
  auto current_value = t.load();
  while (not t.compare_exchange_weak(current_value, current_value + 1)) {}
  return current_value;
}

template<typename T>
auto atomic_decrement(T&& t)
{
  const auto perform_decrement = [](auto& val) {
    auto current_value = val.load();
    while (not val.compare_exchange_weak(current_value, current_value - 1)) {}
    return current_value - 1;
  };

  if constexpr (std::is_rvalue_reference_v<T>) {
    auto value = std::atomic<T>(std::move(t));
    return perform_decrement(value);
  }
  else {
    const auto ref = std::atomic_ref(t);
    return perform_decrement(ref);
  }
}

template<typename T>
auto atomic_decrement(std::atomic<T>& t)
{
  auto current_value = t.load();
  while (not t.compare_exchange_weak(current_value, current_value - 1)) {}
  return current_value - 1;
}

template<typename T>
auto atomic_decrement(std::atomic_ref<T> t)
{
  auto current_value = t.load();
  while (not t.compare_exchange_weak(current_value, current_value - 1)) {}
  return current_value - 1;
}

template<typename T1, typename T2>
T1 atomic_assignment(T1& t1, T2 t2)
{
  auto t1_ref = std::atomic_ref<T1>(t1);
  auto t1_copy = t1_ref.load();
  while (not t1_ref.compare_exchange_weak(t1_copy, t2)) {}
  return t2;
}

template<typename T1, typename T2>
T1 atomic_assignment(std::atomic_ref<T1> t1, T2 t2)
{
  auto t1_copy = t1.load();
  while (not t1.compare_exchange_weak(t1_copy, t2)) {}
  return t2;
}
}// namespace flow
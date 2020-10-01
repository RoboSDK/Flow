#pragma once

#include <array>

#include <farbot/RealtimeMutatable.hpp>

#include "flow/algorithms/make_array.hpp"

namespace flow {
template<typename T>
class double_buffer {
public:
  double_buffer() = default;
  double_buffer(double_buffer<T> const& other) = delete;
  double_buffer& operator=(double_buffer<T> const& other) = delete;
  double_buffer(double_buffer<T>&& other) = delete;
  double_buffer<T>& operator=(double_buffer<T>&& other) = delete;

  double_buffer(T&& t) : m_buffer{ make_array<T, 2>(std::forward<T>(t)) } {}

  double_buffer& operator=(T&& t)
  {
    m_buffer = make_array<T, 2>(std::forward<T>(t));
    return *this;
  }

  template<typename... args_t>
  double_buffer(args_t... args) : m_buffer{ make_array<T, 2>(T{ std::forward<args>(args)... }) }
  {
  }

  T read()
  {
    reader_access_t access{ m_read };
    return **access;
  }

  void write(T&& t)
  {
    *m_write = std::forward<T>(t);

    writer_access_t access{ m_read };
    std::swap(m_write, *access);
  }

private:
  template<typename M>
  using mutable_t = farbot::RealtimeMutatable<M>;

  using writer_access_t = typename mutable_t<T*>::template ScopedAccess<true>;
  using reader_access_t = typename mutable_t<T*>::template ScopedAccess<false>;

  using buffer_t = std::array<T, 2>;
  buffer_t m_buffer{};

  mutable_t<T*> m_read{ &m_buffer.front() };
  T* m_write{ &m_buffer.back() };
};
}// namespace flow

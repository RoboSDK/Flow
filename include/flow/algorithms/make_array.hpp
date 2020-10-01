#pragma once

namespace flow {
template<typename T, std::size_t N, std::size_t Idx = N>
struct array_maker {
  template<typename... Ts>
  static std::array<T, N> make_array(T&& v, Ts... tail)
  {
    return array_maker<T, N, Idx - 1>::make_array(std::forward<T>(v), v, tail...);
  }
};

template<typename T, std::size_t N>
struct array_maker<T, N, 1> {
  template<typename... Ts>
  static std::array<T, N> make_array(T&& v, Ts... tail)
  {
    return std::array<T, N>{ std::forward<T>(v), tail... };
  }
};

template<typename T, std::size_t N>
constexpr auto make_array(T&& t)
{

  return array_maker<T, N>::make_array(std::forward<T>(t));
}
}// namespace flow
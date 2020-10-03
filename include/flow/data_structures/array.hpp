#pragma once

namespace flow {
template<typename T, std::size_t N, std::size_t index_t = N, typename... Ts>
constexpr auto make_array(T t, Ts... ts)
{
  if constexpr (index_t <= 1) {
    return std::array<T, N> {t, ts...};
  } else {
    return make_array<T, N, index_t - 1>(t, t, ts...);
  }
}

}// namespace flow
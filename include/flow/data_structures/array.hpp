#pragma once

namespace flow {

/**
 * Used to make a std::array of a single instance of an object by copying it N times
 *
 * complexity: O(N) at compile time
 *
 * @tparam T The type that will be copied N times
 * @tparam N The size of the array
 * @tparam index_t This function recursively calls itself until index_t reaches 1
 * @tparam Ts The copies of T
 * @param t the instance of T
 * @param ts The copies of T
 * @return Fully filled array
 */
template<typename T, std::size_t N, std::size_t index_t = N, typename... Ts>
constexpr auto make_array(T const& t, Ts... ts)
{
  if constexpr (index_t <= 1) {
    return std::array<T, N> {t, ts...};
  } else {
    return make_array<T, N, index_t - 1>(t, t, ts...);
  }
}

}// namespace flow
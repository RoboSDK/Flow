#ifndef FLOW_MIXEDARRAY_HPP
#define FLOW_MIXEDARRAY_HPP

#include <tuple>
#include <array>
#include <variant>

#include "metaprogramming.hpp"

namespace flow {

template<typename... Types>
struct MixedArray {
  using MixedType = std::variant<Types...>;
  static constexpr std::size_t N = sizeof...(Types);
  using Array = std::array<MixedType, N>;

public:
  constexpr explicit MixedArray(Types &&... ts) : m_items{ { std::forward<decltype(ts)>(ts)... } } {}

  constexpr explicit MixedArray(Types... ts) : m_items(Array{ ts... }) {}
  constexpr decltype(auto) begin() { return std::begin(m_items); }
  constexpr decltype(auto) end() { return std::begin(m_items); }

  [[maybe_unused]] constexpr decltype(auto) front() { return m_items.front(); }
  [[maybe_unused]] constexpr decltype(auto) back() { return m_items.back(); }

  constexpr decltype(auto) operator[](std::size_t index) { return m_items[index]; }
  [[maybe_unused]] constexpr decltype(auto) size() { return m_items.size(); }

private:
  Array m_items;
};

template<typename... Types>
constexpr auto make_mixed_array(Types &&... types) {
  constexpr auto type_set = metaprogramming::make_type_set<Types...>();

  constexpr auto to_mixed_array = [&]<typename... TypeSet>(std::tuple<TypeSet...> /*unused*/) {
    return MixedArray(std::forward<Types...>(types)...);
  };

  return to_mixed_array(type_set);
}

}// namespace flow

#endif//FLOW_MIXEDARRAY_HPP

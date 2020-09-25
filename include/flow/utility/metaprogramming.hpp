#ifndef FLOW_METAPROGRAMMING_HPP
#define FLOW_METAPROGRAMMING_HPP

#include <type_traits>
#include <variant>

namespace flow::metaprogramming {

/**
 * Returns whether the list passed in is empty
 * @tparam A list of types
 * @return Whether the list is empty
 */
template <class... List>
constexpr bool empty()
{
  return sizeof...(List) == 0;
}

/**
 * Base case for the other contains function
 * @tparam EmptyListType Literally an empty type <>
 * @return always false
 */
template <class EmptyListType>
constexpr bool contains()
{
  return false;
}

/**
 * Returns whether the list passed in contains the first template argument
 *
 * complexity: O(n)
 *
 * example:
 * bool contains_item = flow::contains<TypeToFind, TypeList...>();
 * @tparam TypeToFind The type being searched for in the list
 * @tparam CurrentType The item in front of the list
 * @tparam TheRest The rest of the list
 * @return true if item is found
 */
template <class TypeToFind, class CurrentType, class... TheRest>
constexpr bool contains()
{
  if constexpr (std::is_same<TypeToFind, CurrentType>::value) { return true; }
  else if constexpr (not empty<TheRest...>()) { return contains<TypeToFind, TheRest...>(); }
  else { return false; }
}

/**
 * Takes in a list of types as template arguments and returns back an empty tuple containing the type set
 *
 * complexity:
 * This is an O(n^2) algorithm at compile time using brute force, but the lists should typically be short
 *
 * example:
 * std::tuple<int, double> type_set = flow::make_type_set<int, int, double>();
 *
 * @tparam CurrentType This is the current item unfolded from the TypesPassedIn that is currently unfolding
 * @tparam TypesPassedIn This is the rest of the template arguments passed (without the CurrentType)
 * @tparam TypeSet This is the set of types so far
 * @return An empty tuple containing the type set
 */
template <class CurrentType, class... TypesPassedIn, class... TypeSet>
constexpr auto make_type_set(std::tuple<TypeSet...>  /*unused*/ = std::tuple<TypeSet...>{})
{
  if constexpr (contains<CurrentType, TypeSet...>()) {

    if constexpr (not empty<TypesPassedIn...>()) {
      return make_type_set<TypesPassedIn...>(std::tuple<TypeSet...>());
    } else {
      return std::tuple<TypeSet...>();
    }

  } else {

    if constexpr (not empty<TypesPassedIn...>()) {
      return make_type_set<TypesPassedIn...>(std::tuple<TypeSet..., CurrentType>());
    } else {
      return std::tuple<TypeSet..., CurrentType>();
    }

  }
}

/**
 * Takes in a tuple as an argument and returns back a variant of types that tuple contains
 *
 * complexity:
 * This is an O(n^2) algorithm at compile time using brute force, but the lists should typically be short
 *
 * example:
 * std::tuple<int, double> type_set = flow::make_type_set<int, int, double>();
 *
 * @tparam CurrentType This is the current item unfolded from the TypesPassedIn that is currently unfolding
 * @tparam TypesPassedIn This is the rest of the template arguments passed (without the CurrentType)
 * @tparam TypeSet This is the set of types so far
 * @return An empty tuple containing the type set
 */
template <class... Types>
[[maybe_unused]] constexpr auto make_variant(std::tuple<Types...> /*unused*/) { return std::variant<Types...>{}; }
}
#endif//FLOW_METAPROGRAMMING_HPP

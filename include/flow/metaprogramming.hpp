#ifndef FLOW_METAPROGRAMMING_HPP
#define FLOW_METAPROGRAMMING_HPP

#include <concepts>
#include <string_view>
#include <type_traits>
#include <variant>
#include <wyhash/wyhash.h>

namespace flow::metaprogramming {

/*
 * A metaprogramming size_tc to pass in integral arguments as arguments at compile time
 */
template<std::size_t N>
struct size_tc {
  std::size_t data = N;
};

/**
 * Basic container type that can be used to get a type out of a tuple with a single type
 *
 * May also be used to store a type and use the value later
 * @tparam t Any type
 */
template<typename t>
struct type_container {
  using type = t;
  constexpr type_container() = default;
  constexpr type_container(type_container<t>&&) = default;
  constexpr type_container(type_container<t> const&) = default;
  constexpr type_container<t>& operator=(type_container<t>&&) = default;
  constexpr type_container<t>& operator=(type_container<t> const&) = default;
  constexpr type_container(std::tuple<t> /*unused*/) {}
};

/**
 * Get the last of the list
 * @tparam The list of items
 * @return the size of the list
 */
template<typename... items_t>
constexpr std::size_t size([[maybe_unused]] std::tuple<items_t...> list = std::tuple<items_t...>{})
{
  return sizeof...(items_t);
}

/**
 * Returns whether the list passed in is empty
 * @tparam A list of types
 * @return Whether the list is empty
 */
template<class... items_t>
constexpr bool empty([[maybe_unused]] std::tuple<items_t...> list = std::tuple<items_t...>{})
{
  return sizeof...(items_t) == 0;
}


/**
 * pops the first item from the list
 * @tparam removed_t The removed type
 * @tparam the_rest_t The rest...
 * @return A tuple with the rest of the types
 */
template<typename removed_t, typename... the_rest_t>
constexpr auto pop_front([[maybe_unused]] std::tuple<removed_t, the_rest_t...> list = std::tuple<removed_t, the_rest_t...>{})
{
  return std::tuple<the_rest_t...>{};
}

/**
 * Pops N from the front of the list
 * @param size_tc A compile time size type used to pass in the information
 * @tparam removed_t The current removed type
 * @tparam the_rest_t The rest...
 * @return A tuple with N less items in the front
 */
template<typename removed_t, typename... the_rest_t, std::size_t N>
constexpr auto pop_front([[maybe_unused]] size_tc<N>)
{
  if constexpr (empty<removed_t, the_rest_t...>()) {
    return std::tuple<>{};
  }
  else if constexpr (N == 0) {
    return std::tuple<removed_t, the_rest_t...>{};
  }
  else if constexpr (N == 1 and size<removed_t, the_rest_t...>() == 1) {
    return std::tuple<>{};
  }
  else {
    return pop_front<the_rest_t...>(size_tc<N - 1>{});
  }
}

/**
 * Return the next item in the list
 * @tparam next_t The next type
 * @tparam the_rest_t The rest...
 * @return A tuple with the next item
 */
template<typename next_t, typename... the_rest_t>
constexpr auto next(std::tuple<next_t> next_item = std::tuple<next_t>{})
{
  return next_item;
}

/**
 * Determines whether the items in the list are the same
 * @tparam items_t Any items
 * @param A tuple of types may be passed in to get items
 * @return true if they're all the same type
 */
template<typename... items_t>
constexpr bool is_same(std::tuple<items_t...> list = std::tuple<items_t...>{})
{
  if constexpr (empty(list) or size(list) == 1) {
    return true;
  }
  else {
    constexpr auto first = type_container(next(list));
    using first_type = typename decltype(first)::type;

    constexpr auto the_rest = pop_front(list);
    constexpr auto second = type_container(next(the_rest));
    using second_type = typename decltype(second)::type;

    constexpr bool same = std::is_same_v<first_type, second_type>;
    return same and is_same(pop_front(the_rest));
  }
}

//
//template <typename t1, typename t2>
//constexpr auto join() {
// return type_list<t1, t2>{};
//}
//
//template <typename t1>
//constexpr auto join<t1, empty_t>() {
//  return type_list<t1>{};
//}
//
//template <typename current, typename... the_rest_t>
//constexpr auto pop_back() {
//  constexpr std::size_t list_size = size<the_rest_t...>();
//  if constexpr (list_size == 1) {
//    return type_container<current>{};
//  } else if constexpr (list_size > 1) {
//    auto popped_back = pop_back<the_rest_t...>();
//  } else if constexpr (list_size == 0) {
//    return type_container<empty_t>{};
//  }
//}


/**
 * Pops the front of the list and continues the for loop
 * @tparam The message to be popped
 * @tparam The rest of the items
 * @param The callback the for loop is executing
 */
template<typename completed_t, typename... message_ts>
[[maybe_unused]] void pop_and_continue(auto&& callback);

/**
 * Iterates through each type and applies it to a callback that takes a
 * flow::metaprogramming::container<T> as a template argument and handles it
 *
 * flow::for_each<int, double>([&]<typename message_t>(flow::metaprogramming::container<message_t> ) {
 *   const auto& ch = std::any_cast<channel<message_t>>(channels.at(typeid(channel<message_t>)));
 *   ch.do_work();
 * });
 *
 * @tparam the_rest_t The items to be executed in this loop
 * @param callback The function handling the type
 */
//template<typename... the_rest_t>
//void for_each(auto&& callback)
//{
//  if constexpr (flow::metaprogramming::empty<the_rest_t...>()) {
//    return;
//  }
//  else {
//    using next_t = typename flow::metaprogramming::next<the_rest_t...>::type;
//    callback(flow::metaprogramming::container<next_t>{});
//    pop_and_continue<the_rest_t...>(std::forward<decltype(callback)>(callback));
//  }
//}

template<typename completed_t, typename... message_ts>
[[maybe_unused]] void pop_and_continue(auto&& callback)
{
  for_each<message_ts...>(std::forward<decltype(callback)>(callback));
}

/**
 * Base case for the other contains function
 * @tparam EmptyListType Literally an empty type <>
 * @return always false
 */
template<class EmptyListType>
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
template<class TypeToFind, class CurrentType, class... TheRest>
constexpr bool contains()
{
  if constexpr (std::is_same<TypeToFind, CurrentType>::value) { return true; }
  else if constexpr (not empty<TheRest...>()) {
    return contains<TypeToFind, TheRest...>();
  }
  else {
    return false;
  }
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
template<class CurrentType, class... TypesPassedIn, class... TypeSet>
constexpr auto make_type_set(std::tuple<TypeSet...> /*unused*/ = std::tuple<TypeSet...>{})
{
  using CurrentTypeDecayed = std::decay_t<CurrentType>;
  if constexpr (contains<CurrentTypeDecayed, TypeSet...>()) {

    if constexpr (not empty<TypesPassedIn...>()) {
      return make_type_set<TypesPassedIn...>(std::tuple<TypeSet...>());
    }
    else {
      return std::tuple<TypeSet...>();
    }
  }
  else {

    if constexpr (not empty<TypesPassedIn...>()) {
      return make_type_set<TypesPassedIn...>(std::tuple<TypeSet..., CurrentTypeDecayed>());
    }
    else {
      return std::tuple<TypeSet..., CurrentTypeDecayed>();
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
template<class... Types>
[[maybe_unused]] constexpr auto make_variant(std::tuple<Types...> /*unused*/) { return std::variant<Types...>{}; }

/**
 * to_string function for types
 *
 * @tparam INTERFACE_TYPENAME
 * @return returns a view of the type
 */
template<typename INTERFACE_TYPENAME>
[[maybe_unused]] [[nodiscard]] constexpr auto to_string() -> std::string_view
{
  constexpr std::string_view result = __PRETTY_FUNCTION__;
  constexpr std::string_view templateStr = "INTERFACE_TYPENAME = ";

  constexpr size_t bpos = result.find(templateStr) + templateStr.size();//find begin pos after INTERFACE_TYPENAME = entry
  if constexpr (result.find_first_not_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_:") == std::string_view::npos) {
    constexpr size_t len = result.length() - bpos;

    static_assert(!result.substr(bpos, len).empty(), "Cannot infer type name in function call");

    return result.substr(bpos, len);
  }
  else {
    constexpr size_t len = result.substr(bpos).find_first_not_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_:");

    static_assert(!result.substr(bpos, len).empty(), "Cannot infer type name in function call");

    return result.substr(bpos, len);
  }
}

template<typename T>
struct function_traits
  : public function_traits<decltype(&T::operator())> {
};
// For generic types, directly use the result of the signature of its 'operator()'

template<typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType (ClassType::*)(Args...) const>
// we specialize for pointers to member function
{
  enum { arity = sizeof...(Args) };
  // arity is the number of arguments.

  [[maybe_unused]] typedef ReturnType result_type;

  template<size_t i>
  struct argument {
    static_assert(i < arity, "flow::metaprogramming::function_traits: attempted to access index out of bounds");
    typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
    // the i-th argument is equivalent to the i-th tuple element of a tuple
    // composed of those arguments.
  };
};

}// namespace flow::metaprogramming
#endif//FLOW_METAPROGRAMMING_HPP

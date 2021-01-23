#ifndef FLOW_METAPROGRAMMING_HPP
#define FLOW_METAPROGRAMMING_HPP

#include <variant>
#include <tuple>
#include <functional>

/**
 * Metaprogramming utilities
 */

namespace flow::detail::metaprogramming {
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
  constexpr type_container(std::tuple<t> /*unused*/) {}

  constexpr type_container() = default;
  constexpr ~type_container() = default;

  constexpr type_container(type_container<t>&&) noexcept = default;
  constexpr type_container(type_container<t> const&) = default;
  constexpr type_container<t>& operator=(type_container<t>&&) noexcept = default;
  constexpr type_container<t>& operator=(type_container<t> const&) = default;
};

/**
 * Get the last of the list
 * @tparam The list of items
 * @return the size of the list
 */
// TODO: separate out all functions here like this
template<typename... items_t>
constexpr std::size_t size()
{
  return sizeof...(items_t);
}

template<typename... items_t>
constexpr std::size_t size([[maybe_unused]] std::tuple<items_t...> list)
{
  return sizeof...(items_t);
}

/**
 * return_turns whether the list passed in is empty
 * @tparam A list of types
 * @return Whether the list is empty
 */
template<class... items_t>
constexpr bool empty()
{
  return sizeof...(items_t) == 0;
}

template<class... items_t>
constexpr bool empty([[maybe_unused]] std::tuple<items_t...> const& /*unused*/)
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
constexpr auto pop_front([[maybe_unused]] size_tc<N> /*unused*/)
{
  if constexpr (empty<removed_t, the_rest_t...>()) {
    return std::tuple<the_rest_t...>{};
  }
  else if constexpr (N == 0) {
    return std::tuple<removed_t, the_rest_t...>{};
  }
  else if constexpr (N == 1 and size<removed_t, the_rest_t...>() == 1) {
    return std::tuple<the_rest_t...>{};
  }
  else {
    return pop_front<the_rest_t...>(size_tc<N - 1>{});
  }
}

/**
 * return_turn the next item in the list
 * @tparam next_t The next type
 * @tparam the_rest_t The rest...
 * @return A tuple with the next item
 */
template<typename next_t, typename... the_rest_t>
constexpr auto next([[maybe_unused]] std::tuple<next_t, the_rest_t...> next_item = std::tuple<next_t, the_rest_t...>{})
{
  return std::tuple<next_t>{};
}

/**
 * Determines whether the items in the list are the same
 * @tparam items_t Any items
 * @param A tuple of types may be passed in to get items
 * @return true if they're all the same type
 */
template<typename... items_t>
constexpr bool same([[maybe_unused]] std::tuple<items_t...> list = std::tuple<items_t...>{})
{
  if constexpr (empty<items_t...>() or size<items_t...>() == 1) {
    return true;
  }
  else {
    constexpr auto first = type_container(next<items_t...>());
    using first_type = typename decltype(first)::type;

    constexpr auto the_rest = pop_front<items_t...>();
    constexpr auto second = type_container(next(the_rest));
    using second_type = typename decltype(second)::type;

    constexpr bool are_the_same = std::is_same_v<first_type, second_type>;
    return are_the_same and same(pop_front(the_rest));
  }
}

/**
 * Removes the last item of the list
 * @tparam Current the left most item in the list
 * @tparam the_rest_t everything to the right of current
 * @param l an unused argument simple for creating a default constructed argument to pass in pure template args
 * @return
 */
template<typename current, typename... the_rest_t>
constexpr auto pop_back([[maybe_unused]] std::tuple<current, the_rest_t...> l = std::tuple<current, the_rest_t...>{})
{
  constexpr std::size_t total_size = size<current, the_rest_t...>();
  if constexpr (total_size <= 1) {
    return std::tuple<the_rest_t...>{};
  }
  else {
    return std::tuple_cat(std::tuple<current>{}, pop_back<the_rest_t...>());
  }
}

/**
 * Iterates through each type and applies it to a callback that takes a
 * detail::metaprogramming::container<T> as a template argument and handles it
 *
 * flow::for_each<int, double>([&]<typename message_t>(detail::metaprogramming::container<message_t> ) {
 *   const auto& ch = std::any_cast<multi_channel<message_t>>(m_channels.at(typeid(multi_channel<message_t>)));
 *   ch.do_work();
 * });
 *
 * @tparam the_rest_t The items to be executed in this loop
 * @param callback The function handling the type
 */
template<typename... items_t>
void for_each(auto&& callback)
{
  if constexpr (empty<items_t...>()) {
    return;
  }
  else {
    auto next_t = type_container(next<items_t...>());
    callback(type_container<typename decltype(next_t)::type>{});

    auto the_rest = pop_front<items_t...>();
    const auto continue_loop_on = [&]<typename... the_rest_t>([[maybe_unused]] std::tuple<the_rest_t...> /*unused*/)
    {
      for_each<the_rest_t...>(callback);
    };

    continue_loop_on(the_rest);
  }
}

/**
 ase case for the other contains function
 * @tparam EmptyListType Literally an empty type <>
 * @return always false
 */
template<class empty_t>
constexpr bool contains()
{
  return false;
}

/**
 a return_turns whether the list passed in contains the first template argument
 *
 * complexity: O(n)
 *
 * example:
 * bool contains_item = flow::contains<to_find_t, TypeList...>();
 * @tparam to_find_t The type being searched for in the list
 * @tparam current_t The item in front of the list
 * @tparam the_rest_t The rest of the list
 * @return true if item is found
 */
template<class to_find_t, class current_t, class... the_rest_t>
constexpr bool contains()
{
  if constexpr (std::is_same<to_find_t, current_t>::value) { return true; }
  else if constexpr (not empty<the_rest_t...>()) {
    return contains<to_find_t, the_rest_t...>();
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
 * @tparam current_t This is the current item unfolded from the passed_in_t that is currently unfolding
 * @tparam passed_in_t This is the rest of the template arguments passed (without the current_t)
 * @tparam set_t This is the set of types so far
 * @return An empty tuple containing the type set
 */
template<class current_t, class... passed_in_t, class... set_t>
constexpr auto make_type_set(std::tuple<set_t...> /*unused*/ = std::tuple<set_t...>{})
{
  using decayed_current_t = std::decay_t<current_t>;
  if constexpr (contains<decayed_current_t, set_t...>()) {

    if constexpr (not empty<passed_in_t...>()) {
      return make_type_set<passed_in_t...>(std::tuple<set_t...>());
    }
    else {
      return std::tuple<set_t...>();
    }
  }
  else {

    if constexpr (not empty<passed_in_t...>()) {
      return make_type_set<passed_in_t...>(std::tuple<set_t..., decayed_current_t>());
    }
    else {
      return std::tuple<set_t..., decayed_current_t>();
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

template<typename T>
struct function_traits;

template<typename return_t, typename... args_t>
struct function_traits<return_t(args_t...)> {
public:
  static constexpr std::size_t arity = sizeof...(args_t);

  using function_type = return_t (args_t...);
  using return_type = return_t;
  using stl_function_type = std::function<function_type>;
  using pointer = return_t (*)(args_t...);

  template<size_t I>
  struct args {
    static_assert(I < arity, "detail::metaprogramming::function_traits: index is out of range, index must less than sizeof args_t");
    using type = typename std::tuple_element<I, std::tuple<args_t...>>::type;
  };

  using tuple_type = std::tuple<std::remove_cv_t<std::remove_reference_t<args_t>>...>;
  using bare_tuple_type = std::tuple<std::remove_const_t<std::remove_reference_t<args_t>>...>;
};

template<typename return_t, typename... args_t>
struct function_traits<return_t (&)(args_t...)> : function_traits<return_t(args_t...)> {
};

template<typename return_t, typename... args_t>
struct function_traits<return_t (*)(args_t...)> : function_traits<return_t(args_t...)> {
};

//std::function.
template<typename return_t, typename... args_t>
struct function_traits<std::function<return_t(args_t...)>> : function_traits<return_t(args_t...)> {
};

//member function.
#define FUNCTION_TRAITS(...)                                                                                                    \
  template<typename return_turnType, typename ClassType, typename... args_t>                                                    \
  struct function_traits<return_turnType (ClassType::*)(args_t...) __VA_ARGS__> : function_traits<return_turnType(args_t...)> { \
  };

FUNCTION_TRAITS()
FUNCTION_TRAITS(const)
FUNCTION_TRAITS(volatile)
FUNCTION_TRAITS(const volatile)

template<typename Callable>
struct function_traits : function_traits<decltype(&std::decay_t<Callable>::operator())> {
};

template<typename Function>
typename function_traits<Function>::stl_function_type to_function(const Function& lambda)
{
  return static_cast<typename function_traits<Function>::stl_function_type>(lambda);
}

template<typename Function>
typename function_traits<Function>::stl_function_type to_function(Function&& lambda)
{
  return static_cast<typename function_traits<Function>::stl_function_type>(std::forward<Function>(lambda));
}

template<typename Function>
typename function_traits<Function>::pointer to_function_pointer(const Function& lambda)
{
  return static_cast<typename function_traits<Function>::pointer>(lambda);
}

}// namespace detail::metaprogramming
#endif//FLOW_METAPreturn_tOGreturn_tAMMING_HPP

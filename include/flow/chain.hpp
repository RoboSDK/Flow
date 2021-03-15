#pragma once

#include "flow/concepts.hpp"
#include "flow/detail/forward.hpp"

namespace flow {
namespace detail {
template <are_functions... functions_t>
struct chain_impl : chain_tag {
  std::tuple<functions_t ...> functions{};

  explicit chain_impl(std::tuple<functions_t...>&& fs) : functions(std::move(fs)) {}

  void operator()() {}
};
}

template <are_functions... functions_t>
constexpr auto chain(std::tuple<functions_t...>&& functions = std::tuple<>{})
{
  return detail::chain_impl<functions_t...>(std::move(functions));
}

template <are_functions... functions_t>
constexpr auto concat(std::tuple<functions_t...>&& functions, is_function auto&& new_function)
{
  return std::tuple_cat(std::move(functions), std::make_tuple(forward(new_function)));
}

} // namespace flow
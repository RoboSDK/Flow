#pragma once

#include "flow/detail/forward.hpp"

#include "flow/chain.hpp"

namespace flow {
constexpr auto operator|(is_chain auto&& chain_so_far, is_function auto&& f) {
  return chain(concat(forward(chain_so_far.functions), forward(f)));
}
} // namespace flow
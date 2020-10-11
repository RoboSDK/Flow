#pragma once

#include <bitset>
#include <atomic>

namespace flow {
struct configuration {
  struct global {
    static constexpr auto max_callbacks = 64;
  };

  using atomic_bitset_t = std::atomic<std::bitset<global::max_callbacks>>;
};
}
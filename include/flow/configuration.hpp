#pragma once

#include <bitset>
#include <atomic>

namespace flow {
struct configuration {
  struct global {
    static constexpr auto max_callbacks = 64;
  };
};
}
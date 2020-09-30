#pragma once

#include <atomic_queue/atomic_queue.h>

namespace flow {
template <typename T, std::size_t N>
using atomic_queue = atomic_queue::AtomicQueue<T, N>;
}
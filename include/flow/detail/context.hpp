#pragma once

#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>

#include "channel.hpp"
#include "channel_set.hpp"

/**
 * This is a global context that all communicate will be built upon
 *
 * This will be dynamically allocated because coroutines cannot access the stack (in our circumstance)
 */

namespace flow {
using task_t = cppcoro::task<void>;
template<typename configuration_t>
struct context {
  cppcoro::static_thread_pool thread_pool{};
  detail::channel_set<configuration_t> channels{};
  detail::channel_resource_generator<configuration_t> resource_generator{};
  std::vector<task_t> tasks{};
};
}// namespace flow
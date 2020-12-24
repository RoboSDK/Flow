#pragma once

#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>

#include <flow/data_structures/channel_set.hpp>
#include <flow/channel.hpp>

namespace flow {
using task_t = cppcoro::task<void>;
template <typename configuration_t>
struct context {
  cppcoro::static_thread_pool thread_pool{ 0 };
  channel_set<configuration_t> channels{};
  channel_resource_generator<configuration_t> resource_generator{};
  std::vector<task_t> tasks{};
};
}
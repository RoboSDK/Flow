#pragma once

#include <NamedType/crtp.hpp>

namespace flow {
/**
 * The task interface
 * @tparam concrete_task The concrete task type that will be implementing this interface
 *
 * example: class planning : task<planning>
 *
 * A tasks purpose is to do one thing, and do it well.
 */
template<typename concrete_task_t>
class task : fluent::crtp<concrete_task_t, task> {
public:
  void begin(auto& registry);
};

// Implementation
template<typename concrete_task>
void task<concrete_task>::begin(auto& registry)
{
  this->underlying()->begin(registry);
}

template<typename task_t>
void begin(task_t& task, auto& registry)
{
  task.begin(registry);
}
}// namespace flow
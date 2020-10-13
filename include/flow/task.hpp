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
template<typename task_t>
struct task : fluent::crtp<task_t, task> {
  void begin(auto& registry) { this->underlying().begin(registry); }
  void end() { this->underlying().end(); }
};

template<typename task_t>
void begin(flow::task<task_t>& task, auto& registry) { task.begin(registry); }

template<typename task_t>
void end(flow::task<task_t>& task) { task.end(); }
}// namespace flow
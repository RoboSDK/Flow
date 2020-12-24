#pragma once

namespace flow {

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
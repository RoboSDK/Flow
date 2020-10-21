#pragma once

#include <flow/data_structures/mixed_array.hpp>
#include <flow/deprecated/layer.hpp>
#include <flow/metaprogramming.hpp>
#include <flow/task.hpp>

#include <ranges>

namespace mock {
template<class... tasks_t>
class testing_layer : flow::layer<testing_layer<tasks_t...>> {
  static constexpr std::size_t num_tasks = flow::metaprogramming::size<tasks_t...>();

public:
  void begin(auto& registry)
  {
    std::ranges::for_each(tasks, flow::make_visitor([&](auto& task) {
      flow::begin(task, registry);
    }));
  }

  void end()
  {
    std::ranges::for_each(tasks, flow::make_visitor([&](auto& task) {
           flow::end(task);
    }));
  }

private:
  flow::mixed_array<num_tasks, tasks_t...> tasks = flow::make_mixed_array(tasks_t{}...);
};
}// namespace mock
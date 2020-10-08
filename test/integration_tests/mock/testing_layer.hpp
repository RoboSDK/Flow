#pragma once

#include <flow/data_structures/mixed_array.hpp>
#include <flow/metaprogramming.hpp>
#include <flow/layer.hpp>
#include <flow/task.hpp>

namespace mock {
template<class... tasks_t>
class testing_layer : flow::layer<testing_layer<tasks_t...>> {
  static constexpr std::size_t num_tasks = flow::metaprogramming::size<tasks_t...>();

public:
  void register_channels(auto& registry)
  {
    std::for_each(std::begin(tasks), std::end(tasks), flow::make_visitor([&](auto& task) {
      flow::begin(task, registry);
    }));
  }

private:
  flow::mixed_array<num_tasks, tasks_t...> tasks = flow::make_mixed_array(tasks_t{}...);
};
}// namespace mock
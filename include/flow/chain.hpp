#pragma once

#include <cppcoro/task.hpp>

#include "flow/cancellation.hpp"
#include "flow/channel.hpp"
#include "flow/data_structures/channel_set.hpp"
#include "flow/spin.hpp"

namespace flow {

template<typename configuration_t>
struct chain {
  using task_t = cppcoro::task<void>;

  template<typename message_t>
  void make_channel_if_not_exists(std::string_view channel_name)
  {
    if (channels.contains<message_t>(channel_name)) {
      return;
    }

    using channel_t = channel<message_t, configuration_t>;

    auto channel = channel_t{
      channel_name,
      make_channel_resources(resource_generator)
    };

    channels.put(std::move(channel));
  }

  template<typename return_t>
  void push_producer(
    std::string_view channel_name,
    cancellable_function<return_t()>&& producer)
  {
    make_channel_if_not_exists<return_t>(channel_name);
    tasks.push_back(spin_producer(std::move(producer), channels));
  }

  template<typename return_t, typename argument_t>
  void push_transformer(
    std::string_view return_channel_name,
    std::string_view argument_channel_name,
    cancellable_function<return_t(argument_t)>&& transformer)
  {
    make_channel_if_not_exists<return_t>(return_channel_name);
    make_channel_if_not_exists<argument_t>(argument_channel_name);

    tasks.push_back(spin_transformer(std::move(transformer), channels));
  }

  template<typename argument_t>
  void push_consumer(
    std::string_view channel_name,
    cancellable_function<void(argument_t)>&& end)
  {
    make_channel_resources<argument_t>()
    tasks.push_back(spin_subscriber(std::move(end), channels));
  }

  cppcoro::task<> spin()
  {
    std::ranges::reverse(tasks);
    co_await cppcoro::when_all_ready(std::move(tasks));
  }

  channel_set channels{};
  channel_resource_generator resource_generator{};
  std::vector<task_t> tasks{};
};
}// namespace flow
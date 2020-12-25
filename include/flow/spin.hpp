#pragma once

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>

#include "flow/cancellation.hpp"
#include "flow/channel.hpp"

namespace flow {

cppcoro::task<void> spin_spinner(
  auto& scheduler,
  cancellable_function<void()>& spinner)
{
  while (not spinner.is_cancellation_requested()) {
    co_await scheduler.schedule();
    std::invoke(spinner);
  }
}

template<typename return_t>
cppcoro::task<void> spin_producer(
  auto& channel,
  cancellable_function<return_t()>& producer)
{
  while (not producer.is_cancellation_requested()) {
    co_await channel.request();
    auto message = std::invoke(producer);
    channel.publish_message(std::move(message));
  }
}

template<typename argument_t>
cppcoro::task<void> spin_consumer(
  auto& channel,
  cancellable_function<void(argument_t&&)>& consumer)
{
  while (not consumer.is_cancellation_requested()) {
    auto next_message = channel.message_generator();
    auto current_message = co_await next_message.begin();

    while (not consumer.is_cancellation_requested() and current_message != next_message.end()) {
      auto& message = *current_message;
      std::invoke(consumer, std::move(message));
      channel.make_request();
      co_await ++current_message;
    }
  }
}

template<typename return_t, typename argument_t>
cppcoro::task<void> spin_transformer(
  auto& producer_channel,
  auto& consumer_channel,
  cancellable_function<return_t(argument_t&&)> transformer)
{

  while (not transformer.is_cancellation_requested()) {
    auto next_message = producer_channel.message_generator();
    auto current_message = co_await next_message.begin();

    while (not transformer.is_cancellation_requested() and current_message != next_message.end()) {
      auto& message = *current_message;
      auto result = std::invoke(transformer, std::move(message));

      co_await consumer_channel.request();
      consumer_channel.publish_message(std::move(result));

      producer_channel.make_request();
      co_await ++current_message;
    }
  }
}
}// namespace flow
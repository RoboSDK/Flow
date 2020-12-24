#pragma once

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>

#include "flow/cancellation.hpp"
#include "flow/channel.hpp"

namespace flow {
template<typename return_t>
cppcoro::task<void> spin_producer(
  auto& channel,
  cancellable_function<return_t()> & producer)
{
  flow::logging::error("producer: spin");

  while (not producer.is_cancellation_requested()) {
    flow::logging::error("producer: making request");
    co_await channel.request();
    flow::logging::error("producer: making message");
    auto message = std::invoke(producer);
    flow::logging::error("producer: publishing");
    channel.publish_message(std::move(message));
  }
  flow::logging::error("producer: done");
}

template<typename argument_t>
cppcoro::task<void> spin_consumer(
  auto& channel,
  cancellable_function<void(argument_t&&)> & consumer)
{
  flow::logging::error("consumer: spin");

  while (not consumer.is_cancellation_requested()) {
    flow::logging::error("consumer: creating message generator");
    auto next_message = channel.message_generator();
    flow::logging::error("consumer: waiting for message ");
    auto current_message = co_await next_message.begin();

    while(not consumer.is_cancellation_requested() and current_message != next_message.end()) {
      auto& message = *current_message;
      flow::logging::error("consumer: invoke value: {}", message);
      std::invoke(consumer, std::move(message));
      flow::logging::error("consumer: making request");
      channel.make_request();
      flow::logging::error("consumer: req");
      co_await ++current_message;
    }
  }
  flow::logging::error("done consumer");
}

template<typename return_t, typename argument_t>
cppcoro::task<void> spin_transformer(
  auto& producer_channel,
  auto& consumer_channel,
  cancellable_function<return_t(argument_t&&)> transformer)
{
  flow::logging::error("transformer: spin");

  while (not transformer.is_cancellation_requested()) {
    flow::logging::error("transformer: creating message generator");
    auto next_message = producer_channel.message_generator();
    flow::logging::error("transformer: waiting for message ");
    auto current_message = co_await next_message.begin();

    while(not transformer.is_cancellation_requested() and current_message != next_message.end()) {
      auto& message = *current_message;
      flow::logging::error("transformer: invoke value: {}", message);
      auto result = std::invoke(transformer, std::move(message));

      co_await consumer_channel.request();
      flow::logging::error("transformer: making message");
      flow::logging::error("transformer: publishing");
      consumer_channel.publish_message(std::move(result));

      flow::logging::error("transformer: making request");
      producer_channel.make_request();
      flow::logging::error("transformer: req");
      co_await ++current_message;
    }
  }
  flow::logging::error("transformer:d done");
}
}// namespace flow
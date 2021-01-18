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
  flow::logging::error("spinner confirming cancellation");
  spinner.confirm_cancellation();
}

template<typename return_t>
cppcoro::task<void> spin_producer(
  auto& channel,
  cancellable_function<return_t()>& producer)
{
  while (not channel.is_terminated()) {
    co_await channel.request();
    auto message = std::invoke(producer);
    channel.publish_message(std::move(message));
  }
  flow::logging::error("producer confirming cancellation");
  producer.confirm_cancellation();
}

template<typename argument_t>
cppcoro::task<void> spin_consumer(
  auto& channel,
  cancellable_function<void(argument_t&&)>& consumer)
{
  while (not consumer.is_cancellation_requested()) {
    auto next_message = channel.message_generator();
    auto current_message = co_await next_message.begin();

    while (current_message != next_message.end()) {
      auto& message = *current_message;
      std::invoke(consumer, std::move(message));
      channel.make_request();
      co_await ++current_message;
    }
  }

  flow::logging::error("consumer confirming cancellation");
  consumer.confirm_cancellation();
  channel.terminate();

  while (channel.is_waiting()) {
    flow::logging::error("consumer flushing");
    auto next_message = channel.message_generator();
    auto current_message = co_await next_message.begin();

    while (current_message != next_message.end()) {
      auto& message = *current_message;
      std::invoke(consumer, std::move(message));
      channel.make_request();
      co_await ++current_message;
    }
  }
  flow::logging::error("consumer done flushing");
}

template<typename return_t, typename argument_t>
cppcoro::task<void> spin_transformer(
  auto& producer_channel,
  auto& consumer_channel,
  cancellable_function<return_t(argument_t&&)> transformer)
{

  while (not consumer_channel.is_terminated()) {
    auto next_message = producer_channel.message_generator();
    auto current_message = co_await next_message.begin();

    while (current_message != next_message.end()) {
      auto& message = *current_message;
      auto result = std::invoke(transformer, std::move(message));

      flow::logging::error("transformer requesting to publish");
      co_await consumer_channel.request();
      flow::logging::error("transformer publish message");
      consumer_channel.publish_message(std::move(result));

      flow::logging::error("transformer requesting next message");
      producer_channel.make_request();
      co_await ++current_message;
    }
  }

  flow::logging::error("transformer confirming cancellation");
  transformer.confirm_cancellation();
  producer_channel.terminate();

  while (producer_channel.is_waiting()) {
    flow::logging::error("transformer flushing");
    auto next_message = producer_channel.message_generator();
    auto current_message = co_await next_message.begin();

    while (current_message != next_message.end()) {
      auto& message = *current_message;
      std::invoke(transformer, std::move(message));
      producer_channel.make_request();
      co_await ++current_message;
    }
  }
  flow::logging::error("transformer done flushing");
}
}// namespace flow
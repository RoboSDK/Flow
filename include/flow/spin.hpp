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

  flow::logging::error("consumer: creating message generator");
  auto next_message = channel.message_generator();
  flow::logging::error("consumer: waiting for message ");
  auto current_message = co_await next_message.begin();

  while (not consumer.is_cancellation_requested()) {
    flow::logging::error("consumer: get message ref");
    auto& message = *current_message;
    flow::logging::error("consumer: invoke value: {}", message);
    std::invoke(consumer, std::move(message));
    flow::logging::error("consumer: making request");
    channel.make_request();
    flow::logging::error("consumer: request submitted");
    co_await ++current_message;
    flow::logging::error("consumer: increment next msg");
  }
  flow::logging::error("done consumer");
}

template<typename return_t, typename argument_t, typename configuration_t>
cppcoro::task<void> spin_transformer(
  std::string return_channel_name,
  std::string argument_channel_name,
  cancellable_function<void(argument_t)> transformer,
  auto& context)
{
  using argument_channel_t = channel<argument_t, configuration_t>;
  using return_channel_t = channel<return_t, configuration_t>;

  argument_channel_t& argument_channel = context.channels.template at<argument_t>(argument_channel_name);
  auto next_message = argument_channel.message_generator();
  auto current_message = next_message.begin();

  return_channel_t & return_channel = context.channels.template at<return_t>(return_channel_name);
  auto next_request = return_channel.request_generator();
  auto current_request = next_request.begin();

  while (not transformer.is_cancellation_requested()) {
    argument_t& argument = *co_await ++current_message;
    return_t returned = std::invoke(transformer, argument);
    argument_channel.make_request();

    return_t& request = *co_await ++current_request;
    request = std::move(returned);
    return_channel.publish_message();
  }
}
}// namespace flow
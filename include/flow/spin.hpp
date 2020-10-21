#pragma once

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>

#include "flow/cancellation.hpp"
#include "flow/channel.hpp"

namespace flow {
template<typename return_t, typename configuration_t>
cppcoro::task<void> spin_producer(
  std::string_view channel_name,
  cancellable_function<return_t()>&& producer,
  auto& context)
{
  using channel_t = channel<return_t, configuration_t>;

  channel_t& channel = context.channels.template at<return_t>(channel_name);
  auto next_request = channel.request_generator();
  auto current_request = next_request.begin();

  while (not producer.is_cancellation_requested()) {
    auto& message = *co_await ++current_request;
    message = std::invoke(producer);
    channel.fulfill_request();
  }
}

template<typename argument_t, typename configuration_t>
cppcoro::task<void> spin_consumer(
  std::string_view channel_name,
  cancellable_function<void(argument_t)> consumer,
  auto& context)
{
  using channel_t = channel<argument_t, configuration_t>;

  channel_t& channel = context.channels.template at<argument_t>(channel_name);
  auto next_message = channel.message_generator();
  auto current_message = next_message.begin();

  while (not consumer.is_cancellation_requested()) {
    auto& message = *co_await ++current_message;
    std::invoke(consumer, message);
    channel.make_request();
  }
}

template<typename return_t, typename argument_t, typename configuration_t>
cppcoro::task<void> spin_transformer(
  std::string_view argument_channel_name,
  std::string_view return_channel_name,
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
    return_channel.fulfill_request();
  }
}
}// namespace flow
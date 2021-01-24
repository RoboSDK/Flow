#pragma once

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>

#include "flow/network.hpp"
#include "flow/routine_concepts.hpp"

#include "cancellable_function.hpp"
#include "multi_channel.hpp"

/**
 * The purpose of this module is make the coroutines that will can 'spin'
 *
 * Spin meaning to keep repeating in a loop until they are cancelled
 */

namespace flow::detail {

/**
 * Generates a coroutine that keeps calling the spinner_function until it is cancelled
 * @param scheduler a cppcoro::static_thread_pool, cppcoro::io_service, or another cppcoro scheduler
 * @param spinner A cancellable function with no return type and requires no arguments
 * @return A coroutine that continues until the spinner_function is cancelled
 */
cppcoro::task<void> spin_spinner(
  auto& scheduler,
  cancellable_function<void()>& spinner)
{
  while (not spinner.is_cancellation_requested()) {
    co_await scheduler.schedule();
    co_await spinner();
  }
}

/**
 * Generates a coroutine that keeps calling the producer_function until it is cancelled
 *
 * Notice that the producer_function is not flushing. The reason is that the producer_function belongs at
 * the beginning of the network and has no one else in front of it, and therefore nothing
 * to flush
 *
 * @param channel a flow multi_channel that represents a connection between the receiver
 *                for the data that the producer_function produces, and the producer_function itself
 * @param producer A producer_function is a cancellable function with no arguments required to call it and
 *                 a specified return type
 * @return A coroutine that continues until the producer_function is cancelled
 */
template<typename return_t>
cppcoro::task<void> spin_producer(
  auto& channel,
  cancellable_function<return_t()>& producer)
{
  producer_token<return_t> producer_token{};

  while (not channel.is_terminated()) {
    co_await channel.request_permission_to_publish(producer_token);

    for (std::size_t i = 0; i < producer_token.sequences.size(); ++i) {
      return_t message = co_await [&]() -> cppcoro::task<return_t> { co_return std::invoke(producer); }();
      producer_token.messages.push(std::move(message));
    }

    channel.publish_messages(producer_token);
  }
}

/**
 * TODO: Handle many arguments, maybe convert it to a tuple?
 *
 * Generates a coroutine that keeps calling the consumer_function until it is cancelled.
 *
 * The consumer_function will be placed at the end of the function network and will be the one
 * that triggers any cancellation events. It depends on a transformer_function or producer_function to
 * send messages through the multi_channel.
 *
 * When a consumer_function detects that cancellation is requested, then it will process
 * any messages it has left in the buffer.
 *
 * After the main loop it will terminate the multi_channel and flush out any routines
 * currently waiting on the other end of the multi_channel.
 *
 * @param channel a flow multi_channel that represents a connection between a producer_function or transformer_function
 *                that is generating data and the consumer_function that will be receiving the data
 * @param consumer A consumer_function is a cancellable function with at least one argument required to call it and
 *                 a specified return type
 * @return A coroutine that continues until the consumer_function is cancelled
 */
template<typename argument_t>
cppcoro::task<void> spin_consumer(
  auto& channel,
  cancellable_function<void(argument_t&&)>& consumer)
{
  consumer_token<argument_t> consumer_token{};

  while (not consumer.is_cancellation_requested()) {
    auto next_message = channel.message_generator(consumer_token);
    auto current_message = co_await next_message.begin();

    while (current_message != next_message.end()) {
      auto& message = *current_message;
      co_await [&]() -> cppcoro::task<void> { co_return consumer(std::move(message)); }();

      channel.notify_message_consumed(consumer_token);
      co_await ++current_message;
    }
  }

  channel.terminate();
  co_await flush<void>(channel, consumer, consumer_token);
}

/**
 * TODO: Handle many arguments, maybe convert it to a tuple?
 *
 * Generates a coroutine that keeps calling the transformer_function until the multi_channel its
 * sending messages to is terminated by the consumer_function or transformer_function on the other end
 *
 * Transformers will live in between a producer_function and consumer_function.
 *
 * When a transformer_function detects that the next function in line has terminated the multi_channel, then it will process
 * any messages it has left in the buffer and break out of its loop.
 *
 * After the main loop it will terminate the producer_function multi_channel and flush out any routines
 * currently waiting on the other end of the producer_function multi_channel.
 *
 * @param producer_channel The multi_channel that will have a producing function on the other end
 * @param consumer_channel The multi_channel that will have a consuming function on the other end
 * @param transformer A consumer_function is a cancellable function with at least one argument required to call it and
 *                 a specified return type
 * @return A coroutine that continues until the transformer_function is cancelled
 */
template<typename return_t, typename argument_t>
cppcoro::task<void> spin_transformer(
  auto& producer_channel,
  auto& consumer_channel,
  cancellable_function<return_t(argument_t&&)> transformer)
{
  producer_token<return_t> producer_token{};
  consumer_token<argument_t> consumer_token{};

  co_await consumer_channel.request_permission_to_publish(producer_token);

  while (not consumer_channel.is_terminated()) {
    auto next_message = producer_channel.message_generator(consumer_token);
    auto current_message = co_await next_message.begin();

    while (current_message != next_message.end()) {
      auto& message_to_consume = *current_message;

      auto message_to_produce = co_await [&]() -> cppcoro::task<return_t> {
        co_return std::invoke(transformer, std::move(message_to_consume));
      }();

      producer_token.messages.push(std::move(message_to_produce));
      producer_channel.notify_message_consumed(consumer_token);

      if (producer_token.messages.size() == producer_token.sequences.size()) {
        consumer_channel.publish_messages(producer_token);
        co_await consumer_channel.request_permission_to_publish(producer_token);
      }

      co_await ++current_message;
    }
  }

  producer_channel.terminate();
  co_await flush<return_t>(producer_channel, transformer, consumer_token);
}

/**
 * The consumer_function or transformer_function function will flush out any producer_function routines
 * in waiting on the other end of the multi_channel
 *
 * @param channel A communication multi_channel between the consumer_function and producer_function routines
 * @param routine A consumer_function or transformer_function function
 * @return A coroutine
 */
template<typename return_t, flow::function routine_t>
  cppcoro::task<void> flush(auto& channel, routine_t& routine, auto& consumer_token) requires flow::consumer_function<routine_t> or flow::transformer_function<routine_t>
{
  while (channel.is_waiting()) {
    auto next_message = channel.message_generator(consumer_token);
    auto current_message = co_await next_message.begin();

    while (current_message != next_message.end()) {
      auto& message = *current_message;

      co_await [&]() -> cppcoro::task<return_t> {
        co_return std::invoke(routine, std::move(message));
      }();

      channel.notify_message_consumed(consumer_token);
      co_await ++current_message;
    }
  }
}

}// namespace flow::detail
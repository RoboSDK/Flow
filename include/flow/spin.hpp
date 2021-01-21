#pragma once

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>

#include "flow/cancellable_function.hpp"
#include "flow/channel.hpp"

/**
 * The purpose of this module is make the coroutines that will can 'spin'
 *
 * Spin meaning to keep repeating in a loop until they are cancelled
 */

namespace flow {

/**
 * Generates a coroutine that keeps calling the spinner until it is cancelled
 * @param scheduler a cppcoro::static_thread_pool, cppcoro::io_service, or another cppcoro scheduler
 * @param spinner A cancellable function with no return type and requires no arguments
 * @return A coroutine that continues until the spinner is cancelled
 */
cppcoro::task<void> spin_spinner(
  auto& scheduler,
  cancellable_function<void()>& spinner)
{
  while (not spinner.is_cancellation_requested()) {
    co_await scheduler.schedule();
    std::invoke(spinner);
  }
}

/**
 * Generates a coroutine that keeps calling the producer until it is cancelled
 *
 * Notice that the producer is not flushing. The reason is that the producer belongs at
 * the beginning of the network and has no one else in front of it, and therefore nothing
 * to flush
 *
 * @param channel a flow channel that represents a connection between the receiver
 *                for the data that the producer produces, and the producer itself
 * @param producer A producer is a cancellable function with no arguments required to call it and
 *                 a specified return type
 * @return A coroutine that continues until the producer is cancelled
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
      producer_token.messages.push(std::invoke(producer));
    }

    channel.publish_messages(producer_token);
  }
}

/**
 * TODO: Handle many arguments, maybe convert it to a tuple?
 *
 * Generates a coroutine that keeps calling the consumer until it is cancelled.
 *
 * The consumer will be placed at the end of the routine network and will be the one
 * that triggers any cancellation events. It depends on a transformer or producer to
 * send messages through the channel.
 *
 * When a consumer detects that cancellation is requested, then it will process
 * any messages it has left in the buffer.
 *
 * After the main loop it will terminate the channel and flush out any routines
 * currently waiting on the other end of the channel.
 *
 * @param channel a flow channel that represents a connection between a producer or transformer
 *                that is generating data and the consumer that will be receiving the data
 * @param consumer A consumer is a cancellable function with at least one argument required to call it and
 *                 a specified return type
 * @return A coroutine that continues until the consumer is cancelled
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
      std::invoke(consumer, std::move(message));

      channel.notify_message_consumed(consumer_token);
      co_await ++current_message;
    }
  }

  channel.terminate();
  co_await flush(channel, consumer, consumer_token);
}

/**
 * TODO: Handle many arguments, maybe convert it to a tuple?
 *
 * Generates a coroutine that keeps calling the transformer until the channel its
 * sending messages to is terminated by the consumer or transformer on the other end
 *
 * Transformers will live in between a producer and consumer.
 *
 * When a transformer detects that the next routine in line has terminated the channel, then it will process
 * any messages it has left in the buffer and break out of its loop.
 *
 * After the main loop it will terminate the producer channel and flush out any routines
 * currently waiting on the other end of the producer channel.
 *
 * @param producer_channel The channel that will have a producing routine on the other end
 * @param consumer_channel The channel that will have a consuming routine on the other end
 * @param transformer A consumer is a cancellable function with at least one argument required to call it and
 *                 a specified return type
 * @return A coroutine that continues until the transformer is cancelled
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
      auto& message = *current_message;
      producer_token.messages.push(std::invoke(transformer, std::move(message)));
      producer_channel.notify_message_consumed(consumer_token);

      if (producer_token.messages.size() == producer_token.sequences.size()) {
        consumer_channel.publish_messages(producer_token);
        co_await consumer_channel.request_permission_to_publish(producer_token);
      }

      co_await ++current_message;
    }
  }

  producer_channel.terminate();
  co_await flush(producer_channel, transformer, consumer_token);
}

/**
 * The consumer or transformer routine will flush out any producer routines
 * in waiting on the other end of the channel
 *
 * @param channel A communication channel between the consumer and producer routines
 * @param routine A consumer or transformer routine
 * @return A coroutine
 */
template<flow::routine routine_t>
  requires flow::consumer<routine_t> or flow::transformer<routine_t> cppcoro::task<void> flush(auto& channel, routine_t& routine, auto& consumer_token)
{
  while (channel.is_waiting()) {
    auto next_message = channel.message_generator(consumer_token);
    auto current_message = co_await next_message.begin();

    while (current_message != next_message.end()) {
      auto& message = *current_message;
      std::invoke(routine, std::move(message));
      channel.notify_message_consumed(consumer_token);
      co_await ++current_message;
    }
  }
}
}// namespace flow
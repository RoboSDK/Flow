#pragma once

#include <random>

#include <cppcoro/async_mutex.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>

#include "flow/concepts.hpp"
#include "flow/detail/spin_wait.hpp"
#include "flow/network.hpp"

#include "cancellable_function.hpp"
#include "multi_channel.hpp"

/**
 * The purpose of this module is make the coroutines that will can 'spin'
 *
 * Spin meaning to keep repeating in a loop until they are cancelled
 *
 * TODO: This file needs some serious refactoring
 */

namespace flow::detail {
/**
 * Generates a coroutine that keeps calling the spinner_function until it is cancelled
 * @param scheduler a cppcoro::static_thread_pool, cppcoro::io_service, or another cppcoro scheduler
 * @param spinner A cancellable function with no return type and requires no arguments
 * @return A coroutine that continues until the spinner_function is cancelled
 */
cppcoro::task<void> spin_spinner(std::optional<std::chrono::nanoseconds> period,
  auto& scheduler,
  cancellable_function<void()>& spinner)
{
  //    spin_wait rate{period.value()};
  //    while (not spinner.is_cancellation_requested() and co_await rate.async_is_ready()) {
  while (not spinner.is_cancellation_requested()) {
    //      co_await rate.async_reset();
    co_await [&]() -> cppcoro::task<void> { spinner(); co_return; }();
  }
}

/**
 * Generates a coroutine that keeps calling the publisher_function until it is cancelled
 *
 * Notice that the publisher_function is not flushing. The reason is that the publisher_function belongs at
 * the beginning of the network and has no one else in front of it, and therefore nothing
 * to flush
 *
 * @param channel a flow multi_channel that represents a connection between the receiver
 *                for the data that the publisher_function produces, and the publisher_function itself
 * @param publisher A publisher_function is a cancellable function with no arguments required to call it and
 *                 a specified return type
 * @return A coroutine that continues until the publisher_function is cancelled
 */
template<typename return_t>
cppcoro::task<void> spin_publisher(
  std::chrono::nanoseconds period,
  auto& channel,
  cancellable_function<return_t()>& publisher)
{
  publisher_token<return_t> publisher_token{};
  using channel_t = std::decay_t<decltype(channel)>;
  spin_wait rate{ period };

  auto termination_has_initialized = [&]() -> cppcoro::task<bool> {
    static cppcoro::async_mutex mutex;
    auto lock = co_await mutex.scoped_lock_async();
    co_return channel.state() >= channel_t::termination_state::subscriber_initialized;
  };

  while (not co_await termination_has_initialized()) {
    channel.increment_publishers_waiting();
    if (not co_await channel.request_permission_to_publish(publisher_token)) {
      channel.decrement_publishers_waiting();
      break;
    }
    channel.decrement_publishers_waiting();

    std::size_t i = 0;
    while (i < publisher_token.sequences.size()) {
      return_t message = co_await [&]() -> cppcoro::task<return_t> { co_return std::invoke(publisher); }();
      publisher_token.messages.push(std::move(message));
      ++i;
    }

    channel.publish_messages(publisher_token);

    while (not co_await termination_has_initialized() and not co_await rate.async_is_ready()) {
    }
    co_await rate.async_reset();
  }

  channel.confirm_termination();

  while (co_await rate.async_is_ready()) {
    std::this_thread::yield();
  }

  co_await rate.async_reset();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  while (channel.subscribers_are_waiting()) {
    channel.increment_publishers_waiting();
    co_await channel.request_permission_to_publish(publisher_token, true);
    channel.decrement_publishers_waiting();

    while (publisher_token.messages.size() < publisher_token.sequences.size()) {
      publisher_token.messages.push(typename channel_t::message_t{});
    }

    channel.publish_messages(publisher_token);
  }
}

/**
 * TODO: Handle many arguments, maybe convert it to a tuple?
 *
 * Generates a coroutine that keeps calling the subscriber_function until it is cancelled.
 *
 * The subscriber_function will be placed at the end of the function network and will be the one
 * that triggers any cancellation events. It depends on a transformer_function or publisher_function to
 * send messages through the multi_channel.
 *
 * When a subscriber_function detects that cancellation is requested, then it will process
 * any messages it has left in the buffer.
 *
 * After the main loop it will terminate the multi_channel and flush out any routines
 * currently waiting on the other end of the multi_channel.
 *
 * @param channel a flow multi_channel that represents a connection between a publisher_function or transformer_function
 *                that is generating data and the subscriber_function that will be receiving the data
 * @param subscriber A subscriber_function is a cancellable function with at least one argument required to call it and
 *                 a specified return type
 * @return A coroutine that continues until the subscriber_function is cancelled
 */
template<typename argument_t>
cppcoro::task<void> spin_subscriber(
  auto& channel,
  cancellable_function<void(argument_t&&)>& subscriber)
{
  subscriber_token<argument_t> subscriber_token{};
  using channel_t = std::decay_t<decltype(channel)>;

  while (not subscriber.is_cancellation_requested()) {
    auto next_message = channel.message_generator(subscriber_token);
    channel.increment_subscribers_waiting();
    auto current_message = co_await next_message.begin();
    channel.decrement_subscribers_waiting();

    while (current_message != next_message.end()) {

      auto& message = *current_message;
      co_await [&]() -> cppcoro::task<void> { co_return subscriber(std::move(message)); }();

      // TODO: Move notfy_message_consumed outside of this while loop and remov eth conditional in the implementation
      channel.notify_message_consumed(subscriber_token);

      channel.increment_subscribers_waiting();
      co_await ++current_message;
      channel.decrement_subscribers_waiting();
    }
  }

  channel.initialize_termination();

  auto channel_needs_flushing = [&]() -> cppcoro::task<bool> {
    static cppcoro::async_mutex mutex;
    auto lock = co_await mutex.scoped_lock_async();
    co_return channel.state() < channel_t::termination_state::publisher_received and channel.is_waiting() and not channel.is_being_flushed();
  };

  while (co_await channel_needs_flushing()) {
    co_await flush<void>(channel, subscriber, subscriber_token);
  }

  channel.finalize_termination();
}

/**
 * TODO: Handle many arguments, maybe convert it to a tuple?
 *
 * Generates a coroutine that keeps calling the transformer_function until the multi_channel its
 * sending messages to is terminated by the subscriber_function or transformer_function on the other end
 *
 * Transformers will live in between a publisher_function and subscriber_function.
 *
 * When a transformer_function detects that the next function in line has terminated the multi_channel, then it will process
 * any messages it has left in the buffer and break out of its loop.
 *
 * After the main loop it will terminate the publisher_function multi_channel and flush out any routines
 * currently waiting on the other end of the publisher_function multi_channel.
 *
 * @param publisher_channel The multi_channel that will have a producing function on the other end
 * @param subscriber_channel The multi_channel that will have a consuming function on the other end
 * @param transformer A subscriber_function is a cancellable function with at least one argument required to call it and
 *                 a specified return type
 * @return A coroutine that continues until the transformer_function is cancelled
 */
template<typename return_t, typename argument_t>
cppcoro::task<void> spin_transformer(
  auto& publisher_channel,
  auto& subscriber_channel,
  cancellable_function<return_t(argument_t&&)>& transformer)
{
  publisher_token<return_t> publisher_token{};
  subscriber_token<argument_t> subscriber_token{};
  using subscriber_channel_t = std::decay_t<decltype(subscriber_channel)>;
  using publisher_channel_t = std::decay_t<decltype(publisher_channel)>;

  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(0, 5000);

  int id = dist(mt);

  subscriber_channel.increment_publishers_waiting();
  if (not co_await subscriber_channel.request_permission_to_publish(publisher_token)) {
    subscriber_channel.decrement_publishers_waiting();
    co_return;
  }
  subscriber_channel.decrement_publishers_waiting();

  auto termination_has_initialized = [&](auto& channel) {
    return static_cast<std::size_t>(channel.state()) >= static_cast<std::size_t>(subscriber_channel_t::termination_state::subscriber_initialized);
  };

  while (not termination_has_initialized(subscriber_channel) and not termination_has_initialized(publisher_channel)) {
    auto next_message = publisher_channel.message_generator(subscriber_token);

    publisher_channel.increment_subscribers_waiting();
    auto current_message = co_await next_message.begin();
    publisher_channel.decrement_subscribers_waiting();

    while (current_message != next_message.end() and not termination_has_initialized(subscriber_channel)) {
      auto& message_to_consume = *current_message;

      auto message_to_publish = co_await [&]() -> cppcoro::task<return_t> {
        co_return std::invoke(transformer, std::move(message_to_consume));
      }();

      publisher_token.messages.push(std::move(message_to_publish));
      publisher_channel.notify_message_consumed(subscriber_token);

      if (publisher_token.messages.size() == publisher_token.sequences.size() and not termination_has_initialized(subscriber_channel)) {
        subscriber_channel.publish_messages(publisher_token);
        subscriber_channel.increment_publishers_waiting();
        if (not subscriber_channel.any_available()) break;
        if (not co_await subscriber_channel.request_permission_to_publish(publisher_token)) {
          subscriber_channel.decrement_publishers_waiting();
          break;
        }
        subscriber_channel.decrement_publishers_waiting();
      }

      if (termination_has_initialized(subscriber_channel)) {
        subscriber_channel.publish_messages(publisher_token);
        break;
      }

      if (termination_has_initialized(publisher_channel)) {
        break;
      }

      publisher_channel.increment_subscribers_waiting();
      co_await ++current_message;
      publisher_channel.decrement_subscribers_waiting();
    }
  }

  subscriber_channel.confirm_termination();

  auto subscriber_channel_terminated = [&]() -> cppcoro::task<bool> {
    static cppcoro::async_mutex mutex;
    auto lock = co_await mutex.scoped_lock_async();
    co_return subscriber_channel.state() >= subscriber_channel_t::termination_state::subscriber_finalized;
  };

  if (not co_await subscriber_channel_terminated()) {
    subscriber_channel.increment_publishers_waiting();
    if (co_await subscriber_channel.request_permission_to_publish(publisher_token)) {
      subscriber_channel.decrement_publishers_waiting();
      while (publisher_token.messages.size() < publisher_token.sequences.size()) {
        publisher_token.messages.push(typename subscriber_channel_t::message_t{});
      }

      subscriber_channel.publish_messages(publisher_token);
      subscriber_channel.increment_publishers_waiting();
    }
    subscriber_channel.decrement_publishers_waiting();
  }

  publisher_channel.initialize_termination();

  auto publisher_channel_needs_flushing = [&]() -> cppcoro::task<bool> {
    static cppcoro::async_mutex mutex;
    auto lock = co_await mutex.scoped_lock_async();
    co_return publisher_channel.state() < publisher_channel_t::termination_state::publisher_received and publisher_channel.is_waiting() and not publisher_channel.is_being_flushed();
  };

  while (co_await publisher_channel_needs_flushing()) {
    co_await flush<return_t>(publisher_channel, transformer, subscriber_token);
  }

  publisher_channel.finalize_termination();


  while (publisher_channel.is_waiting()) {
    co_await flush<return_t>(publisher_channel, transformer, subscriber_token);
  }
}

/**
 * The subscriber_function or transformer_function function will flush out any publisher_function routines
 * in waiting on the other end of the multi_channel
 *
 * @param channel A communication multi_channel between the subscriber_function and publisher_function routines
 * @param routine A subscriber_function or transformer_function function
 * @return A coroutine
 */
template<typename return_t, flow::is_function routine_t>
cppcoro::task<void> flush(auto& channel, routine_t& routine, auto& subscriber_token) requires flow::is_subscriber_function<routine_t> or flow::is_transformer_function<routine_t>
{
  auto needs_flushing = [&]() -> cppcoro::task<bool> {
    static cppcoro::async_mutex mutex;
    auto lock = co_await mutex.scoped_lock_async();
    co_return channel.is_waiting();
  };

  while (co_await needs_flushing()) {
    auto next_message = channel.message_generator(subscriber_token);
    auto current_message = co_await next_message.begin();

    while (current_message != next_message.end()) {
      auto& message = *current_message;

      co_await [&]() -> cppcoro::task<return_t> {
        co_return std::invoke(routine, std::move(message));
      }();

      channel.notify_message_consumed(subscriber_token);

      channel.increment_subscribers_waiting();
      co_await ++current_message;
      channel.decrement_subscribers_waiting();
    }
  }
}

}// namespace flow::detail

#pragma once

#include <functional>

#include <cppcoro/io_service.hpp>
#include <cppcoro/multi_producer_sequencer.hpp>

#include "flow/data_structures/atomic_queue.hpp"
#include "flow/data_structures/static_vector.hpp"
#include "flow/data_structures/string.hpp"
#include "flow/named_types/request.hpp"

namespace flow {
template<typename message_t>
class subscription;

template<typename message_t>
struct publisher {
  using callback_t = std::function<void()>;
  callback_t on_request;

  void publish(message_t&& message) {
    auto& msg = buffer[seq & indexMask];
    msg.id = i;
    msg.timestamp = steady_clock::now();
    msg.data = 123;

    // Publish the message.
//    flow::logging::info("Sending message thread_id: {}", get_thread_id());
    sequencer.publish(seq);
  }

  uint16_t id;
  std::string channel_name;

  cppcoro::io_service& io;
  cppcoro::multi_producer_sequencer<size_t>& sequencer;
};
}// namespace flow

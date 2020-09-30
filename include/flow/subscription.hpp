#pragma once

#include <functional>

#include "flow/data_structures/string.hpp"
#include "flow/data_structures/static_vector.hpp"
#include "flow/data_structures/atomic_queue.hpp"
#include "flow/named_types/request.hpp"

namespace flow {

template <typename message_t>
class publisher;

template<typename message_t, std::size_t publisher_buffer_size = 10, std::size_t message_buffer_size = 64>
class subscription {
public:
  void send_message(message_t msg)
  {
    if (not m_messages.try_push(std::move(msg))) {
      // TOOD: add logging warning here
    }
  }

  bool has_messages() const { return m_messages.was_size() > 0; }

private:
  std::function<void(message_t)> m_message_callback;

  uint16_t m_id;
  flow::string m_channel_name;

  flow::atomic_queue<message_t, message_buffer_size> m_messages;
  flow::static_vector<publisher<message_t>*, publisher_buffer_size> m_publishers;
};

template <typename message_t>
void send_request(subscription<message_t>& sub)
{

}
}// namespace flow
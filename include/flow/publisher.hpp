#pragma once

#include <functional>

#include "flow/data_structures/atomic_queue.hpp"
#include "flow/data_structures/static_vector.hpp"
#include "flow/data_structures/string.hpp"
#include "flow/named_types/request.hpp"

namespace flow {
template<typename message_t>
class subscription;

template<typename message_t, std::size_t subscription_buffer_size = 10, std::size_t request_buffer_size = 64>
struct publisher {
  friend class subscription<message_t>;
  std::function<void(message_t)> callback;

  uint16_t id;
  flow::string channel_name;

  flow::atomic_queue<request_t, request_buffer_size> requests;
  flow::static_vector<subscription<message_t>*, subscription_buffer_size> publishers;
};
}// namespace flow

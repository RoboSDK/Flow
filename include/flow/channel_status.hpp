#pragma once

#include "flow/atomic.hpp"
#include <optional>

namespace flow {
class channel_status {
  struct connection {
    std::size_t quantity{};
    bool active{};
  };

public:
  auto num_publishers() const {
    return std::atomic_ref(m_publishers.quantity);
  }

  auto num_subscribers() const {
    return std::atomic_ref(m_subscribers.quantity);
  }

  auto publishers_are_active() const {
    return std::atomic_ref(m_publishers.active);
  }

  auto subscribers_are_active() const {
    return std::atomic_ref(m_subscribers.active);
  }

private:
  connection m_subscribers;
  connection m_publishers;
};
}// namespace flow
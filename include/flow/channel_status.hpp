#pragma once
#include <optional>

#include "flow/atomic.hpp"
#include "flow/data_structures/id_generator.hpp"

namespace flow {
class channel_status {
  struct connection {
    std::size_t quantity{};
    bool active{};
  };

public:
  auto num_publishers()
  {
    return std::atomic_ref(m_publishers.quantity);
  }

  auto num_subscribers()
  {
    return std::atomic_ref(m_subscribers.quantity);
  }

  auto publishers_are_active()
  {
    return std::atomic_ref(m_publishers.active);
  }

  auto subscribers_are_active()
  {
    return std::atomic_ref(m_subscribers.active);
  }

  auto get_id() {
    return m_make_id();
  }

private:
  connection m_subscribers{.quantity=0, .active=true};
  connection m_publishers{.quantity=0, .active=false};
  id_generator m_make_id;
};

inline std::string to_string(channel_status& s)
{
  std::stringstream ss;
  ss << "channel_status: {";
  const auto add_pair = [&ss](std::string_view item_name, auto&& item, std::string_view delim = ",") {
    ss << delim << " " << item_name << ": " << std::forward<decltype(item)>(item);
  };
  add_pair("num_publishers", s.num_publishers(), "");
  add_pair("publishers_are_active", s.publishers_are_active() ? "true" : "false");
  add_pair("num_subscribers", s.num_subscribers());
  add_pair("subscribers_are_active", s.subscribers_are_active() ? "true" : "false");
  ss << " }";

  return ss.str();
}
}// namespace flow
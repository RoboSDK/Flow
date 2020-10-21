#pragma once
#include <optional>

#include "flow/atomic.hpp"
#include "flow/data_structures/id_generator.hpp"

namespace flow {
class channel_status {
  struct connection {
    std::size_t quantity{};
  };

public:
  channel_status(std::size_t num_publishers, std::size_t num_subscribers)
    : m_publishers{  .quantity = num_publishers },
      m_subscribers{  .quantity = num_subscribers } {}

  auto num_publishers()
  {
    return std::atomic_ref(m_publishers.quantity);
  }

  auto num_subscribers()
  {
    return std::atomic_ref(m_subscribers.quantity);
  }

  auto generate_coroutine_id()
  {
    return m_make_id();
  }

private:
  connection m_publishers{};
  connection m_subscribers{};
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
  add_pair("num_subscribers", s.num_subscribers());
  ss << " }";

  return ss.str();
}
}// namespace flow
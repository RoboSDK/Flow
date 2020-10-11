#pragma once

#include "flow/cancellation.hpp"

namespace flow {
enum class callback_type { publisher, subscription };

struct callback_info {
  std::size_t id;
  callback_type type;
  std::string channel_name;
  std::reference_wrapper<const std::type_info> message_type;
};

/**
 * A callback handle is a handle that has various controls up the communication heirarchy.
 *
 * It is given back when subscribing or publishing to a channel.
 */
template<typename config_t> class callback_handle {
public:
  callback_handle() = default;
  callback_handle(callback_handle&&) = default;
  callback_handle(callback_handle const&) = default;
  callback_handle& operator=(callback_handle&&) = default;
  callback_handle& operator=(callback_handle const&) = default;

  callback_handle(callback_info&& info, cancellation_handle&& ch, volatile auto* program_is_running)
    : m_info(std::move(info)), m_cancel_handle(std::move(ch)), m_program_is_running(program_is_running)
  {
    auto program_state = m_program_is_running->load(std::memory_order_relaxed);

    auto program_after_enabling_callback = program_state;
    program_after_enabling_callback.set(m_info.id);
    m_program_is_running->compare_exchange_strong(program_state, program_after_enabling_callback);
  }

  std::size_t id() const { return m_info.id; };
  callback_type type() const { return m_info.type; }
  std::string channel_name() const { return m_info.channel_name; }
  std::reference_wrapper<const std::type_info> message_info() const { return m_info.message_type; }

  /**
   * Each callback can 'vote' to turn off the application by disabling itself
   *
   * Once all callbacks are disabled, then the program stops
   *
   * A disabled callback then becomes a noop
   */
  void disable()
  {
    auto program_state = m_program_is_running->load(std::memory_order_relaxed);

    auto program_state_after_disabling_this_callback = program_state;
    program_state_after_disabling_this_callback.reset(m_info.id);
    m_program_is_running->compare_exchange_strong(program_state, program_state_after_disabling_this_callback);

    m_cancel_handle.request_cancellation();
  }

  /**
   * Stops all communication between channels and ends the program
   */
  void stop_everything()
  {
    const auto current_state = m_program_is_running->load(std::memory_order_relaxed);
    m_program_is_running->compare_exchange_strong(current_state, false);
  }

private:
  callback_info m_info;
  cancellation_handle m_cancel_handle;
  volatile typename config_t::atomic_bitset_t* m_program_is_running{ nullptr };
};

std::string to_string(callback_type type)
{
  // TODO: use reflection here
  switch (type) {
  case callback_type::publisher:
    return "publisher";
  case callback_type::subscription:
    return "subscription";
  default:
    flow::logging::critical_throw("Invalid callback type passed in to to_string.");
  }

  return "";
}

template<typename config_t> std::string to_string(callback_handle<config_t> const& handle)
{
  std::stringstream ss;
  ss << "callback info {id: " << handle.id() << ", type: " << to_string(handle.type()) << ", channel_name: "
     << handle.channel_name() << ", message: " << handle.message_info().get().name() << "}";
  return ss.str();
}
}// namespace flow
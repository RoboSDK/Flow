#pragma once

#include "flow/consumer.hpp"
#include "flow/producer_impl.hpp"
#include "flow/spinner.hpp"
#include "flow/transformer.hpp"

namespace flow {

template<typename routine_t>
concept routine_concept = spinner_routine<routine_t> or producer_routine<routine_t> or consumer_routine<routine_t> or transformer_routine<routine_t>;

template<flow::routine_concept routine_t, typename... arguments_t>
auto make_routine(arguments_t&&... arguments)
{
  if constexpr (flow::transformer_routine<routine_t>) {
    return flow::make_transformer(std::forward<arguments_t>(arguments)...);
  }
  else if constexpr (flow::consumer_routine<routine_t>) {
    return flow::make_consumer(std::forward<arguments_t>(arguments)...);
  }
  else if constexpr (flow::producer_routine<routine_t>) {
    return flow::make_producer(std::forward<arguments_t>(arguments)...);
  }
  else if constexpr (flow::spinner_routine<routine_t>) {
    return flow::make_spinner(std::forward<arguments_t>(arguments)...);
  }
}
}// namespace flow
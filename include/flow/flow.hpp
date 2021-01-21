#pragma once

#include <iostream>

#include "flow/network.hpp"
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

namespace flow {

template<typename routine_t>
concept routine = spinner_concept<routine_t> or producer_concept<routine_t> or consumer_concept<routine_t> or transformer_concept<routine_t>;

template<typename... routines_t>
concept routines = (routine<routines_t> and ...);

template <typename callable_t>
concept not_network_or_user_routine = not flow::is_network<callable_t> and not flow::are_user_routines<callable_t>;

template<typename... callables_t>
concept not_network_or_user_routines = (not_network_or_user_routine<callables_t> and ...);


void begin()
{
  try {
    auto async_file = spdlog::basic_logger_mt<spdlog::async_factory>("flow", "logs/flow_log.txt");
    async_file->set_pattern("[%H:%M:%S:%F] [%t] %v");
    async_file->set_level(spdlog::level::trace);
    spdlog::set_default_logger(async_file);
  }
  catch (const spdlog::spdlog_ex& ex) {
    std::cout << "Log initialization failed: " << ex.what() << std::endl;
  }
}

template<typename configuration_t>
auto spin(flow::routines auto&&... routines)
{
  using network_t = flow::network<configuration_t>;
  network_t network{};

  auto routines_array = flow::make_mixed_array(std::forward<decltype(routines)>(routines)...);
  std::for_each(std::begin(routines_array), std::end(routines_array), flow::make_visitor([&](auto& routine) {
    network.push(std::move(routine));
  }));

  return cppcoro::sync_wait(network.spin());
}

auto spin(flow::routines auto&&... routines)
{
  return spin<flow::configuration>(std::forward<decltype(routines)>(routines)...);
}

template<typename configuration_t>
auto spin(flow::not_network_or_user_routines auto&&... callables)
{
  using network_t = flow::network<configuration_t>;
  network_t network{};

  auto callables_array = flow::make_mixed_array(std::forward<decltype(callables)>(callables)...);
  std::for_each(std::begin(callables_array), std::end(callables_array), flow::make_visitor([&]<typename return_t, typename... args_t>(return_t (*callable)(args_t...)) {

          if constexpr (not std::is_void_v<return_t> and not (std::is_void_v<args_t> and ...)) {
            auto transformer = flow::make_transformer(callable);
            network.push(std::move(transformer));
          } else if constexpr(std::is_void_v<return_t> and (not std::is_void_v<args_t> and ...)) {
            auto consumer = flow::make_consumer(callable);
            network.push(std::move(consumer));
          } else if constexpr(not std::is_void_v<return_t> and (std::is_void_v<args_t> and ...)) {
            auto producer = flow::make_producer(callable);
            network.push(std::move(producer));
          } else {
            auto spinner = flow::make_spinner(callable);
            network.push(std::move(spinner));
          }
  }));

  return cppcoro::sync_wait(network.spin());
}


auto spin(not_network_or_user_routines auto&&... routines)
{
  return spin<flow::configuration>(std::forward<decltype(routines)>(routines)...);
}

template<typename configuration_t>
auto spin(flow::are_user_routines auto&&... routines)
{
  using network_t = flow::network<configuration_t>;

  network_t network{};

  auto routines_array = flow::make_mixed_array(std::forward<decltype(routines)>(routines)...);
  std::for_each(std::begin(routines_array), std::end(routines_array), flow::make_visitor([&](auto& routine) {
    routine.initialize(network);
  }));

  return cppcoro::sync_wait(network.spin());
}

auto spin(flow::are_user_routines auto&&... routines)
{
  return spin<flow::configuration>(std::forward<decltype(routines)>(routines)...);
}

auto spin(flow::is_network auto&& main_network)
{
  return cppcoro::sync_wait(main_network.spin());
}
}// namespace flow

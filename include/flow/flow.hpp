#pragma once

#include <iostream>

#include "flow/network.hpp"
#include "flow/producer.hpp"
#include "flow/consumer.hpp"
#include "flow/spinner.hpp"
#include "flow/transformer.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

namespace flow {
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

template<typename configuration_t, flow::not_is_network... callables_t>
auto spin(callables_t&&... callables)
{
  using network_t = flow::network<configuration_t>;
  network_t network{};

  auto callables_array = detail::make_mixed_array(std::forward<decltype(callables)>(callables)...);
  std::for_each(std::begin(callables_array), std::end(callables_array), detail::make_visitor([&](auto& callable) {
          using callable_t = decltype(callable);

          if constexpr (flow::callable_transformer<callable_t>) {
            network.push(flow::make_transformer(callable));
          } else if constexpr(flow::callable_consumer<callable_t>) {
            network.push(flow::make_consumer(callable));
          } else if constexpr(flow::callable_producer<callable_t>) {
            network.push(flow::make_producer(callable));
          } else if constexpr(flow::callable_spinner<callable_t>){
            network.push(flow::make_spinner(callable));
          } else if constexpr(flow::routine<callable_t>) {
            network.push(std::move(callable));
          }
  }));

  return cppcoro::sync_wait(network.spin());
}

auto spin(not_is_network auto&&... routines)
{
  return spin<flow::configuration>(std::forward<decltype(routines)>(routines)...);
}

auto spin(flow::is_network auto&& global_network)
{
  return cppcoro::sync_wait(global_network.spin());
}
}// namespace flow

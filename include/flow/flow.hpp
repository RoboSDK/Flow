#pragma once

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "flow/deprecated/message_registry.hpp"
#include "flow/deprecated/system.hpp"

namespace flow {
void begin()
{
  try
  {
    auto async_file = spdlog::basic_logger_mt<spdlog::async_factory>("flow", "logs/flow_log.txt");
    async_file->set_pattern("[%H:%M:%S:%F] [%t] %v");
    async_file->set_level(spdlog::level::trace);
    spdlog::set_default_logger(async_file);
  }
  catch (const spdlog::spdlog_ex& ex)
  {
    std::cout << "Log initialization failed: " << ex.what() << std::endl;
  }
}

}// namespace flow

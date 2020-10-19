#ifndef MODULES_CORE_LOGGING_HPP
#define MODULES_CORE_LOGGING_HPP

#include <spdlog/spdlog.h>
#include <sstream>

namespace flow::logging {
template<typename... Args>
void info(Args &&... args)
{
  spdlog::set_pattern("[%H:%M:%S:%F] [%t] %v");
  spdlog::info(std::forward<Args>(args)...);
}

template<typename... Args>
void trace(Args &&... args)
{
  spdlog::trace(std::forward<Args>(args)...);
}

template<typename... Args>
void debug(Args &&... args)
{
  spdlog::debug(std::forward<Args>(args)...);
}

template<typename... Args>
void warn(Args &&... args)
{
  spdlog::set_pattern("[%H:%M:%S:%F] [%t] %v");
  spdlog::warn(std::forward<Args>(args)...);
}

template<typename... Args>
void error(Args &&... args)
{
  spdlog::set_pattern("[%H:%M:%S:%F] [%t] %v");
  spdlog::error(std::forward<Args>(args)...);
}

template<typename... Args>
void error_throw(Args &&... args)
{
  spdlog::set_pattern("[%H:%M:%S:%F] [%t] %v");
  spdlog::error(std::forward<Args>(args)...);
  throw std::runtime_error("Not critical error detected.");
}

template<typename... Args>
void critical(Args &&... args)
{
  spdlog::set_pattern("[%H:%M:%S:%F] [%t] %v");
  spdlog::critical(std::forward<Args>(args)...);
}

template<typename... Args>
void critical_throw(Args &&... args)
{
  spdlog::set_pattern("[%H:%M:%S:%F] [%t] %v");
  spdlog::critical(std::forward<Args>(args)...);
  throw std::runtime_error("Critical error detected.");
}

}// namespace src::logging

#endif//MODULES_LOGGING_HPP

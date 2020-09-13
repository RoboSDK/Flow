#ifndef MODULES_CORE_LOGGING_HPP
#define MODULES_CORE_LOGGING_HPP

#include <spdlog/spdlog.h>

namespace flow::logging {
template<typename... Args>
void info(Args &&... args)
{
  spdlog::info(std::forward<Args>(args)...);
}

template<typename... Args>
void debug(Args &&... args)
{
  spdlog::debug(std::forward<Args>(args)...);
}

template<typename... Args>
void warn(Args &&... args)
{
  spdlog::warn(std::forward<Args>(args)...);
}

template<typename... Args>
void error(Args &&... args)
{
  spdlog::error(std::forward<Args>(args)...);
}

template<typename... Args>
void critical(Args &&... args)
{
  spdlog::critical(std::forward<Args>(args)...);
}
}// namespace flow::logging

#endif//MODULES_LOGGING_HPP

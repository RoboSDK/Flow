#ifndef MODULES_CORE_LOGGING_HPP
#define MODULES_CORE_LOGGING_HPP

#include <spdlog/spdlog.h>
#include <sstream>

namespace flow::logging {
template<typename... Args>
void info(Args &&... args)
{
  const auto log_with_thread_id = [](auto&& content, auto&&... the_rest)
  {
    std::stringstream ss;
//    ss << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id()) << "] " << content;
    ss << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id()) << "] " << content;
    spdlog::info(ss.str().c_str(), std::forward<decltype(the_rest)>(the_rest)...);
  };

  log_with_thread_id(std::forward<Args>(args)...);
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
void error_throw(Args &&... args)
{
  spdlog::error(std::forward<Args>(args)...);
  throw std::runtime_error("Not critical error detected.");
}

template<typename... Args>
void critical(Args &&... args)
{
  spdlog::critical(std::forward<Args>(args)...);
}

template<typename... Args>
void critical_throw(Args &&... args)
{
  spdlog::critical(std::forward<Args>(args)...);
  throw std::runtime_error("Critical error detected.");
}

}// namespace src::logging

#endif//MODULES_LOGGING_HPP

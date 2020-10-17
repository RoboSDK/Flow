#include "flow/flow.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace flow {
void begin([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
}

void begin()
{
//  auto async_file = spdlog::create_async_nb<spdlog::sinks::basic_file_sink_mt>("async_file_logger", "flow_log.txt");
//  spdlog::set_default_logger(async_file);
//  spdlog::set_pattern("*** [%H:%M:%S %z] [thread %t] %v ***");
}

}// namespace flow

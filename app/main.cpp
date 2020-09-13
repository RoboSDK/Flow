#include <spdlog/spdlog.h>
#include <flow/flow.hpp>

int main(int argc, const char **argv)
{
  flow::begin(argc, argv);

  //Use the default logger (stdout, multi-threaded, colored)
  spdlog::info("Hello, {}!", "World");
}

#include "../modules/core/include/flow.hpp"
#include "../modules/logging/include/logging/logging.hpp"

int main(int argc, const char **argv)
{
  flow::begin(argc, argv);
  flow::logging::info("Hello, World");
}

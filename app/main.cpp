#include <flow/flow.hpp>
#include <flow/logging.hpp>

int main(int argc, const char **argv)
{
  flow::begin(argc, argv);
  flow::logging::info("Hello, World");
}

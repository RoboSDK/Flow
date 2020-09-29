#include <flow/flow.hpp>
#include <flow/system.hpp>

struct Layer {};

int main()
{
  [[maybe_unused]] auto system = flow::make_system<Layer>();

  auto options = flow::options<1024>{};
  [[maybe_unused]] auto system2 = flow::make_system<decltype(options), Layer>();
}
#include <flow/flow.hpp>
#include <flow/system.hpp>

struct Layer {};

int main()
{
  [[maybe_unused]] constexpr auto system = flow::make_system<Layer>();

  [[maybe_unused]] constexpr auto options =  flow::make_options(flow::linker_buffer_size<1024>{});
  [[maybe_unused]] constexpr auto system2 = flow::make_system<Layer>(options);
}
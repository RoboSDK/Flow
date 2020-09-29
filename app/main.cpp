#include <flow/flow.hpp>
#include <flow/system.hpp>
#include <frozen/string.h>
#include <flow/named_types/name.hpp>
struct Layer {};

template <typename callback_t>
struct subscription {
  constexpr subscription(flow::channel_name name, callback_t&& cb) : channel_name(std::move(name.get())), callback(std::forward<callback_t>(cb)) {}
  frozen::string channel_name;
  callback_t callback;
};

int main()
{
  [[maybe_unused]] constexpr subscription sub(flow::channel_name{"fool"}, /*callback*/ [](){});
  [[maybe_unused]] constexpr auto system = flow::make_system<Layer>();

  [[maybe_unused]] constexpr auto options =  flow::make_options(flow::linker_buffer_size<1024>{});
  [[maybe_unused]] constexpr auto system2 = flow::make_system<Layer>(options);
}
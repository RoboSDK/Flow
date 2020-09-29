#include <flow/system.hpp>
#include <frozen/string.h>
#include <flow/named_types/name.hpp>
#include <flow/named_types/message_callback.hpp>
#include <flow/metaprogramming.hpp>
#include "LidarTask.hpp"

struct Layer {};

template <typename callback_t>
struct subscription {
  constexpr subscription(flow::channel_name name, flow::message_callback<callback_t>&& cb) : channel_name(std::move(name.get())), callback(std::move(cb.get())) {}
  frozen::string channel_name;
  callback_t callback;
};

struct lidar_message : flow::message<lidar_message>
{
  LidarData data;
};

int main()
{
  constexpr auto callback = []([[maybe_unused]] lidar_message&& msg) { return 1; };

  [[maybe_unused]] const auto lidar_task = LidarTask();
  [[maybe_unused]] constexpr subscription sub(flow::channel_name{"fool"}, flow::message_callback{callback});


  [[maybe_unused]] constexpr auto system = flow::make_system<Layer>();
  [[maybe_unused]] constexpr auto options =  flow::make_options(flow::linker_buffer_size<1024>{});
  [[maybe_unused]] constexpr auto system2 = flow::make_system<Layer>(options);
}
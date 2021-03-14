#include <flow/flow.hpp>
#include <spdlog/spdlog.h>

#define forward(value) std::forward<decltype(value)>(value)

std::string hello_world()
{
  return "Hello World";
}

void subscribe_hello(std::string&& message)
{
  spdlog::info("Received Message: {}", message);
}

namespace flow {

struct net_tag {};

template <typename... Fs>
struct net_impl : net_tag {
  explicit net_impl(std::tuple<Fs...> &&fs) : functions{std::move(fs)} {}

  net_impl() = default;
  ~net_impl() = default;

  net_impl(net_impl&&) noexcept = default;
  net_impl& operator=(net_impl&&) noexcept = default;

  net_impl(net_impl const&) = delete;
  net_impl& operator=(net_impl const&) = delete;

  std::tuple<Fs...> functions{};
};

template <typename net_t>
concept is_net = std::is_base_of_v<net_tag, net_t>;


template <typename... Ts>
auto make_net(std::tuple<Ts...> &&functions = std::tuple<>{}) {
  return net_impl<Ts...>{std::move(functions)};
}

auto net() {
  return make_net();
}

auto operator|(flow::is_net auto&& net, flow::is_function auto&& f) {
 return make_net(std::tuple_cat(net.functions, std::make_tuple(forward(f))));
}
}

int main()
{
  using namespace std::literals;

  /**
   * The producer hello_world is going to be publishing to the global std::string multi_channel.
   * The consumer subscribe_hello is going to subscribe to the global std::string multi_channel.
   */
  auto network = flow::net() | hello_world | subscribe_hello;

  /**
   * Note: cancellation begins in 1 ms, but cancellation
   * is non-deterministic. 
   */
//  network.cancel_after(1ms);
//
//  flow::spin(std::move(network));
}

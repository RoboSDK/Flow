#include <flow/flow.hpp>
#include <spdlog/spdlog.h>

std::string hello_world()
{
  return "Hello World";
}

void subscribe_hello(std::string&& message)
{
  spdlog::info("Received Message: {}", std::move(message));
}

std::string tr (std::string && m) {
  return std::move(m);
}

int main()
{
  using namespace std::literals;

  /**
   * The producer hello_world is going to be publishing to the global std::string multi_channel.
   * The consumer subscribe_hello is going to subscribe to the global std::string multi_channel.
   */
  auto net = flow::network(flow::chain() | hello_world | tr | tr | subscribe_hello);

  /**
   * Note: cancellation begins in 1 ms, but cancellation
   * is non-deterministic. 
   */
  net.cancel_after(1ms);

  flow::spin(std::move(net));
}

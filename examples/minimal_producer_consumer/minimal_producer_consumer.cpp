#include <flow/flow.hpp>
#include <spdlog/spdlog.h>

std::string hello_world()
{
  return "Hello World";
}

void subscribe_hello(std::string&& message)
{
  spdlog::info("Received Message: {}", message);
}

int main()
{
  using namespace std::literals;

  /**
   * The producer hello_world is going to be publishing to the global std::string multi_channel.
   * The consumer subscribe_hello is going to subscribe to the global std::string multi_channel.
   */
  auto net = flow::network(hello_world, subscribe_hello);

  /**
   * Note: cancellation begins in 1 ms, but cancellation
   * is non-deterministic. 
   */
  net.cancel_after(1ms);

  flow::spin(std::move(net));
}

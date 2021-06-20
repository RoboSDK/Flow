#include <flow/flow.hpp>
#include <spdlog/spdlog.h>

auto time_now = std::chrono::steady_clock::now();

std::string hello_world()
{
  return "Hello World";
}

void subscribe_hello([[maybe_unused]] std::string&& message)
{
  using namespace std::chrono;
  auto time_elapsed = duration_cast<milliseconds>(steady_clock::now() - time_now);
  spdlog::info("Subscribe hello: Received Message: {}, Time Elapsed: {}", std::move(message), time_elapsed.count());
}

int main()
{
  using namespace flow::literals;

  /**
   * The publisher hello_world is going to be publishing to the global std::string multi_channel.
   * The subscriber subscribe_hello is going to subscribe to the global std::string multi_channel.
   */
  auto net = flow::network(flow::chain(1_q_kHz) | hello_world | subscribe_hello);

  /**
   * Note: cancellation begins in 1 ms, but cancellation
   * is non-deterministic. 
   */
  net.cancel_after(10ms);
  flow::spin(std::move(net));
}

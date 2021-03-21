#include <spdlog/spdlog.h>
#include <flow/flow.hpp>

std::string produce_hello_world()
{
  return "Hello World";
}

std::string reverse_string(std::string&& message)
{
  std::ranges::reverse(message);
  return std::move(message);// no RVO here
}

std::size_t hash_string(std::string&& message)
{
  return std::hash<std::string>{}(std::move(message));
}

// For now all messages are passed in by r-value
void receive_hashed_message([[maybe_unused]]std::size_t&& message)
{
  //spdlog::info("Received Message: {}", message);
}

int main()
{
  using namespace flow;
  using namespace std::literals;

  //  TODO: Take frequency as argument to chain()
  auto network = flow::network(flow::chain() | produce_hello_world | reverse_string | hash_string | receive_hashed_message);

  network.cancel_after(100ms);

  // Alternative (and preferred method)
  // auto network_handle = network.handle();
  // network_handle.request_cancellation();

  flow::spin(std::move(network));
}
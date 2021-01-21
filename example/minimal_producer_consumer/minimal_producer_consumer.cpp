#include <flow/flow.hpp>
#include <flow/logging.hpp>

std::string hello_world()
{
  return "Hello World";
}

void receive_message(std::string&& message)
{
  flow::logging::info("Received Message: {}", message);
}

int main()
{
  using namespace std::literals;

  auto network = flow::make_network(hello_world, receive_message);
  network.cancel_after(2s);

  flow::spin(std::move(network));
}
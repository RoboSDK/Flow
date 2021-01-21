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
  flow::spin(hello_world, receive_message);
}
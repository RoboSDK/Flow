#include <flow/flow.hpp>
#include <flow/logging.hpp>

void receive_message(std::string&& message)
{
  flow::logging::info("Received Message: {}", message);
}

int main()
{
  flow::spin([]{ return std::string("Hello World"); }, receive_message);
}
#include <flow/consumer.hpp>
#include <flow/flow.hpp>
#include <flow/logging.hpp>
#include <flow/producer.hpp>

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
  flow::spin(
    flow::make_producer(hello_world),
    flow::make_consumer(receive_message));
}
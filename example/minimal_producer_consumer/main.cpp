#include <flow/logging.hpp>
#include <flow/spin.hpp>
#include <flow/producer.hpp>
#include <flow/consumer.hpp>

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
  auto routine = flow::make_routine(
    flow::make_producer(hello_world),
    flow::make_consumer(receive_message));

  flow::spin(std::move(routine));
}
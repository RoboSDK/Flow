#include <flow/flow.hpp>
#include <flow/logging.hpp>

std::string make_hello_world()
{
  return "Hello World";
}

std::string reverse_string(std::string&& message)
{
  std::ranges::reverse(message);
  return std::move(message); // no RVO here
}

std::size_t hash_string(std::string&& message)
{
  return std::hash<std::string>{}(std::move(message));
}

// For now all messages are passed in by r-value
void receive_hashed_message(std::size_t&& message)
{
  flow::logging::info("Received Message: {}", message);
}

int main()
{
  using namespace flow;
  using namespace std::literals;
  auto hello_world = make_routine<producer>(make_hello_world, "hello_world");
  auto reverser = make_routine<transformer>(reverse_string, "hello_world", "reversed");
  auto hasher = make_routine<transformer>(hash_string, "reversed", "hashed");
  auto receiver = make_routine<consumer>(receive_hashed_message, "hashed");

//   Order doesn't matter here
  auto network = flow::make_network(std::move(hello_world),
                                    std::move(reverser),
                                    std::move(hasher),
                                    std::move(receiver));

  network.cancel_after(1ms);

  // Alternative (and preferred method)
  // auto network_handle = network.handle();
  // network_handle.request_cancellation();

  flow::spin(std::move(network));
}
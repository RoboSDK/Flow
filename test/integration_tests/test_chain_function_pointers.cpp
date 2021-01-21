#include <chrono>

#include <flow/configuration.hpp>
#include <flow/consumer.hpp>
#include <flow/logging.hpp>
#include <flow/network.hpp>
#include <flow/producer.hpp>
#include <flow/transformer.hpp>

#include <cppcoro/sync_wait.hpp>

using namespace std::literals;

int producer()
{
  //  flow::logging::error("producer_routine");
  static int val = 0;
  return val++;
}

int transformer(int&& val)
{
  //  flow::logging::error("transformer_routine");
  return val * 2;
}

void consumer(int&& val)
{
  flow::logging::error("consumer_routine: {}", val);
}

int main()
{
  using network_t [[maybe_unused]] = flow::network<flow::configuration>;

  network_t network = flow::make_network(
    flow::make_producer(producer, "producer"),
    flow::make_transformer(transformer, "producer", "consumer"),
    flow::make_consumer(consumer, "consumer"));

  network.cancel_after(0s);
  cppcoro::sync_wait(network.spin());
}

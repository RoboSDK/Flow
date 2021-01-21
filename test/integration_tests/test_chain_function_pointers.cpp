#include <chrono>

#include <flow/configuration.hpp>
#include <flow/context.hpp>
#include <flow/logging.hpp>
#include <flow/network.hpp>
#include <flow/producer.hpp>
#include <flow/transformer.hpp>
#include <flow/consumer.hpp>

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
  using context_t = flow::context<flow::configuration>;
  using network_t = flow::network<flow::configuration>;

  auto context = std::make_unique<context_t>();
  network_t network{ context.get() };

  flow::producer<int> producer_handle = flow::make_producer(producer, "producer");
  auto transformer_handle = flow::make_transformer(transformer, "producer", "consumer");
  auto consumer_handle = flow::make_consumer(consumer, "consumer");

  network.push(std::move(producer_handle));
  network.push(std::move(transformer_handle));
  network.push(std::move(consumer_handle));

  network.cancel_after(0s);
  cppcoro::sync_wait(network.spin());
}

#include <chrono>

#include <flow/configuration.hpp>
#include <flow/consumer.hpp>
#include <flow/flow.hpp>
#include <flow/logging.hpp>
#include <flow/network.hpp>
#include <flow/producer.hpp>
#include <flow/transformer.hpp>

#include <cppcoro/sync_wait.hpp>

namespace local {
int producer()
{
//    flow::logging::error("producer_function");
  static int val = 0;
  return val++;
}

int transformer(int&& val)
{
//    flow::logging::error("transformer_function");
  return val * 2;
}

void consumer(int&& /*unused*/)
{
//  flow::logging::error("consumer_function: {}", val);
}
}// namespace local

int main()
{
  using namespace flow;
  using namespace std::literals;

  using network_t = flow::network<flow::configuration>;

  network_t network = make_network(
    make_producer(local::producer, "publish_to"),
    make_transformer(local::transformer, "publish_to", "subscribe_to"),
    make_consumer(local::consumer, "subscribe_to"));

  network.cancel_after(1s);
  flow::spin(std::move(network));
}

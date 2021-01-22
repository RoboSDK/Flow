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
  //  flow::logging::error("callable_producer");
  static int val = 0;
  return val++;
}

int transformer(int&& val)
{
  //  flow::logging::error("callable_transformer");
  return val * 2;
}

void consumer(int&& val)
{
  flow::logging::error("callable_consumer: {}", val);
}
}// namespace local

int main()
{
  using namespace flow;
  using namespace std::literals;

  using network_t = flow::network<flow::configuration>;

  network_t network = make_network(
    make_producer(local::producer, options{ .publish_to = "producer" }),
    make_transformer(local::transformer, options{  .publish_to = "consumer" , .subscribe_to = "producer"}),
    make_consumer(local::consumer, options{ .subscribe_to = "consumer" }));

  network.cancel_after(0s);
  flow::spin(std::move(network));
}

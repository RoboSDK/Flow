#include <flow/chain.hpp>
#include <flow/configuration.hpp>
#include <flow/context.hpp>
#include <flow/logging.hpp>

#include <cppcoro/sync_wait.hpp>

int producer()
{
  static int val = 0;
  return val++;
}

int transformer(int&& val)
{
  return val * 2;
}

void consumer(int&& val)
{
  flow::logging::error("CALLBACK consumer: {}", val);
}

int main()
{
  using context_t = flow::context<flow::configuration>;
  using chain_t = flow::chain<flow::configuration>;

  auto context = std::make_unique<context_t>();
  chain_t chain{ context.get() };

  chain.push_producer(producer, "producer");
  chain.push_transformer(transformer, "producer", "consumer");
  chain.push_consumer(consumer, "consumer");

  cppcoro::sync_wait(chain.spin());
}

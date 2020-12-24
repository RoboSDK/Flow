#include <flow/chain.hpp>
#include <flow/configuration.hpp>
#include <flow/logging.hpp>
#include <flow/metaprogramming.hpp>
#include <flow/context.hpp>

#include <cppcoro/sync_wait.hpp>


int producer_function()
{
  flow::logging::debug("producing");
  return 42;
}

void consumer_function(int&& /*unused*/)
{
  flow::logging::debug("consuming");
}

template<typename T>
using traits = flow::metaprogramming::function_traits<T>;

int main()
{
  using context_t = flow::context<flow::configuration>;
  using chain_t = flow::chain<flow::configuration>;

  auto context = std::make_unique<context_t>();
  chain_t chain{context.get()};

  [[maybe_unused]] auto producer = chain.push_producer(producer_function);
  [[maybe_unused]] auto consumer = chain.push_consumer(consumer_function);

  cppcoro::sync_wait(chain.spin());

  delete producer;
  delete consumer;
}

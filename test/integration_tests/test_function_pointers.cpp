#include <chrono>

#include <flow/chain.hpp>
#include <flow/configuration.hpp>
#include <flow/context.hpp>
#include <flow/logging.hpp>

#include <cppcoro/sync_wait.hpp>

using namespace std::literals;

int producer()
{
  static int val = 0;
  flow::logging::error("CALLBACK producer");
  return val++;
}

int doubler(int&& val)
{
  flow::logging::error("CALLBACK transformer");
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

  chain.push(producer, "producer");
  chain.push(doubler, "producer", "doubler");
  chain.push(consumer, "doubler");

  chain.cancel_after(0ms);

  cppcoro::sync_wait(chain.spin());
}

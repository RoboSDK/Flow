#include <chrono>

#include <flow/configuration.hpp>
#include <flow/context.hpp>
#include <flow/logging.hpp>
#include <flow/network.hpp>

#include <cppcoro/sync_wait.hpp>

using namespace std::literals;

int producer()
{
  //  flow::logging::error("producer");
  static int val = 0;
  return val++;
}

int producer2()
{
  //  flow::logging::error("producer2");
  static int val = -100000;
  return val++;
}

int doubler(int&& val)
{
  //  flow::logging::error("transformer");
  return val * 2;
}

void consumer(int&& val)
{
  flow::logging::error("consumer: {}", val);
}

void consumer2(int&& val)
{
  flow::logging::error("consumer2: {}", val);
}

int main()
{
  using context_t = flow::context<flow::configuration>;
  using chain_t = flow::network<flow::configuration>;

  auto context = std::make_unique<context_t>();
  chain_t chain{ context.get() };

  chain.push(producer, "producer");
  chain.push([] {
    return 0;
  },
    "producer");

  chain.push([] {
    return 1;
  },
    "producer");

  //  chain.push(producer2, "producer");
  chain.push(doubler, "producer", "doubler");
  chain.push(consumer, "doubler");
  chain.push(consumer2, "doubler");
  chain.push([](int&& val) {
    flow::logging::info("consumer lambda: {}", val);
  },
    "doubler");

  chain.push([] {
    int foo = 0;
    for (int i = 0; i < 10'000; ++i) {
      foo += i;
    }
    return foo;
  }, "secondary");

  chain.push([](int&& val){
         flow::logging::info("secondary consumer: {}", val);
       }, "secondary");

  chain.cancel_after(0s);
  cppcoro::sync_wait(chain.spin());
}

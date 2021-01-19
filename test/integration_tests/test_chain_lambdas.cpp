#include <chrono>

#include <flow/chain.hpp>
#include <flow/configuration.hpp>
#include <flow/context.hpp>
#include <flow/logging.hpp>

#include <cppcoro/sync_wait.hpp>

using namespace std::literals;

int main()
{
  using context_t = flow::context<flow::configuration>;
  using chain_t = flow::chain<flow::configuration>;

  auto context = std::make_unique<context_t>();
  chain_t chain{ context.get() };

  chain.push([] {
    static int val = 0;
    return val++;
  },
    "producer");

  chain.push([](int&& val) {
    return 2 * val;
  },
    "producer",
    "doubler");

  chain.push([](int&& /*unused*/) {}, "doubler");

  chain.cancel_after(0ms);
  cppcoro::sync_wait(chain.spin());
}
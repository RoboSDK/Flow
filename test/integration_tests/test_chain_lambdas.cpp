#include <chrono>

#include <flow/configuration.hpp>
#include <flow/context.hpp>
#include <flow/logging.hpp>
#include <flow/network.hpp>

#include <cppcoro/sync_wait.hpp>

using namespace std::literals;

int main()
{
  using context_t = flow::context<flow::configuration>;
  using chain_t = flow::network<flow::configuration>;

  auto context = std::make_unique<context_t>();
  [[maybe_unused]] chain_t chain{ context.get() };
//
//  chain.push([] {
//    static int val = 0;
//    return val++;
//  },
//    "producer_routine");
//
//  chain.push([](int&& val) {
//    return 2 * val;
//  },
//    "producer_routine",
//    "doubler");
//
//  chain.push([](int&& /*unused*/) {}, "doubler");
//
//  chain.cancel_after(0ms);
//  cppcoro::sync_wait(chain.spin());
}

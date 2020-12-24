#include <flow/chain.hpp>
#include <flow/configuration.hpp>
#include <flow/logging.hpp>
#include <flow/metaprogramming.hpp>
#include <flow/context.hpp>

#include <cppcoro/sync_wait.hpp>

int main()
{
  using context_t = flow::context<flow::configuration>;
  using chain_t = flow::chain<flow::configuration>;

  auto context = std::make_unique<context_t>();
  chain_t chain{context.get()};

  chain.push_producer([]{
         static int val = 0;
         flow::logging::error("producing");
         return val++;
  });

  chain.push_consumer([](int&& number){
         flow::logging::error("consuming: {}", number);
  });

  cppcoro::sync_wait(chain.spin());
}

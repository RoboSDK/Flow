#include <cppcoro/task.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all.hpp>
#include <cstdio>

using namespace cppcoro;
task<> work(std::size_t num_work) {
  while (num_work) {
    puts("Doing work!");
    --num_work;
  }
  co_return;
}

int main() {
  when_all([]() -> task<> {
    co_await work(2);
  }());
}
#include <catch2/catch.hpp>
#include <cppcoro/schedule_on.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all.hpp>
#include <flow/cancel.hpp>
#include <iostream>
#include <thread>

namespace {
struct Point {
};
}// namespace

template<typename R, typename M>
void test_cancellable()
{
  auto callback = [&]([[maybe_unused]] M msg) -> R {};

  auto handle = flow::cancellation_handle{};
  [[maybe_unused]] auto cancellable = flow::cancellable_callback<R, M>(handle.token(), std::move(callback));

  cancellable.throw_if_cancellation_requested();// should not throw
  REQUIRE_FALSE(cancellable.is_cancellation_requested());

  handle.request_cancellation();
  REQUIRE(cancellable.is_cancellation_requested());
  try {
    cancellable.throw_if_cancellation_requested();
  }
  catch (cppcoro::operation_cancelled const& e) {
    SUCCEED("Cancelled successfully");
    return;
  }

  FAIL("Did not cancel");
}

TEST_CASE("Test cancellable subscription", "[cancellable_subscription]")
{
  test_cancellable<void, Point const&>();// subscription
  test_cancellable<void, Point&>();// publisher
}

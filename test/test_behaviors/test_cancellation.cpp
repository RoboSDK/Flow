#include <catch2/catch.hpp>
#include <flow/cancellation.hpp>


namespace {
struct Point {
};
}// namespace

template<typename R, typename M>
void test_cancellable(M& dummy_message)
{
  bool called_back = false;
  auto callback = [&]([[maybe_unused]] M msg) -> R {
    called_back = true;
  };

  auto handle = flow::cancellation_handle{};
  [[maybe_unused]] auto cancellable = flow::cancellable_callback<R, M>(handle.token(), std::move(callback));

  cancellable.throw_if_cancellation_requested();// should not throw
  REQUIRE_FALSE(cancellable.is_cancellation_requested());

  handle.request_cancellation();
  REQUIRE(cancellable.is_cancellation_requested());

  cancellable(dummy_message); // should call null callback
  REQUIRE(called_back == false);
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
  Point p{};
  test_cancellable<void, Point const&>(p);// subscription
  test_cancellable<void, Point&>(p);// publisher
}

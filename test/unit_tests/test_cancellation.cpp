#include <catch2/catch.hpp>
#include <flow/detail/cancellable_function.hpp>


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

  [[maybe_unused]] auto cancellable = flow::detail::make_shared_cancellable_function(std::move(callback));

  auto handle = cancellable->handle();

  handle.request_cancellation();
  REQUIRE(cancellable->is_cancellation_requested());

  (*cancellable)(dummy_message);// should call null callback
  REQUIRE(called_back == true);
}

TEST_CASE("Test cancellable subscription", "[cancellable_subscription]")
{
  Point p{};
  test_cancellable<void, Point const&>(p);// subscription
  test_cancellable<void, Point&>(p);// publisher
}

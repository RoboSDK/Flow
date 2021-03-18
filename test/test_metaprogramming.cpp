#include <catch2/catch.hpp>
#include <flow/detail/metaprogramming.hpp>

namespace {
struct Foo {
};

template<typename t>
struct Bar {
};

template<typename... ts>
struct Baz {
};
}// namespace

TEST_CASE("Test size_tc compile time size type", "[size_tc]")
{
  using namespace flow::detail::metaprogramming;
  constexpr auto a = size_tc<4>{};
  constexpr auto b = size_tc(a);
  STATIC_REQUIRE(b.data == 4);
}

TEST_CASE("Test the type container metaprogramming helper class", "[type_container]")
{
  using namespace flow::detail::metaprogramming;
  STATIC_REQUIRE(std::is_same_v<type_container<Foo>, type_container<Foo>>);
  STATIC_REQUIRE(std::is_same_v<type_container<Foo>::type, type_container<Foo>::type>);
  STATIC_REQUIRE_FALSE(std::is_same_v<type_container<Foo>, type_container<Bar<Foo>>>);
  STATIC_REQUIRE_FALSE(std::is_same_v<type_container<Foo>::type, type_container<Bar<Foo>>::type>);

  SECTION("copy and move constructors")
  {
    constexpr auto foo = type_container<Foo>{};
    using container_foo_t = decltype(foo);
    using foo_t = decltype(foo)::type;

    [[maybe_unused]] constexpr auto copy = foo;// test copy constructor
    using container_copy_t = decltype(copy);
    using copy_t = decltype(copy)::type;

    STATIC_REQUIRE(std::is_same_v<container_foo_t, container_copy_t>);
    STATIC_REQUIRE(std::is_same_v<foo_t, copy_t>);

    [[maybe_unused]] constexpr auto moved = std::move(foo);// test move constructor
    using container_moved_t = decltype(moved);
    using moved_t = decltype(moved)::type;

    STATIC_REQUIRE(std::is_same_v<container_foo_t, container_moved_t>);
    STATIC_REQUIRE(std::is_same_v<foo_t, moved_t>);
  }

  SECTION("conversion of tuple to type_container")
  {
    [[maybe_unused]] constexpr type_container<Foo> foo = std::tuple<Foo>{};
  }
}

TEST_CASE("Test the size function", "[size]")
{
  using namespace flow::detail::metaprogramming;
  SECTION("Test one")
  {
    STATIC_REQUIRE(size<Foo>() == 1);
    STATIC_REQUIRE(size<Bar<Foo>>() == 1);
    STATIC_REQUIRE(size<Baz<Foo, Bar<Foo>>>() == 1);

    STATIC_REQUIRE(size(std::tuple<Foo>{}) == 1);
    STATIC_REQUIRE(size(std::tuple<Bar<Foo>>{}) == 1);
    STATIC_REQUIRE(size(std::tuple<Baz<Foo, Bar<Foo>>>{}) == 1);
  }

  SECTION("Test two")
  {
    STATIC_REQUIRE(size<Foo, Bar<Foo>>() == 2);
    STATIC_REQUIRE(size<Bar<Foo>, Foo>() == 2);
    STATIC_REQUIRE(size<Baz<Foo, Bar<Foo>>, Foo>() == 2);

    STATIC_REQUIRE(size(std::tuple<Foo, Bar<Foo>>{}) == 2);
    STATIC_REQUIRE(size(std::tuple<Foo, Bar<Foo>>{}) == 2);
    STATIC_REQUIRE(size(std::tuple<Bar<Foo>, Baz<Foo, Bar<Foo>>>{}) == 2);
  }
}

TEST_CASE("Determine whether type containers are empty (e.g. contain no types)", "[empty]")
{
  using namespace flow::detail::metaprogramming;
  SECTION("Test multiple are not empty")
  {
    STATIC_REQUIRE_FALSE(empty<Foo>());
    STATIC_REQUIRE_FALSE(empty<Foo, Foo>());
    STATIC_REQUIRE_FALSE(empty<Foo, Bar<Foo>, Baz<Bar<Foo>, Foo, Foo>>());

    STATIC_REQUIRE_FALSE(empty(std::tuple<Foo>{}));
    STATIC_REQUIRE_FALSE(empty(std::tuple<Foo, Foo>{}));
    STATIC_REQUIRE_FALSE(empty(std::tuple<Foo, Bar<Foo>, Baz<Bar<Foo>, Foo, Foo>>{}));
  }

  SECTION("Test empty cases")
  {
    STATIC_REQUIRE(empty<>());
    STATIC_REQUIRE(empty());
    STATIC_REQUIRE(empty(std::tuple<>{}));
  }
}

TEST_CASE("Testing popping the first or next item from the list", "[pop_front]")
{
  using namespace flow::detail::metaprogramming;
  // popping empty doesn't compiler (which is what we want)
  SECTION("Pop single items")
  {// they should be empty...
    STATIC_REQUIRE(empty(pop_front<Foo>()));
    STATIC_REQUIRE(empty(pop_front<int>()));
    STATIC_REQUIRE(empty(pop_front<Bar<Foo>>()));

    STATIC_REQUIRE(empty(pop_front(std::tuple<int>{})));
    STATIC_REQUIRE(empty(pop_front(std::tuple<Foo>{})));

    STATIC_REQUIRE(empty(pop_front<Foo>(size_tc<1>{})));
  }

  SECTION("Pop size of two")
  {
    {
      constexpr type_container<Foo> foo_t = pop_front<Foo, Foo>();// converts std::tuple
      STATIC_REQUIRE(std::is_same_v<decltype(foo_t)::type, Foo>);
    }
    {
      constexpr type_container<Foo> foo_t = pop_front(std::tuple<Foo, Foo>{});
      STATIC_REQUIRE(std::is_same_v<decltype(foo_t)::type, Foo>);
    }
    {// should pop Bar<int> out
      constexpr type_container<Foo> foo_t = pop_front<Bar<int>, Foo>();
      STATIC_REQUIRE(std::is_same_v<decltype(foo_t)::type, Foo>);
      constexpr type_container<int> int_t = pop_front<Bar<int>, int>();
      STATIC_REQUIRE(std::is_same_v<decltype(int_t)::type, int>);
    }
    {
      constexpr type_container<Foo> foo_t = pop_front(std::tuple<Bar<int>, Foo>{});
      STATIC_REQUIRE(std::is_same_v<decltype(foo_t)::type, Foo>);
      constexpr type_container<int> int_t = pop_front(std::tuple<Bar<int>, int>{});
      STATIC_REQUIRE(std::is_same_v<decltype(int_t)::type, int>);
    }
  }

  SECTION("Pop two from two")
  {
    constexpr std::tuple<> empty_t = pop_front(pop_front<Foo, Foo>());
    STATIC_REQUIRE(empty(empty_t));
  }

  SECTION("Pop two from three")
  {
    {// no support for tuples here
      constexpr type_container<Bar<Foo>> bar_foo_t = pop_front<Foo, Foo, Bar<Foo>>(size_tc<2>{});
      STATIC_REQUIRE(std::is_same_v<decltype(bar_foo_t)::type, Bar<Foo>>);

      constexpr type_container<Foo> foo_t = pop_front<Foo, Bar<Foo>, Foo>(size_tc<2>{});
      STATIC_REQUIRE(std::is_same_v<decltype(foo_t)::type, Foo>);
    }

    {
      constexpr type_container<Bar<Foo>> bar_foo_t = pop_front(pop_front<Foo, Foo, Bar<Foo>>());
      STATIC_REQUIRE(std::is_same_v<decltype(bar_foo_t)::type, Bar<Foo>>);
    }
  }

  SECTION("Pop three from three")
  {
    constexpr std::tuple<> empty_t = pop_front<Foo, Foo, Bar<Foo>>(size_tc<3>{});
    STATIC_REQUIRE(empty(empty_t));
  }
}

TEST_CASE("Test the next function to return the next type from the list. It could also be called front.", "[next]")
{
  using namespace flow::detail::metaprogramming;
  SECTION("test next on single item")// should this be an error?
  {
    constexpr type_container<int> int_t = next<int>();
    STATIC_REQUIRE(std::is_same_v<decltype(int_t)::type, int>);
  }

  SECTION("test next on two items")// should this be an error?
  {
    constexpr type_container<int> int_t = next<int, Foo>();
    STATIC_REQUIRE(std::is_same_v<decltype(int_t)::type, int>);
  }
}

TEST_CASE("Test whether any items passed in are the same type", "[same]")
{
  using namespace flow::detail::metaprogramming;
  SECTION("test same on an empty item")// should this be an error?
  {
    STATIC_REQUIRE(same<>());

    constexpr type_container<int> int_t = next<int, Foo>();
    STATIC_REQUIRE(same<decltype(int_t)::type, int>());
  }

  SECTION("test same on single item")// should this be an error?
  {
    STATIC_REQUIRE(same<Foo>());
    STATIC_REQUIRE(same<Bar<Foo>>());

    STATIC_REQUIRE(same(std::tuple<Foo>{}));
    STATIC_REQUIRE(same(std::tuple<Bar<Foo>>{}));
  }

  SECTION("test same on two items")// should this be an error?
  {
    STATIC_REQUIRE(same<Foo, Foo>());
    STATIC_REQUIRE_FALSE(same<Foo, Baz<Foo, Foo>>());

    STATIC_REQUIRE(same(std::tuple<Foo, Foo>{}));
    STATIC_REQUIRE_FALSE(same(std::tuple<Foo, Baz<Foo, Foo>>{}));
  }

  SECTION("test same on three items")// should this be an error?
  {
    STATIC_REQUIRE(same<Foo, Foo, Foo>());
    STATIC_REQUIRE(same(std::tuple<Foo, Foo, Foo>{}));
    STATIC_REQUIRE_FALSE(same(std::tuple<Bar<Foo>, Foo, Foo>{}));
  }
}

TEST_CASE("Pop back items from a tuple or list of types", "[pop_back]")
{
  using namespace flow::detail::metaprogramming;
  SECTION("Pop back one from one")// should this be an error?
  {
    {
      [[maybe_unused]] constexpr std::tuple<> empty_t = pop_back<Bar<Foo>>();
    }
    {
      [[maybe_unused]] constexpr std::tuple<> empty_t = pop_back(std::tuple<Foo>{});
    }
  }
  SECTION("Pop back one from two")// should this be an error?
  {
    {
      [[maybe_unused]] constexpr type_container<Foo> foo_t = pop_back<Foo, Bar<Foo>>();
    }
    {
      [[maybe_unused]] constexpr std::tuple<Foo> foo_t = pop_back<Foo, Bar<Foo>>();
    }
  }
  SECTION("Pop back two from two")// should this be an error?
  {
    {
      [[maybe_unused]] constexpr std::tuple<> empty_t = pop_back(pop_back<Foo, Bar<Foo>>());
    }
    {
      [[maybe_unused]] constexpr std::tuple<> empty_t = pop_back(pop_back<Foo, Bar<Foo>>());
    }
  }
}

TEST_CASE("Iterate over items", "[for_each]")
{
  using namespace flow::detail::metaprogramming;

  for_each<int, int, int>([]<typename item_t>([[maybe_unused]] type_container<item_t>) {
    STATIC_REQUIRE(same<item_t, int>());
  });

  for_each<Foo, Foo>([]<typename item_t>([[maybe_unused]] type_container<item_t>) {
    STATIC_REQUIRE_FALSE(same<item_t, int>());
  });
}

TEST_CASE("Determine if a tuple contains another type", "[contains]")
{
  using namespace flow::detail::metaprogramming;
  STATIC_REQUIRE(contains<int, /*|*/ int>());
  STATIC_REQUIRE(contains<int, /*|*/ int, int>());
  STATIC_REQUIRE(contains<Foo, /*|*/ int, int, float, double, double, double, double, Bar<Bar<Foo>>, Foo>());

  STATIC_REQUIRE_FALSE(contains<double, /*|*/ int, int>());
  STATIC_REQUIRE_FALSE(contains<double, /*|*/ int, int, Foo, float, Bar<Foo>>());
}

TEST_CASE("Make an set from a list of types", "[make_type_set]")
{
  using namespace flow::detail::metaprogramming;
  [[maybe_unused]] constexpr std::tuple<int, double> set = make_type_set<int, double, double, int>();
  [[maybe_unused]] constexpr std::tuple<Foo> foos = make_type_set<Foo, Foo, Foo, Foo, Foo, Foo, Foo>();
}

void raw_spinner() {}
int raw_publisher() { return 42; }
void raw_subscriber(int /*unused*/) {}
int raw_transformer(int /*unused*/) { return 42; }


TEST_CASE("Test function traits", "[function_traits]")
{
  using namespace flow::detail::metaprogramming;
  SECTION("Test spinner_function")
  {
    auto test_spinner = [](auto&& spinner) {
      STATIC_REQUIRE(function_traits<decltype(spinner)>::arity == 0);
      STATIC_REQUIRE(std::is_void_v<typename function_traits<decltype(spinner)>::return_type>);
    };

    [[maybe_unused]] constexpr auto f = [] {};
    test_spinner(f);

    [[maybe_unused]] const auto stdf = std::function<void()>{};
    test_spinner(stdf);

    test_spinner(raw_spinner);
  }

  SECTION("Test publisher_function")
  {
    auto test_publisher = [](auto&& publisher) {
           STATIC_REQUIRE(function_traits<decltype(publisher)>::arity == 0);
           STATIC_REQUIRE(not std::is_void_v<typename function_traits<decltype(publisher)>::return_type>);
    };

    [[maybe_unused]] constexpr auto f = [] { return 42; };
    test_publisher(f);

    [[maybe_unused]] const auto stdf = std::function<int()>{f};
    test_publisher(stdf);

    test_publisher(raw_publisher);
  }

  SECTION("Test subscriber_function")
  {
    auto test_subscriber = [](auto&& subscriber) {
           STATIC_REQUIRE(function_traits<decltype(subscriber)>::arity > 0);
           STATIC_REQUIRE(std::is_void_v<typename function_traits<decltype(subscriber)>::return_type>);
    };

    [[maybe_unused]] constexpr auto f = [](int /*unused*/) {};
    test_subscriber(f);

    [[maybe_unused]] const auto stdf = std::function<void(int)>{f};
    test_subscriber(stdf);

    test_subscriber(raw_subscriber);
  }

  SECTION("Test doubler")
  {
    auto test_transformer = [](auto&& transformer) {
           STATIC_REQUIRE(function_traits<decltype(transformer)>::arity > 0);
           STATIC_REQUIRE(not std::is_void_v<typename function_traits<decltype(transformer)>::return_type>);
    };

    [[maybe_unused]] constexpr auto f = [](int /*unused*/) { return 42; };
    test_transformer(f);

    [[maybe_unused]] const auto stdf = std::function<int(int)>{f};
    test_transformer(stdf);

    test_transformer(raw_transformer);
  }
}

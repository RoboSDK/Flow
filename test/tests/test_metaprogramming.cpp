#include <catch2/catch.hpp>
#include <flow/metaprogramming.hpp>

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
  using namespace flow::metaprogramming;
  constexpr auto a = size_tc<4>{};
  constexpr auto b = size_tc(a);
  STATIC_REQUIRE(b.data == 4);
}

TEST_CASE("Test the type container metaprogramming helper class", "[type_container]")
{
  using namespace flow::metaprogramming;
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
  using namespace flow::metaprogramming;
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
  using namespace flow::metaprogramming;
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
  using namespace flow::metaprogramming;
  // popping empty doesn't compiler (which is what we want)
  SECTION("Pop single items")
  { // they should be empty...
    STATIC_REQUIRE(empty(pop_front<Foo>()));
    STATIC_REQUIRE(empty(pop_front<int>()));
    STATIC_REQUIRE(empty(pop_front<Bar<Foo>>()));

    STATIC_REQUIRE(empty(pop_front<Foo>(size_tc<1>{})));
    STATIC_REQUIRE(std::is_same_v<pop_front<Foo>(size_tc<0>{})::type, Foo>);
  }

  SECTION("Pop size of two")
  {
    {
      constexpr type_container<Foo> foo_t = pop_front<Foo, Foo>();// converts std::tuple
      STATIC_REQUIRE(std::is_same_v<decltype(foo_t)::type, Foo>);
    }
    {// should pop Bar<int> out
      constexpr type_container<Foo> foo_t = pop_front<Bar<int>, Foo>();
      STATIC_REQUIRE(std::is_same_v<decltype(foo_t)::type, Foo>);
      constexpr type_container<int> int_t = pop_front<Bar<int>, int>();
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
    {
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

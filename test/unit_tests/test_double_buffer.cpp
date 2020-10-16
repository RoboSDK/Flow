#include <catch2/catch.hpp>
#include <cppitertools/range.hpp>
#include <flow/data_structures/double_buffer.hpp>
#include <thread>

TEST_CASE("Test basic reading and writing", "[read_and_write]")
{
  flow::double_buffer<int> val{ 3 };
  REQUIRE(val.read() == 3);

  val.write(20);
  REQUIRE(val.read() == 20);
}
TEST_CASE("Test reading and writing in different threads", "[read_and_write_multi_threaded]")
{
  struct Point {
    double x;
    double y;
  };

  flow::double_buffer<Point> val{3.0, 4.0};

  constexpr std::size_t N = 1'000;

  const auto read_from_val = [&] {
    for ([[maybe_unused]] const auto i : iter::range(N)) {
      [[maybe_unused]] auto read_val = val.read();
    }
  };

  const auto write_to_val = [&] {
         for ([[maybe_unused]] const auto i : iter::range(N)) {
           val.write(Point{static_cast<double>(N), static_cast<double>(N)});
           REQUIRE(val.read().x == static_cast<double>(N));
           REQUIRE(val.read().y == static_cast<double>(N));
         }
  };

  constexpr auto num_readers = 100;
  std::vector<std::thread> readers{};
  readers.reserve(num_readers);
  std::generate_n(std::back_inserter(readers), num_readers, [&]{ return std::thread(read_from_val); });
  std::thread writer(write_to_val);

  for (auto& t : readers) {
    t.join();
  }

  writer.join();
}

#include <bitset>

#include <catch2/catch.hpp>
#include <flow/flow.hpp>
#include <flow/logging.hpp>

namespace {
static constexpr int magic_number = 100;

std::bitset<3> confirm_functions_called{false};

class Sensor {
public:
  int operator()() {
    flow::logging::info("producing data");
    confirm_functions_called[0] = true;
    return m_data;
  }
private:
  int m_data = magic_number;
};

double transform_data(int&& data) {
  flow::logging::info("transforming data");
  REQUIRE(data == magic_number);
  confirm_functions_called[1] = true;
  return 2 * static_cast<double>(std::move(data));
}

void consume_data(double&& data) {
  flow::logging::info("consuming data");
  confirm_functions_called[2] = true;
  static constexpr double doubled_data = static_cast<double>(2 * magic_number);
  REQUIRE(data == doubled_data);
}
}

TEST_CASE("Test network", "[network]")
{
  using namespace std::literals;

  auto network = flow::make_network(Sensor{}, transform_data, consume_data);
  network.cancel_after(1ms);
  flow::spin(std::move(network));
  REQUIRE(confirm_functions_called.all());
}

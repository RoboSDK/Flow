#include <bitset>
#include <atomic>

#include <catch2/catch.hpp>
#include <flow/flow.hpp>


namespace {
static constexpr int magic_number = 100;

std::bitset<3> confirm_functions_called{false};

class Sensor {
public:
  int operator()() {
    confirm_functions_called[0] = true;
    return m_data;
  }
private:
  int m_data = magic_number;
};


std::mutex test_transform_data_mutex;
double transform_data(int&& data) {
  confirm_functions_called[1] = true;
  return 2 * static_cast<double>(std::move(data));
}


std::mutex test_consume_data_mutex;
void consume_data([[maybe_unused]] double&& data) {
  confirm_functions_called[2] = true;
}
}

TEST_CASE("Test network", "[network]")
{
  using namespace std::literals;

  auto network = flow::network(flow::chain () | Sensor{} | transform_data | consume_data);
  network.cancel_after(1ms);
  flow::spin(std::move(network));

  std::this_thread::sleep_for(100ms);
  REQUIRE(confirm_functions_called.all());
}

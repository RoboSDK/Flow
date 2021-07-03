#include <random>

#include <flow/flow.hpp>
#include <spdlog/spdlog.h>

static std::random_device m_random_device{};
static std::mt19937 m_random_engine{ m_random_device() };
static std::uniform_int_distribution<int> m_distribution{ 1, 100 };

struct Data {
  int data{};
};

class Sensor {
public:
  Data operator()()
  {
    auto data = m_distribution(m_random_engine);
    spdlog::info("sensor:{}", data);
    return Data{ data };
  }

  std::string publish_to() { return "sensor"; }

private:
};

Data low_pass_filter(Data&& msg)
{
  spdlog::info("low:{}", msg.data);
  static int limit = 30;
  msg.data = std::min(msg.data, limit);
  return std::move(msg);
}

Data high_pass_filter(Data&& msg)
{
  spdlog::info("high: {}", msg.data);
  static int limit = 70;
  msg.data = std::max(msg.data, limit);
  return std::move(msg);
}

auto consume_data(Data&& data)
{
  spdlog::info("consuming data: {}", data.data);
}


int main()
{
  using namespace flow::literals;

  auto sensor = flow::chain(20_q_Hz) | Sensor{};
  auto low_pass = flow::chain() | flow::transform(low_pass_filter, "sensor") | consume_data;
  auto high_pass = flow::chain() | flow::transform(high_pass_filter, "sensor") | consume_data;

  auto network = flow::network(std::move(sensor), std::move(low_pass), std::move(high_pass));

  network.cancel_after(200ms);
  flow::spin(std::move(network));
}

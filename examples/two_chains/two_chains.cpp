#include <random>

#include <flow/flow.hpp>
#include <spdlog/spdlog.h>

static std::random_device m_random_device{};
static std::mt19937 m_random_engine{ m_random_device() };
static std::uniform_int_distribution<int> m_distribution{ 1, 100 };

class Sensor {
public:
  int operator()()
  {
    spdlog::info("sensor");
    return m_distribution(m_random_engine);
  }

  std::string publish_to() { return "sensor"; }

private:
};

int low_pass_filter(int&& data)
{
  spdlog::info("low");
  static int limit = 30;
  return std::min(data, limit);
}

int high_pass_filter(int&& data)
{
  spdlog::info("high");
  static int limit = 70;
  return std::max(data, limit);
}

void consume_data(int&& data)
{
  spdlog::info("consuming data: {}", data);
}


int main()
{
  using namespace std::literals;

  auto low_pass = flow::chain() | flow::transformer(low_pass_filter, "sensor") | consume_data;
  auto high_pass = flow::chain() | flow::transformer(high_pass_filter, "sensor") | consume_data;

  auto network = flow::network(Sensor{}, std::move(low_pass), std::move(high_pass));

  network.cancel_after(1ms);
  flow::spin(std::move(network));
}

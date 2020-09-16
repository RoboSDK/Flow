#include <iostream>
#include <variant>
#include <array>
#include <chrono>
#include <thread>
#include <flow/flow.hpp>
#include <logging/logging.hpp>

namespace t0 {

template<typename WorkerType>
struct Worker
{
  [[nodiscard]] decltype(auto) spin() const
  {
    flow::logging::info("lol what");
    auto const& worker = static_cast<const WorkerType &>(*this);
    return worker.spin();
  }
};

struct HappyWorker final : public Worker<HappyWorker>
{
  [[nodiscard]] int spin() const// NOLINT
  {
    std::cout << "happy worker spinning "
              << m_data << "\n";
    return 0;
  };

private:
  int m_data = 0;
};

struct DiligentWorker final : public Worker<DiligentWorker>
{
  [[nodiscard]] int spin() const// NOLINT
  {
    std::cout << "diligent worker spinning "
              << m_data << "\n";
    return 0;
  };

private:
  int m_data = 0;
};

template<typename... Ts>
struct Spinner
{
  [[nodiscard]] int spin() const
  {
    std::cout << "spinning"
              << " size: " << M_WORKERS.size() << std::endl;
    static auto constexpr SLEEP_TIME = std::chrono::milliseconds(1000);
    std::this_thread::sleep_for(SLEEP_TIME);
    for (const auto &worker : M_WORKERS) {
      std::visit([](const auto &w) {
        decltype(auto) base = static_cast<const Worker<std::decay_t<decltype(w)>>&>(w);
        return base.spin();
      }, worker);
    }

    return m_data;
  }

  using WorkerType = std::variant<Ts...>;
  static constexpr std::size_t NUM_WORKERS{ sizeof...(Ts) };
  using WorkerTypes = std::array<WorkerType, NUM_WORKERS>;

  template<std::size_t... Indices>
  static constexpr WorkerTypes make_workers(std::index_sequence<Indices...> /*unused*/)
  {
    return { std::variant_alternative_t<Indices, WorkerType>{}... };
  }

private:
  static constexpr WorkerTypes M_WORKERS = make_workers(std::make_index_sequence<std::variant_size_v<WorkerType>>{});
  int m_data = 0;
};
}// namespace t0

int main(int argc, const char **argv)// NOLINT
{
  flow::begin(argc, argv);
  flow::logging::info("Hello, World");
  t0::Spinner<t0::DiligentWorker, t0::HappyWorker> spinner{};
  return spinner.spin();
}

//#include <flow/system.hpp>
//#include <flow/registry.hpp>
//
//#include "LidarData.hpp"
//#include "SensorLayer.hpp"
//#include "TransformLayer.hpp"
//
//int main()
//{
//  using namespace app;
//  auto mes_registry = flow::make_registry<LidarData>();
//  const auto make_registry = [&]<typename... message_ts>(flow::message_registry<message_ts...> & /*unused*/)
//  {
//         return flow::registry<message_ts...>{};
//  };
//
//  auto registry = make_registry(mes_registry);
//
//  cppcoro::static_thread_pool tp;
//  cppcoro::io_service io;
//  cppcoro::sequence_barrier<std::size_t> barrier;
//  cppcoro::multi_producer_sequencer<std::size_t> sequencer{barrier, 64};
//
//  std::vector<cppcoro::task<void>> tasks{};
//  for (auto& [_, channel] : registry.m_channels) {
//    auto conc_channel = std::get<flow::channel<app::LidarData>>(channel);
//    [[maybe_unused]] auto task = conc_channel.open_communications(tp, io, barrier, sequencer);
////    [[maybe_unused]] auto task = std::visit([&](auto& c1) { return c1.open_communications(tp, io, barrier, sequencer); }, channel);
////    tasks.push_back(std::move(task));
//  }
////  auto system = flow::make_system<TransformLayer, SensorLayer>();
////  flow::spin(system, registry);
//}

#include <flow/algorithms/to_string.hpp>
#include <flow/data_structures/double_buffer.hpp>
#include <iostream>
#include <thread>

flow::double_buffer<std::array<double, 10'000>> buf{};

void read() {
  for (int i = 0; i < 50'000; ++i) {
    buf.read();
  }
}

void write() {
  std::array<double, 10'000> data{};
  std::fill(std::begin(data), std::end(data), 1);
  for (int i = 0; i < 50'000; ++i) {
    buf.write(std::move(data));
  }
}

int main () {

  std::thread t1(read);
  std::thread t2(write);
  t2.join();
  t1.join();

  auto data = buf.read();
  std::cout << flow::to_string(std::begin(data), std::end(data)) << std::endl;
}

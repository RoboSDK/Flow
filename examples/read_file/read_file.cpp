#include "file_handle.hpp"
#include <flow/flow.hpp>
#include <fstream>
#include <iostream>
#include <optional>
#include <spdlog/spdlog.h>

void subscribe_line(std::string&& line)
{
   spdlog::info("line: {}", line);
}

int main(int /* argc */, char** argv)
{
  using namespace flow::literals;

  std::ifstream file;
  flow::FileHandle file_handle(&file, argv[1]);

  /**
   * The publisher hello_world is going to be publishing to the global std::string multi_channel.
   * The subscriber subscribe_hello is going to subscribe to the global std::string multi_channel.
   */
  auto net = flow::network(flow::chain(1_q_kHz) | file_handle | subscribe_line);

  net.cancel_after(5ms); // reaquest_cancellation broken

  flow::spin(std::move(net));
}

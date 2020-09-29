#pragma once

#include <flow/task.hpp>
#include <random>

#include <vector>
#include <chrono>

struct LidarData {
  std::vector<double> points;
};

struct LidarDriver {
  static constexpr std::size_t N = 10'000;
  LidarData drive()
  {
    LidarData data{ .points = std::vector<double>(N) };
    std::generate_n(std::back_inserter(data.points), N, [this] { return dist(rd); });
    return data;
  }
  std::random_device rd;
  std::uniform_real_distribution<double> dist{ 0, 9 };
};

class LidarTask final : public flow::task<LidarTask> {
public:
  void begin() {}
  LidarData spin()
  {
    current_points = m_driver.drive();
    return current_points;
  };

  void end() {}
  LidarDriver m_driver;
  LidarData current_points;
};

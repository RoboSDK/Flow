#pragma once

namespace mock {
static constexpr std::size_t lidar_data_capacity = 100;
static constexpr std::pair lidar_driver_distribution_range = {0, 9};
static constexpr std::size_t total_messages = 1'000;
static constexpr auto lidar_channel_name = "lidar_data";
}

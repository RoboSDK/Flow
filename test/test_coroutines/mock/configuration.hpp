#pragma once

namespace mock {
namespace lidar {
static constexpr std::size_t data_capacity = 100;
static constexpr std::pair driver_distribution_range = {0, 10};
static constexpr std::size_t total_messages = 1'000;
static constexpr auto channel_name = "lidar_data";

static constexpr std::size_t num_message_ids = 100;
static constexpr std::size_t num_lidar_publishers = 10;
static constexpr std::size_t num_lidar_subscribers = 10;
}
}

#pragma once

namespace mock {
namespace defaults {
static constexpr std::size_t message_capacity = 100;
static constexpr std::size_t total_messages = 1'000;

static constexpr std::size_t num_publishers = 10;
static constexpr std::size_t num_tasks = 10;
static constexpr std::size_t num_sequences = num_publishers * total_messages;
}
}

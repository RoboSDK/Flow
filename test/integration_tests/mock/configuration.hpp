#pragma once

namespace mock {
struct configuration {
  struct defaults {
    static constexpr std::size_t message_capacity = 100;
    static constexpr std::size_t total_messages = 1'000;

    static constexpr std::size_t num_publishers = 10;
    static constexpr std::size_t num_subscriptions = 10;
    static constexpr std::size_t num_sequences = num_publishers * total_messages;
  };

  struct single_producer_single_consumer {
    static constexpr std::size_t message_capacity = defaults::message_capacity;
    static constexpr std::size_t total_messages = defaults::total_messages;

    static constexpr std::size_t num_publishers = 1;
    static constexpr std::size_t num_subscriptions = 1;
    static constexpr std::size_t num_sequences = num_publishers * total_messages;
  };

  struct multi_producer_single_consumer {
    static constexpr std::size_t message_capacity = defaults::message_capacity;
    static constexpr std::size_t total_messages = defaults::total_messages;

    static constexpr std::size_t num_publishers = 100;
    static constexpr std::size_t num_subscriptions = 1;
    static constexpr std::size_t num_sequences = num_publishers * total_messages;
  };

  struct single_producer_multi_consumer {
    static constexpr std::size_t message_capacity = defaults::message_capacity;
    static constexpr std::size_t total_messages = defaults::total_messages;

    static constexpr std::size_t num_publishers = 1;
    static constexpr std::size_t num_subscriptions = 100;
    static constexpr std::size_t num_sequences = num_publishers * total_messages;
  };

  struct multi_producer_multi_consumer {
    static constexpr std::size_t message_capacity = defaults::message_capacity;
    static constexpr std::size_t total_messages = defaults::total_messages;

    static constexpr std::size_t num_publishers = 100;
    static constexpr std::size_t num_subscriptions = 100;
    static constexpr std::size_t num_sequences = num_publishers * total_messages;
  };
};
}// namespace mock

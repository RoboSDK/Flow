#pragma once

#include <cstdint>

template <typename request_callback>
class subscriber {
  uint64_t channel_id;
  const scheduler&  channel_owner;
};
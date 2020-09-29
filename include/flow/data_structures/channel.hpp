#pragma once

#include <functional>
#include <frozen/string.h>

namespace flow {

template <typename message_t>
class channel {

  using RequestCallback = std::function<void()>;

  struct subscription {
    using CompletionCallback = std::function<void(message_t &&)>;
    CompletionCallback callback;

    frozen::string channel_name;
  };
};
}
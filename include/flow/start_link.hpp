#pragma once

#include <cppcoro/task.hpp>

namespace flow {

template <typename R>
struct start_link {

  cppcoro::task<R> spin() {

  }


};
}
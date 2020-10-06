#pragma once
#include <typeinfo>

namespace flow {

template<typename T>
std::size_t hash()
{
  return typeid(T).hash_code();
}
}// namespace flow

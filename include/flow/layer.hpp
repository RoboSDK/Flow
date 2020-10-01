#pragma once

#include <NamedType/named_type.hpp>
#include <cppcoro/when_all.hpp>

#include "flow/data_structures/static_vector.hpp"

namespace flow {

template<typename concrete_layer_t>
class layer : fluent::crtp<concrete_layer_t, layer>
{
public:
  void begin(auto& registry) { this->underlying().init(registry); }
};
}// namespace flow
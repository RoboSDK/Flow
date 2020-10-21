#pragma once

#include <NamedType/crtp.hpp>
#include <concepts>

namespace flow {
template<typename layer_t>
struct layer : fluent::crtp<layer_t, layer> {
  void begin(auto& registry) { this->underlying().begin(registry); }
  void end() { this->underlying().end(); }
};

template<typename layer_t>
void begin(flow::layer<layer_t>& layer, auto& registry)
{
  layer.begin(registry);
}

template<typename layer_t>
void end(flow::layer<layer_t>& layer)
{
  layer.end();
}
}// namespace flow
#pragma once

#include <NamedType/crtp.hpp>

namespace flow{

/**
 * The task interface
 * @tparam concrete_task The concrete task type that will be implementing this interface
 *
 * example: class planning : task<planning>
 *
 * A tasks purpose is to do one thing, and do it well.
 */
template <typename concrete_task>
class task : fluent::crtp<concrete_task, task>
{
public:
  void begin();
  decltype(auto) spin();
  void end();
};

// Implementation
template<typename concrete_task>
void task<concrete_task>::begin()
{
  this->underlying()->begin();
}

template<typename concrete_task>
decltype(auto) task<concrete_task>::spin()
{
  this->underlying()->spin();
}

template<typename concrete_task>
void task<concrete_task>::end()
{
  this->underlying()->spin();
}
}
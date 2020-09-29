#pragma once

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
class task
{
public:
  decltype(auto) begin();
  decltype(auto) spin();
  decltype(auto) end();
};

// Implementation
template<typename concrete_task>
decltype(auto) task<concrete_task>::begin()
{
  static_cast<concrete_task &>(*this)->begin();
}

template<typename concrete_task>
decltype(auto) task<concrete_task>::spin()
{
  static_cast<concrete_task const&>(*this)->spin();
}

template<typename concrete_task>
decltype(auto) task<concrete_task>::end()
{
  static_cast<concrete_task &>(*this)->spin();
}
}
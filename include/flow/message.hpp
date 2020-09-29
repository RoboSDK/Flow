#pragma once

#include <NamedType/crtp.hpp>

namespace flow {
template<typename concrete_message>
class message : fluent::crtp<concrete_message, message> {};
}// namespace flow
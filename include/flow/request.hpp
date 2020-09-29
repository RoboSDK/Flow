#pragma once

#include <NamedType/crtp.hpp>

namespace flow {
template<typename concrete_request>
class request : fluent::crtp<concrete_request, request> {};
}// namespace flow

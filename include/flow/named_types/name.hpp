#pragma once

#include <NamedType/named_type.hpp>
#include <frozen/string.h>

namespace flow {
using channel_name = fluent::NamedType<frozen::string, struct name_tag>;
}

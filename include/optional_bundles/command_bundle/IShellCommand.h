#pragma once

#include <fmt/format.h>
#include <flow/Common.h>
#include <flow/Service.h>

namespace flow {
    class IShellCommand : virtual public IService {
    public:
        
        static constexpr InterfaceVersion version = InterfaceVersion{1, 0, 0};
    protected:
        virtual ~IShellCommand() = default;
    };
}
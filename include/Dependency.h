#pragma once

#include <flow/Common.h>

namespace flow {
    struct Dependency {
        flow_CONSTEXPR Dependency(uint64_t _interfaceNameHash, InterfaceVersion _interfaceVersion, bool _required) noexcept : interfaceNameHash(_interfaceNameHash), interfaceVersion(_interfaceVersion), required(_required) {}
        flow_CONSTEXPR Dependency(const Dependency &other) noexcept = default;
        flow_CONSTEXPR Dependency(Dependency &&other) noexcept = default;
        flow_CONSTEXPR Dependency& operator=(const Dependency &other) noexcept = default;
        flow_CONSTEXPR Dependency& operator=(Dependency &&other) noexcept = default;
        flow_CONSTEXPR bool operator==(const Dependency &other) const noexcept {
            return interfaceNameHash == other.interfaceNameHash && interfaceVersion == other.interfaceVersion && required == other.required;
        }

        uint64_t interfaceNameHash;
        InterfaceVersion interfaceVersion;
        bool required;
    };
}
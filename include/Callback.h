#pragma once

#include <flow/Common.h>

namespace flow {
    struct CallbackKey {
        uint64_t id;
        uint64_t type;

        bool operator==(const CallbackKey &other) const {
            return id == other.id && type == other.type;
        }
    };

    struct InterfaceKey {
        uint64_t typenameHash;
        InterfaceVersion version;

        bool operator==(const InterfaceKey &other) const {
            return typenameHash == other.typenameHash && version == other.version;
        }
    };
}

namespace std {
    template <>
    struct hash<flow::CallbackKey> {
        std::size_t operator()(const flow::CallbackKey& k) const {
            return std::hash<uint64_t>()(k.id) ^ std::hash<uint64_t>()(k.type);
        }
    };
    template <>
    struct hash<flow::InterfaceKey> {
        std::size_t operator()(const flow::InterfaceKey& k) const {
            return std::hash<uint64_t>()(k.typenameHash) ^ std::hash<uint64_t>()(k.version.major) ^ std::hash<uint64_t>()(k.version.minor) ^ std::hash<uint64_t>()(k.version.patch);
        }
    };
}
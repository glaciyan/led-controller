#pragma once

#include <cstdint>
#include <array>

namespace util
{
    consteval uint8_t hexCharToInt(char c) {
        return (c >= '0' && c <= '9') ? (c - '0') :
            (c >= 'A' && c <= 'F') ? (c - 'A' + 10) :
            (c >= 'a' && c <= 'f') ? (c - 'a' + 10) : 0;
    }

    // Helper constexpr function to convert two hex characters to a byte
    consteval uint8_t hexPairToByte(char high, char low) {
        return (hexCharToInt(high) << 4) | hexCharToInt(low);
    }
}

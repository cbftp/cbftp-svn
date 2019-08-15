#pragma once

#include <vector>

namespace Core {

typedef std::vector<unsigned char> BinaryData;

enum class AddressFamily
{
    NONE,
    IPV4,
    IPV6,
    IPV4_IPV6,
};

} // namespace Core

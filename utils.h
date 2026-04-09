#pragma once

#include <functional>

namespace Utils {
    // hash combining algorithm from boost::hash_combine
    template<typename T>
    void hashCombine(size_t& seed, const T& val) {
        seed ^= std::hash<T>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}

#ifndef DECAPROTO_ENCODER_H
#define DECAPROTO_ENCODER_H

#include <cstring>

#include "decaproto/message.h"
#include "decaproto/stream/stream.h"

namespace decaproto {

// TODO: We'll be able to replace them by std::bit_cast
template <typename SRC_T, typename DST_T>
inline DST_T MemcpyCast(SRC_T src) {
    return (DST_T)src;
}

template <>
inline uint32_t MemcpyCast<bool, uint32_t>(bool src) {
    return src ? 1 : 0;
}

template <>
inline uint64_t MemcpyCast<double, uint64_t>(double src) {
    uint64_t dst = 0;
    std::memcpy(&dst, &src, sizeof(src));
    return dst;
}

template <>
inline uint32_t MemcpyCast<float, uint32_t>(float src) {
    uint32_t dst = 0;
    std::memcpy(&dst, &src, sizeof(src));
    return dst;
}

inline size_t ComputeEncodedVarintSize(uint64_t value) {
    size_t size = 0;
    do {
        value >>= 7;
        size++;
    } while (value != 0);
    return size;
}

}  // namespace decaproto

#endif  // DECAPROTO_ENCODER_H

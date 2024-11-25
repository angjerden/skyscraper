#ifndef COMMON_ENDIAN_H
#define COMMON_ENDIAN_H

#include "types.h"

inline uint16 READ_LE_UINT16(const void *ptr) {
    struct Unaligned16 { uint16 val; } __attribute__ ((__packed__, __may_alias__));
    return ((const Unaligned16 *)ptr)->val;
}

inline uint16 READ_BE_UINT16(const void *ptr) {
    const uint8 *b = (const uint8 *)ptr;
    return (b[0] << 8) | b[1];
}
inline uint32 READ_BE_UINT32(const void *ptr) {
    const uint8 *b = (const uint8 *)ptr;
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | (b[3]);
}

#endif
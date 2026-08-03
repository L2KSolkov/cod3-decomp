// ============================================================================
// bdBitOperations — bit manipulation (COD3 release, ea: 0x89EB90-0x89EC70)
// ============================================================================

#pragma once

#include <math.h>

typedef unsigned int bdUInt;

// COD3 uses plain functions, not a class with static methods.
// These match the decompiled code exactly.

inline bdUInt bdNextPowerOf2(bdUInt v) {
    bdUInt v2 = v;
    v2 |= v2 >> 1;
    v2 |= v2 >> 2;
    v2 |= v2 >> 4;
    v2 |= v2 >> 8;
    v2 |= v2 >> 16;
    v2 &= ~(v2 >> 1);  // mask out everything but MSB
    if (v2 != v)
        v2 <<= 1;
    return v2;
}

inline bdUInt bdHighBitNumber(bdUInt v) {
    bdUInt i = (v & 0xFFFF0000) ? 16 : 0;
    if ((v >>= i) & 0xFF00) { i |= 8; v >>= 8; }
    if (v & 0xF0)           { i |= 4; v >>= 4; }
    if (v & 0xC)            { i |= 2; v >>= 2; }
    return (i | (v >> 1));
}

inline double bdGetNaNValue64() { return NAN; }  // ea: 0x89EC50
inline float  bdGetNaNValue32() { return NAN; }  // ea: 0x89EC70

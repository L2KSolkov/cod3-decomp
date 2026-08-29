// ============================================================================
// bdBitOperations — bit manipulation (COD3 release, ea: 0x89EB90-0x89EC70)
// ============================================================================

#pragma once

#include <math.h>

typedef unsigned int bdUInt;

class bdBitOperations {
public:
    static bdUInt nextPowerOf2(bdUInt v);
    static bdUInt highBitNumber(bdUInt v);
    static double getNaNValue64();
    static float getNaNValue32();
};

inline bdUInt bdNextPowerOf2(bdUInt v) {
    return bdBitOperations::nextPowerOf2(v);
}

inline bdUInt bdHighBitNumber(bdUInt v) {
    return bdBitOperations::highBitNumber(v);
}

inline double bdGetNaNValue64() {
    return bdBitOperations::getNaNValue64();
}

inline float bdGetNaNValue32() {
    return bdBitOperations::getNaNValue32();
}

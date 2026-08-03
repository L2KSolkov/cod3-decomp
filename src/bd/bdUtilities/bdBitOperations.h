// ============================================================================
// bdBitOperations — bit manipulation utilities (Demonware 2.0, ported from 2.3.4)
// Verified against COD3 ea: 0x89EB90 (nextPowerOf2), 0x89EBD0 (highBitNumber)
// ============================================================================

#pragma once

typedef unsigned int bdUInt;

#define BD_IS_POWER_OF_2(n) (((n) & ((n)-1)) == 0)
#define BD_NEXT_MULTIPLE_OF_M(n, m)  (((n) + ((m)-1)) & (~((m)-1)))
#define BD_PREVIOUS_MULTIPLE_OF_M(n, m) ((n) & ~((m)-1))
#define BD_IS_MULTIPLE_OF_M(n, m) ((n & ((m)-1)) == 0)
#define BD_NUM_BITS_TO_NUM_BYTES(n) (((n)>>3) + (((n) & 0x7)?1:0))

class bdBitOperations {
public:
    static bdUInt nextPowerOf2(bdUInt v) {
        bdUInt v2 = v;
        v2 |= v2 >> 1;
        v2 |= v2 >> 2;
        v2 |= v2 >> 4;
        v2 |= v2 >> 8;
        v2 |= v2 >> 16;
        v2 &= ~(v2 >> 1);
        if (v2 != v) v2 <<= 1;
        return v2;
    }

    static bdUInt highBitNumber(bdUInt v) {
        bdUInt i = (v & 0xFFFF0000) ? 16 : 0;
        if ((v >>= i) & 0xFF00) { i |= 8; v >>= 8; }
        if (v & 0xF0)           { i |= 4; v >>= 4; }
        if (v & 0xC)            { i |= 2; v >>= 2; }
        return (i | (v >> 1));
    }
};

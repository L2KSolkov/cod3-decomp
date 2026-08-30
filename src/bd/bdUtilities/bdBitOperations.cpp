#include "bd/bdUtilities/bdBitOperations.h"
#include "bd/bd_types.h"

extern const char defaultFileName[];

// ea: 0x0089EB90
bdUInt bdBitOperations::nextPowerOf2(bdUInt v) {
    bdUInt v2 = v;
    v2 |= v2 >> 1;
    v2 |= v2 >> 2;
    v2 |= v2 >> 4;
    v2 |= v2 >> 8;
    v2 |= v2 >> 16;
    v2 &= ~(v2 >> 1);
    if (v2 != v)
        v2 <<= 1;
    return v2;
}

// ea: 0x0089EBD0
bdUInt bdBitOperations::highBitNumber(bdUInt v) {
    do {
        if (v == 0) {
            bdMessageProxy proxy(
                ".\\bdUtilities\\bdBitOperations.cpp",
                "unsigned int __cdecl bdBitOperations::highBitNumber(unsigned int)",
                0x29u, "dw/err");
            proxy.log(defaultFileName,
                      "bdBitOperations::highBitNumber, no bits set, so cannot find highest.");
        }
    } while (g_assertFalse);
    bdUInt i = (v & 0xFFFF0000) ? 16 : 0;
    if ((v >>= i) & 0xFF00) { i |= 8; v >>= 8; }
    if (v & 0xF0)           { i |= 4; v >>= 4; }
    if (v & 0xC)            { i |= 2; v >>= 2; }
    return i | (v >> 1);
}

// ea: 0x0089EC50
double bdBitOperations::getNaNValue64() {
    return NAN;
}

// ea: 0x0089EC70
float bdBitOperations::getNaNValue32() {
    return NAN;
}

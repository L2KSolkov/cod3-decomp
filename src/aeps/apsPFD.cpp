// ============================================================================
// apsPFD.cpp — particle field descriptor (4 non-inline funcs + data).
// Source: c:\cod\code\tl\aeps\source\apsPFD.cpp
// Verified against IDA (aeps_xboxr:apsPFD.o):
//   GetFieldByteSize  @0x811FE0
//   RecomputeOffsets  @0x812020
//   ctor              @0x8120A0
//   AddFields         @0x8120C0
//   sElementSizes     @0x1239190 (data, uchar[32])
// ============================================================================
#include "apsPFD.h"

#include <intrin.h>

// tl_system.o (tl_xboxr, ported)
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// Data globals owned by apsPFD.o
// ============================================================================
unsigned char apsPFD::sElementSizes[32] = {
    12, 4, 4, 12, 4, 16, 4, 4,
    4,  4, 4, 4,  4, 4,  16, 4,
    4,  12, 12, 12, 12, 4,  4, 4,
    4,  4, 4, 4,  12, 12, 4, 4,
};

// ============================================================================
// apsPFD::GetFieldByteSize — byte size of a particle field.
// ea: 0x811FE0
// ============================================================================
unsigned char apsPFD::GetFieldByteSize(apsEPFDField field) {
    if (field < apsPFDField_FirstUserField)
        return apsPFD::sElementSizes[field];
    if (_tlAssert("source/apsPFD.cpp", 70, "field < apsPFDField_FirstUserField",
                  "INVALID call to apsPFD::GetFieldByteSize()"))
        __debugbreak();
    return apsPFD::sElementSizes[field];
}

// ============================================================================
// apsPFD::RecomputeOffsets — recompute per-field byte offsets + stride.
// ea: 0x812020
// ============================================================================
void apsPFD::RecomputeOffsets() {
    int curOffset = 0;
    for (int i = 0; i < 32; ++i) {
        if (((1 << i) & this->mFields) != 0) {
            if (curOffset + apsPFD::sElementSizes[i] >= 0x100 &&
                _tlAssert("source/apsPFD.cpp", 83,
                          "(curOffset) + sElementSizes[i] < 256",
                          "Need more than 256 bytes in particle format!"))
                __debugbreak();
            this->mOffsets[i] = (unsigned char)curOffset;
            curOffset += apsPFD::sElementSizes[i];
        } else {
            this->mOffsets[i] = 0;
        }
    }
    this->mStride = (curOffset + 15) & 0xFFFFFFF0;
}

// ============================================================================
// apsPFD::apsPFD — ea: 0x8120A0
// ============================================================================
apsPFD::apsPFD() {
    mFields = 0;
    RecomputeOffsets();
}

// ============================================================================
// apsPFD::AddFields — add fields and recompute offsets.
// ea: 0x8120C0
// ============================================================================
void apsPFD::AddFields(unsigned int iFields) {
    this->mFields |= iFields;
    RecomputeOffsets();
}

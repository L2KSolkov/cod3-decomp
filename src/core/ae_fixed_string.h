// ============================================================================
// AE Fixed String — fixed-capacity string with inline buffer
// Source: c:/cod/code/ae/core/ (used by PoolAllocator::ReportTotals)
// Size: CAPACITY bytes (buffer + length byte)
// ============================================================================

#pragma once

#include <stdint.h>

template <int CAPACITY, typename CHAR = char>
struct ae_fixed_string {
    CHAR            mBuff[CAPACITY - 1];  // +0x00
    unsigned char   mLength;              // +CAPACITY - 1

    ae_fixed_string() : mLength(0) {
        mBuff[0] = 0;
    }

    const CHAR* c_str() const { return mBuff; }
    int length() const { return mLength; }
    static int capacity() { return CAPACITY - 1; }
};

// ae_formatted_string — extends ae_fixed_string via inheritance
template <int CAPACITY, typename CHAR = char>
struct ae_formatted_string : public ae_fixed_string<CAPACITY, CHAR> {
    // Formatted constructor (printf-style)
    // Actual implementation calls AeStringSupport::Concat-like logic
    void format(const CHAR* fmt, ...);
};

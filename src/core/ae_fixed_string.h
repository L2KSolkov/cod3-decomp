// ============================================================================
// AE Fixed String — fixed-capacity string with inline buffer
// Source: c:/cod/code/ae/core/ (used by PoolAllocator::ReportTotals)
// Size: CAPACITY bytes (buffer + length byte)
// ============================================================================

#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

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

    // Append a C string (ae_fixed_string::operator+=, COMDAT).
    ae_fixed_string& operator+=(const CHAR* rhs) {
        const char* src = (const char*)rhs;
        int l = (int)mLength;
        int r = 0;
        while (src[r] != 0 && l < CAPACITY - 1) {
            mBuff[l++] = (CHAR)src[r++];
        }
        mLength = (unsigned char)l;
        mBuff[l] = 0;
        return *this;
    }

    ae_fixed_string& operator+=(const char* rhs) {
        int l = (int)mLength;
        int r = 0;
        while (rhs[r] != 0 && l < CAPACITY - 1) {
            mBuff[l++] = (CHAR)rhs[r++];
        }
        mLength = (unsigned char)l;
        mBuff[l] = 0;
        return *this;
    }
};

// ae_formatted_string — extends ae_fixed_string via inheritance
template <int CAPACITY, typename CHAR = char>
struct ae_formatted_string : public ae_fixed_string<CAPACITY, CHAR> {
    // Formatted constructor (printf-style, COMDAT in ShaderCommon.o).
    ae_formatted_string(const CHAR* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int l;
        if (sizeof(CHAR) == 2) {
            l = _vsnwprintf((wchar_t*)this->mBuff, CAPACITY - 1, (const wchar_t*)fmt, args);
        } else {
            l = vsnprintf((char*)this->mBuff, CAPACITY - 1, (const char*)fmt, args);
        }
        va_end(args);
        if (l < 0)
            l = 0;
        if (l > CAPACITY - 1)
            l = CAPACITY - 1;
        this->mLength = (unsigned char)l;
    }

    // Narrow-format overload (formats into a wide buffer via %ls-style values).
    ae_formatted_string(const char* fmt, ...) {
        char tmp[512];
        va_list args;
        va_start(args, fmt);
        int l = vsnprintf(tmp, sizeof(tmp), fmt, args);
        va_end(args);
        if (l < 0)
            l = 0;
        if (l > CAPACITY - 1)
            l = CAPACITY - 1;
        for (int i = 0; i < l; ++i)
            this->mBuff[i] = (CHAR)(unsigned char)tmp[i];
        this->mBuff[l] = 0;
        this->mLength = (unsigned char)l;
    }
};

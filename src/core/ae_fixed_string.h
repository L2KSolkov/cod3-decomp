// ============================================================================
// AE Fixed String — fixed-capacity string with inline buffer
// Source: c:/cod/code/ae/core/ (used by PoolAllocator::ReportTotals)
// Size: CAPACITY bytes (buffer + length byte)
// Layout verified from IDA: mBuff holds CAPACITY-1 bytes, mLength (word-accessed
// for wide instantiations) sits at byte CAPACITY-2; sizeof == CAPACITY.
// ============================================================================

#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

template <int CAPACITY, typename CHAR = char>
struct ae_fixed_string {
    CHAR            mBuff[(CAPACITY - 1) / sizeof(CHAR)];  // +0x00
    unsigned char   mLength;                               // +sizeof(mBuff)

    ae_fixed_string() : mLength(0) {
        mBuff[0] = 0;
    }

    // ea: 0x4E4890 — CStrToAeStr + length store
    ae_fixed_string(const char* txt) {
        char* d = (char*)mBuff;
        int cap = capacity();
        int i = 0;
        if (txt != nullptr && *txt != 0) {
            while (txt[i] != 0 && i < cap) {
                d[i] = txt[i];
                ++i;
            }
        } else {
            d[0] = 0;
        }
        d[i] = 0;
        mLength = (unsigned char)i;
    }

    const CHAR* c_str() const { return mBuff; }
    int length() const { return mLength; }
    static int capacity() {
        return (CAPACITY - 1) / sizeof(CHAR) * sizeof(CHAR);
    }

    // ea: 0x4E0AC0 — byte search, returns index or -1
    int find(char c, int start_pos) const {
        const char* buf = (const char*)mBuff;
        int len = mLength;
        for (int i = start_pos; i < len; ++i) {
            if (buf[i] == c)
                return i;
        }
        return -1;
    }

    // ea: 0x4E48D0 — SubStr into dst, then store output length
    ae_fixed_string& substr(ae_fixed_string& dst, int begin, int len) const {
        int cap = capacity();
        int out = len;
        if (begin < cap) {
            int end = cap - 1;
            if (end >= begin + len)
                end = begin + len;
            const char* src = (const char*)mBuff;
            char* d = (char*)dst.mBuff;
            out = 0;
            for (int i = begin; i < end; ++i) {
                char c = src[i];
                if (c == 0)
                    break;
                d[out++] = c;
            }
            d[out] = 0;
        }
        dst.mLength = (unsigned char)out;
        return dst;
    }

    // Append a C string (ae_fixed_string::operator+=, COMDAT).
    ae_fixed_string& operator+=(const CHAR* rhs) {
        const char* src = (const char*)rhs;
        int l = (int)mLength;
        int r = 0;
        while (src[r] != 0 && l < (CAPACITY - 1) / sizeof(CHAR)) {
            mBuff[l++] = (CHAR)src[r++];
        }
        mLength = (unsigned char)l;
        mBuff[l] = 0;
        return *this;
    }

    ae_fixed_string& operator+=(const char* rhs) {
        int l = (int)mLength;
        int r = 0;
        while (rhs[r] != 0 && l < (CAPACITY - 1) / sizeof(CHAR)) {
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
            l = _vsnwprintf((wchar_t*)this->mBuff,
                            (CAPACITY - 1) / sizeof(CHAR),
                            (const wchar_t*)fmt, args);
        } else {
            l = vsnprintf((char*)this->mBuff, this->capacity(),
                          (const char*)fmt, args);
        }
        va_end(args);
        if (l < 0)
            l = 0;
        if (l > (CAPACITY - 1) / sizeof(CHAR))
            l = (CAPACITY - 1) / sizeof(CHAR);
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
        if (l > (CAPACITY - 1) / sizeof(CHAR))
            l = (CAPACITY - 1) / sizeof(CHAR);
        for (int i = 0; i < l; ++i)
            this->mBuff[i] = (CHAR)(unsigned char)tmp[i];
        this->mBuff[l] = 0;
        this->mLength = (unsigned char)l;
    }
};

// ============================================================================
// AE String Support — utility string operations
// Source: c:/cod/code/ae/core/ae_fixed_string.cpp (line refs: 57, 58)
// ea: 0x7BED90 (ae_stricmpn), 0x7BEDF0-0x7BF160 (various)
// ============================================================================

#include <stdint.h>
#include <cstring>

// Portable debug break
#ifdef _MSC_VER
#define COD3_BREAK() __debugbreak()
#else
#define COD3_BREAK() __builtin_debugtrap()
#endif

// Forward
namespace AeAssert {
    enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10 };
    extern ECoderId gCurrentAuthor;
    extern const char* gCurrentFile;
    extern int   gCurrentLine;
    extern const char* gCurrentExpr;
    bool IsIgnored();
    bool Assert(const char* msg, ...);
}

// ============================================================================
// ae_stricmpn — case-insensitive string compare (max n chars)
// ea: 0x7BED90
// ============================================================================
int ae_stricmpn(const char* s1, const char* s2, int n) {
    while (n-- > 0) {
        int c1 = (unsigned char)*s1++;
        int c2 = (unsigned char)*s2++;

        if (c1 != c2) {
            if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
            if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
            if (c1 != c2) return c1 - c2;
        }
        if (!c1) return 0;
    }
    return 0;
}

namespace AeStringSupport {

// ============================================================================
// StrStrEqu — compare two length-prefixed strings (requires 4-byte alignment)
// ea: 0x7BEDF0
// ============================================================================
bool StrStrEqu(const char* lhsBuff, int lhsLen, const char* rhsBuff, int rhsLen) {
    // Assert: both buffers are 4-byte aligned
    if (((uintptr_t)lhsBuff & 3) != 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "ae_fixed_string.cpp";
        AeAssert::gCurrentLine = 57;
        AeAssert::gCurrentExpr = "!((size_t)lhsBuff & 0x3)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("string must be 4 byte aligned"))
            COD3_BREAK();
    }
    if (((uintptr_t)rhsBuff & 3) != 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "ae_fixed_string.cpp";
        AeAssert::gCurrentLine = 58;
        AeAssert::gCurrentExpr = "!((size_t)rhsBuff & 0x3)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("string must be 4 byte aligned"))
            COD3_BREAK();
    }

    if (lhsLen != rhsLen && rhsLen >= 0)
        return false;

    // Compare DWORDs first (4 bytes at a time)
    int dwordCount = lhsLen / 4;
    const int* p1 = (const int*)lhsBuff;
    const int* p2 = (const int*)rhsBuff;
    while (dwordCount > 0) {
        if (*p1 != *p2)
            return false;
        ++p1; ++p2;
        --dwordCount;
    }

    // Compare remaining bytes
    int rem = lhsLen % 4;
    const char* cp1 = (const char*)p1;
    const char* cp2 = (const char*)p2;
    while (rem > 0) {
        if (*cp1 != *cp2)
            return false;
        ++cp1; ++cp2;
        --rem;
    }
    return true;
}

// ============================================================================
// StrCStrEqu — compare length-prefixed string with C-string
// ea: 0x7BEF00
// ============================================================================
bool StrCStrEqu(const char* lhsBuff, int lhsLen, const char* rhsBuff, int rhsLen) {
    if (lhsLen != rhsLen && rhsLen >= 0)
        return false;

    int len = lhsLen + 1;
    if (lhsLen == -1) return true;

    int offset = (int)(rhsBuff - lhsBuff);
    for (int i = 0; i < len; ++i) {
        if (lhsBuff[i] != lhsBuff[i + offset])
            return false;
    }
    return true;
}

// ============================================================================
// CStrToAeStr — copy C-string into ae_fixed_string format
// Output: *dstLen = length, dst[0..len] = copied chars (null-terminated)
// ea: 0x7BEF50
// ============================================================================
void CStrToAeStr(char* dst, int* const dstLen, int dstCapacity, const char* src) {
    if (!src || !*src) {
        *dst = 0;
        *dstLen = 0;
        return;
    }

    int i = 0;
    char* d = dst;
    while (src[i] && i < dstCapacity) {
        *d++ = src[i];
        ++i;
    }
    *d = 0;
    *dstLen = i;
}

// ============================================================================
// Concat — append C-string to ae_fixed_string
// ea: 0x7BEFA0
// ============================================================================
void Concat(char* dst, int* const dstLen, int dstCapacity, const char* src) {
    if (!src) return;

    int len = *dstLen;
    char* d = dst + len;

    if (*src) {
        int i = 0;
        while (src[i]) {
            if (len >= dstCapacity) break;
            *d++ = src[i];
            ++len;
            ++i;
        }
    }
    *d = 0;
    *dstLen = len;
}

// ============================================================================
// AeStrCopy — copy string with length tracking
// ea: 0x7BEFE0
// ============================================================================
void AeStrCopy(char* dst, int* dstLen, int dstCapacity, const char* src, int srcLen) {
    int copyLen = srcLen;
    if (dstCapacity < copyLen) copyLen = dstCapacity;

    *dstLen = copyLen;

    // Copy DWORDs
    int dwordCount = copyLen / 4;
    while (dwordCount > 0) {
        *(int*)dst = *(const int*)src;
        dst += 4; src += 4;
        --dwordCount;
    }

    // Copy remaining bytes
    int rem = *dstLen % 4;
    while (rem > 0) {
        *dst = *src;
        ++dst;
        --rem;
    }
    *dst = 0;
}

// ============================================================================
// SubStr — extract substring
// ea: 0x7BF040
// ============================================================================
void SubStr(char* dst, int* dstLen, const char* src, int begin, int count, int srcLen) {
    if (begin >= srcLen) {
        *dstLen = 0;
        *dst = 0;
        return;
    }

    int end = srcLen - 1;
    if (end >= begin + count)
        end = begin + count;

    int out = 0;
    for (int i = begin; i < end; ++i) {
        char c = src[i];
        if (!c) break;
        dst[out++] = c;
    }
    *dstLen = out;
    dst[out] = 0;
}

// ============================================================================
// Split — split a length-prefixed string at the first delimiter
// ea: 0x7BF0A0
// ============================================================================
void Split(char* dstBuff, int* dstLen, char* srcBuff, int* srcLen,
           char splitOn, int capacity) {
    int i;
    for (i = 0; i < *srcLen; ++i) {
        if (srcBuff[i] == splitOn)
            break;
    }

    char tmpbuff[512];
    char* v = srcBuff;
    if (i >= *srcLen) {
        ptrdiff_t offset = dstBuff - srcBuff;
        char c;
        do {
            c = *v;
            v[offset] = *v;
            ++v;
        } while (c != 0);
        *dstLen = *srcLen;
        *srcBuff = 0;
        *srcLen = 0;
    } else {
        ptrdiff_t offset = tmpbuff - srcBuff;
        char c;
        v = srcBuff;
        do {
            c = *v;
            v[offset] = *v;
            ++v;
        } while (c != 0);
        SubStr(dstBuff, dstLen, tmpbuff, 0, i, capacity);
        SubStr(srcBuff, srcLen, tmpbuff, i + 1,
               *srcLen - (i + 1), capacity);
    }
}

// ============================================================================
// GetFileName — extract filename from path
// ea: 0x7BF160
// ============================================================================
void GetFileName(char* dst, int* dstLen, const char* path, int pathLen, bool includeExt) {
    // Walk backwards to find last slash/backslash
    int slashPos = -1;
    for (int i = pathLen - 1; i >= 0; --i) {
        char c = path[i];
        if (c == '\\' || c == '/') {
            slashPos = i;
            break;
        }
    }

    const char* start = path + slashPos + 1;
    int len = pathLen - (slashPos + 1);

    if (!includeExt) {
        // Strip extension
        for (int i = len - 1; i >= 0; --i) {
            if (start[i] == '.') {
                len = i;
                break;
            }
        }
    }

    // Copy
    for (int i = 0; i < len; ++i)
        dst[i] = start[i];
    dst[len] = 0;
    *dstLen = len;
}

} // namespace AeStringSupport

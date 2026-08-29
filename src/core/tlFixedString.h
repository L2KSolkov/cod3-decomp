// ============================================================================
// tlFixedString - 32-byte hashed fixed string (core/tl_xboxr).
// Size: 32 bytes (0x20). Hash: lowercased char + 33*hash (DJB2-style), folded
// through tolower() for letters. Verified against tl_xboxr / core.o COMDATs:
//   ctor()             ea: 0x4B53F0
//   copy ctor          ea: 0x4EAAE0
//   operator=          ea: 0x4B5420
//   operator==         ea: 0x4EAB30
//   operator!=         ea: 0x4EAB70
//   c_str/value/hash   ea: 0x4B5470 / 0x4A53E0 / 0x5E9A60
// ============================================================================

#pragma once

#include <ctype.h>
#include <stdint.h>
#include <string.h>

// Declared `class` (not struct) to match the original binary's MSVC mangling
// (`ABVtlFixedString` etc. use V for class types).
class tlFixedString {
public:
    uint32_t hash;          // +0x00 - DJB2-style hash
    char     str[28];       // +0x04 - inline string (28 bytes = 32 total)

    operator const char*() const;

    operator char*();


    tlFixedString() : hash(0) {
        memset(str, 0, sizeof(str));
    }

    // ea: 0x4A53F0
    tlFixedString(const char* s) : hash(0) {
        memset(str, 0, sizeof(str));
        if (s != NULL) {
            char* d = str;
            int i = 0;
            for (const char* p = s; *p != 0; ++p) {
                char c = *p;
                // Release passes the sign-extended char to the CRT helpers.
                if (isalpha((int)c))
                    c = (char)tolower((int)c);
                hash = (uint32_t)c + 33u * hash;
                if (i < 27) {
                    *d++ = c;
                    ++i;
                }
            }
            str[27] = 0;
        }
    }

    tlFixedString(const tlFixedString& s) {
        *this = s;
    }

    // ea: 0x4B5420
    const tlFixedString& operator=(const tlFixedString& rhs) {
        memcpy(this, &rhs, 32);
        return *this;
    }

    // ea: 0x4EAB30
    bool operator==(const tlFixedString& rhs) const {
        const uint32_t* a = (const uint32_t*)this;
        const uint32_t* b = (const uint32_t*)&rhs;
        for (int i = 0; i < 8; i++) {
            if (a[i] != b[i])
                return false;
        }
        return true;
    }

    // ea: 0x4EAB70
    bool operator!=(const tlFixedString& rhs) const {
        return !(*this == rhs);
    }

    // ea: 0x4B5470
    char* c_str() { return str; }
    // ea: 0x4EABB0
    const char* c_str() const;

    // ea: 0x4A53E0
    uint32_t* value();  // ?value@tlFixedString@@QAEPAIXZ (g.o 0x4A53E0)
    // ea: 0x4B53E0
    const uint32_t* value() const { return (const uint32_t*)this; }

    unsigned int GetHash() const;

    int Order(const tlFixedString& rhs) const;  // ?Order@tlFixedString@@QBEHABV1@@Z (streamer.o 0x6638E0)
};

// ea: 0x00539D30
inline tlFixedString::operator const char*() const
{
    return str;
}

// ea: 0x005E9A50
inline tlFixedString::operator char*()
{
    return str;
}

// ea: 0x005E9A60
inline unsigned int tlFixedString::GetHash() const
{
    return hash;
}

static_assert(sizeof(tlFixedString) == 0x20, "tlFixedString size mismatch");

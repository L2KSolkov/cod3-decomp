// ============================================================================
// tlFixedString - 32-byte hashed fixed string (core/tl_xboxr).
// Size: 32 bytes (0x20). Hash: lowercased char + 33*hash (DJB2-style), folded
// through tolower() for letters. Verified against tl_xboxr / core.o COMDATs:
//   ctor(const char*)  ea: 0x4A53F0
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

    tlFixedString() {
        memset(this, 0, 32);
    }

    tlFixedString(const char* s) {
        memset(this, 0, 32);
        if (s != NULL) {
            char* d = str;
            int i = 0;
            for (const char* p = s; *p != 0; ++p) {
                char c = *p;
                if (isalpha((unsigned char)c))
                    c = (char)tolower((unsigned char)c);
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

    tlFixedString& operator=(const tlFixedString& rhs) {
        memcpy(this, &rhs, 32);
        return *this;
    }

    bool operator==(const tlFixedString& rhs) const {
        const uint32_t* a = (const uint32_t*)this;
        const uint32_t* b = (const uint32_t*)&rhs;
        for (int i = 0; i < 8; i++) {
            if (a[i] != b[i])
                return false;
        }
        return true;
    }

    bool operator!=(const tlFixedString& rhs) const {
        return !(*this == rhs);
    }

    char* c_str() { return str; }
    const char* c_str() const { return str; }

    uint32_t* value() { return (uint32_t*)this; }
    const uint32_t* value() const { return (const uint32_t*)this; }

    unsigned int GetHash() const { return hash; }

    int Order(const tlFixedString& rhs) const;  // ?Order@tlFixedString@@QBEHABV1@@Z (streamer.o 0x6638E0)
};
static_assert(sizeof(tlFixedString) == 0x20, "tlFixedString size mismatch");

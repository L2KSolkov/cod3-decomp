// ============================================================================
// tlFixedString stub — 32-byte hashed string type
// Full implementation in tl_xboxr (not yet ported)
// Size: 32 bytes (0x20)
// ============================================================================

#pragma once

#include <stdint.h>

struct tlFixedString {
    uint32_t hash;          // +0x00 — DJB2 hash
    char     str[28];       // +0x04 — inline string (28 bytes = 32 total)

    tlFixedString() : hash(0) { str[0] = 0; }
    tlFixedString(const char* s) : hash(0) {
        // Simplified — real implementation has proper hash
        int i = 0;
        while (s[i] && i < 27) { str[i] = s[i]; ++i; }
        str[i] = 0;
    }
};
static_assert(sizeof(tlFixedString) == 0x20, "tlFixedString size mismatch");

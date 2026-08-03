// ============================================================================
// AE Hash — simple string hashing
// ea: 0x7BF220
// ============================================================================

#include <stdint.h>

// Forward declaration — real implementation in tlFixedString
struct tlFixedString {
    unsigned int hash;
    // ... rest of type TBD

    tlFixedString(const char* str);
};

unsigned int AeHash(const char* str) {
    if (!str) return 0;
    tlFixedString tmp(str);
    return tmp.hash;
}

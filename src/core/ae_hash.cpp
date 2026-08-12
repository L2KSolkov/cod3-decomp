// ============================================================================
// AE Hash — simple string hashing
// ea: 0x7BF220
// ============================================================================

#include <stdint.h>
#include "core/tlFixedString.h"

unsigned int AeHash(const char* str) {
    if (!str) return 0;
    tlFixedString tmp(str);
    return tmp.hash;
}

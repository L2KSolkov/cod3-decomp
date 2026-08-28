// ============================================================================
// AE Hash -- simple string hashing
// ea: 0x7BF220 -- direct DJB2 with lowercase folding, matches tlFixedString ctor
// ============================================================================

#include "core/tlFixedString.h"
#include <stdint.h>

// ea: 0x7BF220
extern "C" unsigned int AeHash(const char* str) {
    if (str == 0)
        return 0;
    tlFixedString value(str);
    return value.hash;
}

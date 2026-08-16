// ============================================================================
// AE Hash -- simple string hashing
// ea: 0x7BF220 -- direct DJB2 with lowercase folding, matches tlFixedString ctor
// ============================================================================

#include <ctype.h>
#include <stdint.h>

unsigned int AeHash(const char* str) {
    if (!str) return 0;
    unsigned int hash = 0;
    for (const char* p = str; *p; ++p) {
        unsigned char c = (unsigned char)*p;
        if (isalpha(c))
            c = (unsigned char)tolower(c);
        hash = (unsigned int)c + 33u * hash;
    }
    return hash;
}

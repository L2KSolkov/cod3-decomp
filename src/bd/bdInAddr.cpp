// ============================================================================
// bdInAddr.cpp - IPv4 address (5 funcs).
// Source: bdPlatform:bdInAddr.obj
// Verified against IDA (release decompilation).
// ============================================================================

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <winsock2.h>
#else
  #include <arpa/inet.h>
#endif

#include "bd/bd_types.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// bdInAddr::fromString - ea: 0x8B5E60
// ============================================================================
unsigned int bdInAddr::fromString(const char* cp) {
    unsigned int result = inet_addr(cp);
    inUn.m_iaddr = result;
    return result;
}

// ============================================================================
// bdInAddr::toString - ea: 0x8B5E80
// COD3 calls XNetInAddrToString; the Win32 equivalent is a dotted-quad print.
// ============================================================================
unsigned int bdInAddr::toString(char* const pchBuf, int cchBuf) const {
    struct in_addr a;
    a.s_addr = inUn.m_iaddr;
    const char* s = inet_ntoa(a);
    if (s == NULL)
        return 0;
    unsigned int len = (unsigned int)strlen(s);
    if ((int)len + 1 <= cchBuf) {
        strcpy(pchBuf, s);
        return len;
    }
    return 0;
}

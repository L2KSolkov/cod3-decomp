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

extern "C" int __stdcall XNetInAddrToString(struct in_addr ina, char* pchBuf,
                                              int cchBuf);

// ea: 0x008B5E40
bdInAddr::bdInAddr() {
    inUn.m_iaddr = 0xFF00FF00;
}

// ea: 0x008B5E50
bdInAddr::bdInAddr(unsigned int addr) {
    inUn.m_iaddr = addr;
}

// ea: 0x008B5EC0
bdInAddr::bdInAddr(const char* str) {
    inUn.m_iaddr = 0;
    fromString(str);
}

// ============================================================================
// bdInAddr::fromString - ea: 0x8B5E60
// ============================================================================
void bdInAddr::fromString(const char* cp) {
    inUn.m_iaddr = inet_addr(cp);
}

// ============================================================================
// bdInAddr::toString - ea: 0x8B5E80
// COD3 calls XNetInAddrToString; the Win32 equivalent is a dotted-quad print.
// ============================================================================
unsigned int bdInAddr::toString(char* const pchBuf, unsigned int cchBuf) const {
    struct in_addr a;
    a.s_addr = inUn.m_iaddr;
    if (XNetInAddrToString(a, pchBuf, (int)cchBuf) != 0)
        return 0;
    return (unsigned int)strlen(pchBuf);
}

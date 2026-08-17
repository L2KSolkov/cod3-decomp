// ============================================================================
// bdInetAddr.cpp - internet address (25 funcs).
// Source: bdCore:bdInetAddr.obj
// Verified against IDA (release decompilation).
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// bdInetAddr::bdInetAddr (from string) - ea: 0x89D6E0
// ============================================================================
bdInetAddr::bdInetAddr(const char* str) {
    m_addr.fromString(str);
}

// ============================================================================
// bdInetAddr::isValid - ea: 0x89D4E0
// ============================================================================
bool bdInetAddr::isValid() const {
    bdInAddr zero;
    zero.inUn.m_iaddr = 0;
    return m_addr.inUn.m_iaddr != zero.inUn.m_iaddr;
}

// ============================================================================
// bdInetAddr::toString - ea: 0x89D510
// ============================================================================
unsigned int bdInetAddr::toString(char* const pchBuf, unsigned int cchBuf) const {
    if (isValid())
        return m_addr.toString(pchBuf, (int)cchBuf);
    return 0;
}

// ============================================================================
// bdInetAddr::serialize - ea: 0x89D560
// ============================================================================
bool bdInetAddr::serialize(void* buffer, unsigned int bufferSize,
                           unsigned int offset, unsigned int* newOffset) const {
    *newOffset = offset;
    return bdBytePacker::appendBuffer(buffer, bufferSize, offset, newOffset,
                                      (const unsigned char*)this, 4u);
}

// ============================================================================
// bdInetAddr::deserialize - ea: 0x89D590
// ============================================================================
bool bdInetAddr::deserialize(const void* buffer, unsigned int bufferSize,
                             unsigned int offset, unsigned int* newOffset) {
    *newOffset = offset;
    return bdBytePacker::removeBuffer((const unsigned char*)buffer, bufferSize,
                                      offset, newOffset,
                                      (unsigned char*)this, 4u);
}

// ============================================================================
// bdInetAddr::Loopback - ea: 0x89D5C0 (127.0.0.1)
// ============================================================================
bdInetAddr bdInetAddr::Loopback() {
    bdInetAddr result;
    result.m_addr.inUn.m_iaddr = 0x7F000001;
    return result;
}

// ============================================================================
// bdInetAddr::Broadcast - ea: 0x89D620 (255.255.255.255)
// ============================================================================
bdInetAddr bdInetAddr::Broadcast() {
    bdInetAddr result;
    result.m_addr.inUn.m_iaddr = 0xFFFFFFFF;
    return result;
}

// ============================================================================
// bdInetAddr::Any - ea: 0x89D680 (0.0.0.0)
// ============================================================================
bdInetAddr bdInetAddr::Any() {
    bdInetAddr result;
    result.m_addr.inUn.m_iaddr = 0;
    return result;
}

// ============================================================================
// bdInetAddr::isLoopback - ea: 0x89D730
// ============================================================================
bool bdInetAddr::isLoopback() const {
    return m_addr.inUn.m_iaddr == Loopback().m_addr.inUn.m_iaddr;
}

// ============================================================================
// bdInetAddr::isBroadcast - ea: 0x89D750
// ============================================================================
bool bdInetAddr::isBroadcast() const {
    return m_addr.inUn.m_iaddr == Broadcast().m_addr.inUn.m_iaddr;
}

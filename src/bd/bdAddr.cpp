// ============================================================================
// bdAddr.cpp - address + port (18 funcs).
// Source: bdCore:bdAddr.obj
// Verified against IDA (release decompilation).
// ============================================================================

#include "bd/bd_types.h"

#include <stdlib.h>
#include <string.h>

unsigned int bdAddr::serializedSize = 0;

// ============================================================================
// bdAddr::bdAddr (address, port) - ea: 0x9EC910
// ============================================================================
bdAddr::bdAddr(const bdInetAddr& address, unsigned short port)
    : m_address(address), m_port(port) {
}

// ============================================================================
// bdAddr::bdAddr (string "ip:port") - ea: 0x9ECE30
// ============================================================================
bdAddr::bdAddr(const char* str)
    : m_address(), m_port(0) {
    set(str);
}

// ============================================================================
// bdAddr::set (address, port) - ea: 0x9EC940
// ============================================================================
const bdInetAddr& bdAddr::set(const bdInetAddr& address, unsigned short port) {
    m_address.set(address);
    m_port = port;
    return address;
}

// ============================================================================
// bdAddr::set (string "ip:port") - ea: 0x9ECA00
// ============================================================================
void bdAddr::set(const char* cp) {
    do {
        if (cp == NULL) {
            bdMessageProxy proxy(".\\bdSocket\\bdAddr.cpp",
                                 "void __thiscall bdAddr::set(const char *)",
                                 0x2Cu, "dw/err");
            proxy.log("defaultFileName",
                      "bdAddr::set(const bdNChar8 *socketAddress) \nInvalid socket address \n ");
        }
    } while (g_assertFalse);

    const char* colon = strchr(cp, ':');
    if (colon != NULL) {
        char ipPart[16];
        unsigned int len = (unsigned int)(colon - cp) + 1;
        unsigned int copy = (len <= 16) ? len : 16;
        memcpy(ipPart, cp, copy);
        if (len <= 16)
            ipPart[len - 1] = 0;
        else
            ipPart[15] = 0;
        m_address.set(ipPart);
        m_port = (unsigned short)strtoul(colon + 1, NULL, 10);
    } else {
        bdInetAddr addr(cp);
        m_address.set(addr);
        m_port = 0;
    }
}

// ============================================================================
// bdAddr::operator== - ea: 0x9EC960
// ============================================================================
bool bdAddr::operator==(const bdAddr& other) const {
    return m_port == other.m_port && m_address == other.m_address;
}

// ============================================================================
// bdAddr::operator!= - ea: 0x9EC990
// ============================================================================
bool bdAddr::operator!=(const bdAddr& other) const {
    return m_port != other.m_port || m_address != other.m_address;
}

// ============================================================================
// bdAddr::toString - ea: 0x9ECB70
// ============================================================================
unsigned int bdAddr::toString(char* const buf, unsigned int bufSize) const {
    unsigned int len = m_address.toString(buf, bufSize);
    unsigned int remaining = (len <= bufSize) ? bufSize - len : 0;
    return len + (unsigned int)bdSnprintf(&buf[len], remaining, ":%u", m_port);
}

// ============================================================================
// bdAddr::serialize - ea: 0x9ECBD0
// ============================================================================
bool bdAddr::serialize(void* buffer, unsigned int bufferSize,
                       unsigned int offset, unsigned int* newOffset) const {
    unsigned int startOffset = offset;
    *newOffset = offset;
    if (m_address.serialize(buffer, bufferSize, offset, newOffset)) {
        offset = *newOffset;
        unsigned short port = m_port;
        if (bdBytePacker::appendBasicType(buffer, bufferSize, offset, newOffset,
                                          &port, 2u))
            return true;
    }
    *newOffset = startOffset;
    return false;
}

// ============================================================================
// bdAddr::deserialize - ea: 0x9ECC50
// ============================================================================
bool bdAddr::deserialize(const void* buffer, unsigned int bufferSize,
                         unsigned int offset, unsigned int* newOffset) {
    unsigned int startOffset = offset;
    *newOffset = offset;
    if (m_address.deserialize(buffer, bufferSize, offset, newOffset)) {
        unsigned short port = 0;
        if (bdBytePacker::removeBasicType((const unsigned char*)buffer, bufferSize,
                                          *newOffset, newOffset, &port, 2u)) {
            m_port = port;
            return true;
        }
    }
    *newOffset = startOffset;
    return false;
}

// ============================================================================
// bdAddr::getSerializedSize - ea: 0x9ECCD0
// ============================================================================
unsigned int bdAddr::getSerializedSize() const {
    if (serializedSize != 0)
        return serializedSize;
    do {
        serializedSize = 0;
        if (m_address.serialize(NULL, 0xFFFFu, 0, &serializedSize)) {
            unsigned short port = m_port;
            if (bdBytePacker::appendBasicType(NULL, 0xFFFFu, serializedSize,
                                              &serializedSize, &port, 2u)) {
                if (serializedSize != 0)
                    continue;
            } else {
                serializedSize = 0;
            }
        } else {
            serializedSize = 0;
        }
        bdMessageProxy proxy(".\\bdSocket\\bdAddr.cpp",
                             "unsigned int __thiscall bdAddr::getSerializedSize(void) const",
                             0xB1u, "dw/err");
        proxy.log("defaultFileName", "Failed to get serialized size.");
    } while (g_assertFalse);

    do {
        if (serializedSize >= 0x530) {
            bdMessageProxy proxy(".\\bdSocket\\bdAddr.cpp",
                                 "unsigned int __thiscall bdAddr::getSerializedSize(void) const",
                                 0xB2u, "dw/err");
            proxy.log("defaultFileName", "Size is larger than a biggest datagram.");
        }
    } while (g_assertFalse);
    return serializedSize;
}

// ============================================================================
// bdAddr::getHash - ea: 0x9ECE90
// ============================================================================
unsigned int bdAddr::getHash() const {
    unsigned int hash = 0;
    unsigned char data[0x530];
    unsigned int size = 0;
    do {
        size = 0;
        if (m_address.serialize(data, 0x530u, 0, &size)) {
            unsigned short port = m_port;
            if (bdBytePacker::appendBasicType(data, 0x530u, size, &size, &port, 2u))
                continue;
        }
        size = 0;
        bdMessageProxy proxy(".\\bdSocket\\bdAddr.cpp",
                             "unsigned int __thiscall bdAddr::getHash(void) const",
                             0x7Fu, "dw/err");
        proxy.log("defaultFileName", "Failed to serialize.");
    } while (g_assertFalse);

    for (unsigned int i = 0; i < size; i++)
        hash = data[i] + 31 * hash;
    return hash;
}

// ============================================================================
// bdAddr::operator< - ea: 0x9ECFB0
// ============================================================================
bool bdAddr::operator<(const bdAddr& other) const {
    return getHash() < other.getHash();
}

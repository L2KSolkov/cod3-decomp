// ============================================================================
// bdSocket.cpp - UDP socket wrapper (10 funcs).
// Source: bdCore:bdSocket.obj
// Verified against IDA (release decompilation).
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// bdSocket::bdSocket - ea: 0x9EC4E0
// ============================================================================
bdSocket::bdSocket()
    : m_handle(BD_INVALID_SOCKET_HANDLE) {
}

// ============================================================================
// bdSocket::~bdSocket - ea: 0x9EC780
// ============================================================================
bdSocket::~bdSocket() {
    bdPlatformSocket::close(m_handle);
}

// ============================================================================
// bdSocket::create - ea: 0x9EC500
// ============================================================================
bool bdSocket::create(bool blocking) {
    do {
        if (m_handle != BD_INVALID_SOCKET_HANDLE) {
            bdMessageProxy proxy(".\\bdSocket\\bdSocket.cpp",
                                 "bool __thiscall bdSocket::create(const bool)",
                                 0x1Eu, "dw/err");
            proxy.log("defaultFileName", "bdSocket::create(), already created.");
        }
    } while (g_assertFalse);

    m_handle = bdPlatformSocket::create(blocking);
    return m_handle != BD_INVALID_SOCKET_HANDLE && setBlocking(false);
}

// ============================================================================
// bdSocket::bind (bdAddr) - ea: 0x9EC5A0
// ============================================================================
bdSocketStatusCode bdSocket::bind(const bdAddr& addr) {
    return bdPlatformSocket::bind(m_handle, addr.getAddress().getInAddr(),
                                  addr.getPort());
}

// ============================================================================
// bdSocket::bind (port) - ea: 0x9EC7A0
// ============================================================================
bdSocketStatusCode bdSocket::bind(unsigned short port) {
    bdAddr addr(bdInetAddr::Any(), port);
    return bind(addr);
}

// ============================================================================
// bdSocket::sendTo - ea: 0x9EC5E0
// ============================================================================
int bdSocket::sendTo(const bdAddr& addr, const void* data, unsigned int length) {
    return bdPlatformSocket::sendTo(m_handle, addr.getAddress().getInAddr(),
                                    addr.getPort(), data, length);
}

// ============================================================================
// bdSocket::receiveFrom - ea: 0x9EC630
// ============================================================================
int bdSocket::receiveFrom(bdAddr& addr, void* data, unsigned int size) {
    bdInAddr inaddr;
    unsigned short port = 0;
    int status = bdPlatformSocket::receiveFrom(m_handle, inaddr, port, data, size);
    if (status >= 0 || status == BD_NET_CONNECTION_RESET) {
        bdInetAddr address(&inaddr);
        addr.set(address, port);
    }
    return status;
}

// ============================================================================
// bdSocket::close - ea: 0x9EC6F0
// ============================================================================
bool bdSocket::close() {
    return bdPlatformSocket::close(m_handle);
}

// ============================================================================
// bdSocket::setBlocking - ea: 0x9EC700
// ============================================================================
bool bdSocket::setBlocking(bool blocking) {
    return bdPlatformSocket::setBlocking(m_handle, blocking);
}

// ============================================================================
// bdSocket::getHandle - ea: 0x9EC720
// ============================================================================
int bdSocket::getHandle() const {
    if (m_handle == BD_INVALID_SOCKET_HANDLE) {
        bdMessageProxy proxy(".\\bdSocket\\bdSocket.cpp",
                             "int __thiscall bdSocket::getHandle(void) const",
                             0x62u, "dw/warn/");
        proxy.log("socket", "Socket not yet created.");
    }
    return m_handle;
}

// ============================================================================
// bdSocketRouter.cpp - socket routing layer (10 funcs).
// Source: bdSocket:bdSocketRouter-xbox.obj
// Verified against IDA (release decompilation).
// ============================================================================

#include "bd/bdSocketRouter.h"
#include "bd/bdSecurityKeyMap.h"

#include <new>

extern "C" {
int XNetConnect(struct in_addr ina);
int XNetGetConnectStatus(struct in_addr ina);
}

// ============================================================================
// bdSocketRouter::bdSocketRouter - ea: 0x8B73B0
// ============================================================================
bdSocketRouter::bdSocketRouter(bdSocket* socket, bdSecurityKeyMap*, bdDHKey*,
                               const bdArray<bdAddr>&)
    : m_socket(socket), m_qosProber(), m_interceptors() {
}

// ============================================================================
// bdSocketRouter::~bdSocketRouter - ea: 0x8B7350
// ============================================================================
bdSocketRouter::~bdSocketRouter() {
    bdMemory::deallocate(m_interceptors.m_data);
    m_interceptors.m_data = NULL;
    m_interceptors.m_size = 0;
    m_interceptors.m_capacity = 0;
    m_qosProber.cancelProbes();
}

// ============================================================================
// bdSocketRouter::pump - ea: 0x8B6FF0
// ============================================================================
void bdSocketRouter::pump() {
    m_qosProber.pump();
}

// ============================================================================
// bdSocketRouter::getQoSProber - ea: 0x8B7000
// ============================================================================
bdQoSProbe* bdSocketRouter::getQoSProber() {
    return &m_qosProber;
}

// ============================================================================
// bdSocketRouter::connect - ea: 0x8B7070
// ============================================================================
bool bdSocketRouter::connect(bdReference<bdAddrHandle>& addrHandle) {
    struct in_addr ina;
    ina.s_addr = addrHandle.m_ptr->m_addr.inUn.m_iaddr;
    return XNetConnect(ina) == 0;
}

// ============================================================================
// bdSocketRouter::getStatus - ea: 0x8B7090
// ============================================================================
bdSocketAssociationStatus bdSocketRouter::getStatus(
    const bdReference<bdAddrHandle>& addrHandle) {
    struct in_addr ina;
    ina.s_addr = addrHandle.m_ptr->m_addr.inUn.m_iaddr;
    switch (XNetGetConnectStatus(ina)) {
    case 0: return BD_SOCKET_IDLE;
    case 1: return BD_SOCKET_PENDING;
    case 2: return BD_SOCKET_CONNECTED;
    case 3: return BD_SOCKET_LOST;
    default: return BD_SOCKET_IDLE;
    }
}

// ============================================================================
// bdSocketRouter::sendTo - ea: 0x8B70F0
// ============================================================================
int bdSocketRouter::sendTo(const bdReference<bdAddrHandle>& addrHandle,
                           const void* data, unsigned int length) {
    bdInetAddr address(addrHandle.m_ptr->m_addr.inUn.m_iaddr);
    bdAddr dest(address, addrHandle.m_ptr->m_port);
    return m_socket->sendTo(dest, data, length);
}

// ============================================================================
// bdSocketRouter::receiveFrom - ea: 0x8B7190
// ============================================================================
int bdSocketRouter::receiveFrom(bdReference<bdAddrHandle>& addrHandle,
                                void* data, unsigned int size) {
    bdAddr realAddr;
    int result = m_socket->receiveFrom(realAddr, data, size);
    if (realAddr.getAddress().isValid()) {
        bdAddrHandle* handle =
            new (bdMemory::allocate(sizeof(bdAddrHandle))) bdAddrHandle();
        if (addrHandle.m_ptr != NULL && addrHandle.m_ptr->releaseRef() == 0)
            delete addrHandle.m_ptr;
        addrHandle.m_ptr = handle;
        if (handle != NULL)
            handle->addRef();
        handle->m_addr.inUn.m_iaddr = realAddr.getAddress().toUInt32();
        handle->m_port = realAddr.getPort();
    }
    return result;
}

// ============================================================================
// bdSocketRouter::onNATAddrDiscovery - ea: 0x8B72C0
// ============================================================================
void bdSocketRouter::onNATAddrDiscovery(const bdReference<bdCommonAddr>&,
                                        const bdAddr&) {
}

// ============================================================================
// bdSocketRouter::onNATAddrDiscoveryFailed - ea: 0x8B72E0
// ============================================================================
void bdSocketRouter::onNATAddrDiscoveryFailed(const bdReference<bdCommonAddr>&) {
}

// ============================================================================
// bdAddressMap.cpp - address handle <-> common address map (7 funcs).
// Source: bdSocket:bdAddressMap-xbox.obj
// Verified against IDA (release decompilation). Xbox XNet calls are declared
// verbatim per plan section 7 and resolved by the shim.
// ============================================================================

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <winsock2.h>
#endif

#include "bd/bdAddressMap.h"
#include "bd/bdGameInfo.h"

#include <string.h>
#include <new>

// Xbox XNet API declarations (shim resolves later).
extern "C" {
int XNetTsAddrToInAddr(const XNADDR* ptsa, unsigned int dwServiceId,
                       const XNKID* pxnkid, struct in_addr* pina);
int XNetXnAddrToInAddr(const XNADDR* pxna, const XNKID* pxnkid,
                       struct in_addr* pina);
int XNetInAddrToXnAddr(struct in_addr ina, XNADDR* pxna, XNKID* pxnkid);
int XNetInAddrToString(struct in_addr ina, char* pchBuf, int cchBuf);
int XNetGetConnectStatus(struct in_addr ina);
int XNetUnregisterInAddr(struct in_addr ina);
}

// ============================================================================
// bdAddrHandle construction (inline in the binary at the map's call site)
// ============================================================================
bdAddrHandle::bdAddrHandle()
    : m_addr(), m_port(0) {
}

bdAddrHandle::~bdAddrHandle() {
}

// ============================================================================
// bdCommonAddr (hostAddr copy with new addr/port) - ea: 0x8B75F3 usage
// ============================================================================
bdCommonAddr::bdCommonAddr(const bdReference<bdCommonAddr>& hostAddr,
                           const XNADDR& addr, uint16_t port)
    : m_addr(addr), m_port(port), m_titleId(hostAddr.m_ptr->m_titleId),
      m_hash(hostAddr.m_ptr->m_hash),
      m_isLoopback(hostAddr.m_ptr->m_isLoopback),
      m_natType(hostAddr.m_ptr->m_natType) {
}

// ============================================================================
// bdAddressMapImpl::bdAddressMapImpl - ea: 0x8B7440
// ============================================================================
bdAddressMapImpl::bdAddressMapImpl()
    : m_me() {
}

// ============================================================================
// bdAddressMapImpl::getInstance - bdSingleton COMDAT (unresolved extern)
// ============================================================================
bdAddressMapImpl* bdAddressMapImpl::getInstance() {
    return bdSingleton<bdAddressMapImpl>::getInstance();
}

// ============================================================================
// bdAddressMapImpl::commonAddrToAddr - ea: 0x8B7450
// ============================================================================
bool bdAddressMapImpl::commonAddrToAddr(const bdReference<bdCommonAddr>& ca,
                                        const XNKID& xnkid,
                                        bdReference<bdAddrHandle>& addrHandle) {
    bdAddrHandle* handle = new (bdMemory::allocate(sizeof(bdAddrHandle))) bdAddrHandle();
    if (addrHandle.m_ptr != NULL && addrHandle.m_ptr->releaseRef() == 0)
        delete addrHandle.m_ptr;
    addrHandle.m_ptr = handle;
    if (handle != NULL)
        handle->addRef();

    struct in_addr ina;
    int result;
    if (ca.m_ptr->getTitleId() != 0) {
        result = XNetTsAddrToInAddr(ca.m_ptr->getXNAddr(), ca.m_ptr->getTitleId(),
                                    &xnkid, &ina);
    } else {
        result = XNetXnAddrToInAddr(ca.m_ptr->getXNAddr(), &xnkid, &ina);
    }
    handle->m_addr.inUn.m_iaddr = ina.s_addr;
    handle->m_port = ca.m_ptr->getPort();
    return result == 0;
}

// ============================================================================
// bdAddressMapImpl::addrToCommonAddr - ea: 0x8B7560
// ============================================================================
bool bdAddressMapImpl::addrToCommonAddr(const bdReference<bdAddrHandle>& addrHandle,
                                        bdReference<bdCommonAddr>& ca,
                                        XNKID& xnkid) const {
    XNADDR xnAddr;
    struct in_addr ina;
    ina.s_addr = addrHandle.m_ptr->m_addr.inUn.m_iaddr;
    int result = XNetInAddrToXnAddr(ina, &xnAddr, &xnkid);
    bool ok = result == 0;
    if (result != 0) {
        bdMessageProxy proxy(".\\bdAddressMap-xbox.cpp",
                             "bool __thiscall bdAddressMapImpl::addrToCommonAddr(const class bdReference<class bdAddrHandle> &,class bdReference<class bdCommonAddr> &,XNKID &) const",
                             0x46u, "dw/warn/");
        proxy.log("addressmap", "Invalid addr handle.");
        return ok;
    }
    if (m_me.m_ptr == NULL) {
        bdMessageProxy proxy(".\\bdAddressMap-xbox.cpp",
                             "bool __thiscall bdAddressMapImpl::addrToCommonAddr(const class bdReference<class bdAddrHandle> &,class bdReference<class bdCommonAddr> &,XNKID &) const",
                             0x41u, "dw/warn/");
        proxy.log("addressmap", "Invalid title addr.");
        return ok;
    }

    bdCommonAddr* created = new (bdMemory::allocate(sizeof(bdCommonAddr)))
        bdCommonAddr(m_me, xnAddr, addrHandle.m_ptr->m_port);
    if (ca.m_ptr != NULL && ca.m_ptr->releaseRef() == 0)
        delete ca.m_ptr;
    ca.m_ptr = created;
    if (created != NULL)
        created->addRef();
    return ok;
}

// ============================================================================
// bdAddressMapImpl::unregisterAddr - ea: 0x8B7690
// ============================================================================
bool bdAddressMapImpl::unregisterAddr(bdReference<bdAddrHandle>& addrHandle) {
    struct in_addr ina;
    ina.s_addr = addrHandle.m_ptr->m_addr.inUn.m_iaddr;
    return ina.s_addr == 0x7F000001
        || XNetGetConnectStatus(ina) == 3
        || XNetUnregisterInAddr(ina) == 0;
}

// ============================================================================
// bdAddressMapImpl::getTitleCommonAddr - ea: 0x8B76D0
// ============================================================================
bool bdAddressMapImpl::getTitleCommonAddr(bdReference<bdCommonAddr>& ca) {
    if (ca.m_ptr != NULL && ca.m_ptr->releaseRef() == 0)
        delete ca.m_ptr;
    ca.m_ptr = m_me.m_ptr;
    if (ca.m_ptr != NULL)
        ca.m_ptr->addRef();
    return m_me.m_ptr != NULL;
}

// ============================================================================
// bdAddressMapImpl::setTitleCommonAddr - ea: 0x8B7710
// ============================================================================
void bdAddressMapImpl::setTitleCommonAddr(const bdReference<bdCommonAddr>& ca) {
    if (m_me.m_ptr != NULL && m_me.m_ptr->releaseRef() == 0)
        delete m_me.m_ptr;
    m_me.m_ptr = ca.m_ptr;
    if (m_me.m_ptr != NULL)
        m_me.m_ptr->addRef();
}

// ============================================================================
// bdAddressMapImpl::addrToString - ea: 0x8B77A0
// ============================================================================
unsigned int bdAddressMapImpl::addrToString(const bdReference<bdAddrHandle>& addrHandle,
                                            char* pchBuf, unsigned int cchBuf) const {
    struct in_addr ina;
    ina.s_addr = addrHandle.m_ptr->m_addr.inUn.m_iaddr;
    if (XNetInAddrToString(ina, pchBuf, (int)cchBuf) != 0)
        return 0;
    return (unsigned int)strlen(pchBuf);
}

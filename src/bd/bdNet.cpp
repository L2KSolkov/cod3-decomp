// ============================================================================
// bdNetImpl.cpp - network singleton (14 funcs).
// Source: bdNet:bdNet-xbox.obj
// Verified against IDA (release decompilation).
// ============================================================================

#include "bd/bdNet.h"

#include <string.h>
#include <new>

extern "C" {
unsigned int XNetGetTitleXnAddr(XNADDR* pxna);
}

// IDA 0x89CAB0: bdString stores its data pointer at offset 0 and the
// allocation header's length two words before that buffer.
unsigned int bdString::getLength() const {
    return reinterpret_cast<const unsigned int*>(m_string)[-2];
}

// ============================================================================
// bdNetImpl::bdNetImpl - ea: 0x8AFC20
// ============================================================================
bdNetImpl::bdNetImpl()
    : m_processTimer(), m_params(), m_localCommonAddr(), m_keyStore(),
      m_connectionStore(NULL), m_dispatcher(), m_status(BD_NET_STOPPED),
      m_natTravAddrs(), m_tmpSocket() {
}

// ============================================================================
// bdNetImpl::~bdNetImpl - ea: 0x8AFCB0
// ============================================================================
bdNetImpl::~bdNetImpl() {
    stop();
}

// ============================================================================
// bdNetImpl::getStatus - ea: 0x8AF0D0
// ============================================================================
bdNetStatus bdNetImpl::getStatus() const {
    return m_status;
}

// ============================================================================
// bdNetImpl::sendAll - ea: 0x8AF0E0
// ============================================================================
bool bdNetImpl::sendAll() {
    if (m_connectionStore != NULL)
        return m_connectionStore->flushAll();
    return false;
}

// ============================================================================
// bdNetImpl::registerDispatchInterceptor - ea: 0x8AF0F0
// ============================================================================
void bdNetImpl::registerDispatchInterceptor(bdDispatchInterceptor* const interceptor) {
    m_dispatcher.registerInterceptor(interceptor);
}

// ============================================================================
// bdNetImpl::unregisterDispatchInterceptor - ea: 0x8AF100
// ============================================================================
void bdNetImpl::unregisterDispatchInterceptor(bdDispatchInterceptor* const interceptor) {
    m_dispatcher.unregisterInterceptor(interceptor);
}

// ============================================================================
// bdNetImpl::getConnectionStore - ea: 0x8AF110
// ============================================================================
bdConnectionStore* bdNetImpl::getConnectionStore() {
    return m_connectionStore;
}

// ============================================================================
// bdNetImpl::getKeyMap - ea: 0x8AF120
// ============================================================================
bdSecurityKeyMap* bdNetImpl::getKeyMap() {
    return &m_keyStore;
}

// ============================================================================
// bdNetImpl::getParams - ea: 0x8AF130
// ============================================================================
const bdNetStartParams& bdNetImpl::getParams() {
    return m_params;
}

// ============================================================================
// bdNetImpl::receiveAndDispatchAll - ea: 0x8AF250
// ============================================================================
void bdNetImpl::receiveAndDispatchAll() {
    if (m_status != BD_NET_DONE) {
        bdMessageProxy proxy(".\\bdNet-xbox.cpp",
                             "void __thiscall bdNetImpl::receiveAndDispatchAll(void)",
                             0x75u, "dw/err");
        proxy.log("defaultFileName", "bdNetImpl::receiveAndDispatchAll: not ready to work");
    }
    if (m_connectionStore == NULL)
        return;
    bdReference<bdConnection> connection;
    while (m_connectionStore->receiveFrom(connection)) {
        m_dispatcher.process(connection);
        if (connection.m_ptr != NULL && connection.m_ptr->releaseRef() == 0)
            delete connection.m_ptr;
        connection.m_ptr = NULL;
    }
}

// ============================================================================
// bdNetImpl::getLocalCommonAddr - ea: 0x8AF340
// ============================================================================
bdReference<bdCommonAddr> bdNetImpl::getLocalCommonAddr() const {
    bdReference<bdCommonAddr> result;
    result.m_ptr = m_localCommonAddr.m_ptr;
    if (result.m_ptr != NULL)
        result.m_ptr->addRef();
    return result;
}

// ============================================================================
// bdNetImpl::stop - ea: 0x8AF4E0
// ============================================================================
void bdNetImpl::stop() {
    if (m_connectionStore != NULL) {
        m_connectionStore->disconnectAll();
        bdStopwatch timer;
        timer.start();
        while (timer.getElapsedTimeInSeconds() < 1.0f) {
            receiveAndDispatchAll();
            if (m_connectionStore != NULL)
                m_connectionStore->flushAll();
            bdPlatformTiming::sleep(20);
        }
        m_connectionStore->closeAll();
        delete m_connectionStore;
        m_connectionStore = NULL;
    }
    if (m_params.m_socket != NULL) {
        m_params.m_socket->close();
        delete m_params.m_socket;
        m_params.m_socket = NULL;
    }
    if (m_localCommonAddr.m_ptr != NULL && m_localCommonAddr.m_ptr->releaseRef() == 0)
        delete m_localCommonAddr.m_ptr;
    m_localCommonAddr.m_ptr = NULL;
    m_status = BD_NET_STOPPED;
}

// ============================================================================
// bdNetImpl::start - ea: 0x8AF9E0
// ============================================================================
bool bdNetImpl::start(const bdNetStartParams& params) {
    m_params.m_onlineGame = params.m_onlineGame;
    m_params.m_gamePort = params.m_gamePort;
    m_params.m_socket = params.m_socket;
    m_params.m_natTravHosts = params.m_natTravHosts;
    m_params.m_natTravPort = params.m_natTravPort;
    m_params.m_localAddresses = params.m_localAddresses;
    m_params.m_timeout = params.m_timeout;
    m_params.m_upnpTimeout = params.m_upnpTimeout;

    if (m_params.m_socket == NULL) {
        m_params.m_socket = new (bdMemory::allocate(sizeof(bdSocket))) bdSocket();
        m_params.m_socket->create(false);
        m_params.m_socket->setBlocking(false);
    }
    m_params.m_socket->bind(m_params.m_gamePort);
    m_status = BD_NET_PENDING;
    return true;
}

// ============================================================================
// bdNetImpl::pump - ea: 0x8AFAB0
// ============================================================================
void bdNetImpl::pump() {
    if (m_localCommonAddr.m_ptr != NULL || m_status != BD_NET_PENDING)
        return;

    XNADDR addr;
    unsigned int result = XNetGetTitleXnAddr(&addr);
    if (result != 0) {
        if (result == 0x8000 || result == 1) {
            bdMessageProxy proxy(".\\bdNet-xbox.cpp",
                                 "void __thiscall bdNetImpl::pump(void)",
                                 0x31u, "dw/warn/");
            proxy.log("bdNet/net", "XNetGetTitleXnAddr failed.");
            m_status = BD_NET_INIT_FAILED;
        } else {
            bdCommonAddr* ca = new (bdMemory::allocate(sizeof(bdCommonAddr)))
                bdCommonAddr(addr, m_params.m_gamePort);
            if (m_localCommonAddr.m_ptr != NULL && m_localCommonAddr.m_ptr->releaseRef() == 0)
                delete m_localCommonAddr.m_ptr;
            m_localCommonAddr.m_ptr = ca;
            if (ca != NULL)
                ca->addRef();
        }
    }

    if (m_localCommonAddr.m_ptr != NULL) {
        m_status = BD_NET_DONE;
        bdDHKey* dhKey = NULL;
        m_connectionStore = new (bdMemory::allocate(sizeof(bdConnectionStore)))
            bdConnectionStore(m_params.m_socket, m_localCommonAddr, &m_keyStore,
                              dhKey, m_natTravAddrs);
    }
}

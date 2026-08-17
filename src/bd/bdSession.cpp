// ============================================================================
// bdSession.cpp - peer-to-peer session (44 funcs).
// Source: bdPeer:bdSession.obj
// Verified against IDA (release decompilation) + Demonware 2.0 reference.
// ============================================================================

#include "bd/bdSession.h"

#include <new>
#include <string.h>

#include "bd/bdSessionHandler.h"
#include "bd/bdSessionListener.h"
#include "bd/bdSecurityKeyMap.h"
#include "bd/bdSessionHandler.h"
#include "bd/bdAddressMap.h"
#include "bd/bdNet.h"

// bdArray grow-and-append helper (bdArray is a raw data/capacity/size triple).
template <typename T>
void bdArrayAppend(bdArray<T>& arr, const T& value) {
    if (arr.m_size == arr.m_capacity) {
        unsigned int newCap = arr.m_capacity ? arr.m_capacity * 2 : 4;
        arr.m_data = (T*)bdMemory::reallocate(arr.m_data, sizeof(T) * newCap);
        arr.m_capacity = newCap;
    }
    arr.m_data[arr.m_size++] = value;
}
// Cross-object externs (bdNet/bdConnection; unresolved until ported).
class bdNetImpl;
class bdConnectionStore;
class bdSecurityKeyMap;

// ============================================================================
// bdSession::bdSession - ea: 0x8B28A0
// ============================================================================
bdSession::bdSession(bdSessionHandler* const sessionHandler)
    : m_hostConnection(), m_localConnection(), m_joiningPeer(), m_peers(),
      m_interceptors(), m_listeners(), m_handler(sessionHandler),
      m_role(BD_SESSION_PEER), m_status(BD_SESSION_NOT_CONNECTED),
      m_localPeerIndex(0), m_pendingConnections(), m_pendingJoinRequests(),
      m_secID(), m_secKey(), m_sessionJoinRequest(), m_registeredKey(false),
      m_gameSecID(), m_gameSecKey() {
}

// ============================================================================
// bdSession::~bdSession - ea: 0x8B3600
// ============================================================================
bdSession::~bdSession() {
    cleanup();
}

// ============================================================================
// bdSession::getStatus - ea: 0x8B07E0
// ============================================================================
bdSession::bdSessionStatus bdSession::getStatus() const {
    return m_status;
}

// ============================================================================
// bdSession::getRole - ea: 0x8B07F0
// ============================================================================
bdSession::bdSessionRole bdSession::getRole() const {
    return m_role;
}

// ============================================================================
// bdSession::getLocalPeerIndex - ea: 0x8B0800
// ============================================================================
unsigned int bdSession::getLocalPeerIndex() const {
    return m_localPeerIndex;
}

// ============================================================================
// bdSession::readyToConnect - ea: 0x8B0810
// ============================================================================
bool bdSession::readyToConnect() const {
    bool ready = m_status == BD_SESSION_NOT_CONNECTED;
    if (!ready) {
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "bool __thiscall bdSession::readyToConnect(void) const",
                             0x1BEu, "dw/warn/");
        proxy.log("bdPeer/session", "Session status must be BD_SESSION_NOT_CONNECTED to connect.");
    }
    return ready;
}

// ============================================================================
// bdSession::getNumPeers - ea: 0x8B0E10
// ============================================================================
unsigned int bdSession::getNumPeers() const {
    return m_peers.m_size;
}

// ============================================================================
// bdSession::getHost - ea: 0x8B0E20 (returns new reference)
// ============================================================================
bdReference<bdConnection> bdSession::getHost() const {
    bdReference<bdConnection> result;
    result.m_ptr = m_hostConnection.m_ptr;
    if (result.m_ptr != NULL)
        result.m_ptr->addRef();
    return result;
}

// ============================================================================
// bdSession::sendHost - ea: 0x8B0E40
// ============================================================================
bool bdSession::sendHost(const bdReference<bdMessage>& msg, bool reliable) {
    bool sent = false;
    if (m_hostConnection.m_ptr == NULL) {
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "bool __thiscall bdSession::sendHost(class bdReference<class bdMessage>,const bool)",
                             0x16Bu, "dw/err/");
        proxy.log("bdPeer/session", "No host connection.");
        bdMessageProxy proxy2(".\\bdSession\\bdSession.cpp",
                              "bool __thiscall bdSession::sendHost(class bdReference<class bdMessage>,const bool)",
                              0x171u, "dw/info/");
        proxy2.log("bdPeer/session", "Sending message to peer failed.");
        return false;
    }
    sent = m_hostConnection.m_ptr->send(msg, reliable);
    if (!sent) {
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "bool __thiscall bdSession::sendHost(class bdReference<class bdMessage>,const bool)",
                             0x171u, "dw/info/");
        proxy.log("bdPeer/session", "Sending message to peer failed.");
    }
    return sent;
}

// ============================================================================
// bdSession::createJoinRequest - ea: 0x8B0F30
// ============================================================================
bool bdSession::createJoinRequest(bdBitBuffer* const userData) {
    if (m_sessionJoinRequest.m_ptr != NULL) {
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "bool __thiscall bdSession::createJoinRequest(class bdBitBuffer *const )",
                             0x208u, "dw/warn/");
        proxy.log("bdPeer/session", "Join request already exists, old request discarded.");
    }
    bdMessage* msg = new (bdMemory::allocate(sizeof(bdMessage)))
        bdMessage(BD_SESSION_JOIN_REQ, false);
    if (m_sessionJoinRequest.m_ptr != NULL && m_sessionJoinRequest.m_ptr->releaseRef() == 0)
        delete m_sessionJoinRequest.m_ptr;
    m_sessionJoinRequest.m_ptr = msg;
    if (msg != NULL)
        msg->addRef();

    bool result = false;
    if (m_sessionJoinRequest.m_ptr != NULL) {
        if (userData == NULL) {
            return true;
        }
        bdReference<bdBitBuffer> buffer = m_sessionJoinRequest.m_ptr->getPayload();
        result = buffer.m_ptr->append(*userData);
        if (!result) {
            bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                                 "bool __thiscall bdSession::createJoinRequest(class bdBitBuffer *const )",
                                 0x218u, "dw/warn/");
            proxy.log("bdPeer/session", "Failed to append used data to join request");
            if (m_sessionJoinRequest.m_ptr != NULL && m_sessionJoinRequest.m_ptr->releaseRef() == 0)
                delete m_sessionJoinRequest.m_ptr;
            m_sessionJoinRequest.m_ptr = NULL;
        }
    } else {
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "bool __thiscall bdSession::createJoinRequest(class bdBitBuffer *const )",
                             0x21Fu, "dw/warn/");
        proxy.log("bdPeer/session", "Failed to allocate join request message.");
    }
    return result;
}

// ============================================================================
// bdSession::getConnection - ea: 0x8B16C0 (returns new reference)
// ============================================================================
bdReference<bdConnection> bdSession::getConnection(unsigned int index) const {
    bdReference<bdConnection> result;
    bdPeerData* peer = m_peers.m_data[index].m_connection.m_ptr != NULL
        ? &m_peers.m_data[index] : NULL;
    if (peer != NULL) {
        result.m_ptr = peer->m_connection.m_ptr;
        if (result.m_ptr != NULL)
            result.m_ptr->addRef();
    }
    return result;
}

// ============================================================================
// bdSession::send (all peers) - ea: 0x8B16F0
// ============================================================================
bool bdSession::send(const bdReference<bdMessage>& msg, bool reliable) {
    bool sent = true;
    for (unsigned int i = 0; i < m_peers.m_size; i++) {
        bdReference<bdConnection> conn = m_peers.m_data[i].m_connection;
        if (conn.m_ptr != NULL)
            sent = conn.m_ptr->send(msg, reliable) && sent;
    }
    return sent;
}

// ============================================================================
// bdSession::getPeerIndex - ea: 0x8B17D0
// ============================================================================
bool bdSession::getPeerIndex(const bdReference<bdConnection>& connection,
                             unsigned int& index) const {
    for (unsigned int i = 0; i < m_peers.m_size; i++) {
        if (m_peers.m_data[i].m_connection.m_ptr == connection.m_ptr) {
            index = i;
            return true;
        }
    }
    return false;
}

// ============================================================================
// bdSession::getPeerHash - ea: 0x8B1850
// ============================================================================
unsigned int bdSession::getPeerHash(unsigned int index) const {
    if (index < m_peers.m_size)
        return m_peers.m_data[index].m_hash;
    bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                         "unsigned int __thiscall bdSession::getPeerHash(const unsigned int) const",
                         0x1B4u, "dw/warn/");
    proxy.log("bdPeer/session", "Attempt made to read hash of  peer with invalid index: %u", index);
    return 0;
}
// ============================================================================
// bdSession::doLocalHash - ea: 0x8B18B0
// ============================================================================
void bdSession::doLocalHash() {
    unsigned int hash = 0;
    unsigned int numPeers = m_peers.m_size;
    for (unsigned int i = 0; i < numPeers; i++) {
        bdPeerData& peer = m_peers.m_data[i];
        bdReference<bdCommonAddr> addr = peer.m_connection.m_ptr->getAddress();
        if (addr.m_ptr != NULL)
            hash += (i + 1) * addr.m_ptr->getHash();
    }
    if (numPeers <= m_localPeerIndex) {
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "void __thiscall bdSession::doLocalHash(void)",
                             0x263u, "dw/err/");
        proxy.log("bdPeer/session", "Local index is %u but there are only %u peers.",
                  m_localPeerIndex, numPeers);
    } else {
        m_peers.m_data[m_localPeerIndex].m_hash = hash;
    }
}

// ============================================================================
// bdSession::sendConsistencyUpdates - ea: 0x8B19E0
// ============================================================================
void bdSession::sendConsistencyUpdates() {
    unsigned int numPeers = m_peers.m_size;
    if (numPeers <= m_localPeerIndex) {
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "void __thiscall bdSession::sendConsistencyUpdates(void)",
                             0x27Cu, "dw/err/");
        proxy.log("bdPeer/session", "Local index is %u but there are only %u peers.",
                  m_localPeerIndex, numPeers);
        return;
    }
    bdReference<bdMessage> msg(new (bdMemory::allocate(sizeof(bdMessage)))
        bdMessage(BD_SESSION_CONSISTENCY_MESSAGE, false));
    bdReference<bdBitBuffer> bitbuffer = msg.m_ptr->getPayload();
    unsigned int hash = m_peers.m_data[m_localPeerIndex].m_hash;
    bitbuffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_INTEGER32_TYPE);
    bitbuffer.m_ptr->writeBits(&hash, 32);

    for (unsigned int i = 0; i < numPeers; i++) {
        if (i != m_localPeerIndex) {
            bdReference<bdConnection> conn = m_peers.m_data[i].m_connection;
            if (conn.m_ptr != NULL)
                conn.m_ptr->send(msg, true);
        }
    }
}

// ============================================================================
// bdSession::setStatus - ea: 0x8B1B80
// ============================================================================
void bdSession::setStatus(bdSessionStatus status) {
    if (status != m_status) {
        for (unsigned int i = 0; i < m_listeners.m_size; i++)
            m_listeners.m_data[i]->onSessionStatusChange(m_status, status);
        m_status = status;
    }
}

// ============================================================================
// bdSession::setRole - ea: 0x8B1BD0
// ============================================================================
void bdSession::setRole(bdSessionRole role) {
    if (role != m_role) {
        for (unsigned int i = 0; i < m_listeners.m_size; i++)
            m_listeners.m_data[i]->onSessionRoleUpdate(role);
        m_role = role;
    }
}

// ============================================================================
// bdSession::registerInterceptor - ea: 0x8B20C0
// ============================================================================
void bdSession::registerInterceptor(bdSessionInterceptor* const interceptor) {
    m_interceptors.pushBack(interceptor);
}

// ============================================================================
// bdSession::registerListener - ea: 0x8B20F0
// ============================================================================
void bdSession::registerListener(bdSessionListener* const listener) {
    m_listeners.pushBack(listener);
}

// ============================================================================
// bdSession::send (specific peer) - ea: 0x8B2120
// ============================================================================
bool bdSession::send(const bdReference<bdConnection>& peerConnection,
                     const bdReference<bdMessage>& msg, bool reliable) {
    bool result = peerConnection.m_ptr->send(msg, reliable);
    if (!result) {
        unsigned int index = 0;
        getPeerIndex(peerConnection, index);
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "bool __thiscall bdSession::send(class bdReference<class bdConnection>,class bdReference<class bdMessage>,const bool)",
                             0x13Du, "dw/err/");
        proxy.log("bdPeer/session", "Sending of %s message to peer %u failed.",
                  reliable ? "reliable" : "unreliable", index);
    }
    return result;
}

// ============================================================================
// bdSession::unregisterInterceptor - ea: 0x8B2690
// ============================================================================
void bdSession::unregisterInterceptor(bdSessionInterceptor* const interceptor) {
    m_interceptors.removeAllKeepOrder(interceptor);
}

// ============================================================================
// bdSession::unregisterListener - ea: 0x8B26A0
// ============================================================================
void bdSession::unregisterListener(bdSessionListener* const listener) {
    m_listeners.removeAllKeepOrder(listener);
}

// ============================================================================
// bdSession::setHost - ea: 0x8B2930
// ============================================================================
void bdSession::setHost(unsigned int index) {
    bdPeerData* peer = &m_peers.m_data[index];
    if (m_hostConnection.m_ptr != NULL && m_hostConnection.m_ptr->releaseRef() == 0)
        delete m_hostConnection.m_ptr;
    m_hostConnection.m_ptr = peer->m_connection.m_ptr;
    if (m_hostConnection.m_ptr != NULL)
        m_hostConnection.m_ptr->addRef();

    if (index == m_localPeerIndex) {
        if (m_role != BD_SESSION_HOST) {
            setRole(BD_SESSION_HOST);
            bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                                 "void __thiscall bdSession::setHost(const unsigned int)",
                                 0xD0u, "dw/info/");
            proxy.log("bdPeer/session", "Promoted to host.");
        } else {
            bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                                 "void __thiscall bdSession::setHost(const unsigned int)",
                                 0xD4u, "dw/warn/");
            proxy.log("bdPeer/session", "Already host.");
        }
    } else {
        char addrStr[24];
        bdSingleton<bdAddressMapImpl>::getInstance()->addrToString(
            m_hostConnection.m_ptr->getAddressHandle(), addrStr, 22);
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "void __thiscall bdSession::setHost(const unsigned int)",
                             0xDBu, "dw/info/");
        proxy.log("bdPeer/session", "Peer %u (%s) promoted to host.", index, addrStr);
    }
}

// ============================================================================
// bdSession::startConnect - ea: 0x8B2A50
// ============================================================================
bool bdSession::startConnect(bdReference<bdConnection>& connection,
                             const bdReference<bdCommonAddr>& addr,
                             const XNKID& secID,
                             const char* const connectionDesc) {
    bool connecting = false;
    if (connection.m_ptr != NULL) {
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "bool __thiscall bdSession::startConnect(class bdReference<class bdConnection> &,const class bdReference<class bdCommonAddr>,const XNKID &,const char *const )",
                             0x1FCu, "dw/warn/");
        proxy.log("bdPeer/session", "%s connection already exists.", connectionDesc);
        return false;
    }
    // bdConnectionStore::create is an unresolved extern until that unit ports.
    extern bdReference<bdConnection> bdConnectionStore_create(
        const bdReference<bdCommonAddr>&, const XNKID&);
    connection = bdConnectionStore_create(addr, secID);
    if (connection.m_ptr == NULL) {
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "bool __thiscall bdSession::startConnect(class bdReference<class bdConnection> &,const class bdReference<class bdCommonAddr>,const XNKID &,const char *const )",
                             0x1F7u, "dw/warn/");
        proxy.log("bdPeer/session", "Failed to create %s connection.", connectionDesc);
        return false;
    }
    connecting = connection.m_ptr->connect();
    if (connecting) {
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "bool __thiscall bdSession::startConnect(class bdReference<class bdConnection> &,const class bdReference<class bdCommonAddr>,const XNKID &,const char *const )",
                             0x1EDu, "dw/info/");
        proxy.log("bdPeer/session", "Connecting to %s.", connectionDesc);
    } else {
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "bool __thiscall bdSession::startConnect(class bdReference<class bdConnection> &,const class bdReference<class bdCommonAddr>,const XNKID &,const char *const )",
                             0x1F1u, "dw/warn/");
        proxy.log("bdPeer/session", "Failed to initialize %s connection.", connectionDesc);
    }
    return connecting;
}

// ============================================================================
// bdSession::connectToLocalHost - ea: 0x8B3820
// ============================================================================
bool bdSession::connectToLocalHost(const XNKID& secID) {
    bdReference<bdCommonAddr> localAddr =
        bdSingleton<bdNetImpl>::getInstance()->getLocalCommonAddr();
    return startConnect(m_hostConnection, localAddr, secID, "local host");
}

// ============================================================================
// bdSession::connectToRemoteHost - ea: 0x8B38C0
// ============================================================================
bool bdSession::connectToRemoteHost(const bdReference<bdCommonAddr>& hostAddr,
                                    const XNKID& secID) {
    return startConnect(m_hostConnection, hostAddr, secID, "remote host");
}

// ============================================================================
// bdSession::connectToLocalPeer - ea: 0x8B3940
// ============================================================================
bool bdSession::connectToLocalPeer(const XNKID& secID) {
    bdReference<bdCommonAddr> localAddr =
        bdSingleton<bdNetImpl>::getInstance()->getLocalCommonAddr();
    return startConnect(m_localConnection, localAddr, secID, "local peer");
}

// ============================================================================
// bdSession::join - ea: 0x8B5330
// ============================================================================
bool bdSession::join(const bdReference<bdCommonAddr>& hostAddr, const XNKID& secID,
                     const XNKEY& secKey, bdBitBuffer* const userData) {
    bool ok = readyToConnect();
    if (!ok)
        return false;

    if (hostAddr.m_ptr->isLoopback()) {
        ok = connectToLocalHost(secID);
        if (ok) {
            m_localConnection = m_hostConnection;
            m_hostConnection.m_ptr->registerListener(this);
            setRole(BD_SESSION_HOST);
            setStatus(BD_SESSION_CONNECTING_TO_PEERS);
        }
    } else {
        ok = connectToRemoteHost(hostAddr, secID);
        ok = ok && connectToLocalPeer(secID);
        ok = ok && createJoinRequest(userData);
        if (ok) {
            m_hostConnection.m_ptr->registerListener(this);
            m_localConnection.m_ptr->registerListener(this);
            setRole(BD_SESSION_PEER);
            setStatus(BD_SESSION_CONNECTING_TO_HOST);
        }
    }

    if (ok) {
        bdSingleton<bdNetImpl>::getInstance()->registerDispatchInterceptor(this);
        m_secID = secID;
        m_secKey = secKey;
        m_gameSecID = secID;
        m_gameSecKey = secKey;
        m_registeredKey = false;
    } else {
        cleanup();
    }
    return ok;
}

// ============================================================================
// bdSession::leave - ea: 0x8B3790
// ============================================================================
void bdSession::leave() {
    if (m_status != BD_SESSION_NOT_CONNECTED) {
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "void __thiscall bdSession::leave(void)",
                             0x8Fu, "dw/info/");
        proxy.log("bdPeer/session", "Leaving session");
        setStatus(BD_SESSION_NOT_READY);
        cleanup();
        setStatus(BD_SESSION_NOT_CONNECTED);
    } else {
        bdMessageProxy proxy(".\\bdSession\\bdSession.cpp",
                             "void __thiscall bdSession::leave(void)",
                             0x9Cu, "dw/warn/");
        proxy.log("bdPeer/session", "Called while the session was not connected");
    }
}

// ============================================================================
// bdSession::cleanup - ea: 0x8B32E0
// ============================================================================
void bdSession::cleanup() {
    if (m_hostConnection.m_ptr != NULL) {
        m_hostConnection.m_ptr->unregisterListener(this);
        m_hostConnection.m_ptr->disconnect();
        if (m_hostConnection.m_ptr != NULL && m_hostConnection.m_ptr->releaseRef() == 0)
            delete m_hostConnection.m_ptr;
        m_hostConnection.m_ptr = NULL;
    }
    if (m_localConnection.m_ptr != NULL) {
        m_localConnection.m_ptr->unregisterListener(this);
        m_localConnection.m_ptr->disconnect();
        if (m_localConnection.m_ptr != NULL && m_localConnection.m_ptr->releaseRef() == 0)
            delete m_localConnection.m_ptr;
        m_localConnection.m_ptr = NULL;
    }
    if (m_joiningPeer.m_ptr != NULL) {
        m_joiningPeer.m_ptr->unregisterListener(this);
        m_joiningPeer.m_ptr->disconnect();
        if (m_joiningPeer.m_ptr != NULL && m_joiningPeer.m_ptr->releaseRef() == 0)
            delete m_joiningPeer.m_ptr;
        m_joiningPeer.m_ptr = NULL;
    }

    for (unsigned int i = 0; i < m_peers.m_size; i++) {
        bdPeerData& peer = m_peers.m_data[i];
        if (peer.m_connection.m_ptr != NULL)
            peer.m_connection.m_ptr->unregisterListener(this);
        if (peer.m_connection.m_ptr != NULL)
            peer.m_connection.m_ptr->disconnect();
    }
    bdMemory::deallocate(m_peers.m_data);
    m_peers.m_data = NULL;
    m_peers.m_size = 0;
    m_peers.m_capacity = 0;

    m_role = BD_SESSION_PEER;
    m_status = BD_SESSION_NOT_CONNECTED;
    m_localPeerIndex = 0;

    for (unsigned int i = 0; i < m_pendingConnections.m_size; i++) {
        if (m_pendingConnections.m_data[i].m_ptr != NULL)
            m_pendingConnections.m_data[i].m_ptr->unregisterListener(this);
        if (m_pendingConnections.m_data[i].m_ptr != NULL)
            m_pendingConnections.m_data[i].m_ptr->disconnect();
    }
    bdMemory::deallocate(m_pendingConnections.m_data);
    m_pendingConnections.m_data = NULL;
    m_pendingConnections.m_size = 0;
    m_pendingConnections.m_capacity = 0;

    m_pendingJoinRequests.clear();

    memset(&m_secKey, 0, sizeof(m_secKey));
    memset(&m_secID, 0, sizeof(m_secID));

    if (m_sessionJoinRequest.m_ptr != NULL && m_sessionJoinRequest.m_ptr->releaseRef() == 0)
        delete m_sessionJoinRequest.m_ptr;
    m_sessionJoinRequest.m_ptr = NULL;

    if (m_registeredKey) {
        bdSecurityKeyMap* keyMap =
            bdSingleton<bdNetImpl>::getInstance()->getKeyMap();
        if (keyMap != NULL)
            keyMap->remove(m_gameSecID);
        m_registeredKey = false;
    }
    memset(&m_gameSecKey, 0, sizeof(m_gameSecKey));
    memset(&m_gameSecID, 0, sizeof(m_gameSecID));

    bdSingleton<bdNetImpl>::getInstance()->unregisterDispatchInterceptor(this);
    bdSingleton<bdNetImpl>::getInstance()->unregisterDispatchInterceptor(this);
}

// ============================================================================
// bdSession::accept - ea: 0x8B5500 (dispatch interceptor)
// ============================================================================
bool bdSession::accept(bdReceivedMessage& message) {
    unsigned char type = message.getMessage().m_ptr->getType();
    switch (type) {
    case BD_SESSION_JOIN_REQ:
        handleJoinRequest(message);
        return true;
    case BD_SESSION_JOIN_REPLY:
        handleJoinReply(message);
        return true;
    case BD_SESSION_CONSISTENCY_MESSAGE:
        handleConsistencyMessage(message);
        return true;
    default:
        return false;
    }
}

// ============================================================================
// bdSession::onConnect - ea: 0x8B3BB0 (connection listener)
// ============================================================================
void bdSession::onConnect(const bdReference<bdConnection>& connection) {
    if (m_status == BD_SESSION_CONNECTING_TO_PEERS) {
        unsigned int index = 0;
        if (getPeerIndex(connection, index)) {
            // connected to the pending peer
        }
        if (m_pendingConnections.m_size == 0) {
            // local loopback peer
            bdPeerData data(m_localConnection, 0);
            bdArrayAppend(m_peers, data);
            m_localPeerIndex = m_peers.m_size - 1;
            doLocalHash();
            checkSessionConsistency();
            sendConsistencyUpdates();
            for (unsigned int i = 0; i < m_listeners.m_size; i++)
                m_listeners.m_data[i]->onSessionConnectSuccess();
        }
    } else if (m_status == BD_SESSION_CONNECTING_TO_HOST && connection.m_ptr == m_hostConnection.m_ptr) {
        setStatus(BD_SESSION_CONNECTED_TO_HOST);
        if (m_sessionJoinRequest.m_ptr != NULL) {
            m_hostConnection.m_ptr->send(bdReference<bdMessage>(m_sessionJoinRequest.m_ptr), true);
            if (m_sessionJoinRequest.m_ptr != NULL && m_sessionJoinRequest.m_ptr->releaseRef() == 0)
                delete m_sessionJoinRequest.m_ptr;
            m_sessionJoinRequest.m_ptr = NULL;
        }
    }
}

// ============================================================================
// bdSession::onConnectFailed - ea: 0x8B3EB0 (connection listener)
// ============================================================================
void bdSession::onConnectFailed(const bdReference<bdConnection>& connection) {
    bool hostFailed = m_hostConnection.m_ptr == connection.m_ptr;
    unsigned int index = 0;
    bool peerFailed = false;
    for (unsigned int i = 0; i < m_pendingConnections.m_size; i++) {
        if (m_pendingConnections.m_data[i].m_ptr == connection.m_ptr) {
            index = i;
            peerFailed = true;
            break;
        }
    }
    if (peerFailed || hostFailed) {
        switch (m_status) {
        case BD_SESSION_CONNECTING_TO_HOST:
            if (m_sessionJoinRequest.m_ptr != NULL && m_sessionJoinRequest.m_ptr->releaseRef() == 0)
                delete m_sessionJoinRequest.m_ptr;
            m_sessionJoinRequest.m_ptr = NULL;
            setStatus(BD_SESSION_CONNECTING_TO_HOST_FAILED);
            break;
        case BD_SESSION_CONNECTING_TO_PEERS:
            setStatus(BD_SESSION_CONNECTING_TO_PEERS_FAILED);
            break;
        default:
            setStatus(BD_SESSION_NOT_CONNECTED);
            break;
        }
        for (unsigned int i = 0; i < m_listeners.m_size; i++)
            m_listeners.m_data[i]->onSessionConnectFail();
        setStatus(BD_SESSION_NOT_CONNECTED);
        cleanup();
    }
}

// ============================================================================
// bdSession::onDisconnect - ea: 0x8B4050 (connection listener)
// ============================================================================
void bdSession::onDisconnect(const bdReference<bdConnection>& connection) {
    bool hostDisconnected = m_hostConnection.m_ptr == connection.m_ptr;
    bool localDisconnected = m_localConnection.m_ptr == connection.m_ptr;
    bool joiningDisconnected = m_joiningPeer.m_ptr == connection.m_ptr;

    // find and remove from peers
    for (unsigned int i = 0; i < m_peers.m_size; i++) {
        if (m_peers.m_data[i].m_connection.m_ptr == connection.m_ptr) {
            // shift down
            for (unsigned int j = i; j + 1 < m_peers.m_size; j++)
                m_peers.m_data[j] = m_peers.m_data[j + 1];
            m_peers.m_size--;
            break;
        }
    }
    // remove from pending
    for (unsigned int i = 0; i < m_pendingConnections.m_size; i++) {
        if (m_pendingConnections.m_data[i].m_ptr == connection.m_ptr) {
            for (unsigned int j = i; j + 1 < m_pendingConnections.m_size; j++)
                m_pendingConnections.m_data[j] = m_pendingConnections.m_data[j + 1];
            m_pendingConnections.m_size--;
            break;
        }
    }

    if (hostDisconnected)
        m_hostConnection.m_ptr = NULL;
    if (localDisconnected)
        m_localConnection.m_ptr = NULL;
    if (joiningDisconnected)
        m_joiningPeer.m_ptr = NULL;

    if (m_peers.m_size == 0) {
        setStatus(BD_SESSION_NOT_CONNECTED);
        return;
    }
    if (m_role == BD_SESSION_HOST) {
        doLocalHash();
        sendConsistencyUpdates();
    }
    checkSessionConsistency();
}

// ============================================================================
// bdSession::checkSessionConsistency - ea: 0x8B39E0
// ============================================================================
void bdSession::checkSessionConsistency() {
    bool consistent = true;
    unsigned int numPeers = m_peers.m_size;
    for (unsigned int i = 1; consistent && i < numPeers; i++) {
        consistent = (m_peers.m_data[0].m_hash == m_peers.m_data[i].m_hash);
    }
    if (consistent) {
        setStatus(BD_SESSION_READY);
        for (unsigned int i = 0; m_joiningPeer.m_ptr != NULL && i < numPeers; i++) {
            if (m_joiningPeer.m_ptr == m_peers.m_data[i].m_connection.m_ptr) {
                for (unsigned int l = 0; l < m_listeners.m_size; l++)
                    m_listeners.m_data[l]->onSessionConnect(m_joiningPeer);
                m_joiningPeer.m_ptr = NULL;
            }
        }
        processPendingJoinRequest();
    } else {
        setStatus(BD_SESSION_NOT_READY);
    }
}

// ============================================================================
// bdSession::processJoinRequest - ea: 0x8B2C40
// ============================================================================
void bdSession::processJoinRequest(const bdReceivedMessage& joinRequest) {
    bdReference<bdMessage> message = joinRequest.getMessage();
    bdReference<bdConnection> connection = joinRequest.getConnection();
    bool joinAccepted = true;

    unsigned int numPeers = m_peers.m_size;
    unsigned char numPeers8 = (unsigned char)numPeers;
    if (numPeers > numPeers8)
        joinAccepted = false;

    bdBitBuffer userData;
    if (m_handler != NULL && joinAccepted) {
        bdReference<bdBitBuffer> buffer = message.m_ptr->getPayload();
        joinAccepted = m_handler->onSessionConnectRequest(buffer, &userData);
    }

    bdReference<bdMessage> joinReply(new (bdMemory::allocate(sizeof(bdMessage)))
        bdMessage(BD_SESSION_JOIN_REPLY, false));
    bdReference<bdBitBuffer> buffer = joinReply.m_ptr->getPayload();
    buffer.m_ptr->writeBool(joinAccepted);

    if (joinAccepted) {
        buffer.m_ptr->writeBits(m_gameSecID.ab, 64);
        buffer.m_ptr->writeBits(m_gameSecKey.ab, 128);
        buffer.m_ptr->writeBits(&numPeers8, 8);
        for (unsigned int i = 0; i < numPeers; i++) {
            bdReference<bdCommonAddr> addr = m_peers.m_data[i].m_connection.m_ptr->getAddress();
            uint8_t tmp[44];
            addr.m_ptr->serialize(tmp);
            buffer.m_ptr->writeBits(tmp, 0x150u);
        }
        m_joiningPeer = connection;
    } else {
        connection.m_ptr->unregisterListener(this);
        buffer.m_ptr->append(userData);
    }
    connection.m_ptr->send(joinReply, true);
}

// ============================================================================
// bdSession::processPendingJoinRequest - ea: 0x8B3260
// ============================================================================
void bdSession::processPendingJoinRequest() {
    if (m_role == BD_SESSION_HOST && m_joiningPeer.m_ptr == NULL &&
        m_status == BD_SESSION_READY && m_pendingJoinRequests.m_head != NULL) {
        const bdReceivedMessage& joinRequest = m_pendingJoinRequests.getHead();
        processJoinRequest(joinRequest);
        m_pendingJoinRequests.removeHead();
    }
}

// ============================================================================
// bdSession::handleJoinRequest - ea: 0x8B4430
// ============================================================================
void bdSession::handleJoinRequest(const bdReceivedMessage& message) {
    if (m_role == BD_SESSION_HOST && m_status == BD_SESSION_READY &&
        m_joiningPeer.m_ptr == NULL) {
        processJoinRequest(message);
    } else {
        bdReceivedMessage* copy = new (bdMemory::allocate(sizeof(bdReceivedMessage)))
            bdReceivedMessage(message.getMessage(), message.getConnection());
        m_pendingJoinRequests.addTail(*copy);
        bdMemory::deallocate(copy);
    }
}

// ============================================================================
// bdSession::handleJoinReply - ea: 0x8B45C0
// ============================================================================
void bdSession::handleJoinReply(const bdReceivedMessage& message) {
    bdReference<bdMessage> msg = message.getMessage();
    bdReference<bdBitBuffer> buffer = msg.m_ptr->getPayload();
    bool joinAllowed = false;
    bool ok = buffer.m_ptr->readBool(joinAllowed);
    bool cleanUp = false;

    if (ok && joinAllowed) {
        bdReference<bdCommonAddr> localAddr =
            bdSingleton<bdNetImpl>::getInstance()->getLocalCommonAddr();
        XNKID gameSecID;
        XNKEY gameSecKey;
        memset(&gameSecID, 0, sizeof(gameSecID));
        memset(&gameSecKey, 0, sizeof(gameSecKey));
        ok = ok && buffer.m_ptr->readBits(gameSecID.ab, 64);
        ok = ok && buffer.m_ptr->readBits(gameSecKey.ab, 128);
        unsigned char numPeers = 0;
        ok = ok && buffer.m_ptr->readBits(&numPeers, 8);

        for (unsigned int i = 0; ok && i < numPeers; i++) {
            bdReference<bdCommonAddr> addr(new (bdMemory::allocate(sizeof(bdCommonAddr))) bdCommonAddr());
            uint8_t tmp[44];
            ok = buffer.m_ptr->readBits(tmp, 0x150u);
            if (ok)
                ok = addr.m_ptr->deserialize(localAddr, tmp);
        }

        if (ok) {
            for (unsigned int i = 0; i < m_listeners.m_size; i++)
                m_listeners.m_data[i]->onSessionJoinAccepted();
            setStatus(BD_SESSION_CONNECTING_TO_PEERS);
        } else {
            cleanUp = true;
        }
    } else if (ok) {
        for (unsigned int i = 0; i < m_listeners.m_size; i++)
            m_listeners.m_data[i]->onSessionJoinRefused(buffer);
        cleanUp = true;
    } else {
        cleanUp = true;
    }

    if (cleanUp) {
        setStatus(BD_SESSION_NOT_CONNECTED);
        cleanup();
    }
}

// ============================================================================
// bdSession::handleConsistencyMessage - ea: 0x8B5020
// ============================================================================
void bdSession::handleConsistencyMessage(const bdReceivedMessage& message) {
    if (m_localConnection.m_ptr == NULL)
        return;
    bdReference<bdMessage> msg = message.getMessage();
    bdReference<bdBitBuffer> bitbuffer = msg.m_ptr->getPayload();
    bdReference<bdConnection> connection = message.getConnection();
    unsigned int index = 0;
    bool ok = false;
    if (getPeerIndex(connection, index)) {
        unsigned int hash = 0;
        ok = bitbuffer.m_ptr->readDataType(bdBitBuffer::BD_BB_UNSIGNED_INTEGER32_TYPE);
        ok = ok && bitbuffer.m_ptr->readBits(&hash, 32);
        if (ok)
            m_peers.m_data[index].m_hash = hash;
    } else {
        if (m_joiningPeer.m_ptr == NULL) {
            connection.m_ptr->registerListener(this);
        }
        bdPeerData data(connection, 0);
        unsigned int hash = 0;
        ok = bitbuffer.m_ptr->readDataType(bdBitBuffer::BD_BB_UNSIGNED_INTEGER32_TYPE);
        ok = ok && bitbuffer.m_ptr->readBits(&hash, 32);
        if (ok) {
            data.m_hash = hash;
            bdArrayAppend(m_peers, data);
            doLocalHash();
            sendConsistencyUpdates();
            m_joiningPeer = connection;
        }
    }
    if (ok)
        checkSessionConsistency();
}

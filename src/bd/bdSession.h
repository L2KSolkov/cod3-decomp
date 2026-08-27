// ============================================================================
// bdSession - peer-to-peer session (0x98 bytes).
// Source: bdPeer:bdSession.obj (44 funcs).
// Layout verified against IDA (ctor @0x8B28A0, cleanup @0x8B32E0):
//   +0 bdDispatchInterceptor base, +4 bdConnectionListener base,
//   +8 m_hostConnection, +0xC m_localConnection, +0x10 m_joiningPeer,
//   +0x14 m_peers, +0x20 m_interceptors, +0x2C m_listeners, +0x38 m_handler,
//   +0x3C m_role, +0x40 m_status, +0x44 m_localPeerIndex,
//   +0x48 m_pendingConnections, +0x54 m_pendingJoinRequests,
//   +0x60 m_secID, +0x68 m_secKey, +0x78 m_sessionJoinRequest,
//   +0x7C m_registeredKey, +0x7D m_gameSecID, +0x85 m_gameSecKey.
// ============================================================================

#ifndef COD3_BD_BDSESSION_H
#define COD3_BD_BDSESSION_H

#include "bd/bd_types.h"
#include "bd/bdGameInfo.h"
#include "bd/bdDispatchInterceptor.h"

class bdSessionHandler;
class bdSessionInterceptor;
class bdSessionListener;
class bdReceivedMessage;
class bdBitBuffer;

// ============================================================================
// bdPeerData - one peer's connection + consistency hash (8 bytes).
// ============================================================================
struct bdPeerData {
    bdReference<bdConnection> m_connection;  // +0x00
    unsigned int m_hash;                     // +0x04

    bdPeerData() : m_connection(), m_hash(0) {}
    bdPeerData(const bdReference<bdConnection>& connection, unsigned int hash)
        : m_connection(connection), m_hash(hash) {
        if (m_connection.m_ptr != NULL)
            m_connection.m_ptr->addRef();
    }
    ~bdPeerData() {
        if (m_connection.m_ptr != NULL && m_connection.m_ptr->releaseRef() == 0)
            delete m_connection.m_ptr;
        m_connection.m_ptr = NULL;
    }
    bdPeerData(const bdPeerData& other)
        : m_connection(other.m_connection), m_hash(other.m_hash) {
        if (m_connection.m_ptr != NULL)
            m_connection.m_ptr->addRef();
    }
    bdPeerData& operator=(const bdPeerData& other) {
        if (this != &other) {
            if (m_connection.m_ptr != NULL && m_connection.m_ptr->releaseRef() == 0)
                delete m_connection.m_ptr;
            m_connection.m_ptr = other.m_connection.m_ptr;
            if (m_connection.m_ptr != NULL)
                m_connection.m_ptr->addRef();
            m_hash = other.m_hash;
        }
        return *this;
    }
};
static_assert(sizeof(bdPeerData) == 0x08, "bdPeerData size mismatch");

// ============================================================================
// bdSession
// ============================================================================
class bdSession : public bdDispatchInterceptor, public bdConnectionListener {
public:
    enum bdSessionRole {
        BD_SESSION_HOST = 0,
        BD_SESSION_PEER = 1,
    };

    enum bdSessionStatus {
        BD_SESSION_NOT_CONNECTED = 0,
        BD_SESSION_CONNECTING_TO_HOST = 1,
        BD_SESSION_CONNECTED_TO_HOST = 2,
        BD_SESSION_CONNECTING_TO_PEERS = 3,
        BD_SESSION_NOT_READY = 4,
        BD_SESSION_READY = 5,
        BD_SESSION_CONNECTING_TO_HOST_FAILED = 6,
        BD_SESSION_CONNECTING_TO_PEERS_FAILED = 7,
    };

    bdSession(bdSessionHandler* const sessionHandler);
    virtual ~bdSession();

    // bdDispatchInterceptor
    virtual bool accept(bdReceivedMessage& message);

    // bdConnectionListener
    virtual void onConnect(bdReference<bdConnection> connection);
    virtual void onConnectFailed(bdReference<bdConnection> connection);
    virtual void onDisconnect(bdReference<bdConnection> connection);

    bool join(const bdReference<bdCommonAddr>& hostAddr, const XNKID& secID,
              const XNKEY& secKey, bdBitBuffer* const userData);
    void leave();
    bool send(const bdReference<bdMessage>& msg, bool reliable);
    bool send(const bdReference<bdConnection>& peerConnection,
              const bdReference<bdMessage>& msg, bool reliable);
    bool sendHost(const bdReference<bdMessage>& msg, bool reliable);
    void registerListener(bdSessionListener* const listener);
    void unregisterListener(bdSessionListener* const listener);
    void registerInterceptor(bdSessionInterceptor* const interceptor);
    void unregisterInterceptor(bdSessionInterceptor* const interceptor);
    void setHost(unsigned int index);
    bdSessionStatus getStatus() const;
    bdSessionRole getRole() const;
    unsigned int getNumPeers() const;
    unsigned int getLocalPeerIndex() const;
    bool readyToConnect() const;
    bdReference<bdConnection> getConnection(unsigned int index) const;
    bdReference<bdConnection> getHost() const;
    bool getPeerIndex(const bdReference<bdConnection>& connection,
                      unsigned int& index) const;
    unsigned int getPeerHash(unsigned int index) const;

protected:
    bool startConnect(bdReference<bdConnection>& connection,
                      const bdReference<bdCommonAddr>& addr, const XNKID& secID,
                      const char* const connectionDesc);
    bool connectToLocalHost(const XNKID& secID);
    bool connectToRemoteHost(const bdReference<bdCommonAddr>& hostAddr,
                             const XNKID& secID);
    bool connectToLocalPeer(const XNKID& secID);
    bool createJoinRequest(bdBitBuffer* const userData);
    void checkSessionConsistency();
    void doLocalHash();
    void sendConsistencyUpdates();
    void setStatus(bdSessionStatus status);
    void setRole(bdSessionRole role);
    void processJoinRequest(const bdReceivedMessage& joinRequest);
    void processPendingJoinRequest();
    void handleJoinRequest(const bdReceivedMessage& message);
    void handleJoinReply(const bdReceivedMessage& message);
    void handleConsistencyMessage(const bdReceivedMessage& message);
    void cleanup();

protected:
    bdReference<bdConnection> m_hostConnection;       // +0x08
    bdReference<bdConnection> m_localConnection;      // +0x0C
    bdReference<bdConnection> m_joiningPeer;          // +0x10
    bdArray<bdPeerData> m_peers;                      // +0x14
    bdFastArray<bdSessionInterceptor*> m_interceptors;  // +0x20
    bdFastArray<bdSessionListener*> m_listeners;      // +0x2C
    bdSessionHandler* m_handler;                      // +0x38
    bdSessionRole m_role;                             // +0x3C
    bdSessionStatus m_status;                         // +0x40
    unsigned int m_localPeerIndex;                    // +0x44
    bdArray<bdReference<bdConnection> > m_pendingConnections;   // +0x48
    bdLinkedList<bdReceivedMessage> m_pendingJoinRequests;      // +0x54
    XNKID m_secID;                                    // +0x60
    XNKEY m_secKey;                                   // +0x68
    bdReference<bdMessage> m_sessionJoinRequest;      // +0x78
    bool m_registeredKey;                             // +0x7C
    XNKID m_gameSecID;                                // +0x7D (unaligned)
    XNKEY m_gameSecKey;                               // +0x85 (unaligned)
};
static_assert(sizeof(bdSession) == 0x98, "bdSession size mismatch");

// bdSession message types (COD3 wire values)
enum {
    BD_SESSION_JOIN_REQ = 10,
    BD_SESSION_JOIN_REPLY = 11,
    BD_SESSION_CONSISTENCY_MESSAGE = 12,
    BD_UPDATE_PEER_LIST = 13,
};

#endif // COD3_BD_BDSESSION_H

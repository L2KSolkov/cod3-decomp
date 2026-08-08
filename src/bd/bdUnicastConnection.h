// ============================================================================
// bdUnicastConnection - reliable unicast connection state machine (312 bytes).
// Source: bdConnection:bdUnicastConnection.obj (36 funcs).
// Layout verified against IDA (ctor 0x8A6D00 / 0x8A6E30):
//   +0x64 m_reliableSendWindow, +0x68 m_reliableRecvWindow,
//   +0x6C m_unreliableSendWindow, +0x80 m_unreliableReceiveWindow,
//   +0x94 m_outQueue, +0xA0 m_sendTimer, +0xB0 m_receiveTimer,
//   +0xC0 m_state, +0xC4 m_localTag, +0xC8 m_peerTag, +0xD0 m_initTimer,
//   +0xE0 m_initResends, +0xE8 m_cookieTimer, +0xF8 m_cookieResends,
//   +0x100 m_shutdownTimer, +0x110 m_shutdownResends, +0x118 m_shutdownGuard,
//   +0x128 m_initAckChunk, +0x12C m_smoothedRTT, +0x130 m_RTTVariation.
// ============================================================================

#ifndef COD3_BD_BDUNICASTCONNECTION_H
#define COD3_BD_BDUNICASTCONNECTION_H

#include "bd/bd_types.h"
#include "bd/bdGameInfo.h"
#include "bd/bdStopwatch.h"

class bdReliableSendWindow;
class bdReliableReceiveWindow;
class bdUnreliableSendWindow;
class bdUnreliableReceiveWindow;
class bdInitAckChunk;
class bdInitChunk;

// ============================================================================
// bdControlChunkStore - outbound control chunk queue entry (8 bytes).
// ============================================================================
struct bdControlChunkStore {
    bdReference<bdChunk> m_chunk;   // +0x00
    bool m_lone;                    // +0x04

    bdControlChunkStore() : m_chunk(), m_lone(false) {}
    bdControlChunkStore(const bdReference<bdChunk>& chunk, bool lone = false)
        : m_chunk(chunk), m_lone(lone) {
        if (m_chunk.m_ptr != NULL)
            m_chunk.m_ptr->addRef();
    }
    bdControlChunkStore(const bdControlChunkStore& other)
        : m_chunk(other.m_chunk), m_lone(other.m_lone) {
        if (m_chunk.m_ptr != NULL)
            m_chunk.m_ptr->addRef();
    }
    bdControlChunkStore& operator=(const bdControlChunkStore& other) {
        if (this != &other) {
            if (m_chunk.m_ptr != NULL && m_chunk.m_ptr->releaseRef() == 0)
                delete m_chunk.m_ptr;
            m_chunk.m_ptr = other.m_chunk.m_ptr;
            if (m_chunk.m_ptr != NULL)
                m_chunk.m_ptr->addRef();
            m_lone = other.m_lone;
        }
        return *this;
    }
    ~bdControlChunkStore() {
        if (m_chunk.m_ptr != NULL && m_chunk.m_ptr->releaseRef() == 0)
            delete m_chunk.m_ptr;
        m_chunk.m_ptr = NULL;
    }
};

// bdLinkedList<T> releases stored values on removal; this overload makes sure
// the bdControlChunkStore destructor (and its chunk ref) runs.
inline void bdListRelease(bdControlChunkStore& value) {
    value.~bdControlChunkStore();
}

// ============================================================================
// bdUnicastConnection
// ============================================================================
class bdUnicastConnection : public bdConnection {
public:
    virtual ~bdUnicastConnection();

    virtual bool connect();
    virtual void disconnect();
    virtual void close();
    virtual Status getStatus() const;
    virtual bool send(const bdReference<bdMessage>& message, bool reliable = false);
    virtual bool getMessageToDispatch(bdReference<bdMessage>& message);

protected:
    friend class bdConnectionStore;
    bdUnicastConnection();
    bdUnicastConnection(const bdReference<bdCommonAddr>& dest);

    virtual bool receive(const unsigned char* buffer, unsigned int bufferSize);
    virtual unsigned int getDataToSend(unsigned char* buffer, unsigned int bufferSize);

    bool handleInit(bdReference<bdChunk>& chunk);
    bool handleInitAck(bdReference<bdChunk>& chunk, unsigned int vtag);
    bool handleCookieEcho(bdReference<bdChunk>& chunk, unsigned int vtag);
    bool handleCookieAck(bdReference<bdChunk>& chunk, unsigned int vtag);
    bool handleData(bdReference<bdChunk>& chunk);
    bool handleSAck(bdReference<bdChunk>& chunk);
    bool handleHeartbeat(bdReference<bdChunk>& chunk);
    bool handleHeartbeatAck(bdReference<bdChunk>& chunk);
    bool handleShutdown(bdReference<bdChunk>& chunk);
    bool handleShutdownAck(bdReference<bdChunk>& chunk);
    bool handleShutdownComplete(bdReference<bdChunk>& chunk);

    bool sendInit();
    bool sendInitAck(const bdReference<bdInitChunk>& chunk);
    bool sendCookieEcho(const bdReference<bdInitAckChunk>& chunk);
    bool sendCookieAck();
    bool sendHeartbeat(const bdReference<bdInitChunk>& chunk);
    bool sendHeartbeatAck(const bdReference<bdInitChunk>& chunk);
    bool sendShutdown();
    bool sendShutdownAck();
    bool sendShutdownComplete();

    void reset();
    void callListenersConnect(bool success);
    void callListenersDisconnect();
    void callListenersReconnect();
    bool windowsEmpty() const;

protected:
    enum bdUCState {
        BD_UC_CLOSED = 0,
        BD_UC_COOKIE_WAIT = 1,
        BD_UC_COOKIE_ECHOED = 2,
        BD_UC_ESTABLISHED = 3,
        BD_UC_SHUTDOWN_PENDING = 4,
        BD_UC_SHUTDOWN_SENT = 5,
        BD_UC_SHUTDOWN_RECEIVED = 6,
        BD_UC_SHUTDOWN_ACK_SENT = 7,
    };

    bdReliableSendWindow* m_reliableSendWindow;      // +0x64
    bdReliableReceiveWindow* m_reliableRecvWindow;   // +0x68
    bdUnreliableSendWindow m_unreliableSendWindow;   // +0x6C
    bdUnreliableReceiveWindow m_unreliableReceiveWindow;  // +0x80
    bdLinkedList<bdControlChunkStore> m_outQueue;    // +0x94
    bdStopwatch m_sendTimer;                         // +0xA0
    bdStopwatch m_receiveTimer;                      // +0xB0
    bdUCState m_state;                               // +0xC0
    unsigned int m_localTag;                         // +0xC4
    unsigned int m_peerTag;                          // +0xC8
    bdStopwatch m_initTimer;                         // +0xD0
    unsigned char m_initResends;                     // +0xE0
    bdStopwatch m_cookieTimer;                       // +0xE8
    unsigned char m_cookieResends;                   // +0xF8
    bdStopwatch m_shutdownTimer;                     // +0x100
    unsigned char m_shutdownResends;                 // +0x110
    bdStopwatch m_shutdownGuard;                     // +0x118
    bdReference<bdInitAckChunk> m_initAckChunk;      // +0x128
    float m_smoothedRTT;                             // +0x12C
    float m_RTTVariation;                            // +0x130
};
static_assert(sizeof(bdUnicastConnection) == 0x138, "bdUnicastConnection size mismatch");

#endif // COD3_BD_BDUNICASTCONNECTION_H
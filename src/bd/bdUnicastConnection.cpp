// ============================================================================
// bdUnicastConnection.cpp - reliable unicast connection state machine
// (35 non-inline funcs). Source: bdConnection:bdUnicastConnection.obj
// Verified against IDA (release decompilation) + Demonware 2.0 reference.
// ============================================================================

#include "bd/bdUnicastConnection.h"

#include <new>
#include <math.h>
#include <string.h>

#include "bd/bdAddressMap.h"

#define BD_MAX_MESSAGE_SIZE 0x504
#define BD_UC_INIT_TIMEOUT 0.5f
#define BD_UC_COOKIE_TIMEOUT 0.5f
#define BD_UC_SHUTDOWN_TIMEOUT 0.5f
#define BD_UC_SHUTDOWN_GUARD 5.0f
#define BD_UC_ALIVE_PERIOD 9.0f
#define BD_UC_KEEP_ALIVE_TIMEOUT 1.8f
#define BD_UC_INIT_RESENDS 5
#define BD_UC_COOKIE_RESENDS 5
#define BD_UC_SHUTDOWN_RESENDS 5
#define BD_UC_RTO_ALPHA 0.125f
#define BD_UC_RTO_BETA 0.25f
#define BD_UC_CLOCK_GRANULARITY 100.0f
#define BD_UC_RTO_MIN 0.02f
#define BD_UC_RTO_MAX 2.0f

struct bdTrulyRandomImpl {
public:
    unsigned int getRandomUInt();
};

// ============================================================================
// bdUnicastConnection (default) - ea: 0x8A6E30
// ============================================================================
bdUnicastConnection::bdUnicastConnection()
    : bdConnection(), m_reliableSendWindow(NULL), m_reliableRecvWindow(NULL),
      m_unreliableSendWindow(), m_unreliableReceiveWindow(), m_outQueue(),
      m_sendTimer(), m_receiveTimer(), m_state(BD_UC_CLOSED), m_localTag(0),
      m_peerTag(0), m_initTimer(), m_initResends(0), m_cookieTimer(),
      m_cookieResends(0), m_shutdownTimer(), m_shutdownResends(0),
      m_shutdownGuard(), m_initAckChunk(), m_smoothedRTT(0.0f),
      m_RTTVariation(0.0f) {
    reset();
}

// ============================================================================
// bdUnicastConnection (dest) - ea: 0x8A6D00
// ============================================================================
bdUnicastConnection::bdUnicastConnection(const bdReference<bdCommonAddr>& dest)
    : bdConnection(dest), m_reliableSendWindow(NULL), m_reliableRecvWindow(NULL),
      m_unreliableSendWindow(), m_unreliableReceiveWindow(), m_outQueue(),
      m_sendTimer(), m_receiveTimer(), m_state(BD_UC_CLOSED), m_localTag(0),
      m_peerTag(0), m_initTimer(), m_initResends(0), m_cookieTimer(),
      m_cookieResends(0), m_shutdownTimer(), m_shutdownResends(0),
      m_shutdownGuard(), m_initAckChunk(), m_smoothedRTT(0.0f),
      m_RTTVariation(0.0f) {
    reset();
}

// ============================================================================
// bdUnicastConnection::~bdUnicastConnection - ea: 0x8A5D00
// ============================================================================
bdUnicastConnection::~bdUnicastConnection() {
    if (m_reliableRecvWindow != NULL)
        delete m_reliableRecvWindow;
    if (m_reliableSendWindow != NULL)
        delete m_reliableSendWindow;
    if (m_initAckChunk.m_ptr != NULL && m_initAckChunk.m_ptr->releaseRef() == 0)
        delete m_initAckChunk.m_ptr;
    m_initAckChunk.m_ptr = NULL;
    m_outQueue.clear();
}

// ============================================================================
// bdUnicastConnection::connect - ea: 0x8A6C90
// ============================================================================
bool bdUnicastConnection::connect() {
    if (m_state != BD_UC_CLOSED || !bdConnection::connect() || !sendInit()) {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::connect(void)",
                             0xB2u, "dw/warn/");
        proxy.log("bdConnection/connections", "failed to connect!");
        return false;
    }
    m_state = BD_UC_COOKIE_WAIT;
    return true;
}

// ============================================================================
// bdUnicastConnection::disconnect - ea: 0x8A5950
// ============================================================================
void bdUnicastConnection::disconnect() {
    if (m_state > BD_UC_CLOSED) {
        if (m_state <= BD_UC_COOKIE_ECHOED) {
            close();
        } else if (m_state == BD_UC_ESTABLISHED) {
            m_state = BD_UC_SHUTDOWN_PENDING;
            m_shutdownGuard.start();
            callListenersDisconnect();
        }
    }
}

// ============================================================================
// bdUnicastConnection::close - ea: 0x8A5990
// ============================================================================
void bdUnicastConnection::close() {
    if (m_state != BD_UC_CLOSED) {
        if (m_state < BD_UC_ESTABLISHED) {
            callListenersConnect(false);
        } else if (m_state == BD_UC_ESTABLISHED) {
            callListenersDisconnect();
        }
    }
    m_state = BD_UC_CLOSED;
}

// ============================================================================
// bdUnicastConnection::getStatus - ea: 0x8A43E0
// ============================================================================
bdConnection::Status bdUnicastConnection::getStatus() const {
    switch (m_state) {
    case BD_UC_CLOSED: return BD_DISCONNECTED;
    case BD_UC_COOKIE_WAIT:
    case BD_UC_COOKIE_ECHOED: return BD_CONNECTING;
    case BD_UC_ESTABLISHED: return BD_CONNECTED;
    case BD_UC_SHUTDOWN_PENDING:
    case BD_UC_SHUTDOWN_SENT:
    case BD_UC_SHUTDOWN_RECEIVED:
    case BD_UC_SHUTDOWN_ACK_SENT: return BD_DISCONNECTING;
    default:
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "enum bdConnection::Status __thiscall bdUnicastConnection::getStatus(void) const",
                             0x128u, "dw/warn/");
        proxy.log("bdConnection/connections", "Unknown state");
        return BD_DISCONNECTED;
    }
}

// ============================================================================
// bdUnicastConnection::send - ea: 0x8A4D20
// ============================================================================
bool bdUnicastConnection::send(const bdReference<bdMessage>& message, bool reliable) {
    if (m_state != BD_UC_ESTABLISHED) {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::send(const class bdReference<class bdMessage>,const bool)",
                             0xE3u, "dw/warn/");
        proxy.log("bdConnection/connections", "connection not established.");
    }

    unsigned int payloadSize = 0;
    if (message.m_ptr->hasPayload())
        payloadSize += message.m_ptr->getPayload().m_ptr->getDataSize();
    if (message.m_ptr->hasUnencryptedPayload())
        payloadSize += message.m_ptr->getUnencryptedPayload().m_ptr->getMaxReadSize();

    bool sent = false;
    if (payloadSize > BD_MAX_MESSAGE_SIZE) {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::send(const class bdReference<class bdMessage>,const bool)",
                             0x106u, "dw/warn/");
        proxy.log("bdConnection/connections", "message size (%u) > BD_MAX_MESSAGE_SIZE.", payloadSize);
    } else if (m_state == BD_UC_ESTABLISHED) {
        bdDataFlags flags = reliable ? BD_DC_NONE : BD_DC_UNRELIABLE;
        bdReference<bdDataChunk> chunk(
            new (bdMemory::allocate(sizeof(bdDataChunk))) bdDataChunk(message, flags));
        if (reliable) {
            if (m_reliableSendWindow == NULL)
                m_reliableSendWindow = new (bdMemory::allocate(sizeof(bdReliableSendWindow))) bdReliableSendWindow();
            sent = m_reliableSendWindow->add(chunk);
            if (!sent) {
                bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                                     "bool __thiscall bdUnicastConnection::send(const class bdReference<class bdMessage>,const bool)",
                                     0xF6u, "dw/warn/");
                proxy.log("bdConnection/connections", "Failed to add message to reliable send window.");
            }
        } else {
            m_unreliableSendWindow.add(chunk);
            sent = true;
        }
    }
    if (!sent) {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::send(const class bdReference<class bdMessage>,const bool)",
                             0x10Bu, "dw/warn/");
        proxy.log("bdConnection/connections", "Message not sent.");
    }
    return sent;
}

// ============================================================================
// bdUnicastConnection::getMessageToDispatch - ea: 0x8A4FE0
// ============================================================================
bool bdUnicastConnection::getMessageToDispatch(bdReference<bdMessage>& message) {
    bdReference<bdMessage> result;
    bool found = false;

    if (m_reliableRecvWindow != NULL) {
        bdReference<bdDataChunk> chunk = m_reliableRecvWindow->getNextToRead();
        if (chunk.m_ptr != NULL) {
            result = chunk.m_ptr->getMessage();
            found = true;
        }
    }
    if (!found) {
        bdReference<bdDataChunk> chunk = m_unreliableReceiveWindow.getNextToRead();
        if (chunk.m_ptr != NULL) {
            result = chunk.m_ptr->getMessage();
            found = true;
        }
    }
    if (found) {
        if (message.m_ptr != NULL && message.m_ptr->releaseRef() == 0)
            delete message.m_ptr;
        message.m_ptr = result.m_ptr;
        if (message.m_ptr != NULL)
            message.m_ptr->addRef();
    }
    return found;
}
// ============================================================================
// bdUnicastConnection::receive - ea: 0x8A7DC0
// ============================================================================
bool bdUnicastConnection::receive(const unsigned char* buffer, unsigned int bufferSize) {
    if (m_shutdownGuard.getElapsedTimeInSeconds() > BD_UC_SHUTDOWN_GUARD) {
        close();
        return false;
    }

    bool result = false;
    bdPacket packet;
    bool valid = packet.deserialize(buffer, bufferSize);

    if (valid) {
        bdReference<bdChunk> chunk;
        packet.getNextChunk(chunk);
        if (chunk.m_ptr != NULL && chunk.m_ptr->isControl()) {
            switch (chunk.m_ptr->getType()) {
            case (bdChunkTypes)3:
                result = handleInit(chunk);
                break;
            case (bdChunkTypes)4:
                result = handleInitAck(chunk, packet.getVerificationTag());
                break;
            case (bdChunkTypes)13:
                result = handleCookieEcho(chunk, packet.getVerificationTag());
                break;
            case (bdChunkTypes)14:
                result = handleCookieAck(chunk, packet.getVerificationTag());
                break;
            default:
                break;
            }
            if (result)
                chunk.m_ptr = NULL;
        }

        if (m_localTag == packet.getVerificationTag()) {
            do {
                if (chunk.m_ptr != NULL) {
                    switch (chunk.m_ptr->getType()) {
                    case (bdChunkTypes)2:
                        result = handleData(chunk);
                        break;
                    case (bdChunkTypes)5:
                        result = handleSAck(chunk);
                        break;
                    case (bdChunkTypes)6:
                        result = handleHeartbeat(chunk);
                        break;
                    case (bdChunkTypes)7:
                        m_receiveTimer.start();
                        result = true;
                        break;
                    case (bdChunkTypes)9:
                        result = handleShutdown(chunk);
                        break;
                    case (bdChunkTypes)10:
                        result = handleShutdownAck(chunk);
                        break;
                    case (bdChunkTypes)11:
                        result = handleShutdownComplete(chunk);
                        break;
                    default:
                        break;
                    }
                }
            } while (packet.getNextChunk(chunk));
        }

        if (result)
            m_receiveTimer.start();
    } else {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::receive(unsigned char *,const unsigned int)",
                             0x1C7u, "dw/warn/");
        proxy.log("bdConnection/connections", "bdUnicastConnection::receive(): Invalid packet received.");
    }

    if (m_state == BD_UC_SHUTDOWN_SENT && !sendShutdown())
        close();
    return result;
}

// ============================================================================
// bdUnicastConnection::getDataToSend - ea: 0x8A6F00
// ============================================================================
unsigned int bdUnicastConnection::getDataToSend(unsigned char* buffer, unsigned int bufferSize) {
    bdPacket packet(m_peerTag, bufferSize);
    unsigned int dataToSend = 0;

    if (m_shutdownGuard.getElapsedTimeInSeconds() > BD_UC_SHUTDOWN_GUARD) {
        close();
        return 0;
    }

    if (m_outQueue.m_size == 0) {
        if (m_initTimer.getElapsedTimeInSeconds() > BD_UC_INIT_TIMEOUT) {
            if (!sendInit())
                close();
        } else if (m_cookieTimer.getElapsedTimeInSeconds() > BD_UC_COOKIE_TIMEOUT) {
            if (!sendCookieEcho(m_initAckChunk))
                close();
        } else if (m_shutdownTimer.getElapsedTimeInSeconds() > BD_UC_SHUTDOWN_TIMEOUT) {
            switch (m_state) {
            case BD_UC_SHUTDOWN_SENT:
                if (!sendShutdown())
                    close();
                break;
            case BD_UC_SHUTDOWN_RECEIVED:
            case BD_UC_SHUTDOWN_ACK_SENT:
                if (!sendShutdownAck())
                    close();
                break;
            default:
                break;
            }
        }
    }

    while (!m_outQueue.isEmpty()) {
        bdControlChunkStore& store = m_outQueue.getHead();
        if (store.m_lone) {
            unsigned int tagToUse = m_peerTag;
            if (store.m_chunk.m_ptr->getType() == (bdChunkTypes)4)
                tagToUse = ((bdInitAckChunk*)store.m_chunk.m_ptr)->getPeerTag();
            bdPacket lone(tagToUse, bufferSize);
            lone.addChunk(store.m_chunk);
            m_outQueue.removeHead();
            dataToSend = lone.serialize(buffer, bufferSize);
            break;
        } else {
            if (packet.addChunk(store.m_chunk)) {
                m_outQueue.removeHead();
            } else {
                break;
            }
        }
    }

    if (dataToSend == 0) {
        float lastReceived = m_receiveTimer.getElapsedTimeInSeconds();
        if (m_state >= BD_UC_ESTABLISHED && m_state <= BD_UC_SHUTDOWN_RECEIVED) {
            if (lastReceived > BD_UC_ALIVE_PERIOD) {
                bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                                     "unsigned int __thiscall bdUnicastConnection::getDataToSend(unsigned char *const ,const unsigned int)",
                                     0x25Eu, "dw/warn/");
                proxy.log("bdConnection/connections", "bdUnicastConnection: Connection timed out.");
                close();
            } else {
                if (m_reliableRecvWindow != NULL)
                    m_reliableRecvWindow->getDataToSend(packet);
                if (m_reliableSendWindow != NULL)
                    m_reliableSendWindow->getDataToSend(packet);
                m_unreliableSendWindow.getDataToSend(packet);
            }
        }

        switch (m_state) {
        case BD_UC_SHUTDOWN_PENDING:
            if (packet.isEmpty() && windowsEmpty()) {
                if (sendShutdown()) {
                    m_state = BD_UC_SHUTDOWN_SENT;
                } else {
                    close();
                }
            }
            break;
        case BD_UC_SHUTDOWN_RECEIVED:
            if (packet.isEmpty() && windowsEmpty()) {
                if (sendShutdownAck()) {
                    m_state = BD_UC_SHUTDOWN_ACK_SENT;
                } else {
                    close();
                }
            }
            break;
        default:
            break;
        }

        if (m_state == BD_UC_ESTABLISHED && packet.isEmpty()) {
            float lastSent = m_sendTimer.getElapsedTimeInSeconds();
            if (lastSent > BD_UC_KEEP_ALIVE_TIMEOUT) {
                bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                                     "unsigned int __thiscall bdUnicastConnection::getDataToSend(unsigned char *const ,const unsigned int)",
                                     0x2ACu, "dw/info/");
                proxy.log("bdConnection/connections", "Sending Heartbeat. Last send %.2fs ago.", lastSent);
                bdReference<bdChunk> hb(new (bdMemory::allocate(sizeof(bdHeartbeatChunk))) bdHeartbeatChunk());
                packet.addChunk(hb);
            }
        }

        if (!packet.isEmpty()) {
            dataToSend = packet.serialize(buffer, bufferSize);
            if (dataToSend > bufferSize) {
                bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                                     "unsigned int __thiscall bdUnicastConnection::getDataToSend(unsigned char *const ,const unsigned int)",
                                     0x2BFu, "dw/err/");
                proxy.log("bdConnection/connections", "buffer overflow!");
            }
            m_sendTimer.start();
        }
    }
    return dataToSend;
}

// ============================================================================
// Handlers
// ============================================================================
bool bdUnicastConnection::handleInit(bdReference<bdChunk>& chunk) {
    bdInitChunk* init = (bdInitChunk*)chunk.m_ptr;
    XNKID id;
    bdReference<bdCommonAddr> addr;
    if (!bdSingleton<bdAddressMapImpl>::getInstance()->addrToCommonAddr(m_addrHandle, addr, id)) {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::handleInit(class bdReference<class bdChunk> &)",
                             0x375u, "dw/warn/");
        proxy.log("bdConnection/connections", "handleInit: failed to get addr from map");
        return false;
    }
    if (init->getInitTag() == 0) {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::handleInit(class bdReference<class bdChunk> &)",
                             0x36Fu, "dw/warn/");
        proxy.log("bdConnection/connections", "handleInit: invalid init tag (%u)", init->getInitTag());
        return false;
    }
    bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                         "bool __thiscall bdUnicastConnection::handleInit(class bdReference<class bdChunk> &)",
                         0x36Au, "dw/info/");
    proxy.log("bdConnection/connections", "uc::handling init: m_localTag: %d", m_localTag);
    return sendInitAck(bdReference<bdInitChunk>(init));
}

bool bdUnicastConnection::handleInitAck(bdReference<bdChunk>& chunk, unsigned int vtag) {
    bool result = false;
    if (m_state == BD_UC_COOKIE_WAIT) {
        if (vtag == m_localTag) {
            bdInitAckChunk* ia = (bdInitAckChunk*)chunk.m_ptr;
            m_peerTag = ia->getInitTag();
            if (m_peerTag != 0) {
                bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                                     "bool __thiscall bdUnicastConnection::handleInitAck(class bdReference<class bdChunk> &,const unsigned int)",
                                     0x38Eu, "dw/info/");
                proxy.log("bdConnection/connections", "uc::handling init ack: m_localTag/m_peerTag: %d/%d",
                          m_localTag, m_peerTag);
                if (sendCookieEcho(bdReference<bdInitAckChunk>(ia))) {
                    m_initTimer.reset();
                    m_state = BD_UC_COOKIE_ECHOED;
                    result = true;
                }
            } else {
                bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                                     "bool __thiscall bdUnicastConnection::handleInitAck(class bdReference<class bdChunk> &,const unsigned int)",
                                     0x39Au, "dw/warn/");
                proxy.log("bdConnection/connections", "handleInitAck: invalid init tag (%u)", 0);
            }
        } else {
            bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                                 "bool __thiscall bdUnicastConnection::handleInitAck(class bdReference<class bdChunk> &,const unsigned int)",
                                 0x39Fu, "dw/warn/");
            proxy.log("bdConnection/connections", "Invalid verification tag on init ack.");
        }
    }
    return result;
}

bool bdUnicastConnection::handleCookieEcho(bdReference<bdChunk>& chunk, unsigned int) {
    bool result = false;
    bdCookieEchoChunk* echo = (bdCookieEchoChunk*)chunk.m_ptr;
    bdReference<bdCookie> cookie;
    if (echo->getCookie(cookie)) {
        unsigned int localTag = cookie.m_ptr->getLocalTag();
        unsigned int peerTag = cookie.m_ptr->getPeerTag();
        unsigned int localTieTag = cookie.m_ptr->getLocalTieTag();
        unsigned int peerTieTag = cookie.m_ptr->getPeerTieTag();

        if (m_state == BD_UC_CLOSED) {
            XNKID id;
            bdReference<bdCommonAddr> addr;
            if (!bdSingleton<bdAddressMapImpl>::getInstance()->addrToCommonAddr(m_addrHandle, addr, id)) {
                bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                                     "bool __thiscall bdUnicastConnection::handleCookieEcho(class bdReference<class bdChunk> &,const unsigned int)",
                                     0x3CCu, "dw/warn/");
                proxy.log("bdConnection/connections", "Couldn't get address from map!");
                return false;
            }
            m_localTag = localTag;
            m_peerTag = peerTag;
            bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                                 "bool __thiscall bdUnicastConnection::handleCookieEcho(class bdReference<class bdChunk> &,const unsigned int)",
                                 0x3C2u, "dw/info/");
            proxy.log("bdConnection/connections", "uc::handling cookie echo: m_localTag/m_peerTag: %d/%d",
                      m_localTag, m_peerTag);
            if (sendCookieAck()) {
                m_state = BD_UC_ESTABLISHED;
                callListenersConnect(true);
                result = true;
            }
        } else {
            if (m_localTag != localTag && m_peerTag != peerTag &&
                m_localTag == localTieTag && m_peerTag == peerTieTag) {
                reset();
                if (sendCookieAck()) {
                    bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                                         "bool __thiscall bdUnicastConnection::handleCookieEcho(class bdReference<class bdChunk> &,const unsigned int)",
                                         0x40Du, "dw/info/");
                    proxy.log("bdConnection/connections", "uc::handling cookie echo: m_localTag/m_peerTag: %d/%d",
                              m_localTag, m_peerTag);
                    XNKID id;
                    bdReference<bdCommonAddr> addr;
                    if (bdSingleton<bdAddressMapImpl>::getInstance()->addrToCommonAddr(m_addrHandle, addr, id)) {
                        m_localTag = localTag;
                        m_peerTag = peerTag;
                        m_state = BD_UC_ESTABLISHED;
                        callListenersReconnect();
                        result = true;
                    } else {
                        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                                             "bool __thiscall bdUnicastConnection::handleCookieEcho(class bdReference<class bdChunk> &,const unsigned int)",
                                             0x41Du, "dw/warn/");
                        proxy.log("bdConnection/connections", "Couldn't get address from map!");
                    }
                }
            } else if ((m_localTag == localTag && m_peerTag != peerTag) ||
                       (m_localTag == localTag && peerTieTag == 0)) {
                if (m_state != BD_UC_ESTABLISHED) {
                    m_state = BD_UC_ESTABLISHED;
                    callListenersConnect(true);
                }
                m_peerTag = peerTag;
                m_initTimer.reset();
                m_cookieTimer.reset();
                result = sendCookieAck();
            } else if (m_localTag != localTag && m_peerTag == peerTag) {
                // late cookie - discard silently
            } else if (m_localTag == localTag && m_peerTag == peerTag) {
                if (m_state != BD_UC_ESTABLISHED) {
                    m_state = BD_UC_ESTABLISHED;
                    callListenersConnect(true);
                }
                m_initTimer.reset();
                m_cookieTimer.reset();
                result = sendCookieAck();
            } else {
                result = true;
            }
        }
    }
    return result;
}

bool bdUnicastConnection::handleCookieAck(bdReference<bdChunk>&, unsigned int vtag) {
    if (vtag == m_localTag) {
        if (m_state == BD_UC_COOKIE_ECHOED) {
            bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                                 "bool __thiscall bdUnicastConnection::handleCookieAck(class bdReference<class bdChunk> &,const unsigned int)",
                                 0x46Cu, "dw/info/");
            proxy.log("bdConnection/connections", "uc::handling cookie ack: m_localTag/m_peerTag: %d/%d",
                      m_localTag, m_peerTag);
            m_state = BD_UC_ESTABLISHED;
            callListenersConnect(true);
            m_cookieTimer.reset();
            return true;
        }
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::handleCookieAck(class bdReference<class bdChunk> &,const unsigned int)",
                             0x477u, "dw/warn/");
        proxy.log("bdConnection/connections", "Cookie ack received in invalid state.");
        return false;
    }
    bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                         "bool __thiscall bdUnicastConnection::handleCookieAck(class bdReference<class bdChunk> &,const unsigned int)",
                         0x47Cu, "dw/warn/");
    proxy.log("bdConnection/connections", "Invalid verification tag on cookie ack. (%u)", vtag);
    return false;
}

bool bdUnicastConnection::handleData(bdReference<bdChunk>& chunk) {
    bdDataChunk* data = (bdDataChunk*)chunk.m_ptr;
    bool result = false;
    if (data->getFlags() & BD_DC_UNRELIABLE) {
        result = m_unreliableReceiveWindow.add(bdReference<bdDataChunk>(data));
    } else {
        if (m_reliableRecvWindow == NULL)
            m_reliableRecvWindow = new (bdMemory::allocate(sizeof(bdReliableReceiveWindow))) bdReliableReceiveWindow();
        result = m_reliableRecvWindow->add(bdReference<bdDataChunk>(data));
        if (!result) {
            bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                                 "bool __thiscall bdUnicastConnection::handleData(class bdReference<class bdChunk> &)",
                                 0x301u, "dw/warn/");
            proxy.log("bdConnection/connections", "receive window full.");
        }
    }
    return result;
}

bool bdUnicastConnection::handleSAck(bdReference<bdChunk>& chunk) {
    bool result = false;
    bdSAckChunk* sack = (bdSAckChunk*)chunk.m_ptr;
    if (m_reliableSendWindow != NULL) {
        float rtt = 0.0f;
        result = m_reliableSendWindow->handleAck(bdReference<bdSAckChunk>(sack), rtt);
        if (result && rtt > 0.0f) {
            if (m_smoothedRTT != 0.0f) {
                float delta = (float)fabs((double)(m_smoothedRTT - rtt));
                m_smoothedRTT = m_smoothedRTT * (1.0f - BD_UC_RTO_ALPHA) + rtt * BD_UC_RTO_ALPHA;
                m_RTTVariation = m_RTTVariation * (1.0f - BD_UC_RTO_BETA) + delta * BD_UC_RTO_BETA;
            } else {
                m_smoothedRTT = rtt;
                m_RTTVariation = rtt * 0.5f;
            }
            m_stats.setLastRTT(m_smoothedRTT);
            if (m_RTTVariation == 0.0f)
                m_RTTVariation = BD_UC_CLOCK_GRANULARITY;
            float rto = m_smoothedRTT + 2.0f * m_RTTVariation;
            if (rto < BD_UC_RTO_MIN)
                rto = BD_UC_RTO_MIN;
            else if (rto > BD_UC_RTO_MAX)
                rto = BD_UC_RTO_MAX;
            m_reliableSendWindow->setTimeoutPeriod(rto);
        }
    } else {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::handleSAck(class bdReference<class bdChunk> &)",
                             0x357u, "dw/warn/");
        proxy.log("bdConnection/connections", "invalid stream id.");
    }
    return result;
}

bool bdUnicastConnection::handleHeartbeat(bdReference<bdChunk>& chunk) {
    m_receiveTimer.start();
    return sendHeartbeatAck(bdReference<bdInitChunk>((bdInitChunk*)chunk.m_ptr));
}

bool bdUnicastConnection::handleHeartbeatAck(bdReference<bdChunk>&) {
    m_receiveTimer.start();
    return true;
}

bool bdUnicastConnection::handleShutdown(bdReference<bdChunk>&) {
    switch (m_state) {
    case BD_UC_ESTABLISHED: {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::handleShutdown(class bdReference<class bdChunk> &)",
                             0x497u, "dw/info/");
        proxy.log("bdConnection/connections", "uc::handling shutdown (a)");
        m_state = BD_UC_SHUTDOWN_RECEIVED;
        break;
    }
    case BD_UC_SHUTDOWN_SENT: {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::handleShutdown(class bdReference<class bdChunk> &)",
                             0x49Cu, "dw/info/");
        proxy.log("bdConnection/connections", "uc::handling shutdown (b)");
        m_state = BD_UC_SHUTDOWN_RECEIVED;
        if (!sendShutdownAck())
            return true;
        break;
    }
    default: {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::handleShutdown(class bdReference<class bdChunk> &)",
                             0x4A1u, "dw/warn/");
        proxy.log("bdConnection/connections", "uc::handling shutdown (c) - unexpected (%u)!", (unsigned int)m_state);
        return true;
    }
    }
    bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                         "bool __thiscall bdUnicastConnection::handleShutdown(class bdReference<class bdChunk> &)",
                         0x4A7u, "dw/info/");
    proxy.log("bdConnection/connections", "uc::handling shutdown. Calling disconnect listeners.");
    callListenersDisconnect();
    m_shutdownGuard.start();
    return true;
}

bool bdUnicastConnection::handleShutdownAck(bdReference<bdChunk>&) {
    if (m_state == BD_UC_SHUTDOWN_SENT) {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::handleShutdownAck(class bdReference<class bdChunk> &)",
                             0x4B6u, "dw/info/");
        proxy.log("bdConnection/connections", "uc::handling shutdown ack (a)");
        sendShutdownComplete();
        close();
    } else if (m_state == BD_UC_SHUTDOWN_ACK_SENT) {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::handleShutdownAck(class bdReference<class bdChunk> &)",
                             0x4BDu, "dw/info/");
        proxy.log("bdConnection/connections", "uc::handling shutdown ack (b)");
        sendShutdownComplete();
        close();
    } else {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::handleShutdownAck(class bdReference<class bdChunk> &)",
                             0x4C4u, "dw/info/");
        proxy.log("bdConnection/connections", "uc::handling shutdown ack (c) - unexpected (%u).", (unsigned int)m_state);
        return false;
    }
    m_shutdownTimer.reset();
    return true;
}

bool bdUnicastConnection::handleShutdownComplete(bdReference<bdChunk>&) {
    if (m_state == BD_UC_SHUTDOWN_ACK_SENT) {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::handleShutdownComplete(class bdReference<class bdChunk> &)",
                             0x4D2u, "dw/info/");
        proxy.log("bdConnection/connections", "uc::handling shutdown complete (a)");
        m_shutdownTimer.reset();
        close();
        return true;
    }
    bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                         "bool __thiscall bdUnicastConnection::handleShutdownComplete(class bdReference<class bdChunk> &)",
                         0x4D8u, "dw/warn/");
    proxy.log("bdConnection/connections", "uc::handling shutdown complete (b) - unexpected!");
    return false;
}

// ============================================================================
// Senders
// ============================================================================
bool bdUnicastConnection::sendInit() {
    bool valid = m_state == BD_UC_CLOSED || m_state == BD_UC_COOKIE_WAIT;
    if (!valid) {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::sendInit(void)",
                             0x4E4u, "dw/err");
        proxy.log("defaultFileName", "invalid state to send init from.");
    }
    if (m_initResends++ >= BD_UC_INIT_RESENDS)
        return false;
    m_initTimer.start();
    bdReference<bdChunk> chunk(new (bdMemory::allocate(sizeof(bdInitChunk)))
        bdInitChunk(m_localTag, 1500));
    m_outQueue.addTail(bdControlChunkStore(chunk, false));
    bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                         "bool __thiscall bdUnicastConnection::sendInit(void)",
                         0x4EEu, "dw/info/");
    proxy.log("bdConnection/connections", "uc::sending init: m_localTag: %d", m_localTag);
    return true;
}

bool bdUnicastConnection::sendInitAck(const bdReference<bdInitChunk>& chunk) {
    unsigned int peerTag = chunk.m_ptr->getInitTag();
    unsigned int localTag = 0;
    unsigned int peerTieTag = 0;
    unsigned int localTieTag = 0;

    switch (m_state) {
    case BD_UC_CLOSED:
    case BD_UC_SHUTDOWN_ACK_SENT:
        m_peerTag = peerTag;
        localTag = m_localTag;
        break;
    case BD_UC_COOKIE_ECHOED:
        peerTieTag = m_peerTag;
        localTieTag = m_localTag;
    case BD_UC_COOKIE_WAIT:
        localTag = m_localTag;
        break;
    case BD_UC_ESTABLISHED:
    case BD_UC_SHUTDOWN_PENDING:
    case BD_UC_SHUTDOWN_RECEIVED:
    case BD_UC_SHUTDOWN_SENT:
        localTag = bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUInt();
        peerTieTag = m_peerTag;
        localTieTag = m_localTag;
        break;
    default: {
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::sendInitAck(class bdReference<class bdInitChunk>)",
                             0x542u, "dw/warn/");
        proxy.log("bdConnection/connections", "bdUnicastConnection::sendInitAck(): Failed to send init ack.");
        return false;
    }
    }

    bdReference<bdCookie> cookie(new (bdMemory::allocate(sizeof(bdCookie)))
        bdCookie(localTag, peerTag, localTieTag, peerTieTag));
    bdReference<bdChunk> iachunk(new (bdMemory::allocate(sizeof(bdInitAckChunk)))
        bdInitAckChunk(localTag, cookie, 1500, peerTag));
    m_outQueue.addTail(bdControlChunkStore(iachunk, true));
    bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                         "bool __thiscall bdUnicastConnection::sendInitAck(class bdReference<class bdInitChunk>)",
                         0x53Eu, "dw/info/");
    proxy.log("bdConnection/connections", "uc::sending init ack: m_localTag/localTag/m_peerTag: %d/%d/%d",
              m_localTag, localTag, m_peerTag);
    return true;
}

bool bdUnicastConnection::sendCookieEcho(const bdReference<bdInitAckChunk>& chunk) {
    if (m_initAckChunk.m_ptr != chunk.m_ptr) {
        if (m_initAckChunk.m_ptr != NULL && m_initAckChunk.m_ptr->releaseRef() == 0)
            delete m_initAckChunk.m_ptr;
        m_initAckChunk.m_ptr = chunk.m_ptr;
        if (m_initAckChunk.m_ptr != NULL)
            m_initAckChunk.m_ptr->addRef();
    }
    if (m_cookieResends++ >= BD_UC_COOKIE_RESENDS)
        return false;
    bdReference<bdByteBuffer> cookie;
    if (chunk.m_ptr->getCookie(cookie)) {
        m_cookieTimer.start();
        bdReference<bdChunk> echo(new (bdMemory::allocate(sizeof(bdCookieEchoChunk)))
            bdCookieEchoChunk(cookie));
        m_outQueue.addTail(bdControlChunkStore(echo, false));
        bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                             "bool __thiscall bdUnicastConnection::sendCookieEcho(class bdReference<class bdInitAckChunk>)",
                             0x559u, "dw/info/");
        proxy.log("bdConnection/connections", "uc::sending cookie echo: m_localTag/m_peerTag: %d/%d",
                  m_localTag, m_peerTag);
    }
    return true;
}

bool bdUnicastConnection::sendCookieAck() {
    bdReference<bdChunk> ack(new (bdMemory::allocate(sizeof(bdCookieAckChunk))) bdCookieAckChunk());
    m_outQueue.addTail(bdControlChunkStore(ack, false));
    bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                         "bool __thiscall bdUnicastConnection::sendCookieAck(void)",
                         0x56Cu, "dw/info/");
    proxy.log("bdConnection/connections", "uc::sending cookie ack: m_localTag/m_peerTag: %d/%d",
              m_localTag, m_peerTag);
    return true;
}

bool bdUnicastConnection::sendHeartbeat(const bdReference<bdInitChunk>&) {
    bdReference<bdChunk> hb(new (bdMemory::allocate(sizeof(bdHeartbeatChunk))) bdHeartbeatChunk());
    m_outQueue.addTail(bdControlChunkStore(hb, false));
    return true;
}

bool bdUnicastConnection::sendHeartbeatAck(const bdReference<bdInitChunk>&) {
    if (m_outQueue.isEmpty() && windowsEmpty()) {
        bdReference<bdChunk> ack(new (bdMemory::allocate(sizeof(bdHeartbeatAckChunk))) bdHeartbeatAckChunk());
        m_outQueue.addTail(bdControlChunkStore(ack, false));
    }
    return true;
}

bool bdUnicastConnection::sendShutdown() {
    if (m_shutdownResends++ >= BD_UC_SHUTDOWN_RESENDS)
        return false;
    bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                         "bool __thiscall bdUnicastConnection::sendShutdown(void)",
                         0x58Du, "dw/info/");
    proxy.log("bdConnection/connections", "uc::sending shutdown (%u/%u)", m_shutdownResends, 5);
    bdReference<bdChunk> chunk(new (bdMemory::allocate(sizeof(bdShutdownChunk))) bdShutdownChunk());
    m_outQueue.addTail(bdControlChunkStore(chunk, false));
    m_shutdownTimer.start();
    return true;
}

bool bdUnicastConnection::sendShutdownAck() {
    if (m_shutdownResends++ >= BD_UC_SHUTDOWN_RESENDS)
        return false;
    bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                         "bool __thiscall bdUnicastConnection::sendShutdownAck(void)",
                         0x5A1u, "dw/info/");
    proxy.log("bdConnection/connections", "uc::sending shutdown ack (%u/%u)", m_shutdownResends, 5);
    bdReference<bdChunk> chunk(new (bdMemory::allocate(sizeof(bdShutdownAckChunk))) bdShutdownAckChunk());
    m_outQueue.addTail(bdControlChunkStore(chunk, false));
    m_shutdownTimer.start();
    return true;
}

bool bdUnicastConnection::sendShutdownComplete() {
    bdMessageProxy proxy(".\\bdUnicastConnection.cpp",
                         "bool __thiscall bdUnicastConnection::sendShutdownComplete(void)",
                         0x5B3u, "dw/info/");
    proxy.log("bdConnection/connections", "uc::sending shutdown complete");
    bdReference<bdChunk> chunk(new (bdMemory::allocate(sizeof(bdShutdownCompleteChunk))) bdShutdownCompleteChunk());
    m_outQueue.addTail(bdControlChunkStore(chunk, false));
    m_shutdownTimer.reset();
    return true;
}

// ============================================================================
// Helpers
// ============================================================================
void bdUnicastConnection::reset() {
    delete m_reliableRecvWindow;
    delete m_reliableSendWindow;
    m_reliableRecvWindow = NULL;
    m_reliableSendWindow = NULL;
    while (!m_outQueue.isEmpty())
        m_outQueue.removeHead();
    m_unreliableReceiveWindow.reset();
    m_unreliableSendWindow.reset();
    m_sendTimer.start();
    m_receiveTimer.start();
    m_initResends = 0;
    m_cookieResends = 0;
    m_shutdownResends = 0;
    m_localTag = bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUInt();
    m_peerTag = 0;
    m_smoothedRTT = 0.0f;
    m_RTTVariation = 0.0f;
}

void bdUnicastConnection::callListenersConnect(bool success) {
    for (unsigned int i = 0; i < m_listeners.m_size; i++) {
        if (success)
            m_listeners[i]->onConnect(bdReference<bdConnection>(this));
        else
            m_listeners[i]->onConnectFailed(bdReference<bdConnection>(this));
    }
}

void bdUnicastConnection::callListenersDisconnect() {
    for (unsigned int i = 0; i < m_listeners.m_size; i++)
        m_listeners[i]->onDisconnect(bdReference<bdConnection>(this));
}

void bdUnicastConnection::callListenersReconnect() {
    for (unsigned int i = 0; i < m_listeners.m_size; i++)
        m_listeners[i]->onReconnect(bdReference<bdConnection>(this));
}

bool bdUnicastConnection::windowsEmpty() const {
    return m_reliableSendWindow == NULL || m_reliableSendWindow->isEmpty();
}

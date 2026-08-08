// ============================================================================
// bdDiscoveryServer.cpp - LAN discovery server (9 funcs).
// Source: bdNet:bdDiscoveryServer.obj
// Verified against IDA (release decompilation).
// ============================================================================

#include "bd/bdDiscovery.h"

#define BD_MIN_DISCOVERY_SIZE 11
#define BD_NONCE_SIZE 8

// ============================================================================
// bdDiscoveryServer::bdDiscoveryServer - ea: 0x8B0230
// ============================================================================
bdDiscoveryServer::bdDiscoveryServer()
    : m_gameInfo(), m_socket(), m_listeners(), m_status(BD_DISCOVERY_IDLE) {
}

// ============================================================================
// bdDiscoveryServer::~bdDiscoveryServer - ea: 0x8B0280
// ============================================================================
bdDiscoveryServer::~bdDiscoveryServer() {
    m_socket.close();
    m_status = BD_DISCOVERY_IDLE;
    bdMemory::deallocate(m_listeners.m_data);
    m_listeners.m_data = NULL;
    m_listeners.m_size = 0;
    m_listeners.m_capacity = 0;
    if (m_gameInfo.m_ptr != NULL && m_gameInfo.m_ptr->releaseRef() == 0)
        delete m_gameInfo.m_ptr;
    m_gameInfo.m_ptr = NULL;
}

// ============================================================================
// bdDiscoveryServer::start - ea: 0x8AFE40
// ============================================================================
bool bdDiscoveryServer::start(const bdReference<bdGameInfo>& gameInfo,
                              const bdInetAddr& localAddr) {
    bool ok = true;
    if (!m_socket.create(true)) {
        m_status = BD_DISCOVERY_ERROR;
        ok = false;
    } else {
        bdAddr bindAddr(localAddr, BD_DEFAULT_DISCOVERY_PORT);
        if (m_socket.bind(bindAddr) == BD_NET_SUCCESS) {
            m_status = BD_DISCOVERY_PENDING;
            if (m_gameInfo.m_ptr != NULL && m_gameInfo.m_ptr->releaseRef() == 0)
                delete m_gameInfo.m_ptr;
            m_gameInfo.m_ptr = gameInfo.m_ptr;
            if (m_gameInfo.m_ptr != NULL)
                m_gameInfo.m_ptr->addRef();
        } else {
            m_status = BD_DISCOVERY_ERROR;
            ok = false;
        }
    }
    return ok;
}

// ============================================================================
// bdDiscoveryServer::update - ea: 0x8AFF70
// ============================================================================
void bdDiscoveryServer::update() {
    if (m_status != BD_DISCOVERY_PENDING) {
        bdMessageProxy proxy(".\\bdDiscovery\\bdDiscoveryServer.cpp",
                             "void __thiscall bdDiscoveryServer::update(void)",
                             0x6Eu, "dw/warn/");
        proxy.log("bdNet/discovery server", "Not initialized.");
        return;
    }

    bdAddr from;
    unsigned char data[1328];
    int received = m_socket.receiveFrom(from, data, sizeof(data));

    if (received > BD_MIN_DISCOVERY_SIZE) {
        bdBitBuffer payload(data, (unsigned int)received << 3, true);
        unsigned char type8 = 0;
        if (payload.readDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE) &&
            payload.readBits(&type8, 8) && type8 == BD_DISCOVERY_BROADCAST) {
            unsigned char nonce[BD_NONCE_SIZE];
            payload.readDataType(bdBitBuffer::BD_BB_FULL_TYPE);
            bool ok = payload.readBits(nonce, BD_NONCE_SIZE << 3);
            unsigned int titleID = 0;
            if (ok) {
                payload.readDataType(bdBitBuffer::BD_BB_UNSIGNED_INTEGER32_TYPE);
                ok = payload.readBits(&titleID, 32);
            }
            if (ok && titleID == m_gameInfo.m_ptr->getTitleID()) {
                bdBitBuffer reply(0, false);
                unsigned char replyType = BD_DISCOVERY_REPLY;
                reply.writeDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE);
                reply.writeBits(&replyType, 8);
                reply.writeDataType(bdBitBuffer::BD_BB_FULL_TYPE);
                reply.writeBits(nonce, BD_NONCE_SIZE << 3);
                m_gameInfo.m_ptr->serialize(reply);

                bdInetAddr broadcast = bdInetAddr::Broadcast();
                bdAddr sendTo(broadcast, from.getPort());
                if (m_socket.sendTo(sendTo, reply.getData(), reply.getDataSize()) > 0) {
                    char addrStr[24];
                    from.toString(addrStr, sizeof(addrStr));
                    bdMessageProxy proxy(".\\bdDiscovery\\bdDiscoveryServer.cpp",
                                         "void __thiscall bdDiscoveryServer::update(void)",
                                         0x61u, "dw/info/");
                    proxy.log("bdNet/discovery server", "Sent discovery reply to: %s", addrStr);
                    for (unsigned int i = 0; i < m_listeners.m_size; i++)
                        m_listeners[i]->onRequest();
                }
            }
        }
    }
}

// ============================================================================
// bdDiscoveryServer::stop - ea: 0x8AFE10
// ============================================================================
void bdDiscoveryServer::stop() {
    m_socket.close();
    m_status = BD_DISCOVERY_IDLE;
}

// ============================================================================
// bdDiscoveryServer::registerListener - ea: 0x8B0310
// ============================================================================
void bdDiscoveryServer::registerListener(bdDiscoveryListener* listener) {
    m_listeners.pushBack(listener);
}

// ============================================================================
// bdDiscoveryServer::unregisterListener - ea: 0x8B0340
// ============================================================================
void bdDiscoveryServer::unregisterListener(bdDiscoveryListener* listener) {
    m_listeners.removeAll(listener);
}

// ============================================================================
// bdDiscoveryServer::getStatus - ea: 0x8AFE30
// ============================================================================
bdDiscoveryStatus bdDiscoveryServer::getStatus() const {
    return m_status;
}

// ============================================================================
// bdDiscoveryServer::getGameInfo - ea: 0x8AFF50 (returns new reference)
// ============================================================================
bdReference<bdGameInfo> bdDiscoveryServer::getGameInfo() {
    bdReference<bdGameInfo> result;
    result.m_ptr = m_gameInfo.m_ptr;
    if (result.m_ptr != NULL)
        result.m_ptr->addRef();
    return result;
}

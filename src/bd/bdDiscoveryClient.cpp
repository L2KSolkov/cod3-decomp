// ============================================================================
// bdDiscoveryClient.cpp - LAN discovery client (8 funcs).
// Source: bdNet:bdDiscoveryClient.obj
// Verified against IDA (release decompilation).
// ============================================================================

#include "bd/bdDiscovery.h"
#include "bd/bdNet.h"

#include <new>
#include <string.h>

#define BD_MIN_DISCOVERY_SIZE 11
#define BD_NONCE_SIZE 8
#define BD_MAX_DISCOVERY_RESPONSE_SIZE 1500

// Cross-object externs (bdNet/bdCore units; unresolved until ported).
struct bdTrulyRandomImpl {
public:
    void getRandomUByte8(unsigned char* buf, int count);
};

// ============================================================================
// bdDiscoveryClient::bdDiscoveryClient - ea: 0x8AE1C0
// ============================================================================
bdDiscoveryClient::bdDiscoveryClient()
    : m_status(BD_DISCOVERY_IDLE), m_timeout(0), m_timer(), m_socket(),
      m_listeners() {
    memset(m_nonce, 0, sizeof(m_nonce));
}

// ============================================================================
// bdDiscoveryClient::~bdDiscoveryClient - ea: 0x8AE1F0
// ============================================================================
bdDiscoveryClient::~bdDiscoveryClient() {
    m_socket.close();
    bdMemory::deallocate(m_listeners.m_data);
    m_listeners.m_data = NULL;
    m_listeners.m_size = 0;
    m_listeners.m_capacity = 0;
}

// ============================================================================
// bdDiscoveryClient::discover - ea: 0x8AE370
// ============================================================================
bool bdDiscoveryClient::discover(unsigned int titleID, float timeout,
                                 const bdInetAddr& addr) {
    if (m_status == BD_DISCOVERY_PENDING) {
        bdMessageProxy proxy(".\\bdDiscovery\\bdDiscoveryClient.cpp",
                             "bool __thiscall bdDiscoveryClient::discover(const unsigned int,const float,const class bdInetAddr &)",
                             0x46u, "dw/warn/");
        proxy.log("bdNet/discovery",
                  "bdDiscoveryClient::discover, LAN discovery already running");
        return false;
    }

    if (!m_socket.create(true)) {
        bdMessageProxy proxy(".\\bdDiscovery\\bdDiscoveryClient.cpp",
                             "bool __thiscall bdDiscoveryClient::discover(const unsigned int,const float,const class bdInetAddr &)",
                             0x41u, "dw/err/");
        proxy.log("bdNet/discovery",
                  "bdDiscoveryClient::discover, Could not create socket");
        return false;
    }

    bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUByte8(m_nonce, BD_NONCE_SIZE);
    m_timeout = timeout;

    bdBitBuffer* payload = new (bdMemory::allocate(sizeof(bdBitBuffer)))
        bdBitBuffer(0, false);
    unsigned char type = BD_DISCOVERY_BROADCAST;
    payload->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE);
    payload->writeBits(&type, 8);
    payload->writeDataType(bdBitBuffer::BD_BB_FULL_TYPE);
    payload->writeBits(m_nonce, BD_NONCE_SIZE << 3);
    payload->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_INTEGER32_TYPE);
    payload->writeBits(&titleID, 32);

    bdAddr bcAddr(addr, BD_DEFAULT_DISCOVERY_PORT);
    bool result = false;
    if (m_socket.sendTo(bcAddr, payload->getData(), payload->getDataSize()) >= 0) {
        m_timer.start();
        m_status = BD_DISCOVERY_PENDING;
        result = true;
        bdMessageProxy proxy(".\\bdDiscovery\\bdDiscoveryClient.cpp",
                             "bool __thiscall bdDiscoveryClient::discover(const unsigned int,const float,const class bdInetAddr &)",
                             0x3Cu, "dw/info/");
        proxy.log("bdNet/discovery", "Starting: will run for %f seconds.", timeout);
    } else {
        m_status = BD_DISCOVERY_ERROR;
        bdMessageProxy proxy(".\\bdDiscovery\\bdDiscoveryClient.cpp",
                             "bool __thiscall bdDiscoveryClient::discover(const unsigned int,const float,const class bdInetAddr &)",
                             0x35u, "dw/err/");
        proxy.log("bdNet/discovery",
                  "bdDiscoveryClient::discover, Socket sendTo failed");
    }

    if (payload->releaseRef() == 0)
        delete payload;
    return result;
}

// ============================================================================
// bdDiscoveryClient::update - ea: 0x8AE5C0
// ============================================================================
void bdDiscoveryClient::update() {
    if (m_status != BD_DISCOVERY_PENDING) {
        bdMessageProxy proxy(".\\bdDiscovery\\bdDiscoveryClient.cpp",
                             "void __thiscall bdDiscoveryClient::update(void)",
                             0x89u, "dw/warn/");
        proxy.log("bdNet/discovery",
                  "bdDiscoveryClient::update, Called while client was not running");
        return;
    }

    bdAddr from;
    unsigned char data[BD_MAX_DISCOVERY_RESPONSE_SIZE];
    int received = m_socket.receiveFrom(from, data, sizeof(data));

    if (received > BD_MIN_DISCOVERY_SIZE) {
        bdBitBuffer payload(data, (unsigned int)received << 3, true);
        unsigned char type8 = 0;
        if (payload.readDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE) &&
            payload.readBits(&type8, 8)) {
            if (type8 == BD_DISCOVERY_REPLY) {
                char addrStr[24];
                from.toString(addrStr, sizeof(addrStr));
                bdMessageProxy proxy(".\\bdDiscovery\\bdDiscoveryClient.cpp",
                                     "void __thiscall bdDiscoveryClient::update(void)",
                                     0x61u, "dw/info/");
                proxy.log("bdNet/discovery", "Reply received from %s ", addrStr);

                unsigned char tempNonce[BD_NONCE_SIZE];
                payload.readDataType(bdBitBuffer::BD_BB_FULL_TYPE);
                if (payload.readBits(tempNonce, BD_NONCE_SIZE << 3) &&
                    memcmp(m_nonce, tempNonce, BD_NONCE_SIZE) == 0) {
                    bdReference<bdGameInfo> gameFound;
                    gameFound.m_ptr = bdSingleton<bdGameInfoFactoryImpl>::getInstance()->create();
                    if (gameFound.m_ptr != NULL)
                        gameFound.m_ptr->addRef();
                    bdReference<bdCommonAddr> localAddr =
                        bdSingleton<bdNetImpl>::getInstance()->getLocalCommonAddr();
                    gameFound.m_ptr->deserialize(localAddr, payload);
                    bdMessageProxy proxy(".\\bdDiscovery\\bdDiscoveryClient.cpp",
                                         "void __thiscall bdDiscoveryClient::update(void)",
                                         0x6Bu, "dw/info/");
                    proxy.log("bdNet/discovery",
                              "Reply is relevant to client , firing listeners");
                    fireOnDiscoveryListeners(gameFound);
                } else {
                    bdMessageProxy proxy(".\\bdDiscovery\\bdDiscoveryClient.cpp",
                                         "void __thiscall bdDiscoveryClient::update(void)",
                                         0x70u, "dw/info/");
                    proxy.log("bdNet/discovery",
                              "Reply is not relevant to client , ignoring");
                }
            } else {
                bdMessageProxy proxy(".\\bdDiscovery\\bdDiscoveryClient.cpp",
                                     "void __thiscall bdDiscoveryClient::update(void)",
                                     0x75u, "dw/warn/");
                proxy.log("bdNet/discovery",
                          "Received message is not a discovery reply");
            }
        } else {
            bdMessageProxy proxy(".\\bdDiscovery\\bdDiscoveryClient.cpp",
                                 "void __thiscall bdDiscoveryClient::update(void)",
                                 0x79u, "dw/err/");
            proxy.log("bdNet/discovery", "Malformed message received");
        }
    }

    if (m_timer.getElapsedTimeInSeconds() > m_timeout) {
        for (unsigned int i = 0; i < m_listeners.m_size; i++)
            m_listeners[i]->onDiscoveryFinished();
        m_status = BD_DISCOVERY_IDLE;
        m_socket.close();
    }
}

// ============================================================================
// bdDiscoveryClient::getStatus - ea: 0x8ADE00
// ============================================================================
bdDiscoveryStatus bdDiscoveryClient::getStatus() const {
    return m_status;
}

// ============================================================================
// bdDiscoveryClient::registerListener - ea: 0x8AE270
// ============================================================================
void bdDiscoveryClient::registerListener(bdDiscoveryListener* listener) {
    m_listeners.pushBack(listener);
}

// ============================================================================
// bdDiscoveryClient::unregisterListener - ea: 0x8AE8C0
// ============================================================================
void bdDiscoveryClient::unregisterListener(bdDiscoveryListener* listener) {
    m_listeners.removeAll(listener);
}

// ============================================================================
// bdDiscoveryClient::fireOnDiscoveryListeners - ea: 0x8AE0C0
// ============================================================================
void bdDiscoveryClient::fireOnDiscoveryListeners(
    const bdReference<bdGameInfo>& gameInfo) const {
    for (unsigned int i = 0; i < m_listeners.m_size; i++)
        m_listeners[i]->onDiscovery(gameInfo);
}

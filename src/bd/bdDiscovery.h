// ============================================================================
// bdDiscovery.h - LAN discovery listener/server/client.
// Layouts verified against IDA (bdNet:bdDiscovery*.obj).
// ============================================================================

#pragma once

#include "bd/bd_types.h"
#include "bd/bdStopwatch.h"
#include "bd/bdGameInfo.h"

// ============================================================================
// bdDiscoveryListener - discovery event callbacks.
// Vtable (verified): [dtor, onDiscovery(+4), onDiscoveryFinished(+8),
// onRequest(+12)].
// ============================================================================
class bdDiscoveryListener {
public:
    bdDiscoveryListener();
    virtual ~bdDiscoveryListener();
    virtual void onDiscovery(const bdReference<bdGameInfo>& gameInfo);
    virtual void onDiscoveryFinished();
    virtual void onRequest();
};

// ============================================================================
// bdDiscoveryServer - advertises a game on the LAN (28 bytes).
// Layout verified against IDA (ctor @0x8B0230): m_gameInfo +0, m_socket +4,
// m_listeners +0x0C, m_status +0x18.
// ============================================================================
enum bdDiscoveryStatus {
    BD_DISCOVERY_IDLE = 0,
    BD_DISCOVERY_PENDING = 1,
    BD_DISCOVERY_ERROR = 2,
};

class bdDiscoveryServer {
public:
    bdDiscoveryServer();
    ~bdDiscoveryServer();
    bool start(const bdReference<bdGameInfo>& gameInfo, const bdInetAddr& localAddr);
    void update();
    void stop();
    void registerListener(bdDiscoveryListener* listener);
    void unregisterListener(bdDiscoveryListener* listener);
    bdDiscoveryStatus getStatus() const;
    bdReference<bdGameInfo> getGameInfo();

protected:
    bdReference<bdGameInfo> m_gameInfo;   // +0x00
    bdSocket m_socket;                    // +0x04
    bdFastArray<bdDiscoveryListener*> m_listeners;  // +0x0C
    bdDiscoveryStatus m_status;           // +0x18
};
static_assert(sizeof(bdDiscoveryServer) == 0x1C, "bdDiscoveryServer size mismatch");

// ============================================================================
// bdDiscoveryClient - searches the LAN for a game (64 bytes).
// Layout verified against IDA (ctor @0x8AE1C0): m_timeout +8, m_timer +0x10,
// m_socket +0x20, m_nonce +0x28, m_listeners +0x30, m_status +0x3C.
// ============================================================================
class bdDiscoveryClient {
public:
    bdDiscoveryClient();
    virtual ~bdDiscoveryClient();
    bool discover(unsigned int titleID, float timeout, const bdInetAddr& addr);
    void update();
    bdDiscoveryStatus getStatus() const;
    void registerListener(bdDiscoveryListener* listener);
    void unregisterListener(bdDiscoveryListener* listener);
    void fireOnDiscoveryListeners(const bdReference<bdGameInfo>& gameInfo) const;

#pragma pack(push, 4)
protected:
    uint8_t _pad04[4];                    // +0x04
    float m_timeout;                      // +0x08
    uint8_t _pad0C[4];                    // +0x0C
    bdStopwatch m_timer;                  // +0x10
    bdSocket m_socket;                    // +0x20
    uint8_t m_nonce[8];                   // +0x28
    bdFastArray<bdDiscoveryListener*> m_listeners;  // +0x30
    bdDiscoveryStatus m_status;           // +0x3C
#pragma pack(pop)
};
static_assert(sizeof(bdDiscoveryClient) == 0x40, "bdDiscoveryClient size mismatch");

// Message types on the discovery wire.
enum {
    BD_DISCOVERY_BROADCAST = 26,
    BD_DISCOVERY_REPLY = 27,
};
static const unsigned short BD_DEFAULT_DISCOVERY_PORT = 0xC350;  // 50000

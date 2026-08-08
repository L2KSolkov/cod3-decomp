// ============================================================================
// bdNetImpl - network singleton (0x8C bytes).
// Source: bdNet:bdNet-xbox.obj (14 funcs).
// Layout verified against IDA (ctor @0x8AFC20, start @0x8AF9E0):
//   +0x08 m_processTimer, +0x18 m_params (0x2C), +0x44 m_localCommonAddr,
//   +0x4C m_keyStore, +0x50 m_connectionStore, +0x54 m_dispatcher,
//   +0x60 m_status, +0x70 m_natTravAddrs, +0x84 m_tmpSocket.
// ============================================================================

#ifndef COD3_BD_BDNET_H
#define COD3_BD_BDNET_H

#include "bd/bd_types.h"
#include "bd/bdGameInfo.h"
#include "bd/bdStopwatch.h"
#include "bd/bdSecurityKeyMap.h"
#include "bd/bdConnectionStore.h"

class bdDHKey;
class bdSession;

// ============================================================================
// bdString - minimal placeholder (used only inside bdArray<bdString>).
// ============================================================================
struct bdString {
    char* m_buffer;
    unsigned int m_length;
    unsigned int m_capacity;
};

// ============================================================================
// bdNetStartParams - startup parameters (0x2C bytes).
// ============================================================================
struct bdNetStartParams {
    bool m_onlineGame;                     // +0x00
    uint8_t _pad01[1];                     // +0x01
    uint16_t m_gamePort;                   // +0x02
    bdSocket* m_socket;                    // +0x04
    bdArray<bdString> m_natTravHosts;      // +0x08
    uint16_t m_natTravPort;                // +0x14
    uint8_t _pad16[2];                     // +0x16
    bdArray<bdInetAddr> m_localAddresses;  // +0x18
    float m_timeout;                       // +0x24
    float m_upnpTimeout;                   // +0x28

    bdNetStartParams() : m_onlineGame(false), m_gamePort(0), m_socket(NULL),
                         m_natTravPort(0), m_timeout(0.0f), m_upnpTimeout(0.0f) {
        memset(&m_natTravHosts, 0, sizeof(m_natTravHosts));
        memset(&m_localAddresses, 0, sizeof(m_localAddresses));
    }
};
static_assert(sizeof(bdNetStartParams) == 0x2C, "bdNetStartParams size mismatch");

// ============================================================================
// bdNetImpl - network singleton.
// ============================================================================
enum bdNetStatus {
    BD_NET_STOPPED = 0,
    BD_NET_PENDING = 1,
    BD_NET_DONE = 2,
    BD_NET_INIT_FAILED = 3,
};

class bdNetImpl {
public:
    virtual ~bdNetImpl();

    bdNetStatus getStatus() const;
    bool sendAll();
    void registerDispatchInterceptor(bdDispatchInterceptor* const interceptor);
    void unregisterDispatchInterceptor(bdDispatchInterceptor* const interceptor);
    bdConnectionStore* getConnectionStore();
    bdSecurityKeyMap* getKeyMap();
    const bdNetStartParams& getParams();
    void receiveAndDispatchAll();
    bdReference<bdCommonAddr> getLocalCommonAddr() const;
    void stop();
    bool start(const bdNetStartParams& params);
    void pump();

protected:
    friend class bdSingleton<bdNetImpl>;
    bdNetImpl();

protected:
    bdStopwatch m_processTimer;        // +0x08
    bdNetStartParams m_params;         // +0x18
    bdReference<bdCommonAddr> m_localCommonAddr;  // +0x44
    uint8_t _pad48[4];                 // +0x48
    bdSecurityKeyMap m_keyStore;       // +0x4C
    bdConnectionStore* m_connectionStore;  // +0x50
    bdDispatcher m_dispatcher;         // +0x54
    bdNetStatus m_status;              // +0x60
    uint8_t _pad64[12];                // +0x64
    bdArray<bdAddr> m_natTravAddrs;    // +0x70
    uint8_t _pad7C[8];                 // +0x7C
    bdSocket m_tmpSocket;              // +0x84
};
static_assert(sizeof(bdNetImpl) == 0x90, "bdNetImpl size mismatch");

#endif // COD3_BD_BDNET_H

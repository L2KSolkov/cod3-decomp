// ============================================================================
// bdSocketRouter - routes data through the underlying socket (44 bytes).
// Source: bdSocket:bdSocketRouter-xbox.obj (10 funcs).
// Layout verified against IDA (ctor @0x8B73B0): m_socket +4, m_qosProber +8
// (bdQoSProbe, 24 bytes), m_interceptors +0x20.
// Vtable: [dtor, sendTo(+4), receiveFrom(+8), onNATAddrDiscovery(+12),
// onNATAddrDiscoveryFailed(+16)].
// ============================================================================

#ifndef COD3_BD_BDSOCKETROUTER_H
#define COD3_BD_BDSOCKETROUTER_H

#include "bd/bd_types.h"
#include "bd/bdQoSProbe.h"

class bdSecurityKeyMap;
struct bdDHKey;

enum bdSocketStatus {
    BD_SOCKET_ROUTER_UNINITIALIZED = 0,
    BD_SOCKET_ROUTER_INITIALIZED = 1,
    BD_SOCKET_ROUTER_QUIT = 2,
};

enum bdSocketAssociationStatus {
    BD_SOCKET_IDLE = 0,
    BD_SOCKET_PENDING = 1,
    BD_SOCKET_CONNECTED = 2,
    BD_SOCKET_LOST = 3,
};

class bdSocketRouter {
public:
    bdSocketRouter(bdSocket* socket, bdSecurityKeyMap* keyMap, bdDHKey* dhKey,
                   const bdArray<bdAddr>& localAddresses);
    virtual ~bdSocketRouter();

    void pump();
    bdQoSProbe* getQoSProber();
    bool connect(bdReference<bdAddrHandle>& addrHandle);
    bdSocketAssociationStatus getStatus(const bdReference<bdAddrHandle>& addrHandle);
    virtual int sendTo(const bdReference<bdAddrHandle>& addrHandle,
                       const void* data, unsigned int length);
    virtual int receiveFrom(bdReference<bdAddrHandle>& addrHandle,
                            void* data, unsigned int size);
    virtual void onNATAddrDiscovery(bdReference<bdCommonAddr> addr,
                                    const bdAddr& realAddr);
    virtual void onNATAddrDiscoveryFailed(bdReference<bdCommonAddr> addr);

protected:
    bdSocket* m_socket;                       // +0x04
    bdQoSProbe m_qosProber;                   // +0x08
    bdFastArray<struct bdDispatchInterceptor*> m_interceptors;  // +0x20
};
static_assert(sizeof(bdSocketRouter) == 0x2C, "bdSocketRouter size mismatch");

#endif // COD3_BD_BDSOCKETROUTER_H

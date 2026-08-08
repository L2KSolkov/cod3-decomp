// ============================================================================
// bdConnectionStore - manages all connections (0x60 bytes).
// Source: bdConnection:bdConnectionStore.obj (12 funcs).
// Layout verified against IDA (ctor @0x8A2DE0): embedded bdSocketRouter +0
// (0x2C), m_securityKeyMap +0x2C, m_dhKey +0x30, m_me +0x34,
// m_connectionMap +0x38 (0x18), m_listeners +0x50 (0xC), m_status +0x5C.
// ============================================================================

#ifndef COD3_BD_BDCONNECTIONSTORE_H
#define COD3_BD_BDCONNECTIONSTORE_H

#include "bd/bd_types.h"
#include "bd/bdGameInfo.h"
#include "bd/bdHashMap.h"
#include "bd/bdSocketRouter.h"

class bdConnectionListener;
class bdDHKey;
class bdSecurityKeyMap;

enum bdConnectionStoreStatus {
    BD_CONNECTION_STORE_UNINITIALIZED = 0,
    BD_CONNECTION_STORE_INITIALIZED = 1,
    BD_CONNECTION_STORE_SHUTTING_DOWN = 2,
    BD_CONNECTION_STORE_ERROR = 3,
};

class bdConnectionStore {
public:
    bdConnectionStore(bdSocket* socket, const bdReference<bdCommonAddr>& me,
                      bdSecurityKeyMap* securityKeyMap, bdDHKey* dhKey,
                      const bdArray<bdAddr>& localAddresses);
    ~bdConnectionStore();

    bdSocketRouter* getSocketRouter();
    unsigned int flush(bdReference<bdConnection>& connection);
    bool flushAll();
    bool receiveFrom(bdReference<bdConnection>& connection);
    void disconnectAll();
    void closeAll();
    void registerListener(bdConnectionListener* listener);
    void unregisterListener(bdConnectionListener* listener);
    void remove(const bdReference<bdConnection>& connection);
    bdReference<bdConnection> create(const bdReference<bdCommonAddr>& addr,
                                     const XNKID& secID);

protected:
    bdSocketRouter m_socket;   // +0x00 (0x2C)
    bdSecurityKeyMap* m_securityKeyMap;   // +0x2C
    bdDHKey* m_dhKey;                     // +0x30
    bdReference<bdCommonAddr> m_me;       // +0x34
    bdHashMap<bdAddrHandleHashmapWrapper, bdReference<bdConnection>,
              bdAddrHandleHashmapWrapper> m_connectionMap;  // +0x38 (0x18)
    bdFastArray<bdConnectionListener*> m_listeners;  // +0x50
    bdConnectionStoreStatus m_status;     // +0x5C
};
static_assert(sizeof(bdConnectionStore) == 0x60, "bdConnectionStore size mismatch");

#endif // COD3_BD_BDCONNECTIONSTORE_H
// ============================================================================
// bdSessionListener - peer-to-peer session event callbacks.
// Source: bdPeer:bdSessionListener.obj (10 funcs).
// Vtable (verified @0xD564B8): dtor, onSessionJoinAccepted(+4),
// onSessionJoinRefused(+8), onSessionConnectFail(+12),
// onSessionConnectSuccess(+16), onSessionConnect(+20),
// onSessionDisconnect(+24), onSessionStatusChange(+28),
// onSessionRoleUpdate(+32). COD3 has no onForceLeave slot.
// ============================================================================

#ifndef COD3_BD_BDSESSIONLISTENER_H
#define COD3_BD_BDSESSIONLISTENER_H

#include "bd/bd_types.h"
#include "bd/bdSession.h"

class bdSessionListener {
public:
    bdSessionListener();
    virtual ~bdSessionListener();

    virtual void onSessionJoinAccepted();
    virtual void onSessionJoinRefused(bdReference<bdBitBuffer> userData);
    virtual void onSessionConnectFail();
    virtual void onSessionConnectSuccess();
    virtual void onSessionConnect(bdReference<bdConnection> connection);
    virtual void onSessionDisconnect(bdReference<bdConnection> connection);
    virtual void onSessionStatusChange(bdSession::bdSessionStatus previous,
                                       bdSession::bdSessionStatus current);
    virtual void onSessionRoleUpdate(bdSession::bdSessionRole role);
};

#endif // COD3_BD_BDSESSIONLISTENER_H

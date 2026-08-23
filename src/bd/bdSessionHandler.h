// ============================================================================
// bdSessionHandler — default session connect handler (3 non-inline funcs).
// Source: bdPeer:bdSessionHandler.obj
// Verified against IDA:
//   ctor @0x8B5CA0 (??0bdSessionHandler@@QAE@XZ)
//   dtor @0x8B5CB0 (??1bdSessionHandler@@UAE@XZ)
//   onSessionConnectRequest @0x8B5CE0 (?onSessionConnectRequest@bdSessionHandler@@UAE_NV?$bdReference@VbdBitBuffer@@@@QAVbdBitBuffer@@@Z)
// ============================================================================
#ifndef COD3_BD_BDSSESSIONHANDLER_H
#define COD3_BD_BDSSESSIONHANDLER_H

#include "bd/bd_types.h"

// ============================================================================
// bdSessionInterceptor — session connection-intercept interface (2-slot vtable)
// ============================================================================
class bdSessionInterceptor {
public:
    bdSessionInterceptor();                                    // @0x8B5C60
    virtual ~bdSessionInterceptor();                           // @0x8B5C70
    virtual bool onSessionConnectRequest(bdReference<bdBitBuffer> buffer, bdBitBuffer* const result) = 0;
};

// ============================================================================
// bdSessionHandler — default handler; accepts every connect request.
// ============================================================================
class bdSessionHandler {
public:
    bdSessionHandler();                                        // @0x8B5CA0
    virtual ~bdSessionHandler();                               // @0x8B5CB0
    virtual bool onSessionConnectRequest(bdReference<bdBitBuffer> buffer,
                                         bdBitBuffer* const result); // @0x8B5CE0
};

#endif // COD3_BD_BDSSESSIONHANDLER_H

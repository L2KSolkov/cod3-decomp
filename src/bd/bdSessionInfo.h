// ============================================================================
// bdSessionInfo — session info formatting helpers (3 non-inline funcs).
// Source: bdPeer:bdSessionInfo.obj
// Verified against IDA:
//   ctor       @0x8B56C0 (??0bdSessionInfo@@IAE@XZ)
//   getInfo    @0x8B56D0 (?getInfo@bdSessionInfo@@SAIQBVbdSession@@PADI@Z)
//   getPeerInfo @0x8B5970 (?getPeerInfo@bdSessionInfo@@SAIQBVbdSession@@PADII@Z)
// ============================================================================
#ifndef COD3_BD_BDSSESSIONINFO_H
#define COD3_BD_BDSSESSIONINFO_H

#include "bd/bd_types.h"
#include "bd/bdAddressMap.h"

// ============================================================================
// bdSession - defined in bd/bdSession.h
// ============================================================================
#include "bd/bdSession.h"

// ============================================================================
// bdSessionInfo — static info formatters.
// ============================================================================
class bdSessionInfo {
public:
    bdSessionInfo();
    ~bdSessionInfo() {}

    static int  getInfo(const bdSession* session, char* buf, unsigned int bufSize);  // @0x8B56D0
    static unsigned char* getPeerInfo(const bdSession* session, char* buf, int a3,
                                      unsigned int a4);  // @0x8B5970
};

// ============================================================================
// Free helpers (inline COMDATs)
// ============================================================================
inline int bdSnprintf(char* buf, unsigned int maxlen, const char* format, ...);

#endif // COD3_BD_BDSSESSIONINFO_H

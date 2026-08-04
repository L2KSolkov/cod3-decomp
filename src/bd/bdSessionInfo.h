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

// ============================================================================
// bdSession — peer-to-peer session (opaque here; methods are in bdSession.obj)
// ============================================================================
struct bdSession {
    enum bdSessionRole {
        BD_SESSION_HOST = 0,
        BD_SESSION_PEER = 1,
    };

    enum bdSessionStatus {
        BD_SESSION_NOT_CONNECTED = 0,
        BD_SESSION_CONNECTING_TO_HOST = 1,
        BD_SESSION_CONNECTED_TO_HOST = 2,
        BD_SESSION_CONNECTING_TO_PEERS = 3,
        BD_SESSION_NOT_READY = 4,
        BD_SESSION_READY = 5,
        BD_SESSION_CONNECTING_TO_HOST_FAILED = 6,
        BD_SESSION_CONNECTING_TO_PEERS_FAILED = 7,
    };

    bdSessionRole   getRole() const;
    bdSessionStatus getStatus() const;
    unsigned int    getNumPeers() const;
    bdReference<bdConnection> getHost() const;
    bdReference<bdConnection> getConnection(unsigned int index) const;
    unsigned int    getPeerHash(unsigned int index) const;
};

// ============================================================================
// bdAddressMapImpl — address-to-string mapping (opaque; bdAddressMap-xbox.obj)
// ============================================================================
class bdAddressMapImpl {
public:
    static bdAddressMapImpl* getInstance();
    unsigned int addrToString(const bdReference<bdAddrHandle>& addrHandle,
                              char* pchBuf, unsigned int cchBuf) const;
};

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

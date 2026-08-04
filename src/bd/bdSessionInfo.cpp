// ============================================================================
// bdSessionInfo.cpp — session info formatting helpers (3 non-inline funcs).
// Source: bdPeer:bdSessionInfo.obj
// Verified against IDA:
//   ctor       @0x8B56C0 (??0bdSessionInfo@@IAE@XZ)
//   getInfo    @0x8B56D0 (?getInfo@bdSessionInfo@@SAIQBVbdSession@@PADI@Z)
//   getPeerInfo @0x8B5970 (?getPeerInfo@bdSessionInfo@@SAIQBVbdSession@@PADII@Z)
// ============================================================================
#include "bdSessionInfo.h"

#include <stdarg.h>
#include <stdio.h>

// ============================================================================
// bdSnprintf — bounded snprintf that always null-terminates. (inline COMDAT)
// ea: 0x776830
// ============================================================================
int bdSnprintf(char* buf, unsigned int maxlen, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    int v3 = _vscprintf(format, ap);
    vsnprintf(buf, maxlen, format, ap);
    buf[maxlen - 1] = 0;
    return v3;
}

// ============================================================================
// bdSessionInfo::bdSessionInfo — ea: 0x8B56C0
// ============================================================================
bdSessionInfo::bdSessionInfo() {
}

// ============================================================================
// bdSessionInfo::getInfo — format role/status/peer lines into a buffer.
// ea: 0x8B56D0
// ============================================================================
int bdSessionInfo::getInfo(const bdSession* session, char* buf, unsigned int bufSize) {
    char* cur = buf;
    char* end = &buf[bufSize];

    const char* role = "BD_SESSION_PEER";
    if (session->getRole() == bdSession::BD_SESSION_HOST)
        role = "BD_SESSION_HOST";

    int n = bdSnprintf(cur, (unsigned int)(end - cur), "Role: %s\n", role);
    if (n <= 0) {
        if (bufSize != 0)
            *(end - 1) = 0;
    } else {
        cur = &cur[n];
    }

    const char* statusNames[8] = {
        "BD_SESSION_NOT_CONNECTED",
        "BD_SESSION_CONNECTING_TO_HOST",
        "BD_SESSION_CONNECTED_TO_HOST",
        "BD_SESSION_CONNECTING_TO_PEERS",
        "BD_SESSION_NOT_READY",
        "BD_SESSION_READY",
        "BD_SESSION_CONNECTING_TO_HOST_FAILED",
        "BD_SESSION_CONNECTING_TO_PEERS_FAILED",
    };
    const char* status = "**UNKNOWN**";
    bdSession::bdSessionStatus Status = session->getStatus();
    if (Status < 8)
        status = statusNames[Status];

    n = bdSnprintf(cur, (unsigned int)(end - cur), "Status: %s\n", status);
    if (n <= 0) {
        if (bufSize != 0)
            *(end - 1) = 0;
    } else {
        cur = &cur[n];
    }

    unsigned int numPeers = session->getNumPeers();
    bdReference<bdConnection> host = session->getHost();

    const char* connNames[5] = {
        "BD_NOT_CONNECTED",
        "BD_CONNECTING",
        "BD_CONNECTED",
        "BD_DISCONNECTING",
        "BD_DISCONNECTED",
    };

    for (unsigned int i = 0; i < numPeers; ++i) {
        bdReference<bdConnection> conn = session->getConnection(i);
        const char* hostSuffix = "\n";
        if (conn.m_ptr == host.m_ptr)
            hostSuffix = "(host)\n";

        const char* connStatus = "**UNKNOWN**";
        unsigned int cs = conn.m_ptr->getStatus();
        if (cs < 5)
            connStatus = connNames[cs];

        bdConnectionStatistics* stats = conn.m_ptr->getStats();
        unsigned int rtt = (unsigned int)(stats->getAvgRTT() * 1000.0);

        char addr[24];
        const bdReference<bdAddrHandle>& addrHandle = conn.m_ptr->getAddressHandle();
        bdAddressMapImpl::getInstance();
        bdAddressMapImpl::getInstance()->addrToString(addrHandle, addr, 22);

        n = bdSnprintf(cur, (unsigned int)(end - cur),
                       "%u - Peer: %s, %u ms RTT, %s, %s", i, addr, rtt, connStatus, hostSuffix);
        if (n <= 0) {
            if (bufSize != 0)
                *(end - 1) = 0;
        } else {
            cur = &cur[n];
        }
    }

    return (int)(bufSize + cur - end);
}

// ============================================================================
// bdSessionInfo::getPeerInfo — format a single peer's info line.
// ea: 0x8B5970
// ============================================================================
unsigned char* bdSessionInfo::getPeerInfo(const bdSession* session, char* buf, int a3,
                                          unsigned int a4) {
    char* cur = buf;
    char* end = &buf[a3];
    unsigned int numPeers = session->getNumPeers();
    bdReference<bdConnection> host = session->getHost();

    if (a4 >= numPeers) {
        return NULL;
    }

    bdReference<bdConnection> conn = session->getConnection(a4);

    const char* connNames[5] = {
        "BD_NOT_CONNECTED",
        "BD_CONNECTING",
        "BD_CONNECTED",
        "BD_DISCONNECTING",
        "BD_DISCONNECTED",
    };
    const char* hostSuffix = "";
    if (conn.m_ptr == host.m_ptr)
        hostSuffix = "(host)";

    const char* connStatus = "**UNKNOWN**";
    unsigned int cs = conn.m_ptr->getStatus();
    if (cs < 5)
        connStatus = connNames[cs];

    bdConnectionStatistics* stats = conn.m_ptr->getStats();
    unsigned int rtt = (unsigned int)(stats->getAvgRTT() * 1000.0);

    char addr[24];
    const bdReference<bdAddrHandle>& addrHandle = conn.m_ptr->getAddressHandle();
    bdAddressMapImpl::getInstance();
    bdAddressMapImpl::getInstance()->addrToString(addrHandle, addr, 22);

    unsigned int peerHash = session->getPeerHash(a4);

    int n = bdSnprintf(cur, (unsigned int)(end - cur),
                       "H:%u %u - IP: %s, %u ms RTT, %s, %s",
                       peerHash, a4, addr, rtt, connStatus, hostSuffix);
    unsigned char* result;
    if (n <= 0) {
        if (a3 != 0)
            *(end - 1) = 0;
        result = (unsigned char*)(end - 1);
    } else {
        result = (unsigned char*)(cur + n);
    }
    return result;
}

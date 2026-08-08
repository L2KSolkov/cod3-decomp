// ============================================================================
// bdQoSProbe - Xbox XNet QoS probing (24 bytes).
// Source: bdSocket:bdQoSProbe-xbox.obj (11 funcs).
// Layout verified against IDA (ctor @0x8B6810): m_maxBandwidth +0,
// m_requests +4, m_id +0x10.
// ============================================================================

#ifndef COD3_BD_BDQOSPROBE_H
#define COD3_BD_BDQOSPROBE_H

#include "bd/bd_types.h"
#include "bd/bdGameInfo.h"

class bdQoSProbeListener;

// ============================================================================
// bdQoSRemoteAddr - remote endpoint for QoS (28 bytes).
// ============================================================================
struct bdQoSRemoteAddr {
    bdReference<bdCommonAddr> m_addr;   // +0x00
    XNKID m_id;                         // +0x04
    XNKEY m_key;                        // +0x0C
};
static_assert(sizeof(bdQoSRemoteAddr) == 0x1C, "bdQoSRemoteAddr size mismatch");

// ============================================================================
// bdQoSProbeInfo - completed probe result (36 bytes).
// Layout verified against IDA (pump @0x8B6BC0): m_addr +0, m_realAddr +4,
// m_latency +0xC, m_data +0x10, m_dataSize +0x14, m_disabled +0x18,
// m_bandwidthUp +0x1C, m_bandwidthDown +0x20.
// ============================================================================
struct bdQoSProbeInfo {
    bdReference<bdCommonAddr> m_addr;    // +0x00
    bdAddr m_realAddr;                   // +0x04
    float m_latency;                     // +0x0C
    unsigned char* m_data;               // +0x10
    unsigned int m_dataSize;             // +0x14
    bool m_disabled;                     // +0x18
    unsigned int m_bandwidthUp;          // +0x1C
    unsigned int m_bandwidthDown;        // +0x20

    bdQoSProbeInfo();
    ~bdQoSProbeInfo();
};
static_assert(sizeof(bdQoSProbeInfo) == 0x24, "bdQoSProbeInfo size mismatch");

// ============================================================================
// bdQoSProbe - XNet QoS probe session (24 bytes).
// ============================================================================
class bdQoSProbe {
public:
    bdQoSProbe();

    void setMaxBandwidth(unsigned int maxBandwidth);
    bool listen(const XNKID& xnkid, unsigned char* data, unsigned int dataSize);
    bool setData(unsigned char* data, unsigned int dataSize);
    void disableListener();
    void enableListener();
    void shutdownListener();
    bool probe(bdQoSRemoteAddr& addr, bdQoSProbeListener* listener);
    bool probe(bdFastArray<bdQoSRemoteAddr>& addrs, bdQoSProbeListener* listener);
    void pump();
    void cancelProbes();

protected:
    unsigned int m_maxBandwidth;          // +0x00 (default 0x4000)
    bdLinkedList<struct bdQoSProbeEntryWrapper*> m_requests;  // +0x04
    XNKID m_id;                           // +0x10
};
static_assert(sizeof(bdQoSProbe) == 0x18, "bdQoSProbe size mismatch");

#endif // COD3_BD_BDQOSPROBE_H

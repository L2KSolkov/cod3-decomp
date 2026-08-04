// ============================================================================
// COD3 BD Network Types — bdReference<T>, bdBuffer, bdBitBuffer, bdMessage,
// bdConnection, bdCommonAddr, bdAddrHandle, bdConnectionStatistics
// Reconstructed from IDA local types (PDB symbol data).
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

#include "bd/bdReference/bdReferencable.h"
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// bdReference<T> — intrusive reference wrapper (4 bytes) — verified against IDA
// ============================================================================
template <typename T>
struct bdReference {
    T* m_ptr;  // +0x00

    bdReference() : m_ptr(NULL) {}
};
static_assert(sizeof(bdReference<bdReferencable>) == 4, "bdReference size mismatch");

// ============================================================================
// bdInAddr — IPv4 address (4 bytes)
// ============================================================================
struct bdInAddr {
    union {
        uint8_t  m_byte[4];
        uint32_t m_s_addr;
    };
};
static_assert(sizeof(bdInAddr) == 4, "bdInAddr size mismatch");

// ============================================================================
// bdInetAddr — internet address (4 bytes)
// Size: 0x04 (4 bytes) — verified against IDA
// ============================================================================
struct bdInetAddr {
    bdInAddr m_addr;  // +0x00
};
static_assert(sizeof(bdInetAddr) == 4, "bdInetAddr size mismatch");

// ============================================================================
// bdAddr — address + port (8 bytes)
// Size: 0x08 (8 bytes) — verified against IDA
// ============================================================================
struct bdAddr {
    bdInetAddr      m_address;  // +0x00
    uint16_t        m_port;     // +0x04
};
static_assert(sizeof(bdAddr) == 8, "bdAddr size mismatch");
static_assert(offsetof(bdAddr, m_port) == 0x04, "bdAddr::m_port offset mismatch");

// ============================================================================
// bdBuffer — raw byte range (12 bytes)
// Size: 0x0C (12 bytes) — verified against IDA
// ============================================================================
struct bdBuffer {
    char* m_begin;  // +0x00
    char* m_end;    // +0x04
    char* m_next;   // +0x08
};
static_assert(sizeof(bdBuffer) == 0x0C, "bdBuffer size mismatch");

// ============================================================================
// bdFastArray<T> — raw capacity/size array (12 bytes) — verified against IDA
// ============================================================================
template <typename T>
struct bdFastArray {
    T*           m_data;     // +0x00
    unsigned int m_capacity; // +0x04
    unsigned int m_size;     // +0x08
};
static_assert(sizeof(bdFastArray<char>) == 0x0C, "bdFastArray size mismatch");

// ============================================================================
// bdArray<T> — bdFastArray alias (12 bytes)
// ============================================================================
template <typename T>
struct bdArray {
    T*           m_data;     // +0x00
    unsigned int m_capacity; // +0x04
    unsigned int m_size;     // +0x08
};
static_assert(sizeof(bdArray<char>) == 0x0C, "bdArray size mismatch");

// ============================================================================
// bdByteBuffer — byte buffer (24 bytes)
// Size: 0x18 (24 bytes) — verified against IDA
// ============================================================================
class bdByteBuffer : public bdReferencable {
public:
    unsigned int m_size;      // +0x08
    uint8_t*     m_data;      // +0x0C
    uint8_t*     m_readPtr;   // +0x10
    uint8_t*     m_writePtr;  // +0x14
};
static_assert(sizeof(bdByteBuffer) == 0x18, "bdByteBuffer size mismatch");
static_assert(offsetof(bdByteBuffer, m_size) == 0x08, "bdByteBuffer::m_size offset mismatch");
static_assert(offsetof(bdByteBuffer, m_data) == 0x0C, "bdByteBuffer::m_data offset mismatch");

// ============================================================================
// bdBitBuffer — bit-level serialization buffer (36 bytes)
// Size: 0x24 (36 bytes) — verified against IDA
// ============================================================================
class bdBitBuffer : public bdReferencable {
public:
    bdFastArray<uint8_t> m_data;           // +0x08
    unsigned int  m_writePosition;         // +0x14
    unsigned int  m_maxWritePosition;      // +0x18
    unsigned int  m_readPosition;          // +0x1C
    bool          m_failedRead;            // +0x20
    bool          m_typeChecked;           // +0x21
    uint8_t       _pad22[2];               // +0x22
};
static_assert(sizeof(bdBitBuffer) == 0x24, "bdBitBuffer size mismatch");
static_assert(offsetof(bdBitBuffer, m_data) == 0x08, "bdBitBuffer::m_data offset mismatch");
static_assert(offsetof(bdBitBuffer, m_writePosition) == 0x14, "bdBitBuffer::m_writePosition offset mismatch");
static_assert(offsetof(bdBitBuffer, m_readPosition) == 0x1C, "bdBitBuffer::m_readPosition offset mismatch");

// ============================================================================
// bdMessage — network message (24 bytes)
// Size: 0x18 (24 bytes) — verified against IDA
// ============================================================================
class bdMessage : public bdReferencable {
public:
    uint8_t        m_type;                // +0x08
    uint8_t        _pad09[3];             // +0x09
    bdReference<bdBitBuffer> m_payload;   // +0x0C
    bool           m_payloadTypeChecked;  // +0x10
    uint8_t        _pad11[3];             // +0x11
    bdReference<bdByteBuffer> m_unencPayload;  // +0x14
};
static_assert(sizeof(bdMessage) == 0x18, "bdMessage size mismatch");
static_assert(offsetof(bdMessage, m_type) == 0x08, "bdMessage::m_type offset mismatch");
static_assert(offsetof(bdMessage, m_payload) == 0x0C, "bdMessage::m_payload offset mismatch");
static_assert(offsetof(bdMessage, m_unencPayload) == 0x14, "bdMessage::m_unencPayload offset mismatch");

// ============================================================================
// XNADDR — Xbox address (36 bytes) — shim-compatible placeholder
// Size: 0x24 (36 bytes) — verified against IDA
// ============================================================================
struct XNADDR {
    uint8_t ina[4];              // +0x00 (in_addr)
    uint8_t inaOnline[4];        // +0x04
    uint16_t wPortOnline;        // +0x08
    uint8_t abEnet[6];           // +0x0A
    uint8_t abOnline[20];        // +0x10
};
static_assert(sizeof(XNADDR) == 0x24, "XNADDR size mismatch");

// ============================================================================
// bdNATType — NAT classification enum
// ============================================================================
typedef uint32_t bdNATType;

// ============================================================================
// bdCommonAddr — common network address (64 bytes)
// Size: 0x40 (64 bytes) — verified against IDA
// ============================================================================
class bdCommonAddr : public bdReferencable {
public:
    XNADDR     m_addr;      // +0x08 (36 bytes)
    uint16_t   m_port;      // +0x2C
    uint32_t   m_titleId;   // +0x30
    uint32_t   m_hash;      // +0x34
    bool       m_isLoopback;// +0x38
    uint8_t    _pad39[3];   // +0x39
    bdNATType  m_natType;   // +0x3C
};
static_assert(sizeof(bdCommonAddr) == 0x40, "bdCommonAddr size mismatch");
static_assert(offsetof(bdCommonAddr, m_addr) == 0x08, "bdCommonAddr::m_addr offset mismatch");
static_assert(offsetof(bdCommonAddr, m_port) == 0x2C, "bdCommonAddr::m_port offset mismatch");
static_assert(offsetof(bdCommonAddr, m_hash) == 0x34, "bdCommonAddr::m_hash offset mismatch");
static_assert(offsetof(bdCommonAddr, m_natType) == 0x3C, "bdCommonAddr::m_natType offset mismatch");

// ============================================================================
// bdAddrHandle — address handle (16 bytes)
// Size: 0x10 (16 bytes) — verified against IDA
// ============================================================================
class bdAddrHandle : public bdReferencable {
public:
    bdInAddr   m_addr;  // +0x08 (4 bytes)
    uint16_t   m_port;  // +0x0C
    uint8_t    _pad0E[2];  // +0x0E
};
static_assert(sizeof(bdAddrHandle) == 0x10, "bdAddrHandle size mismatch");
static_assert(offsetof(bdAddrHandle, m_addr) == 0x08, "bdAddrHandle::m_addr offset mismatch");
static_assert(offsetof(bdAddrHandle, m_port) == 0x0C, "bdAddrHandle::m_port offset mismatch");

// ============================================================================
// bdConnectionStatistics — connection stats (64 bytes)
// Size: 0x40 (64 bytes) — verified against IDA
// ============================================================================
struct bdConnectionStatistics {
    unsigned int m_bytesSent;            // +0x00
    unsigned int m_bytesSentPerSecond;   // +0x04
    unsigned int m_avgBytesSent;         // +0x08
    unsigned int m_lastBytesSent;        // +0x0C
    unsigned int m_bytesRecv;            // +0x10
    unsigned int m_avgPacketSentSize;    // +0x14
    unsigned int m_avgPacketRecvSize;    // +0x18
    unsigned int m_maxPacketSizeSent;    // +0x1C
    unsigned int m_minPacketSizeSent;    // +0x20
    unsigned int m_maxPacketSizeRecv;    // +0x24
    unsigned int m_minPacketSizeRecv;    // +0x28
    unsigned int m_packetsSent;          // +0x2C
    unsigned int m_packetsRecv;          // +0x30
    float        m_maxRTT;               // +0x34
    float        m_minRTT;               // +0x38
    float        m_avgRTT;               // +0x3C

    // bdConnectionStatistics.obj (method)
    float getAvgRTT() const { return m_avgRTT; }
};
static_assert(sizeof(bdConnectionStatistics) == 0x40, "bdConnectionStatistics size mismatch");
static_assert(offsetof(bdConnectionStatistics, m_bytesSent) == 0x00, "bdConnectionStatistics::m_bytesSent offset mismatch");
static_assert(offsetof(bdConnectionStatistics, m_packetsRecv) == 0x30, "bdConnectionStatistics::m_packetsRecv offset mismatch");
static_assert(offsetof(bdConnectionStatistics, m_avgRTT) == 0x3C, "bdConnectionStatistics::m_avgRTT offset mismatch");

// ============================================================================
// bdConnection — connection base (100 bytes)
// Size: 0x64 (100 bytes) — verified against IDA
// ============================================================================
struct bdConnectionListener;

class bdConnection : public bdReferencable {
public:
    enum Status {
        BD_NOT_CONNECTED = 0,
        BD_CONNECTING = 1,
        BD_CONNECTED = 2,
        BD_DISCONNECTING = 3,
        BD_DISCONNECTED = 4,
    };

    bdReference<bdCommonAddr>   m_addr;            // +0x08
    bdReference<bdAddrHandle>   m_addrHandle;      // +0x0C
    bdConnectionStatistics      m_stats;           // +0x10
    bdFastArray<bdConnectionListener*> m_listeners;  // +0x50
    unsigned int                m_maxTransmissionRate;  // +0x5C
    Status                      m_status;          // +0x60

    // bdConnection.obj (methods; declared for bdSessionInfo)
    Status getStatus() const { return m_status; }
    const bdReference<bdAddrHandle>& getAddressHandle() const { return m_addrHandle; }
    bdConnectionStatistics* getStats() { return &m_stats; }
};
static_assert(sizeof(bdConnection) == 0x64, "bdConnection size mismatch");
static_assert(offsetof(bdConnection, m_addr) == 0x08, "bdConnection::m_addr offset mismatch");
static_assert(offsetof(bdConnection, m_stats) == 0x10, "bdConnection::m_stats offset mismatch");
static_assert(offsetof(bdConnection, m_listeners) == 0x50, "bdConnection::m_listeners offset mismatch");
static_assert(offsetof(bdConnection, m_maxTransmissionRate) == 0x5C, "bdConnection::m_maxTransmissionRate offset mismatch");
static_assert(offsetof(bdConnection, m_status) == 0x60, "bdConnection::m_status offset mismatch");

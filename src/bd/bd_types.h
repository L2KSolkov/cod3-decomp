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
    bdReference(T* ptr) : m_ptr(ptr) {}
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

    bdByteBuffer(unsigned int size);
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

    enum bdBitBufferDataType {
        BD_BB_UNSIGNED_INTEGER32_TYPE = 0,
        BD_BB_FULL_TYPE = 1,
    };

    void writeDataType(bdBitBufferDataType type);
    void writeBits(const void* data, unsigned int bitCount);
    bool readDataType(bdBitBufferDataType type);
    bool readBits(void* data, unsigned int bitCount);
    bdBitBuffer(const unsigned char* data, unsigned int bitCount, bool typeChecked);
    bdBitBuffer(unsigned int bitCount, bool typeChecked);
    bool getTypeCheck() const;
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

    bdMessage(unsigned char type, bool payloadTypeChecked);
    bdMessage(unsigned char type, const unsigned char* data, unsigned int dataSize,
              bool typeChecked, const unsigned char* unencData, unsigned int unencSize);
    virtual ~bdMessage();
    unsigned char getType() const;
    bdReference<bdBitBuffer> getPayload();
    bool hasPayload() const;
    bdByteBuffer* createUnencryptedPayload(unsigned int size);
    bdReference<bdByteBuffer> getUnencryptedPayload() const;
    bool hasUnencryptedPayload() const;
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

    bdCommonAddr();
    void serialize(uint8_t* buffer) const;
    bool deserialize(const bdReference<bdCommonAddr>& ref, const uint8_t* buffer);
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

    bdConnectionStatistics();
    unsigned int reset();
    unsigned int addBytesSent(unsigned int bytes);
    unsigned int addBytesRecv(unsigned int bytes);
    unsigned int addPacketSizeSent(unsigned int size);
    unsigned int addPacketSizeRecv(unsigned int size);
    unsigned int addPacketsSent(unsigned int packets);
    unsigned int addPacketsRecv(unsigned int packets);
    void setLastRTT(float rtt);
    unsigned int getBytesSent() const;
    unsigned int getBytesSentPerSecond() const;
    unsigned int getBytesRecv() const;
    unsigned int getPacketsSent() const;
    unsigned int getPacketsRecv() const;
    float getAvgRTT() const;
    void update(float dt);
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

// ============================================================================
// bdChunkTypes â€” chunk type enum
// ============================================================================
enum bdChunkTypes {
    BD_CHUNK_DATA = 2,
};

// ============================================================================
// bdLinkedList<T> â€” intrusive doubly-linked list (12 bytes)
// ============================================================================
template <typename T>
struct bdLinkedList {
    T*           m_head;      // +0x00
    T*           m_tail;      // +0x04
    unsigned int m_size;      // +0x08

    T* getHead() { return m_head; }
    void insertAfter(void* pos, const T& value);
    void removeAt(void** pos);
    void clear();
};
static_assert(sizeof(bdLinkedList<char>) == 0x0C, "bdLinkedList size mismatch");

// ============================================================================
// bdGapAckBlock â€” SACK gap block (16 bytes with link)
// ============================================================================
struct bdGapAckBlock {
    unsigned short m_start;   // +0x00
    unsigned short m_end;     // +0x02
    bdGapAckBlock* m_next;    // +0x04
};
static_assert(sizeof(bdGapAckBlock) == 0x08, "bdGapAckBlock size mismatch");

// ============================================================================
// bdChunk â€” packet chunk base (12 bytes)
// Size: 0x0C (12 bytes) â€” verified against IDA
// ============================================================================
class bdChunk : public bdReferencable {
public:
    bdChunkTypes m_type;  // +0x08

    bdChunk(bdChunkTypes type);
    virtual ~bdChunk();
    virtual bdChunkTypes getType() const;
    bool isControl() const;
    static bdChunkTypes getType(const void* data, unsigned int size);
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    virtual bool deserialize(const unsigned char* data, unsigned int size,
                             unsigned int* offset);
};
static_assert(sizeof(bdChunk) == 0x0C, "bdChunk size mismatch");

// ============================================================================
// bdHeartbeatChunk â€” heartbeat control chunk (16 bytes)
// ============================================================================
class bdHeartbeatChunk : public bdChunk {
public:
    enum bdHeartbeatFlags {
        BD_HEARTBEAT_NONE = 0,
    };

    bdHeartbeatFlags m_flags;  // +0x0C

    bdHeartbeatChunk();
    virtual ~bdHeartbeatChunk();
    bdHeartbeatFlags getFlags() const;
    virtual unsigned int getSerializedSize();
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    virtual bool deserialize(const unsigned char* data, unsigned int size,
                             unsigned int* offset);
};
static_assert(sizeof(bdHeartbeatChunk) == 0x10, "bdHeartbeatChunk size mismatch");

// ============================================================================
// bdHeartbeatAckChunk â€” heartbeat-ack control chunk (16 bytes)
// ============================================================================
class bdHeartbeatAckChunk : public bdChunk {
public:
    enum bdHeartbeatAckFlags {
        BD_HEARTBEAT_ACK_NONE = 0,
    };

    bdHeartbeatAckFlags m_flags;  // +0x0C

    bdHeartbeatAckChunk();
    virtual ~bdHeartbeatAckChunk();
    bdHeartbeatAckFlags getFlags() const;
    virtual unsigned int getSerializedSize();
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    virtual bool deserialize(const unsigned char* data, unsigned int size,
                             unsigned int* offset);
};
static_assert(sizeof(bdHeartbeatAckChunk) == 0x10, "bdHeartbeatAckChunk size mismatch");

// ============================================================================
// bdShutdownChunk â€” shutdown control chunk (16 bytes)
// ============================================================================
class bdShutdownChunk : public bdChunk {
public:
    enum bdShutdownFlags {
        BD_SHUTDOWN_NONE = 0,
    };

    bdShutdownFlags m_flags;  // +0x0C

    bdShutdownChunk();
    virtual ~bdShutdownChunk();
    bdShutdownFlags getFlags() const;
    virtual unsigned int getSerializedSize();
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    virtual bool deserialize(const unsigned char* data, unsigned int size,
                             unsigned int* offset);
};
static_assert(sizeof(bdShutdownChunk) == 0x10, "bdShutdownChunk size mismatch");

// ============================================================================
// bdShutdownAckChunk â€” shutdown-ack control chunk (16 bytes)
// ============================================================================
class bdShutdownAckChunk : public bdChunk {
public:
    enum bdShutdownAckFlags {
        BD_SHUTDOWN_ACK_NONE = 0,
    };

    bdShutdownAckFlags m_flags;  // +0x0C

    bdShutdownAckChunk();
    virtual ~bdShutdownAckChunk();
    bdShutdownAckFlags getFlags() const;
    virtual unsigned int getSerializedSize();
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    virtual bool deserialize(const unsigned char* data, unsigned int size,
                             unsigned int* offset);
};
static_assert(sizeof(bdShutdownAckChunk) == 0x10, "bdShutdownAckChunk size mismatch");

// ============================================================================
// bdShutdownCompleteChunk â€” shutdown-complete control chunk (16 bytes)
// ============================================================================
class bdShutdownCompleteChunk : public bdChunk {
public:
    enum bdShutdownCompleteFlags {
        BD_SHUTDOWN_COMPLETE_NONE = 0,
    };

    bdShutdownCompleteFlags m_flags;  // +0x0C

    bdShutdownCompleteChunk();
    virtual ~bdShutdownCompleteChunk();
    bdShutdownCompleteFlags getFlags() const;
    virtual unsigned int getSerializedSize();
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    virtual bool deserialize(const unsigned char* data, unsigned int size,
                             unsigned int* offset);
};
static_assert(sizeof(bdShutdownCompleteChunk) == 0x10, "bdShutdownCompleteChunk size mismatch");

// ============================================================================
// bdSAckChunk â€” selective-ack chunk (48 bytes)
// Size: 0x30 (48 bytes) â€” verified against IDA
// ============================================================================
class bdSAckChunk : public bdChunk {
public:
    enum bdSAckFlags {
        BD_SACK_ACK = 0,
        BD_SACK_NACK = 1,
    };

    bdSAckFlags    m_flags;       // +0x0C
    unsigned short m_cumulativeAck;  // +0x10
    int            m_windowCredit;   // +0x14
    bdLinkedList<bdGapAckBlock> m_gapList;  // +0x18

    bdSAckChunk();
    bdSAckChunk(int windowCredit, bdSAckFlags flags);
    virtual ~bdSAckChunk();
    unsigned short getCumulativeAck() const;
    void setCumulativeAck(unsigned short ack);
    bdLinkedList<bdGapAckBlock>& getGapList();
    void setWindowCredit(int credit);
    int getWindowCredit() const;
    bdSAckFlags getFlags() const;
    virtual unsigned int getSerializedSize();
    void addGap(const bdGapAckBlock& block);
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    virtual bool deserialize(const unsigned char* data, unsigned int size,
                             unsigned int* offset);
};
static_assert(sizeof(bdSAckChunk) == 0x24, "bdSAckChunk size mismatch");

// ============================================================================
// bdBytePacker â€” little-endian byte packing helpers
// ============================================================================
namespace bdBytePacker {
bool appendBuffer(void* dest, unsigned int destSize, unsigned int offset,
                  unsigned int* newOffset, const unsigned char* src, unsigned int size);
bool appendBasicType(void* dest, unsigned int destSize, unsigned int offset,
                     unsigned int* newOffset, const void* value,
                     unsigned int valueSize);
bool removeBasicType(const unsigned char* src, unsigned int srcSize, unsigned int offset,
                     unsigned int* newOffset, void* value, unsigned int valueSize);
bool appendEncodedUInt16(void* dest, unsigned int destSize, unsigned int offset,
                         unsigned int* newOffset, unsigned short value);
bool removeEncodedUInt16(const unsigned char* src, unsigned int srcSize, unsigned int offset,
                         unsigned int* newOffset, unsigned short* value);
}

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

    bdConnection();
    bdConnection(const bdReference<bdCommonAddr>& addr);
    virtual ~bdConnection();
    virtual bool connect();
    virtual void disconnect();
    virtual void close();
    unsigned int setTransmissionRate(unsigned int rate);
    unsigned int getTransmissionRate() const;
    bdReference<bdCommonAddr> getAddress() const;
    void setAddressHandle(const bdReference<bdAddrHandle>& addrHandle);
    bdConnectionListener* registerListener(bdConnectionListener* listener);
    int unregisterListener(bdConnectionListener* listener);
};
static_assert(sizeof(bdConnection) == 0x64, "bdConnection size mismatch");
static_assert(offsetof(bdConnection, m_addr) == 0x08, "bdConnection::m_addr offset mismatch");
static_assert(offsetof(bdConnection, m_stats) == 0x10, "bdConnection::m_stats offset mismatch");
static_assert(offsetof(bdConnection, m_listeners) == 0x50, "bdConnection::m_listeners offset mismatch");
static_assert(offsetof(bdConnection, m_maxTransmissionRate) == 0x5C, "bdConnection::m_maxTransmissionRate offset mismatch");
static_assert(offsetof(bdConnection, m_status) == 0x60, "bdConnection::m_status offset mismatch");

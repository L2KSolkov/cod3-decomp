// ============================================================================
// COD3 BD Network Types — bdReference<T>, bdBuffer, bdBitBuffer, bdMessage,
// bdConnection, bdCommonAddr, bdAddrHandle, bdConnectionStatistics
// Reconstructed from IDA local types (PDB symbol data).
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

#include "bd/bdReference/bdReferencable.h"
#include "bd/bdTiming/bdShortTimer.h"
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// Logging shim (bdLog's bdMessageProxy) + assert flag - unresolved externs,
// tolerated by /FORCE:UNRESOLVED until bdLog is ported.
// ============================================================================
struct bdMessageProxy {
    bdMessageProxy(const char* file, const char* func, unsigned int line, const char* flags);
    void log(const char* channel, const char* format, ...) const;
};

extern bool g_assertFalse;

namespace bdMemory {
void* allocate(unsigned int size);
void  deallocate(void* p);
}

// ============================================================================
// bdReference<T> — intrusive reference wrapper (4 bytes) — verified against IDA
// ============================================================================
template <typename T>
struct bdReference {
    T* m_ptr;  // +0x00

    bdReference() : m_ptr(NULL) {}
    bdReference(T* ptr) : m_ptr(ptr) {}
    template <typename U>
    bdReference(const bdReference<U>& other) : m_ptr(other.m_ptr) {
        // Converting ref (ea: 0x8A8B60) addRefs in the binary, but our bare
        // bdReference has no dtor to balance it, so the net effect of a
        // temporary here is zero refs (matching the binary's +1/-1 pair).
    }
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

    T& operator[](unsigned int index) { return m_data[index]; }
    const T& operator[](unsigned int index) const { return m_data[index]; }
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
    unsigned int getMaxReadSize() const { return m_size + (unsigned int)(m_data - m_readPtr); }
    const unsigned char* getData() const { return m_data; }
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
    unsigned int getNumBitsWritten() const { return m_writePosition; }
    unsigned int getDataSize() const { return m_data.m_size; }
    const unsigned char* getData() const { return m_data.m_data; }
    void resetReadPosition() { m_readPosition = 1; }  // COD3: header is 1 bit (ea: 0x8A410A writes 1)
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
class bdConnection;
class bdConnectionListener;

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
inline void bdListAddRef(const T&) {}

template <typename T>
inline void bdListAddRef(const bdReference<T>& value) {
    if (value.m_ptr != NULL)
        value.m_ptr->addRef();
}

template <typename T>
inline void bdListRelease(T&) {}

template <typename T>
inline void bdListRelease(bdReference<T>& value) {
    if (value.m_ptr != NULL && value.m_ptr->releaseRef() == 0)
        delete value.m_ptr;
    value.m_ptr = NULL;
}

template <typename T>
struct bdLinkedList {
    struct Node {
        T      m_value;   // +0x00
        Node*  m_next;    // +sizeof(T)
        Node*  m_prev;    // +sizeof(T) + 4
    };

    Node*        m_head;   // +0x00
    Node*        m_tail;   // +0x04
    unsigned int m_size;   // +0x08

    bdLinkedList() : m_head(NULL), m_tail(NULL), m_size(0) {}
    ~bdLinkedList() { clear(); }

    bool isEmpty() const { return m_size == 0; }
    unsigned int getSize() const { return m_size; }
    void* getHeadPosition() const { return m_head; }
    void* getTailPosition() const { return m_tail; }
    T& getHead() { return m_head->m_value; }
    T& getAt(void* pos) { return ((Node*)pos)->m_value; }
    T& forward(void*& pos) {
        Node* cur = (Node*)pos;
        pos = cur->m_next;
        return cur->m_value;
    }
    void addTail(const T& value) { insertAfter(m_tail, value); }
    void removeHead() { void* pos = m_head; removeAt(pos); }

    void insertAfter(void* pos, const T& value) {
        Node* node = (Node*)bdMemory::allocate(sizeof(Node));
        if (node != NULL) {
            node->m_value = value;
            node->m_next = NULL;
            node->m_prev = NULL;
            bdListAddRef(value);
        }
        if (pos != NULL) {
            Node* at = (Node*)pos;
            node->m_next = at->m_next;
            node->m_prev = at;
            if (at->m_next != NULL) {
                at->m_next->m_prev = node;
                at->m_next = node;
                ++m_size;
            } else {
                if (at != m_tail) {
                    bdMessageProxy proxy("..\\bdCore/bdContainers/bdLinkedList.inl",
                                         "void __thiscall bdLinkedList::insertAfter(void *const ,const class bdReference &)",
                                         0x16Bu, "dw/err");
                    proxy.log("defaultFileName",
                              "bdLinkedList::insertAfter, node has no next entry, but is not the tail.");
                }
                m_tail = node;
                at->m_next = node;
                ++m_size;
            }
        } else {
            node->m_next = NULL;
            node->m_prev = m_tail;
            if (m_tail != NULL) {
                m_tail->m_next = node;
            } else {
                m_head = node;
            }
            m_tail = node;
            ++m_size;
        }
    }

    void removeAt(void*& pos) {
        Node* node = (Node*)pos;
        if (node != NULL) {
            pos = node->m_next;
            if (node == m_head)
                m_head = m_head->m_next;
            else
                node->m_prev->m_next = node->m_next;
            if (node == m_tail)
                m_tail = node->m_prev;
            else
                node->m_next->m_prev = node->m_prev;
            bdListRelease(node->m_value);
            bdMemory::deallocate(node);
            --m_size;
        }
    }

    void clear() {
        Node* node = m_head;
        while (node != NULL) {
            Node* next = node->m_next;
            bdListRelease(node->m_value);
            bdMemory::deallocate(node);
            node = next;
        }
        m_head = NULL;
        m_tail = NULL;
        m_size = 0;
    }
};
static_assert(sizeof(bdLinkedList<char>) == 0x0C, "bdLinkedList size mismatch");

// ============================================================================
// bdQueue<T> - thin FIFO wrapper over bdLinkedList (12 bytes)
// ============================================================================
template <typename T>
struct bdQueue {
    bdLinkedList<T> m_list;   // +0x00

    bool isEmpty() const { return m_list.m_size == 0; }
    unsigned int getSize() const { return m_list.m_size; }
    T& peek() {
        if (m_list.m_size == 0) {
            bdMessageProxy proxy("..\\bdCore/bdContainers/bdQueue.inl",
                                 "class bdReference &__thiscall bdQueue::peek(void)",
                                 0x1Bu, "dw/err");
            proxy.log("defaultFileName", "bdQueue::dequeue, queue empty, can't peek.");
        }
        return m_list.getHead();
    }
    void enqueue(const T& value) { m_list.addTail(value); }
    void dequeue() {
        if (m_list.m_size == 0) {
            bdMessageProxy proxy("..\\bdCore/bdContainers/bdQueue.inl",
                                 "void __thiscall bdQueue::dequeue(void)",
                                 0x14u, "dw/err");
            proxy.log("defaultFileName", "bdQueue::dequeue, queue empty, can't dequeue.");
        }
        void* pos = m_list.m_head;
        m_list.removeAt(pos);
    }
};
static_assert(sizeof(bdQueue<char>) == 0x0C, "bdQueue size mismatch");

// ============================================================================
// bdGapAckBlock - SACK gap block value (8 bytes; list Node adds links at +8)
// ============================================================================
struct bdGapAckBlock {
    unsigned int m_start;   // +0x00 (16-bit value on the wire)
    unsigned int m_end;     // +0x04

    bdGapAckBlock() : m_start(0), m_end(0) {}
    bdGapAckBlock(unsigned int start, unsigned int end) : m_start(start), m_end(end) {}
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
    virtual unsigned int getSerializedSize();
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
// bdCookie â€” cookie with HMAC tag (24 bytes)
// ============================================================================
class bdCookie : public bdReferencable {
public:
    unsigned int m_localTag;      // +0x08
    unsigned int m_peerTag;       // +0x0C
    unsigned int m_localTieTag;   // +0x10
    unsigned int m_peerTieTag;    // +0x14

    bdCookie();
    bdCookie(unsigned int localTag, unsigned int peerTag,
             unsigned int localTieTag, unsigned int peerTieTag);
    virtual ~bdCookie();
    unsigned int getLocalTag() const;
    unsigned int getPeerTag() const;
    unsigned int getLocalTieTag() const;
    unsigned int getPeerTieTag() const;
    virtual unsigned int getSerializedSize();
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    virtual bool deserialize(const unsigned char* data, unsigned int size,
                             unsigned int* offset);

    static unsigned char m_secret[20];
    static bool          m_secretInitialized;
};
static_assert(sizeof(bdCookie) == 0x18, "bdCookie size mismatch");

// ============================================================================
// bdCookieEchoChunk â€” cookie echo chunk (32 bytes)
// ============================================================================
class bdCookieEchoChunk : public bdChunk {
public:
    enum bdCookieEchoFlags {
        BD_COOKIE_ECHO_NONE = 0,
    };

    bdCookieEchoFlags m_flags;                  // +0x0C
    bdReference<bdCookie> m_cookie;             // +0x10
    bdReference<bdByteBuffer> m_rawCookie;      // +0x14

    bdCookieEchoChunk();
    bdCookieEchoChunk(const bdReference<bdByteBuffer>& rawCookie);
    virtual ~bdCookieEchoChunk();
    bdCookieEchoFlags getFlags() const;
    virtual unsigned int getSerializedSize();
    bool getCookie(bdReference<bdCookie>& cookie) const;
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    virtual bool deserialize(const unsigned char* data, unsigned int size,
                             unsigned int* offset);
};
static_assert(sizeof(bdCookieEchoChunk) == 0x18, "bdCookieEchoChunk size mismatch");

// ============================================================================
// bdCookieAckChunk â€” cookie-ack control chunk (16 bytes)
// ============================================================================
class bdCookieAckChunk : public bdChunk {
public:
    enum bdCookieAckFlags {
        BD_COOKIE_ACK_NONE = 0,
    };

    bdCookieAckFlags m_flags;  // +0x0C

    bdCookieAckChunk();
    virtual ~bdCookieAckChunk();
    bdCookieAckFlags getFlags() const;
    virtual unsigned int getSerializedSize();
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    virtual bool deserialize(const unsigned char* data, unsigned int size,
                             unsigned int* offset);
};
static_assert(sizeof(bdCookieAckChunk) == 0x10, "bdCookieAckChunk size mismatch");

// ============================================================================
// bdInitChunk â€” connection init chunk (40 bytes)
// ============================================================================
class bdInitChunk : public bdChunk {
public:
    enum bdInitChunkFlags {
        BD_INIT_NONE = 0,
    };

    unsigned int m_initTag;       // +0x0C
    bdInitChunkFlags m_flags;     // +0x10
    bdReference<bdCookie> m_cookie;  // +0x14
    int            m_windowCredit;   // +0x18

    bdInitChunk();
    bdInitChunk(unsigned int initTag, int windowCredit);
    virtual ~bdInitChunk();
    unsigned int getInitTag() const;
    bdInitChunkFlags getFlags() const;
    int getWindowCredit() const;
    virtual unsigned int getSerializedSize();
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    virtual bool deserialize(const unsigned char* data, unsigned int size,
                             unsigned int* offset);
};
static_assert(sizeof(bdInitChunk) == 0x1C, "bdInitChunk size mismatch");

// ============================================================================
// bdInitAckChunk â€” connection init-ack chunk (40 bytes)
// ============================================================================
class bdInitAckChunk : public bdChunk {
public:
    enum bdInitAckFlags {
        BD_INIT_ACK_NONE = 0,
    };

    unsigned int m_initTag;       // +0x0C
    bdInitAckFlags m_flags;       // +0x10
    bdReference<bdByteBuffer> m_rawCookie;  // +0x14
    bdReference<bdCookie> m_cookie;  // +0x18
    int            m_windowCredit;   // +0x1C
    unsigned int   m_peerTag;        // +0x20

    bdInitAckChunk();
    bdInitAckChunk(unsigned int initTag, const bdReference<bdCookie>& cookie,
                   int windowCredit, unsigned int peerTag);
    virtual ~bdInitAckChunk();
    unsigned int getInitTag() const;
    int getWindowCredit() const;
    unsigned int getPeerTag() const;
    bool getCookie(bdReference<bdByteBuffer>& cookie) const;
    virtual unsigned int getSerializedSize();
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    virtual bool deserialize(const unsigned char* data, unsigned int size,
                             unsigned int* offset);
};
static_assert(sizeof(bdInitAckChunk) == 0x24, "bdInitAckChunk size mismatch");

// ============================================================================
// bdPacket â€” packet container (32 bytes)
// ============================================================================
class bdPacket : public bdReferencable {
public:
    struct bdChunkNode {
        bdReference<bdChunk> m_chunk;   // +0x00
        bdChunkNode* m_next;            // +0x04
    };

    bdChunkNode* m_head;          // +0x08
    bdChunkNode* m_tail;          // +0x0C
    unsigned int m_size;          // +0x10
    bdReference<bdChunk> m_nextChunk;  // +0x14
    unsigned int m_verificationTag;    // +0x18
    unsigned int m_maxSize;       // +0x1C
    unsigned int m_curSize;       // +0x20

    bdPacket();
    bdPacket(unsigned int verificationTag, unsigned int maxSize);
    virtual ~bdPacket();
    unsigned int getVerificationTag() const;
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    bool isEmpty() const;
    bool deserialize(const unsigned char* data, unsigned int size);
    bool addChunk(const bdReference<bdChunk>& chunk);
    bool getNextChunk(bdReference<bdChunk>& chunk);
};
static_assert(sizeof(bdPacket) == 0x24, "bdPacket size mismatch");

static_assert(sizeof(bdPacket) == 0x24, "bdPacket size mismatch");

// ============================================================================
// bdDataChunk - data chunk (24 bytes). Layout verified against IDA
// (bdDataChunk.obj): m_message +0x10, m_flags +0x14, m_sequenceNumber +0x16.
// ============================================================================
enum bdDataFlags {
    BD_DC_NONE = 0,
    BD_DC_UNRELIABLE = 1,
    BD_DC_ENC_DATA = 2,
    BD_DC_UNENC_DATA = 4,
};

class bdDataChunk : public bdChunk {
public:
    uint8_t _pad0C[4];                 // +0x0C (never written)
    bdReference<bdMessage> m_message;  // +0x10
    uint8_t  m_flags;                  // +0x14
    uint8_t  _pad15[1];                // +0x15
    uint16_t m_sequenceNumber;         // +0x16

    bdDataChunk();
    bdDataChunk(const bdReference<bdMessage>& message, bdDataFlags flags);
    virtual ~bdDataChunk();
    void setSequenceNumber(unsigned short sequenceNumber);
    unsigned short getSequenceNumber() const;
    uint8_t getFlags() const;
    bdReference<bdMessage> getMessage() const;
    virtual unsigned int getSerializedSize();
    unsigned int serializeUnencrypted(unsigned char* data, unsigned int size);
    virtual unsigned int serialize(unsigned char* data, unsigned int size);
    virtual bool deserialize(const unsigned char* data, unsigned int size,
                             unsigned int* offset);
    bool deserialize(const unsigned char* data, unsigned int size,
                     unsigned int* offset, const unsigned char* unencData,
                     unsigned int unencSize, unsigned int* unencReadOffset);
};
static_assert(sizeof(bdDataChunk) == 0x18, "bdDataChunk size mismatch");
static_assert(offsetof(bdDataChunk, m_message) == 0x10, "bdDataChunk::m_message offset mismatch");
static_assert(offsetof(bdDataChunk, m_flags) == 0x14, "bdDataChunk::m_flags offset mismatch");
static_assert(offsetof(bdDataChunk, m_sequenceNumber) == 0x16, "bdDataChunk::m_sequenceNumber offset mismatch");

// ============================================================================
// bdSAckChunk - selective-ack chunk (40 bytes). Layout verified against IDA:
// m_flags +0x10, m_cumulativeAck +0x14, m_gapList +0x18, m_windowCredit +0x24.
// ============================================================================
class bdSAckChunk : public bdChunk {
public:
    enum bdSAckFlags {
        BD_SACK_ACK = 0,
        BD_SACK_NACK = 1,
    };

    uint8_t _pad0C[4];                 // +0x0C (never written)
    bdSAckFlags m_flags;               // +0x10 (byte used on the wire)
    uint16_t m_cumulativeAck;          // +0x14
    uint8_t  _pad16[2];                // +0x16
    bdLinkedList<bdGapAckBlock> m_gapList;  // +0x18
    int m_windowCredit;                // +0x24

    bdSAckChunk();
    bdSAckChunk(int windowCredit, bdSAckFlags flags = BD_SACK_ACK);
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
static_assert(sizeof(bdSAckChunk) == 0x28, "bdSAckChunk size mismatch");
static_assert(offsetof(bdSAckChunk, m_flags) == 0x10, "bdSAckChunk::m_flags offset mismatch");
static_assert(offsetof(bdSAckChunk, m_cumulativeAck) == 0x14, "bdSAckChunk::m_cumulativeAck offset mismatch");
static_assert(offsetof(bdSAckChunk, m_gapList) == 0x18, "bdSAckChunk::m_gapList offset mismatch");
static_assert(offsetof(bdSAckChunk, m_windowCredit) == 0x24, "bdSAckChunk::m_windowCredit offset mismatch");

// ============================================================================
// bdSequenceNumber - RFC1982 serial-number arithmetic (4 bytes)
// ============================================================================
class bdSequenceNumber {
public:
    bdSequenceNumber(int seqNum = -1) : m_seqNum(seqNum) {}
    bdSequenceNumber(const bdSequenceNumber& last, unsigned int seqNumber,
                     unsigned int bits);
    void set(const bdSequenceNumber& last, unsigned int seqNumber,
             unsigned int bits);
    int getValue() const { return m_seqNum; }

    bdSequenceNumber operator+(const bdSequenceNumber& other) const;
    bdSequenceNumber& operator+=(const bdSequenceNumber& other);
    bdSequenceNumber& operator++();
    bdSequenceNumber operator++(int);
    bdSequenceNumber operator-(const bdSequenceNumber& other) const;
    bool operator>(const bdSequenceNumber& other) const;
    bool operator<(const bdSequenceNumber& other) const;
    bool operator<=(const bdSequenceNumber& other) const;
    bool operator>=(const bdSequenceNumber& other) const;
    bool operator==(const bdSequenceNumber& other) const;
    bool operator!=(const bdSequenceNumber& other) const;

protected:
    int m_seqNum;
};
static_assert(sizeof(bdSequenceNumber) == 4, "bdSequenceNumber size mismatch");

// ============================================================================
// bd connection window constants (bdConnectionConfig)
// ============================================================================
enum {
    BD_MAX_WINDOW_SIZE = 128,
    BD_MAX_DATAGRAM_SIZE = 1328,             // 0x530
    BD_DEFAULT_RECEIVE_WINDOW_CREDIT = 1500, // 0x5DC
    BD_FAST_RETRANSMIT_THRESH = 3,
};
static const float BD_RTT_START_VALUE = 0.3f;
static const float BD_UC_RTO_MAX = 2.0f;

// ============================================================================
// bdReliableSendWindow - outgoing reliable-message window (~1576 bytes).
// Layout verified against IDA (bdReliableSendWindow.obj ctor @0x8A8CB0):
// m_frame[128] at +0x10, credit +0x610, flight +0x614, partial +0x618,
// ssthresh +0x61C, cwnd +0x620, m_lastSent +0x624.
// ============================================================================
class bdReliableSendWindow {
public:
    class bdMessageFrame {
    public:
        bdMessageFrame()
            : m_chunk(), m_timer(), m_sendCount(0), m_missingCount(0),
              m_gapAcked(false) {}
        bdMessageFrame(const bdReference<bdDataChunk>& chunk);
        ~bdMessageFrame();
        bdMessageFrame& operator=(const bdMessageFrame& other);

        bdReference<bdDataChunk> m_chunk;   // +0x00
        bdShortTimer m_timer;               // +0x04
        uint8_t m_sendCount;                // +0x08
        uint8_t m_missingCount;             // +0x09
        bool m_gapAcked;                    // +0x0A
        uint8_t _pad0B;                     // +0x0B
    };
    static_assert(sizeof(bdMessageFrame) == 0x0C, "bdMessageFrame size mismatch");

    enum bdCongestionWindowDecreaseReason {
        BD_CWDR_PACKET_LOSS_DETECTED = 0,
        BD_CWDR_RESEND_TIMER_EXPIRED = 1,
        BD_CWDR_INACTIVE = 2,
    };

    bdReliableSendWindow();
    ~bdReliableSendWindow();
    void setTimeoutPeriod(float secs);
    float getTimeoutPeriod() const;
    bool add(const bdReference<bdDataChunk>& chunk);
    void getDataToSend(bdPacket& packet);
    bool handleAck(const bdReference<bdSAckChunk>& chunk, float& rtt);
    bool isEmpty() const;

protected:
    void increaseCongestionWindow(const bdReference<bdSAckChunk>& chunk,
                                  unsigned int bytesAcked);
    void decreaseCongestionWindow(bdCongestionWindowDecreaseReason reason);

protected:
    bdSequenceNumber m_lastAcked;              // +0x00
    bdSequenceNumber m_nextFree;               // +0x04
    float m_timeoutPeriod;                     // +0x08
    uint8_t m_retransmitCountThreshold;        // +0x0C
    uint8_t _pad0D[3];                         // +0x0D
    bdMessageFrame m_frame[BD_MAX_WINDOW_SIZE];     // +0x10
    unsigned int m_remoteReceiveWindowCredit;  // +0x610
    unsigned int m_flightSize;                 // +0x614
    unsigned int m_partialBytesAcked;          // +0x618
    unsigned int m_slowStartThresh;            // +0x61C
    unsigned int m_congestionWindow;           // +0x620
    bdShortTimer m_lastSent;                   // +0x624
};
static_assert(sizeof(bdReliableSendWindow) == 0x628, "bdReliableSendWindow size mismatch");

// ============================================================================
// bdReliableReceiveWindow - incoming reliable-message window (540 bytes).
// Layout verified against IDA (ctor @0x8A8230): m_frame[128] at +0x0C,
// m_shouldAck +0x20C, credit +0x210, used +0x214, m_sack +0x218.
// ============================================================================
class bdReliableReceiveWindow {
public:
    bdReliableReceiveWindow();
    ~bdReliableReceiveWindow();
    bool add(const bdReference<bdDataChunk>& chunk);
    void getDataToSend(bdPacket& packet);
    bdReference<bdDataChunk> getNextToRead();

protected:
    void calculateAck();

protected:
    bdSequenceNumber m_newest;                 // +0x00
    bdSequenceNumber m_lastCumulative;         // +0x04
    bdSequenceNumber m_lastDispatched;         // +0x08
    bdReference<bdDataChunk> m_frame[BD_MAX_WINDOW_SIZE];  // +0x0C
    bool m_shouldAck;                          // +0x20C
    uint8_t _pad20D[3];                        // +0x20D
    unsigned int m_recvWindowCredit;           // +0x210
    unsigned int m_recvWindowUsedCredit;       // +0x214
    bdReference<bdSAckChunk> m_sack;           // +0x218
};
static_assert(sizeof(bdReliableReceiveWindow) == 0x21C, "bdReliableReceiveWindow size mismatch");

// ============================================================================
// bdUnreliableSendWindow - outgoing unreliable queue (20 bytes).
// vtable +0, m_seqNumber +0x04 (16-bit), m_sendQueue +0x08. (ctor @0x8AA0A0)
// ============================================================================
class bdUnreliableSendWindow {
public:
    bdUnreliableSendWindow();
    virtual ~bdUnreliableSendWindow();
    void add(const bdReference<bdDataChunk>& chunk);
    void getDataToSend(bdPacket& packet);
    void reset();

protected:
    uint16_t m_seqNumber;                      // +0x04
    uint8_t _pad06[2];                         // +0x06
    bdQueue<bdReference<bdDataChunk> > m_sendQueue;  // +0x08
};
static_assert(sizeof(bdUnreliableSendWindow) == 0x14, "bdUnreliableSendWindow size mismatch");

// ============================================================================
// bdUnreliableReceiveWindow - incoming unreliable queue (20 bytes).
// vtable +0, m_seqNumber +0x04, m_recvQueue +0x08. (ctor @0x8AAD30)
// ============================================================================
class bdUnreliableReceiveWindow {
public:
    bdUnreliableReceiveWindow();
    virtual ~bdUnreliableReceiveWindow();
    bool add(const bdReference<bdDataChunk>& chunk);
    bdReference<bdDataChunk> getNextToRead();
    void reset();

protected:
    bdSequenceNumber m_seqNumber;              // +0x04
    bdQueue<bdReference<bdDataChunk> > m_recvQueue;  // +0x08
};
static_assert(sizeof(bdUnreliableReceiveWindow) == 0x14, "bdUnreliableReceiveWindow size mismatch");
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

class bdConnectionListener {
public:
    bdConnectionListener();
    virtual ~bdConnectionListener();
    virtual void onConnect(const bdReference<bdConnection>& connection);
    virtual void onConnectFailed(const bdReference<bdConnection>& connection);
    virtual void onDisconnect(const bdReference<bdConnection>& connection);
    virtual void onReconnect(const bdReference<bdConnection>& connection);
};

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

    // bdConnection.obj (methods; declared for bdSessionInfo). Virtual order
    // matches the COD3 vtable (loopback vtable @0xD52864):
    // dtor, receive, send, getMessageToDispatch, getStatus, connect,
    // disconnect, close, getDataToSend.
    const bdReference<bdAddrHandle>& getAddressHandle() const { return m_addrHandle; }
    bdConnectionStatistics* getStats() { return &m_stats; }

    bdConnection();
    bdConnection(const bdReference<bdCommonAddr>& addr);
    virtual ~bdConnection();
    virtual bool receive(const unsigned char* buffer, unsigned int bufferSize) = 0;
    virtual bool send(const bdReference<bdMessage>& message, bool reliable = false) = 0;
    virtual bool getMessageToDispatch(bdReference<bdMessage>& message) = 0;
    virtual Status getStatus() const;
    virtual bool connect();
    virtual void disconnect();
    virtual void close();
    unsigned int setTransmissionRate(unsigned int rate);
    unsigned int getTransmissionRate() const;
    bdReference<bdCommonAddr> getAddress() const;
    void setAddressHandle(const bdReference<bdAddrHandle>& addrHandle);
    bdConnectionListener* registerListener(bdConnectionListener* listener);
    int unregisterListener(bdConnectionListener* listener);

protected:
    virtual unsigned int getDataToSend(unsigned char* buffer, unsigned int bufferSize) = 0;
};
static_assert(sizeof(bdConnection) == 0x64, "bdConnection size mismatch");
static_assert(offsetof(bdConnection, m_addr) == 0x08, "bdConnection::m_addr offset mismatch");
static_assert(offsetof(bdConnection, m_stats) == 0x10, "bdConnection::m_stats offset mismatch");
static_assert(offsetof(bdConnection, m_listeners) == 0x50, "bdConnection::m_listeners offset mismatch");
static_assert(offsetof(bdConnection, m_maxTransmissionRate) == 0x5C, "bdConnection::m_maxTransmissionRate offset mismatch");
static_assert(offsetof(bdConnection, m_status) == 0x60, "bdConnection::m_status offset mismatch");

// ============================================================================
// bdLoopbackConnection - loopback connection (116 bytes).
// Layout verified against IDA (ctor @0x8A4180): m_messages queue at +0x64,
// a byte flag at +0x70 (set to 1 on send, ea 0x8A414D).
// ============================================================================
class bdLoopbackConnection : public bdConnection {
public:
    virtual ~bdLoopbackConnection();
    virtual bool receive(const unsigned char* buffer, unsigned int bufferSize);
    virtual bool send(const bdReference<bdMessage>& message, bool reliable = false);
    virtual bool getMessageToDispatch(bdReference<bdMessage>& message);

protected:
    friend class bdConnectionStore;
    bdLoopbackConnection(const bdReference<bdCommonAddr>& addr);
    virtual unsigned int getDataToSend(unsigned char* buffer, unsigned int bufferSize);
    void updateStatus();

protected:
    bdQueue<bdReference<bdMessage> > m_messages;   // +0x64
    uint8_t m_flag70;                              // +0x70 (set 1 on send)
    uint8_t _pad71[3];                             // +0x71
};
static_assert(sizeof(bdLoopbackConnection) == 0x74, "bdLoopbackConnection size mismatch");

// ============================================================================
// bdReceivedMessage - message + originating connection (8 bytes).
// Layout verified against IDA (ctor @0x8A10C0): m_message +0, m_connection +4.
// ============================================================================
class bdReceivedMessage {
public:
    bdReceivedMessage(const bdReference<bdMessage>& message,
                      const bdReference<bdConnection>& connection);
    ~bdReceivedMessage();
    bdReference<bdMessage> getMessage() const;
    bdReference<bdConnection> getConnection() const;

protected:
    bdReference<bdMessage> m_message;       // +0x00
    bdReference<bdConnection> m_connection; // +0x04
};
static_assert(sizeof(bdReceivedMessage) == 0x08, "bdReceivedMessage size mismatch");

// ============================================================================
// bdHeartbeatChunk.cpp â€” heartbeat control chunks (12 funcs, 2 units).
// Source: .\bdPacket\bdHeartbeatChunk.cpp / bdHeartbeatAckChunk.cpp
// Verified against IDA (bdConnection:bdHeartbeatChunk.obj /
// bdHeartbeatAckChunk.obj).
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// bdHeartbeatChunk::bdHeartbeatChunk â€” ea: 0x8AC040
// ============================================================================
bdHeartbeatChunk::bdHeartbeatChunk()
    : bdChunk((bdChunkTypes)6),
      m_flags(BD_HEARTBEAT_NONE) {
}

// ============================================================================
// bdHeartbeatChunk::~bdHeartbeatChunk â€” ea: 0x8AC060
// ============================================================================
bdHeartbeatChunk::~bdHeartbeatChunk() {
}

// ============================================================================
// bdHeartbeatChunk::getFlags â€” ea: 0x8AC070
// ============================================================================
bdHeartbeatChunk::bdHeartbeatFlags bdHeartbeatChunk::getFlags() const {
    return this->m_flags;
}

// ============================================================================
// bdHeartbeatChunk::getSerializedSize â€” ea: 0x8AC080
// ============================================================================
unsigned int bdHeartbeatChunk::getSerializedSize() {
    return 4;
}

// ============================================================================
// bdHeartbeatChunk::serialize â€” ea: 0x8AC0B0
// ============================================================================
unsigned int bdHeartbeatChunk::serialize(unsigned char* data, unsigned int size) {
    unsigned int v3 = size;
    unsigned int v7 = 0;
    unsigned int v5 = bdChunk::serialize(data, size);
    unsigned char flags = (unsigned char)this->m_flags;
    v7 = v5;
    if (bdBytePacker::appendBasicType(data, v3, v5, &v7, &flags, 1u)) {
        unsigned short zero = 0;
        bdBytePacker::appendBasicType(data, v3, v7, &v7, &zero, 2u);
    }
    return v7;
}

// ============================================================================
// bdHeartbeatChunk::deserialize â€” ea: 0x8AC130
// ============================================================================
bool bdHeartbeatChunk::deserialize(const unsigned char* data, unsigned int size,
                                   unsigned int* offset) {
    unsigned int v4 = size;
    unsigned int v9 = *offset;
    unsigned char v6 = 0;
    unsigned int tmp;
    bool ok = false;
    if (bdChunk::deserialize(data, size, &v9)
        && bdBytePacker::removeBasicType(data, v4, v9, &v9, &tmp, 1u)) {
        v6 = (unsigned char)tmp;
        ok = true;
    }
    this->m_flags = (bdHeartbeatFlags)v6;
    if (!ok || !bdBytePacker::removeBasicType(data, v4, v9, &v9, &tmp, 2u))
        return false;
    *offset = v9;
    return true;
}

// ============================================================================
// bdHeartbeatAckChunk::bdHeartbeatAckChunk â€” ea: 0x8AC1D0
// ============================================================================
bdHeartbeatAckChunk::bdHeartbeatAckChunk()
    : bdChunk((bdChunkTypes)7),
      m_flags(BD_HEARTBEAT_ACK_NONE) {
}

// ============================================================================
// bdHeartbeatAckChunk::~bdHeartbeatAckChunk â€” ea: 0x8AC1F0
// ============================================================================
bdHeartbeatAckChunk::~bdHeartbeatAckChunk() {
}

// ============================================================================
// bdHeartbeatAckChunk::getFlags â€” ea: 0x8AC200
// ============================================================================
bdHeartbeatAckChunk::bdHeartbeatAckFlags bdHeartbeatAckChunk::getFlags() const {
    return this->m_flags;
}

// ============================================================================
// bdHeartbeatAckChunk::getSerializedSize â€” ea: 0x8AC210
// ============================================================================
unsigned int bdHeartbeatAckChunk::getSerializedSize() {
    return 4;
}

// ============================================================================
// bdHeartbeatAckChunk::serialize â€” ea: 0x8AC240
// ============================================================================
unsigned int bdHeartbeatAckChunk::serialize(unsigned char* data, unsigned int size) {
    unsigned int v3 = size;
    unsigned int v7 = 0;
    unsigned int v5 = bdChunk::serialize(data, size);
    unsigned char flags = (unsigned char)this->m_flags;
    v7 = v5;
    if (bdBytePacker::appendBasicType(data, v3, v5, &v7, &flags, 1u)) {
        unsigned short zero = 0;
        bdBytePacker::appendBasicType(data, v3, v7, &v7, &zero, 2u);
    }
    return v7;
}

// ============================================================================
// bdHeartbeatAckChunk::deserialize â€” ea: 0x8AC2C0
// ============================================================================
bool bdHeartbeatAckChunk::deserialize(const unsigned char* data, unsigned int size,
                                      unsigned int* offset) {
    unsigned int v4 = size;
    unsigned int v9 = *offset;
    unsigned char v6 = 0;
    unsigned int tmp;
    bool ok = false;
    if (bdChunk::deserialize(data, size, &v9)
        && bdBytePacker::removeBasicType(data, v4, v9, &v9, &tmp, 1u)) {
        v6 = (unsigned char)tmp;
        ok = true;
    }
    this->m_flags = (bdHeartbeatAckFlags)v6;
    if (!ok || !bdBytePacker::removeBasicType(data, v4, v9, &v9, &tmp, 2u))
        return false;
    *offset = v9;
    return true;
}

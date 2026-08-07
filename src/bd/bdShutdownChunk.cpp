// ============================================================================
// bdShutdownChunk.cpp â€” shutdown control chunks (18 funcs, 3 units).
// Source: .\bdPacket\bdShutdownChunk.cpp / bdShutdownAckChunk.cpp /
//         bdShutdownCompleteChunk.cpp
// Verified against IDA (bdConnection:bdShutdown*Chunk.obj).
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// bdShutdownChunk::bdShutdownChunk â€” ea: 0x8AC360
// ============================================================================
bdShutdownChunk::bdShutdownChunk()
    : bdChunk((bdChunkTypes)9),
      m_flags(BD_SHUTDOWN_NONE) {
}

// ============================================================================
// bdShutdownChunk::~bdShutdownChunk â€” ea: 0x8AC380
// ============================================================================
bdShutdownChunk::~bdShutdownChunk() {
}

// ============================================================================
// bdShutdownChunk::getFlags â€” ea: 0x8AC390
// ============================================================================
bdShutdownChunk::bdShutdownFlags bdShutdownChunk::getFlags() const {
    return this->m_flags;
}

// ============================================================================
// bdShutdownChunk::getSerializedSize â€” ea: 0x8AC3A0
// ============================================================================
unsigned int bdShutdownChunk::getSerializedSize() {
    return 4;
}

// ============================================================================
// bdShutdownChunk::serialize â€” ea: 0x8AC3D0
// ============================================================================
unsigned int bdShutdownChunk::serialize(unsigned char* data, unsigned int size) {
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
// bdShutdownChunk::deserialize â€” ea: 0x8AC450
// ============================================================================
bool bdShutdownChunk::deserialize(const unsigned char* data, unsigned int size,
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
    this->m_flags = (bdShutdownFlags)v6;
    if (!ok || !bdBytePacker::removeBasicType(data, v4, v9, &v9, &tmp, 2u))
        return false;
    *offset = v9;
    return true;
}

// ============================================================================
// bdShutdownAckChunk::bdShutdownAckChunk â€” ea: 0x8AC4F0
// ============================================================================
bdShutdownAckChunk::bdShutdownAckChunk()
    : bdChunk((bdChunkTypes)10),
      m_flags(BD_SHUTDOWN_ACK_NONE) {
}

// ============================================================================
// bdShutdownAckChunk::~bdShutdownAckChunk â€” ea: 0x8AC510
// ============================================================================
bdShutdownAckChunk::~bdShutdownAckChunk() {
}

// ============================================================================
// bdShutdownAckChunk::getFlags â€” ea: 0x8AC520
// ============================================================================
bdShutdownAckChunk::bdShutdownAckFlags bdShutdownAckChunk::getFlags() const {
    return this->m_flags;
}

// ============================================================================
// bdShutdownAckChunk::getSerializedSize â€” ea: 0x8AC530
// ============================================================================
unsigned int bdShutdownAckChunk::getSerializedSize() {
    return 4;
}

// ============================================================================
// bdShutdownAckChunk::serialize â€” ea: 0x8AC560
// ============================================================================
unsigned int bdShutdownAckChunk::serialize(unsigned char* data, unsigned int size) {
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
// bdShutdownAckChunk::deserialize â€” ea: 0x8AC5E0
// ============================================================================
bool bdShutdownAckChunk::deserialize(const unsigned char* data, unsigned int size,
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
    this->m_flags = (bdShutdownAckFlags)v6;
    if (!ok || !bdBytePacker::removeBasicType(data, v4, v9, &v9, &tmp, 2u))
        return false;
    *offset = v9;
    return true;
}

// ============================================================================
// bdShutdownCompleteChunk::bdShutdownCompleteChunk â€” ea: 0x8AC680
// ============================================================================
bdShutdownCompleteChunk::bdShutdownCompleteChunk()
    : bdChunk((bdChunkTypes)11),
      m_flags(BD_SHUTDOWN_COMPLETE_NONE) {
}

// ============================================================================
// bdShutdownCompleteChunk::~bdShutdownCompleteChunk â€” ea: 0x8AC6A0
// ============================================================================
bdShutdownCompleteChunk::~bdShutdownCompleteChunk() {
}

// ============================================================================
// bdShutdownCompleteChunk::getFlags â€” ea: 0x8AC6B0
// ============================================================================
bdShutdownCompleteChunk::bdShutdownCompleteFlags bdShutdownCompleteChunk::getFlags() const {
    return this->m_flags;
}

// ============================================================================
// bdShutdownCompleteChunk::getSerializedSize â€” ea: 0x8AC6C0
// ============================================================================
unsigned int bdShutdownCompleteChunk::getSerializedSize() {
    return 4;
}

// ============================================================================
// bdShutdownCompleteChunk::serialize â€” ea: 0x8AC6F0
// ============================================================================
unsigned int bdShutdownCompleteChunk::serialize(unsigned char* data, unsigned int size) {
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
// bdShutdownCompleteChunk::deserialize â€” ea: 0x8AC770
// ============================================================================
bool bdShutdownCompleteChunk::deserialize(const unsigned char* data, unsigned int size,
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
    this->m_flags = (bdShutdownCompleteFlags)v6;
    if (!ok || !bdBytePacker::removeBasicType(data, v4, v9, &v9, &tmp, 2u))
        return false;
    *offset = v9;
    return true;
}

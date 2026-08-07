// ============================================================================
// bdChunk.cpp â€” packet chunk base (7 funcs).
// Source: bdConnection:bdChunk.obj
// Verified against IDA (bdConnection:bdChunk.obj).
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// bdChunk::bdChunk â€” ea: 0x8AD510
// ============================================================================
bdChunk::bdChunk(bdChunkTypes type)
    : m_type(type) {
}

// ============================================================================
// bdChunk::~bdChunk â€” ea: 0x8AD530
// ============================================================================
bdChunk::~bdChunk() {
}

// ============================================================================
// bdChunk::getType â€” ea: 0x8AD540
// ============================================================================
bdChunkTypes bdChunk::getType() const {
    return this->m_type;
}

// ============================================================================
// bdChunk::isControl â€” ea: 0x8AD550
// ============================================================================
bool bdChunk::isControl() const {
    return this->m_type != BD_CHUNK_DATA;
}

// ============================================================================
// bdChunk::getSerializedSize â€” default 0 (overridden by derived chunks)
// ============================================================================
unsigned int bdChunk::getSerializedSize() {
    return 0;
}

// ============================================================================
// bdChunk::getType (static) â€” ea: 0x8AD580
// ============================================================================
bdChunkTypes bdChunk::getType(const void* data, unsigned int size) {
    unsigned int offset = 0;
    unsigned char type = 0;
    if (!bdBytePacker::removeBasicType((const unsigned char*)data, size, 0, &offset,
                                       &type, 1u))
        return (bdChunkTypes)0;
    return (bdChunkTypes)type;
}

// ============================================================================
// bdChunk::serialize â€” ea: 0x8AD5C0
// ============================================================================
unsigned int bdChunk::serialize(unsigned char* data, unsigned int size) {
    unsigned int result = 0;
    unsigned int newOffset = 0;
    if (size >= 4) {
        unsigned char type = (unsigned char)this->m_type;
        bdBytePacker::appendBasicType(data, size, 0, &newOffset, &type, 1u);
        return newOffset;
    }
    return result;
}

// ============================================================================
// bdChunk::deserialize â€” ea: 0x8AD600
// ============================================================================
bool bdChunk::deserialize(const unsigned char* data, unsigned int size,
                          unsigned int* offset) {
    unsigned char type = 0;
    if (size - *offset < 4 ||
        !bdBytePacker::removeBasicType(data, size, *offset, offset,
                                       &type, 1u))
        return false;
    this->m_type = (bdChunkTypes)type;
    return true;
}

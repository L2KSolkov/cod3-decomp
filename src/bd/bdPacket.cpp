// ============================================================================
// bdPacket.cpp â€” packet container (9 funcs).
// Source: .\bdPacket\bdPacket.cpp (bdConnection)
// Verified against IDA (bdConnection:bdPacket.obj).
// ============================================================================

#include "bd/bd_types.h"

#include <new>

// ============================================================================
// Cross-object externs
// ============================================================================
namespace bdMemory {
void* allocate(unsigned int size);
}

// ============================================================================
// bdPacket::getVerificationTag â€” ea: 0x8AC810
// ============================================================================
unsigned int bdPacket::getVerificationTag() const {
    return this->m_verificationTag;
}

// ============================================================================
// bdPacket::bdPacket (default) â€” ea: 0x8AD3B0
// ============================================================================
bdPacket::bdPacket()
    : m_head(NULL),
      m_tail(NULL),
      m_size(0),
      m_nextChunk(),
      m_verificationTag(0),
      m_maxSize(0),
      m_curSize(0) {
}

// ============================================================================
// bdPacket::bdPacket (tag, max) â€” ea: 0x8AD3D0
// ============================================================================
bdPacket::bdPacket(unsigned int verificationTag, unsigned int maxSize)
    : m_head(NULL),
      m_tail(NULL),
      m_size(0),
      m_nextChunk(),
      m_verificationTag(verificationTag),
      m_maxSize(maxSize - 6),
      m_curSize(6) {
}

// ============================================================================
// bdPacket::~bdPacket â€” ea: 0x8AD410
// ============================================================================
bdPacket::~bdPacket() {
    if (this->m_nextChunk.m_ptr != NULL && this->m_nextChunk.m_ptr->releaseRef() == 0)
        delete this->m_nextChunk.m_ptr;
    bdChunkNode* node = this->m_head;
    while (node != NULL) {
        bdChunkNode* next = node->m_next;
        if (node->m_chunk.m_ptr != NULL && node->m_chunk.m_ptr->releaseRef() == 0)
            delete node->m_chunk.m_ptr;
        delete node;
        node = next;
    }
    this->m_head = NULL;
    this->m_tail = NULL;
    this->m_size = 0;
}

// ============================================================================
// bdPacket::serialize â€” ea: 0x8AC9B0
// ============================================================================
unsigned int bdPacket::serialize(unsigned char* data, unsigned int size) {
    unsigned int v27 = 2;
    bool v33 = bdBytePacker::appendBasicType(data, size, 2u, &v27,
                                             &this->m_verificationTag, 4u);
    unsigned int v28 = v27 - 2;
    unsigned int v7 = size - v27;
    bdChunkNode* v6 = this->m_head;
    while (v6 != NULL) {
        if (!v33)
            break;
        bdReference<bdChunk> chunk;
        chunk.m_ptr = v6->m_chunk.m_ptr;
        if (chunk.m_ptr != NULL)
            chunk.m_ptr->addRef();
        v6 = v6->m_next;
        unsigned int v11 = chunk.m_ptr->getSerializedSize();
        if (v7 < v11) {
            v33 = false;
            bdMessageProxy proxy(".\\bdPacket\\bdPacket.cpp",
                                 "unsigned int __thiscall bdPacket::serialize(unsigned char *,const unsigned int)",
                                 0x51u, "dw/warn/");
            proxy.log("bdConnection/packet", "Buffer not big enough for all chunks.");
        } else {
            v7 -= v11;
            unsigned int v12 = chunk.m_ptr->serialize(&data[v27], size - v27);
            v28 += v12;
            v27 += v12;
        }
        if (chunk.m_ptr != NULL && chunk.m_ptr->releaseRef() == 0)
            delete chunk.m_ptr;
    }
    // unencrypted payload pass (bdDataChunk chunks only)
    bdChunkNode* v15 = this->m_head;
    while (v15 != NULL) {
        if (!v33)
            break;
        bdReference<bdChunk> chunk;
        chunk.m_ptr = v15->m_chunk.m_ptr;
        if (chunk.m_ptr != NULL)
            chunk.m_ptr->addRef();
        v15 = v15->m_next;
        if (chunk.m_ptr->getType() == BD_CHUNK_DATA) {
            bdDataChunk* dc = (bdDataChunk*)chunk.m_ptr;
            unsigned int v20 = dc->serializeUnencrypted(&data[v27], size - v27);
            v27 += v20;
        }
        if (chunk.m_ptr != NULL && chunk.m_ptr->releaseRef() == 0)
            delete chunk.m_ptr;
    }
    if (!v33)
        return 0;
    if ((unsigned short)v28 != (unsigned int)v28)
        return 0;
    *(unsigned short*)data = (unsigned short)v28;
    return v27;
}

// ============================================================================
// bdPacket::isEmpty â€” ea: 0x8ACC40
// ============================================================================
bool bdPacket::isEmpty() const {
    return this->m_size == 0;
}

// ============================================================================
// bdPacket::deserialize â€” ea: 0x8ACE20
// ============================================================================
bool bdPacket::deserialize(const unsigned char* data, unsigned int size) {
    bool valid = data != NULL && size > 6;
    unsigned int offset = 0;
    unsigned short tagSize = 0;
    unsigned int v32 = 0;
    bool result = false;
    if (valid && bdBytePacker::removeBasicType(data, size, 0, &offset, &tagSize, 2u)) {
        this->m_verificationTag = tagSize;
        result = true;
    }
    if (result) {
        const unsigned char* chunkData = data + offset;
        unsigned int remaining = size - offset;
        while (remaining > 0 && offset < size) {
            unsigned char type = (unsigned char)bdChunk::getType(chunkData, remaining);
            bdChunk* chunk = NULL;
            switch (type) {
            case 2:
                chunk = new (bdMemory::allocate(0x18u)) bdDataChunk();
                break;
            case 3:
                chunk = new (bdMemory::allocate(0x28u)) bdInitChunk();
                break;
            case 4:
                chunk = new (bdMemory::allocate(0x28u)) bdInitAckChunk();
                break;
            case 5:
                chunk = new (bdMemory::allocate(0x28u)) bdSAckChunk();
                break;
            case 6:
                chunk = new (bdMemory::allocate(0x14u)) bdHeartbeatChunk();
                break;
            case 7:
                chunk = new (bdMemory::allocate(0x14u)) bdHeartbeatAckChunk();
                break;
            case 9:
                chunk = new (bdMemory::allocate(0x14u)) bdShutdownChunk();
                break;
            case 10:
                chunk = new (bdMemory::allocate(0x14u)) bdShutdownAckChunk();
                break;
            case 11:
                chunk = new (bdMemory::allocate(0x14u)) bdShutdownCompleteChunk();
                break;
            case 13:
                chunk = new (bdMemory::allocate(0x1Cu)) bdCookieEchoChunk();
                break;
            case 14:
                chunk = new (bdMemory::allocate(0x14u)) bdCookieAckChunk();
                break;
            default:
                bdMessageProxy proxy(".\\bdPacket\\bdPacket.cpp",
                                     "bool __thiscall bdPacket::deserialize(const unsigned char *,const unsigned int)",
                                     0xDEu, "dw/warn/");
                proxy.log("bdConnection/packet", "unknown chunk type.");
                return false;
            }
            if (chunk == NULL)
                return false;
            chunk->addRef();
            bool ok = false;
            if (type == 2) {
                bdDataChunk* dc = (bdDataChunk*)chunk;
                ok = dc->deserialize(data + offset, size - offset, &offset);
            } else {
                ok = chunk->deserialize(data + offset, size - offset, &offset);
            }
            if (ok) {
                this->addChunk(bdReference<bdChunk>(chunk));
            } else {
                bdMessageProxy proxy(".\\bdPacket\\bdPacket.cpp",
                                     "bool __thiscall bdPacket::deserialize(const unsigned char *,const unsigned int)",
                                     0xF4u, "dw/warn/");
                proxy.log("bdConnection/packet", "Chunk deserialization failed.");
            }
            if (chunk->releaseRef() == 0)
                delete chunk;
            remaining = size - offset;
        }
    }
    return result;
}

// ============================================================================
// bdPacket::addChunk â€” ea: 0x8AD2E0
// ============================================================================
bool bdPacket::addChunk(const bdReference<bdChunk>& chunk) {
    unsigned int v5 = chunk.m_ptr->getSerializedSize();
    if (chunk.m_ptr->isControl()) {
        if (v5 <= 0x7F)
            ++v5;
        else
            v5 += 4;
    }
    if (v5 + this->m_curSize < this->m_maxSize) {
        bdChunkNode* node = new bdChunkNode();
        node->m_chunk.m_ptr = chunk.m_ptr;
        if (chunk.m_ptr != NULL)
            chunk.m_ptr->addRef();
        node->m_next = NULL;
        if (this->m_tail != NULL)
            this->m_tail->m_next = node;
        else
            this->m_head = node;
        this->m_tail = node;
        ++this->m_size;
        this->m_curSize += v5;
        return true;
    }
    return false;
}

// ============================================================================
// bdPacket::getNextChunk â€” ea: 0x8AD490
// ============================================================================
bool bdPacket::getNextChunk(bdReference<bdChunk>& chunk) {
    if (this->m_head != NULL) {
        bdChunkNode* node = this->m_head;
        if (chunk.m_ptr != NULL && chunk.m_ptr->releaseRef() == 0)
            delete chunk.m_ptr;
        chunk.m_ptr = node->m_chunk.m_ptr;
        if (chunk.m_ptr != NULL)
            chunk.m_ptr->addRef();
        this->m_head = node->m_next;
        if (this->m_head == NULL)
            this->m_tail = NULL;
        --this->m_size;
        delete node;
        return true;
    }
    return false;
}

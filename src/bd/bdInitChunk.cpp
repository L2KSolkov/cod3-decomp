// ============================================================================
// bdInitChunk.cpp â€” connection init chunks (19 funcs, 2 units).
// Source: .\bdPacket\bdInitChunk.cpp / bdInitAckChunk.cpp
// Verified against IDA (bdConnection:bdInitChunk.obj /
// bdInitAckChunk.obj).
// ============================================================================

#include "bd/bd_types.h"

#include <new>

// ============================================================================
// Cross-object externs
// ============================================================================
namespace bdMemory {
void* allocate(unsigned int size);
}

namespace bdBytePacker {
bool removeBuffer(const unsigned char* src, unsigned int srcSize, unsigned int offset,
                  unsigned int* newOffset, unsigned char* out, unsigned int size);
}

// ============================================================================
// bdInitChunk::getInitTag â€” ea: 0x8AAD80
// ============================================================================
unsigned int bdInitChunk::getInitTag() const {
    return this->m_initTag;
}

// ============================================================================
// bdInitChunk::getFlags â€” ea: 0x8AAD90
// ============================================================================
bdInitChunk::bdInitChunkFlags bdInitChunk::getFlags() const {
    return this->m_flags;
}

// ============================================================================
// bdInitChunk::getWindowCredit â€” ea: 0x8AADA0
// ============================================================================
int bdInitChunk::getWindowCredit() const {
    return this->m_windowCredit;
}

// ============================================================================
// bdInitChunk::getSerializedSize â€” ea: 0x8AADB0
// ============================================================================
unsigned int bdInitChunk::getSerializedSize() {
    return 8;
}

// ============================================================================
// bdInitChunk::bdInitChunk (default) â€” ea: 0x8AADC0
// ============================================================================
bdInitChunk::bdInitChunk()
    : bdChunk((bdChunkTypes)3),
      m_initTag(0),
      m_flags(BD_INIT_NONE),
      m_cookie(),
      m_windowCredit(0) {
}

// ============================================================================
// bdInitChunk::bdInitChunk (tag, credit) â€” ea: 0x8AADF0
// ============================================================================
bdInitChunk::bdInitChunk(unsigned int initTag, int windowCredit)
    : bdChunk((bdChunkTypes)3),
      m_initTag(initTag),
      m_flags(BD_INIT_NONE),
      m_cookie(),
      m_windowCredit(windowCredit) {
}

// ============================================================================
// bdInitChunk::~bdInitChunk â€” ea: 0x8AAE20
// ============================================================================
bdInitChunk::~bdInitChunk() {
    if (this->m_cookie.m_ptr != NULL && this->m_cookie.m_ptr->releaseRef() == 0) {
        delete this->m_cookie.m_ptr;
        this->m_cookie.m_ptr = NULL;
    }
}

// ============================================================================
// bdInitChunk::serialize â€” ea: 0x8AAEC0
// ============================================================================
unsigned int bdInitChunk::serialize(unsigned char* data, unsigned int size) {
    unsigned int v3 = size;
    unsigned int result = 0;
    unsigned int v7 = 0;
    if (size >= 0x2E) {
        unsigned int v6 = bdChunk::serialize(data, size);
        unsigned char flags = (unsigned char)this->m_flags;
        v7 = v6;
        if (bdBytePacker::appendBasicType(data, v3, v6, &v7, &flags, 1u)) {
            unsigned short zero = 0;
            if (bdBytePacker::appendBasicType(data, v3, v7, &v7, &zero, 2u)) {
                unsigned int tag = this->m_initTag;
                bdBytePacker::appendBasicType(data, v3, v7, &v7, &tag, 4u);
            }
        }
        return v7;
    }
    return result;
}

// ============================================================================
// bdInitChunk::deserialize â€” ea: 0x8AAF60
// ============================================================================
bool bdInitChunk::deserialize(const unsigned char* data, unsigned int size,
                              unsigned int* offset) {
    unsigned int v4 = size;
    unsigned int v6 = size - *offset;
    bool result = true;
    unsigned int v10 = *offset;
    if (v6 <= 4)
        goto done;
    unsigned char v8 = 0;
    unsigned int tmp;
    bool ok = false;
    if (bdChunk::deserialize(data, size, &v10)
        && bdBytePacker::removeBasicType(data, v4, v10, &v10, &tmp, 1u)) {
        v8 = (unsigned char)tmp;
        ok = true;
    }
    this->m_flags = (bdInitChunkFlags)v8;
    if (ok
        && bdBytePacker::removeBasicType(data, v4, v10, &v10, &tmp, 2u)
        && bdBytePacker::removeBasicType(data, v4, v10, &v10, &tmp, 4u)) {
        this->m_initTag = tmp;
        result = true;
    done:
        *offset = v10;
        return result;
    }
    return false;
}

// ============================================================================
// bdInitAckChunk::getInitTag â€” ea: 0x8AB040
// ============================================================================
unsigned int bdInitAckChunk::getInitTag() const {
    return this->m_initTag;
}

// ============================================================================
// bdInitAckChunk::getWindowCredit â€” ea: 0x8AB050
// ============================================================================
int bdInitAckChunk::getWindowCredit() const {
    return this->m_windowCredit;
}

// ============================================================================
// bdInitAckChunk::getPeerTag â€” ea: 0x8AB060
// ============================================================================
unsigned int bdInitAckChunk::getPeerTag() const {
    return this->m_peerTag;
}

// ============================================================================
// bdInitAckChunk::bdInitAckChunk (default) â€” ea: 0x8AB0D0
// ============================================================================
bdInitAckChunk::bdInitAckChunk()
    : bdChunk((bdChunkTypes)4),
      m_initTag(0),
      m_flags(BD_INIT_ACK_NONE),
      m_rawCookie(),
      m_cookie(),
      m_windowCredit(0),
      m_peerTag(0) {
}

// ============================================================================
// bdInitAckChunk::bdInitAckChunk (full) â€” ea: 0x8AB100
// ============================================================================
bdInitAckChunk::bdInitAckChunk(unsigned int initTag, const bdReference<bdCookie>& cookie,
                               int windowCredit, unsigned int peerTag)
    : bdChunk((bdChunkTypes)4),
      m_initTag(initTag),
      m_flags(BD_INIT_ACK_NONE),
      m_rawCookie(),
      m_cookie(cookie),
      m_windowCredit(windowCredit),
      m_peerTag(peerTag) {
    if (cookie.m_ptr != NULL)
        cookie.m_ptr->addRef();
    if (cookie.m_ptr != NULL && cookie.m_ptr->releaseRef() == 0)
        delete cookie.m_ptr;
}

// ============================================================================
// bdInitAckChunk::~bdInitAckChunk â€” ea: 0x8AB190
// ============================================================================
bdInitAckChunk::~bdInitAckChunk() {
    if (this->m_rawCookie.m_ptr != NULL && this->m_rawCookie.m_ptr->releaseRef() == 0) {
        delete this->m_rawCookie.m_ptr;
        this->m_rawCookie.m_ptr = NULL;
    }
    if (this->m_cookie.m_ptr != NULL && this->m_cookie.m_ptr->releaseRef() == 0) {
        delete this->m_cookie.m_ptr;
        this->m_cookie.m_ptr = NULL;
    }
}

// ============================================================================
// bdInitAckChunk::getCookie â€” ea: 0x8AB230
// ============================================================================
bool bdInitAckChunk::getCookie(bdReference<bdByteBuffer>& cookie) const {
    if (this->m_rawCookie.m_ptr != NULL && &this->m_rawCookie != &cookie) {
        if (cookie.m_ptr != NULL && cookie.m_ptr->releaseRef() == 0)
            delete cookie.m_ptr;
        cookie.m_ptr = this->m_rawCookie.m_ptr;
        if (cookie.m_ptr != NULL)
            cookie.m_ptr->addRef();
    }
    return this->m_rawCookie.m_ptr != NULL;
}

// ============================================================================
// bdInitAckChunk::getSerializedSize â€” ea: 0x8AB280
// ============================================================================
unsigned int bdInitAckChunk::getSerializedSize() {
    return this->m_cookie.m_ptr->getSerializedSize() + 8;
}

// ============================================================================
// bdInitAckChunk::serialize â€” ea: 0x8AB2B0
// ============================================================================
unsigned int bdInitAckChunk::serialize(unsigned char* data, unsigned int size) {
    unsigned char* result = NULL;
    if (this->m_cookie.m_ptr != NULL) {
        unsigned int v6 = size;
        if (size >= 4) {
            unsigned char* v7 = data;
            unsigned int v8 = bdChunk::serialize(data, size);
            unsigned char flags = (unsigned char)this->m_flags;
            unsigned int v14 = v8;
            bool appended = bdBytePacker::appendBasicType(v7, v6, v8, &v14, &flags, 1u);
            unsigned int v10 = v14 + 2;
            v14 += 2;
            bool v12 = false;
            if (appended) {
                unsigned int tag = this->m_initTag;
                if (!bdBytePacker::appendBasicType(v7, v6, v10, &v14, &tag, 4u))
                    v12 = true;
            }
            unsigned int v13 = this->m_cookie.m_ptr->serialize(&v7[v14], v6 - v14);
            v14 += v13;
            if (v12) {
                bdBytePacker::appendBasicType(v7, v6, v10, &v14, &v13, 2u);
            }
            return v14;
        }
    }
    return (unsigned int)result;
}

// ============================================================================
// bdInitAckChunk::deserialize â€” ea: 0x8AB3A0
// ============================================================================
bool bdInitAckChunk::deserialize(const unsigned char* data, unsigned int size,
                                 unsigned int* offset) {
    unsigned int v5 = size;
    unsigned int v6 = size - *offset;
    bool result = true;
    unsigned int v17 = *offset;
    if (v6 <= 4)
        goto done;
    unsigned char v9 = 0;
    unsigned int tmp;
    bool ok = false;
    if (bdChunk::deserialize(data, size, &v17)
        && bdBytePacker::removeBasicType(data, v5, v17, &v17, &tmp, 1u)) {
        v9 = (unsigned char)tmp;
        ok = true;
    }
    this->m_flags = (bdInitAckFlags)v9;
    unsigned short v20 = 0;
    if (ok
        && bdBytePacker::removeBasicType(data, v5, v17, &v17, &tmp, 2u)
        && (v20 = (unsigned short)tmp,
            bdBytePacker::removeBasicType(data, v5, v17, &v17, &tmp, 4u))) {
        this->m_initTag = tmp;
        ok = true;
    } else {
        ok = false;
    }
    bdByteBuffer* v13 = (bdByteBuffer*)bdMemory::allocate(0x18u);
    bdByteBuffer* v12 = v13 != NULL ? new (v13) bdByteBuffer(v20) : NULL;
    if (this->m_rawCookie.m_ptr != NULL && this->m_rawCookie.m_ptr->releaseRef() == 0)
        delete this->m_rawCookie.m_ptr;
    this->m_rawCookie.m_ptr = v12;
    if (v12 != NULL)
        v12->addRef();
    if (ok && bdBytePacker::removeBuffer(data, v5, v17, &v17,
                                         this->m_rawCookie.m_ptr->m_data, v20)) {
        result = true;
    done:
        *offset = v17;
        return result;
    }
    return false;
}

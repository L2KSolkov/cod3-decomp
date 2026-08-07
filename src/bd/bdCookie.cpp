// ============================================================================
// bdCookie.cpp â€” cookie + cookie echo/ack chunks (24 funcs, 3 units).
// Source: .\bdPacket\bdCookie.cpp / bdCookieEchoChunk.cpp /
//         bdCookieAckChunk.cpp
// Verified against IDA (bdConnection:bdCookie.obj /
// bdCookieEchoChunk.obj / bdCookieAckChunk.obj).
// ============================================================================

#include "bd/bd_types.h"

#include <new>
#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
namespace bdMemory {
void* allocate(unsigned int size);
}

struct bdHMacSHA1 {
    bdHMacSHA1(const unsigned char* key, unsigned int keyLen);
    bool process(const unsigned char* data, unsigned int len);
    bool getData(unsigned char* out, unsigned int* outLen);
    ~bdHMacSHA1();
};

struct bdTrulyRandomImpl {
    void getRandomUByte8(unsigned char* out, int count);
};

struct bdMessageProxy {
    bdMessageProxy(const char* file, const char* func, unsigned int line, const char* flags);
    void log(const char* channel, const char* format, ...) const;
};

template <typename T>
struct bdSingleton {
    static T* getInstance();
};

extern bool g_assertFalse;

// ============================================================================
// bdCookie statics
// ============================================================================
unsigned char bdCookie::m_secret[20] = {0};
bool          bdCookie::m_secretInitialized = false;

// ============================================================================
// bdCookie::bdCookie (default) â€” ea: 0x8AB540
// ============================================================================
bdCookie::bdCookie()
    : m_localTag(0),
      m_peerTag(0),
      m_localTieTag(0),
      m_peerTieTag(0) {
}

// ============================================================================
// bdCookie::bdCookie (tags) â€” ea: 0x8AB910
// ============================================================================
bdCookie::bdCookie(unsigned int localTag, unsigned int peerTag,
                   unsigned int localTieTag, unsigned int peerTieTag)
    : m_localTag(localTag),
      m_peerTag(peerTag),
      m_localTieTag(localTieTag),
      m_peerTieTag(peerTieTag) {
    if (!bdCookie::m_secretInitialized) {
        bdTrulyRandomImpl* Instance = bdSingleton<bdTrulyRandomImpl>::getInstance();
        Instance->getRandomUByte8(bdCookie::m_secret, 20);
        bdCookie::m_secretInitialized = true;
    }
}

// ============================================================================
// bdCookie::~bdCookie â€” ea: 0x8AB560
// ============================================================================
bdCookie::~bdCookie() {
}

// ============================================================================
// bdCookie::getLocalTag â€” ea: 0x8AB570
// ============================================================================
unsigned int bdCookie::getLocalTag() const {
    return this->m_localTag;
}

// ============================================================================
// bdCookie::getPeerTag â€” ea: 0x8AB580
// ============================================================================
unsigned int bdCookie::getPeerTag() const {
    return this->m_peerTag;
}

// ============================================================================
// bdCookie::getLocalTieTag â€” ea: 0x8AB590
// ============================================================================
unsigned int bdCookie::getLocalTieTag() const {
    return this->m_localTieTag;
}

// ============================================================================
// bdCookie::getPeerTieTag â€” ea: 0x8AB5A0
// ============================================================================
unsigned int bdCookie::getPeerTieTag() const {
    return this->m_peerTieTag;
}

// ============================================================================
// bdCookie::serialize â€” ea: 0x8AB5D0
// ============================================================================
unsigned int bdCookie::serialize(unsigned char* data, unsigned int size) {
    unsigned int v3 = size;
    unsigned char* v4 = data;
    unsigned int v7 = 20;
    unsigned int v8 = 0;
    if (bdBytePacker::appendBasicType(data, size, 0x14u, &v7, &v8, 4u)) {
        unsigned int tag = this->m_localTag;
        if (bdBytePacker::appendBasicType(v4, v3, v7, &v7, &tag, 4u)) {
            tag = this->m_peerTag;
            if (bdBytePacker::appendBasicType(v4, v3, v7, &v7, &tag, 4u)) {
                tag = this->m_peerTieTag;
                if (bdBytePacker::appendBasicType(v4, v3, v7, &v7, &tag, 4u)) {
                    tag = this->m_localTieTag;
                    bdBytePacker::appendBasicType(v4, v3, v7, &v7, &tag, 4u);
                }
            }
        }
    }
    if (v4 != NULL) {
        bdHMacSHA1 hmac(bdCookie::m_secret, 0x14u);
        hmac.process(v4 + 20, v7 - 20);
        unsigned int len = 20;
        hmac.getData(v4, &len);
    }
    return v7;
}

// ============================================================================
// bdCookie::deserialize â€” ea: 0x8AB730
// ============================================================================
bool bdCookie::deserialize(const unsigned char* data, unsigned int size,
                           unsigned int* offset) {
    unsigned int v4 = *offset;
    unsigned int v5 = size;
    unsigned int v6 = size - *offset;
    bool v11 = true;
    if (v6 <= 0x14)
        return true;
    const unsigned char* v7 = data;
    const unsigned char* v8 = data + v4;
    bdHMacSHA1 hmac(bdCookie::m_secret, 0x14u);
    hmac.process(v8 + 20, v6 - 20);
    unsigned char v16[20];
    unsigned int len = 20;
    hmac.getData(v16, &len);
    if (memcmp(v8, v16, 0x14u) == 0) {
        unsigned int a3 = v4 + 20;
        unsigned char v14[4];
        if (bdBytePacker::removeBasicType(v7, v5, a3, &a3, v14, 4u)
            && bdBytePacker::removeBasicType(v7, v5, a3, &a3, &this->m_localTag, 4u)
            && bdBytePacker::removeBasicType(v7, v5, a3, &a3, &this->m_peerTag, 4u)
            && bdBytePacker::removeBasicType(v7, v5, a3, &a3, &this->m_peerTieTag, 4u)
            && bdBytePacker::removeBasicType(v7, v5, a3, &a3, &this->m_localTieTag, 4u)) {
            v11 = true;
            *offset = a3;
        } else {
            v11 = false;
        }
    } else {
        bdMessageProxy proxy(".\\bdPacket\\bdCookie.cpp",
                             "bool __thiscall bdCookie::deserialize(const unsigned char *const ,const unsigned int,unsigned int &)",
                             0x83u, "dw/warn/");
        proxy.log("bdConnection/packet", "cookie failed HMac test.");
    }
    return v11;
}

// ============================================================================
// bdCookie::getSerializedSize â€” ea: 0x8AB900
// ============================================================================
unsigned int bdCookie::getSerializedSize() {
    return this->serialize(NULL, 0xFFFFFFFFu);
}

// ============================================================================
// bdCookieEchoChunk::getFlags â€” ea: 0x8AB9A0
// ============================================================================
bdCookieEchoChunk::bdCookieEchoFlags bdCookieEchoChunk::getFlags() const {
    return this->m_flags;
}

// ============================================================================
// bdCookieEchoChunk::getSerializedSize â€” ea: 0x8AB9B0
// ============================================================================
unsigned int bdCookieEchoChunk::getSerializedSize() {
    return 100;
}

// ============================================================================
// bdCookieEchoChunk::bdCookieEchoChunk (default) â€” ea: 0x8ABA40
// ============================================================================
bdCookieEchoChunk::bdCookieEchoChunk()
    : bdChunk((bdChunkTypes)13),
      m_flags(BD_COOKIE_ECHO_NONE),
      m_cookie(),
      m_rawCookie() {
}

// ============================================================================
// bdCookieEchoChunk::bdCookieEchoChunk (raw) â€” ea: 0x8ABA60
// ============================================================================
bdCookieEchoChunk::bdCookieEchoChunk(const bdReference<bdByteBuffer>& rawCookie)
    : bdChunk((bdChunkTypes)13),
      m_flags(BD_COOKIE_ECHO_NONE),
      m_cookie(),
      m_rawCookie(rawCookie) {
}

// ============================================================================
// bdCookieEchoChunk::~bdCookieEchoChunk â€” ea: 0x8ABAE0
// ============================================================================
bdCookieEchoChunk::~bdCookieEchoChunk() {
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
// bdCookieEchoChunk::getCookie â€” ea: 0x8ABB80
// ============================================================================
bool bdCookieEchoChunk::getCookie(bdReference<bdCookie>& cookie) const {
    if (this->m_cookie.m_ptr != NULL && &this->m_cookie != &cookie) {
        if (cookie.m_ptr != NULL && cookie.m_ptr->releaseRef() == 0)
            delete cookie.m_ptr;
        cookie.m_ptr = this->m_cookie.m_ptr;
        if (cookie.m_ptr != NULL)
            cookie.m_ptr->addRef();
    }
    return this->m_cookie.m_ptr != NULL;
}

// ============================================================================
// bdCookieEchoChunk::serialize â€” ea: 0x8ABBF0
// ============================================================================
unsigned int bdCookieEchoChunk::serialize(unsigned char* data, unsigned int size) {
    unsigned int result = 0;
    unsigned int v16 = 0;
    if (this->m_cookie.m_ptr != NULL || this->m_rawCookie.m_ptr != NULL) {
        unsigned char* v6 = data;
        unsigned int v7 = size;
        unsigned int v8 = bdChunk::serialize(data, size);
        unsigned char flags = (unsigned char)this->m_flags;
        v16 = v8;
        bool v9 = !bdBytePacker::appendBasicType(v6, v7, v8, &v16, &flags, 1u);
        bool v11 = !v9;
        if (this->m_rawCookie.m_ptr != NULL) {
            unsigned int rawSize = this->m_rawCookie.m_ptr->m_size;
            if (!v9) {
                if (bdBytePacker::appendBasicType(v6, v7, v16, &v16, &rawSize, 2u)) {
                    bdBytePacker::appendBuffer(v6, v7, v16, &v16,
                                               this->m_rawCookie.m_ptr->m_data, rawSize);
                    return v16;
                }
            }
        } else {
            unsigned int v14 = v7 - (v16 + 2);
            v16 += 2;
            unsigned int v15 = this->m_cookie.m_ptr->serialize(&v6[v16], v14);
            v16 += v15;
            if (v11) {
                bdBytePacker::appendBasicType(v6, v7, v16 - v15 - 2, &v16, &v15, 2u);
            }
        }
        return v16;
    }
    return result;
}

// ============================================================================
// bdCookieEchoChunk::deserialize â€” ea: 0x8ABD10
// ============================================================================
bool bdCookieEchoChunk::deserialize(const unsigned char* data, unsigned int size,
                                    unsigned int* offset) {
    unsigned int v4 = size;
    unsigned int v17 = *offset;
    unsigned char v6 = 0;
    unsigned int tmp;
    bool ok = false;
    if (bdChunk::deserialize(data, size, &v17)
        && bdBytePacker::removeBasicType(data, v4, v17, &v17, &tmp, 1u)) {
        v6 = (unsigned char)tmp;
        ok = true;
    }
    this->m_flags = (bdCookieEchoFlags)v6;
    unsigned int v18 = 0;
    bool ok2 = ok && bdBytePacker::removeBasicType(data, v4, v17, &v17, &v18, 2u);
    bdCookie* v9 = (bdCookie*)bdMemory::allocate(0x18u);
    bdCookie* v10 = v9 != NULL ? new (v9) bdCookie() : NULL;
    if (this->m_cookie.m_ptr != NULL && this->m_cookie.m_ptr->releaseRef() == 0)
        delete this->m_cookie.m_ptr;
    this->m_cookie.m_ptr = v10;
    if (v10 != NULL)
        v10->addRef();
    unsigned int v14 = v17;
    if (ok2) {
        if (this->m_cookie.m_ptr->deserialize(data, v4, &v17)
            && v17 - v14 != v18) {
            bdMessageProxy proxy(".\\bdPacket\\bdCookieEchoChunk.cpp",
                                 "bool __thiscall bdCookieEchoChunk::deserialize(const unsigned char *const ,const unsigned int,unsigned int &)",
                                 0x6Bu, "dw/warn/");
            proxy.log("bdConnection/packet", "Invalid cookie echo.");
            return false;
        }
        *offset = v17;
    }
    return ok2;
}

// ============================================================================
// bdCookieAckChunk::bdCookieAckChunk â€” ea: 0x8ABEA0
// ============================================================================
bdCookieAckChunk::bdCookieAckChunk()
    : bdChunk((bdChunkTypes)14),
      m_flags(BD_COOKIE_ACK_NONE) {
}

// ============================================================================
// bdCookieAckChunk::~bdCookieAckChunk â€” ea: 0x8ABEC0
// ============================================================================
bdCookieAckChunk::~bdCookieAckChunk() {
}

// ============================================================================
// bdCookieAckChunk::getFlags â€” ea: 0x8ABED0
// ============================================================================
bdCookieAckChunk::bdCookieAckFlags bdCookieAckChunk::getFlags() const {
    return this->m_flags;
}

// ============================================================================
// bdCookieAckChunk::getSerializedSize â€” ea: 0x8ABEE0
// ============================================================================
unsigned int bdCookieAckChunk::getSerializedSize() {
    return 4;
}

// ============================================================================
// bdCookieAckChunk::serialize â€” ea: 0x8ABF10
// ============================================================================
unsigned int bdCookieAckChunk::serialize(unsigned char* data, unsigned int size) {
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
// bdCookieAckChunk::deserialize â€” ea: 0x8ABF90
// ============================================================================
bool bdCookieAckChunk::deserialize(const unsigned char* data, unsigned int size,
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
    this->m_flags = (bdCookieAckFlags)v6;
    if (!ok || !bdBytePacker::removeBasicType(data, v4, v9, &v9, &tmp, 2u))
        return false;
    *offset = v9;
    return true;
}

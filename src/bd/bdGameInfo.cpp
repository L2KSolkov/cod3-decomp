// ============================================================================
// bdGameInfo.cpp â€” discovery game-info object + factory (18 funcs).
// Source: .\bdDiscovery\bdGameInfo.cpp / bdGameInfoFactory.cpp (bdNet)
// Verified against IDA (bdNet:bdGameInfo.obj).
// ============================================================================

#include "bd/bdGameInfo.h"

#include <new>
#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
namespace bdMemory {
void* allocate(unsigned int size);
}

struct bdMessageProxy {
    bdMessageProxy(const char* file, const char* func, unsigned int line, const char* flags);
    void log(const char* channel, const char* format, ...) const;
};

extern bool g_assertFalse;

// ============================================================================
// bdGameInfo::getTitleID â€” ea: 0x8AE8D0
// ============================================================================
unsigned int bdGameInfo::getTitleID() const {
    if (this->m_titleId == 666) {
        bdMessageProxy proxy(".\\bdDiscovery\\bdGameInfo.cpp",
                             "unsigned int __thiscall bdGameInfo::getTitleID(void) const",
                             0x29u, "dw/warn/");
        proxy.log("discovery/gameinfo", "This object appears to be uninitialized.");
    }
    return this->m_titleId;
}

// ============================================================================
// bdGameInfo::getSecurityID â€” ea: 0x8AE920
// ============================================================================
const XNKID& bdGameInfo::getSecurityID() const {
    return this->m_secID;
}

// ============================================================================
// bdGameInfo::getSecurityKey â€” ea: 0x8AE930
// ============================================================================
const XNKEY& bdGameInfo::getSecurityKey() const {
    return this->m_secKey;
}

// ============================================================================
// bdGameInfo::setTitleID â€” ea: 0x8AE940
// ============================================================================
unsigned int bdGameInfo::setTitleID(unsigned int titleId) {
    this->m_titleId = titleId;
    return titleId;
}

// ============================================================================
// bdGameInfo::setSecurityID â€” ea: 0x8AE950
// ============================================================================
void bdGameInfo::setSecurityID(const XNKID& secID) {
    memcpy(this->m_secID.ab, secID.ab, sizeof(XNKID));
}

// ============================================================================
// bdGameInfo::setSecurityKey â€” ea: 0x8AE970
// ============================================================================
void bdGameInfo::setSecurityKey(const XNKEY& secKey) {
    memcpy(this->m_secKey.ab, secKey.ab, sizeof(XNKEY));
}

// ============================================================================
// bdGameInfo::bdGameInfo (default) â€” ea: 0x8AEA00
// ============================================================================
bdGameInfo::bdGameInfo()
    : m_titleId(666) {
    this->m_hostAddr.m_ptr = NULL;
}

// ============================================================================
// bdGameInfo::bdGameInfo (full) â€” ea: 0x8AEA20
// ============================================================================
bdGameInfo::bdGameInfo(unsigned int titleId, const XNKID& secID, const XNKEY& secKey,
                       const bdReference<bdCommonAddr>& hostAddr)
    : m_titleId(titleId),
      m_secID(secID),
      m_secKey(secKey) {
    this->m_hostAddr.m_ptr = hostAddr.m_ptr;
    if (hostAddr.m_ptr != NULL)
        hostAddr.m_ptr->addRef();
}

// ============================================================================
// bdGameInfo::~bdGameInfo â€” ea: 0x8AEA90
// ============================================================================
bdGameInfo::~bdGameInfo() {
    if (this->m_hostAddr.m_ptr != NULL && this->m_hostAddr.m_ptr->releaseRef() == 0) {
        delete this->m_hostAddr.m_ptr;
        this->m_hostAddr.m_ptr = NULL;
    }
}

// ============================================================================
// bdGameInfo::getHostAddr â€” ea: 0x8AEB10
// ============================================================================
bdReference<bdCommonAddr> bdGameInfo::getHostAddr() const {
    if (this->m_hostAddr.m_ptr == NULL) {
        bdMessageProxy proxy(".\\bdDiscovery\\bdGameInfo.cpp",
                             "class bdReference<class bdCommonAddr> __thiscall bdGameInfo::getHostAddr(void) const",
                             0x33u, "dw/warn/");
        proxy.log("discovery/gameinfo", "This object appears to be uninitialized.");
    }
    bdReference<bdCommonAddr> result;
    result.m_ptr = this->m_hostAddr.m_ptr;
    if (result.m_ptr != NULL)
        result.m_ptr->addRef();
    return result;
}

// ============================================================================
// bdGameInfo::setHostAddr â€” ea: 0x8AEB70
// ============================================================================
void bdGameInfo::setHostAddr(const bdReference<bdCommonAddr>& hostAddr) {
    if (hostAddr.m_ptr != this->m_hostAddr.m_ptr) {
        if (this->m_hostAddr.m_ptr != NULL && this->m_hostAddr.m_ptr->releaseRef() == 0)
            delete this->m_hostAddr.m_ptr;
        this->m_hostAddr.m_ptr = hostAddr.m_ptr;
        if (hostAddr.m_ptr != NULL)
            hostAddr.m_ptr->addRef();
    }
}

// ============================================================================
// bdGameInfo::serialize â€” ea: 0x8AEC00
// ============================================================================
void bdGameInfo::serialize(bdBitBuffer& buffer) const {
    if (this->m_titleId == 666) {
        bdMessageProxy proxy(".\\bdDiscovery\\bdGameInfo.cpp",
                             "void __thiscall bdGameInfo::serialize(class bdBitBuffer &) const",
                             0x5Au, "dw/warn/");
        proxy.log("discovery/gameinfo", "This object appears to be uninitialized.");
    }
    if (this->m_hostAddr.m_ptr != NULL) {
        buffer.writeDataType(bdBitBuffer::BD_BB_UNSIGNED_INTEGER32_TYPE);
        buffer.writeBits(&this->m_titleId, 0x20u);
        buffer.writeDataType(bdBitBuffer::BD_BB_FULL_TYPE);
        buffer.writeBits(&this->m_secID, 0x40u);
        buffer.writeDataType(bdBitBuffer::BD_BB_FULL_TYPE);
        buffer.writeBits(&this->m_secKey, 0x80u);
        uint8_t v8[44];
        this->m_hostAddr.m_ptr->serialize(v8);
        buffer.writeBits(v8, 0x150u);
    } else {
        bdMessageProxy proxy(".\\bdDiscovery\\bdGameInfo.cpp",
                             "void __thiscall bdGameInfo::serialize(class bdBitBuffer &) const",
                             0x5Fu, "dw/err/");
        proxy.log("discovery/gameinfo", "Serialized failed as data is uninitialized.");
    }
}

// ============================================================================
// bdGameInfo::deserialize â€” ea: 0x8AED10
// ============================================================================
bool bdGameInfo::deserialize(const bdReference<bdCommonAddr>& hostAddr, bdBitBuffer& buffer) {
    bool ok = false;
    if (buffer.readDataType(bdBitBuffer::BD_BB_UNSIGNED_INTEGER32_TYPE)
        && buffer.readBits(&this->m_titleId, 0x20u)) {
        buffer.readDataType(bdBitBuffer::BD_BB_FULL_TYPE);
        if (buffer.readBits(&this->m_secID, 0x40u)) {
            buffer.readDataType(bdBitBuffer::BD_BB_FULL_TYPE);
            if (buffer.readBits(&this->m_secKey, 0x80u)) {
                uint8_t v27[44];
                if (buffer.readBits(v27, 0x150u)) {
                    bdCommonAddr* v6 = (bdCommonAddr*)bdMemory::allocate(0x40u);
                    bdCommonAddr* v7 = v6 != NULL ? new (v6) bdCommonAddr() : NULL;
                    if (v7 != NULL)
                        v7->addRef();
                    if (v7->deserialize(hostAddr, v27)) {
                        this->setHostAddr(bdReference<bdCommonAddr>(v7));
                        ok = true;
                    }
                    if (v7 != NULL && v7->releaseRef() == 0)
                        delete v7;
                }
            }
        }
    }
    if (!ok) {
        bdMessageProxy proxy(".\\bdDiscovery\\bdGameInfo.cpp",
                             "bool __thiscall bdGameInfo::deserialize(const class bdReference<class bdCommonAddr>,class bdBitBuffer &)",
                             0x89u, "dw/err/");
        proxy.log("discovery/gameinfo", "Deserialization failed");
    }
    return ok;
}

// ============================================================================
// bdGameInfoFactoryImpl::create â€” ea: 0x8AEF60
// ============================================================================
bdGameInfo* bdGameInfoFactoryImpl::create() const {
    if (this->m_creator == NULL) {
        bdMessageProxy proxy(".\\bdDiscovery\\bdGameInfoFactory.cpp",
                             "class bdGameInfo *__thiscall bdGameInfoFactoryImpl::create(void) const",
                             0x13u, "dw/err");
        proxy.log("", "bdGameInfoFactoryImpl::create, must set a game info class with BD_REGISTER_GAME_INFO_CLASS before calling create.");
    }
    return this->m_creator->create();
}

// ============================================================================
// bdGameInfoFactoryImpl::setClass â€” ea: 0x8AEFB0
// ============================================================================
void bdGameInfoFactoryImpl::setClass(bdCreatorBase<bdGameInfo>* creator) {
    if (this->m_creator != NULL) {
        bdMessageProxy proxy(".\\bdDiscovery\\bdGameInfoFactory.cpp",
                             "void __thiscall bdGameInfoFactoryImpl::setClass(class bdCreatorBase<class bdGameInfo> *const )",
                             0x1Du, "dw/warn/");
        proxy.log("gameInfoFactory",
                  "Game info class already set ,BD_REGISTER_GAME_INFO_CLASS should only be called once otherwise memory is leaked.");
    } else {
        this->m_creator = creator;
    }
}

// ============================================================================
// bdGameInfoFactoryImpl::bdGameInfoFactoryImpl â€” ea: 0x8AF000
// ============================================================================
bdGameInfoFactoryImpl::bdGameInfoFactoryImpl()
    : m_creator(NULL) {
}

// ============================================================================
// bdGameInfoFactoryImpl::~bdGameInfoFactoryImpl â€” ea: 0x8AF010
// ============================================================================
bdGameInfoFactoryImpl::~bdGameInfoFactoryImpl() {
    if (this->m_creator != NULL)
        delete this->m_creator;
}

// ============================================================================
// bdGameInfoFactory::bdGameInfoFactory â€” ea: 0x8AF030
// ============================================================================
bdGameInfoFactory::bdGameInfoFactory() {
}

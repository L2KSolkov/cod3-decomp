// ============================================================================
// bdGameInfo â€” discovery game-info object + factory.
// Source: .\bdDiscovery\bdGameInfo.cpp (bdNet)
// Verified against IDA (bdNet:bdGameInfo.obj):
//   getTitleID/setTitleID/getSecurityID/setSecurityID/getSecurityKey/
//   setSecurityKey, ctors/dtor, getHostAddr/setHostAddr, serialize/
//   deserialize, bdGameInfoFactoryImpl create/setClass/ctors, factory ctor
// ============================================================================

#pragma once

#include "bd/bd_types.h"

// ============================================================================
// XNKID / XNKEY â€” Xbox Live security id/key blobs
// ============================================================================
struct XNKID {
    unsigned char ab[8];
};
struct XNKEY {
    unsigned char ab[16];
};

class bdGameInfoFactory;

// ============================================================================
// bdGameInfo â€” game metadata for discovery (40 bytes)
// ============================================================================
class bdGameInfo : public bdReferencable {
public:
    unsigned int                    m_titleId;    // +0x08
    XNKID                           m_secID;      // +0x0C
    XNKEY                           m_secKey;     // +0x14
    bdReference<bdCommonAddr>       m_hostAddr;   // +0x24

    bdGameInfo();
    bdGameInfo(unsigned int titleId, const XNKID& secID, const XNKEY& secKey,
               const bdReference<bdCommonAddr>& hostAddr);
    virtual ~bdGameInfo();

    unsigned int getTitleID() const;
    const XNKID& getSecurityID() const;
    const XNKEY& getSecurityKey() const;
    unsigned int setTitleID(unsigned int titleId);
    void setSecurityID(const XNKID& secID);
    void setSecurityKey(const XNKEY& secKey);
    bdReference<bdCommonAddr> getHostAddr() const;
    void setHostAddr(const bdReference<bdCommonAddr>& hostAddr);
    virtual void serialize(bdBitBuffer& buffer) const;
    bool deserialize(const bdReference<bdCommonAddr>& hostAddr, bdBitBuffer& buffer);
};
static_assert(sizeof(bdGameInfo) == 0x28, "bdGameInfo size mismatch");

// ============================================================================
// bdCreatorBase<T> â€” creator hook for factory
// ============================================================================
template <typename T>
class bdCreatorBase {
public:
    virtual ~bdCreatorBase() {}
    virtual T* create() = 0;
};

// ============================================================================
// bdGameInfoFactoryImpl â€” concrete factory (4 bytes)
// ============================================================================
class bdGameInfoFactoryImpl {
public:
    bdCreatorBase<bdGameInfo>* m_creator;  // +0x00

    bdGameInfoFactoryImpl();
    virtual ~bdGameInfoFactoryImpl();
    bdGameInfo* create() const;
    void setClass(bdCreatorBase<bdGameInfo>* creator);
};

// ============================================================================
// bdGameInfoFactory â€” factory base (4 bytes)
// ============================================================================
class bdGameInfoFactory {
public:
    bdGameInfoFactoryImpl m_impl;  // +0x00

    bdGameInfoFactory();
};

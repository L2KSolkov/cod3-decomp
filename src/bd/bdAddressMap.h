// ============================================================================
// bdAddressMapImpl - maps address handles to common addresses (4 bytes).
// Source: bdSocket:bdAddressMap-xbox.obj (7 funcs).
// Verified against IDA: singleton with bdCommonAddrRef m_me at +0.
// ============================================================================

#ifndef COD3_BD_BDADDRESSMAP_H
#define COD3_BD_BDADDRESSMAP_H

#include "bd/bd_types.h"

struct XNKID;

class bdAddressMapImpl {
public:
    bdAddressMapImpl();
    static bdAddressMapImpl* getInstance();

    bool commonAddrToAddr(const bdReference<bdCommonAddr>& ca, const XNKID& xnkid,
                          bdReference<bdAddrHandle>& addrHandle);
    bool addrToCommonAddr(const bdReference<bdAddrHandle>& addrHandle,
                          bdReference<bdCommonAddr>& ca, XNKID& xnkid) const;
    bool unregisterAddr(bdReference<bdAddrHandle>& addrHandle);
    bool getTitleCommonAddr(bdReference<bdCommonAddr>& ca);
    void setTitleCommonAddr(const bdReference<bdCommonAddr>& ca);
    unsigned int addrToString(const bdReference<bdAddrHandle>& addrHandle,
                              char* pchBuf, unsigned int cchBuf) const;

protected:
    bdReference<bdCommonAddr> m_me;   // +0x00
};
static_assert(sizeof(bdAddressMapImpl) == 0x04, "bdAddressMapImpl size mismatch");

#endif // COD3_BD_BDADDRESSMAP_H

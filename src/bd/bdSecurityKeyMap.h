// ============================================================================
// bdSecurityKeyMap - Xbox security id/key registry (empty class).
// Source: bdCore:bdSecurityKey-xbox.obj (6 funcs).
// On Xbox the XNet layer owns the key store, so this is a thin wrapper.
// ============================================================================

#ifndef COD3_BD_BDSECURITYKEYMAP_H
#define COD3_BD_BDSECURITYKEYMAP_H

#include "bd/bd_types.h"
#include "bd/bdGameInfo.h"

class bdSecurityKeyMap {
public:
    bdSecurityKeyMap();

    bool create(XNKID& xnkid, XNKEY& xnkey);
    bool put(const XNKID& xnkid, const XNKEY& xnkey);
    bool remove(const XNKID& xnkid);
    bool get(const XNKID& xnkid, XNKEY& xnkey);
    bool contains(const XNKID& xnkid);
};

#endif // COD3_BD_BDSECURITYKEYMAP_H

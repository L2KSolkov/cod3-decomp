// ============================================================================
// bdSecurityKeyMap.cpp - Xbox security id/key registry (6 funcs).
// Source: bdCore:bdSecurityKey-xbox.obj
// Verified against IDA (release decompilation).
// ============================================================================

#include "bd/bdSecurityKeyMap.h"

extern "C" {
int __stdcall XNetCreateKey(XNKID* pxnkid, XNKEY* pxnkey);
int __stdcall XNetRegisterKey(const XNKID* pxnkid, const XNKEY* pxnkey);
int __stdcall XNetUnregisterKey(const XNKID* pxnkid);
}

// ============================================================================
// bdSecurityKeyMap::bdSecurityKeyMap - ea: 0x89EA90
// ============================================================================
bdSecurityKeyMap::bdSecurityKeyMap() {
}

// ============================================================================
// bdSecurityKeyMap::create - ea: 0x89EAA0
// ============================================================================
bool bdSecurityKeyMap::create(XNKID& xnkid, XNKEY& xnkey) {
    return XNetCreateKey(&xnkid, &xnkey) == 0;
}

// ============================================================================
// bdSecurityKeyMap::put - ea: 0x89EAC0
// ============================================================================
bool bdSecurityKeyMap::put(const XNKID& xnkid, const XNKEY& xnkey) {
    return XNetRegisterKey(&xnkid, &xnkey) == 0;
}

// ============================================================================
// bdSecurityKeyMap::remove - ea: 0x89EAE0
// ============================================================================
bool bdSecurityKeyMap::remove(const XNKID& xnkid) {
    return XNetUnregisterKey(&xnkid) == 0;
}

// ============================================================================
// bdSecurityKeyMap::get - ea: 0x89EB00
// ============================================================================
bool bdSecurityKeyMap::get(const XNKID&, XNKEY&) {
    bdMessageProxy proxy(".\\bdSocket\\bdSecurityKey-xbox.cpp",
                         "bool __thiscall bdSecurityKeyMap::get(const XNKID &,XNKEY &)",
                         0x28u, "dw/err/");
    proxy.log("securitykey-xbox",
              "No implementation on XBox. This function isn't required on XBox as the XBox net code looks after the keys for us");
    return false;
}

// ============================================================================
// bdSecurityKeyMap::contains - ea: 0x89EB40
// ============================================================================
bool bdSecurityKeyMap::contains(const XNKID&) {
    bdMessageProxy proxy(".\\bdSocket\\bdSecurityKey-xbox.cpp",
                         "bool __thiscall bdSecurityKeyMap::contains(const XNKID &)",
                         0x31u, "dw/err/");
    proxy.log("securitykey-xbox",
              "No implementation on XBox. This function isn't required on XBox as the XBox net code looks after the keys for us");
    return false;
}

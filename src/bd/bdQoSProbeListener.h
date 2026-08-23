// ============================================================================
// bdQoSProbeListener - QoS probe notification interface (3-slot vtable).
// Source: bdSocket:bdQoSProbeListener.obj
// vtable: [dtor, onQoSProbeSuccess(+4), onQoSProbeFail(+8)] (verified @0x8B7400)
// ============================================================================
#ifndef COD3_BD_BDQOSPROBELISTENER_H
#define COD3_BD_BDQOSPROBELISTENER_H

#include "bd/bd_types.h"

class bdQoSProbeInfo;

class bdQoSProbeListener {
public:
    bdQoSProbeListener();                                             // @0x8B7400
    virtual ~bdQoSProbeListener();                                    // @0x8B7410
    virtual void onQoSProbeSuccess(const bdQoSProbeInfo* info) = 0;  // slot +4
    virtual void onQoSProbeFail(bdReference<bdCommonAddr> addr) = 0; // slot +8
};

#endif // COD3_BD_BDQOSPROBELISTENER_H

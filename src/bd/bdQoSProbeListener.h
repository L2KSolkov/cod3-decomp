// ============================================================================
// bdQoSProbeListener — QoS probe notification interface (3-slot vtable).
// Source: bdSocket:bdQoSProbeListener.obj
// ============================================================================
#ifndef COD3_BD_BDQOSPROBELISTENER_H
#define COD3_BD_BDQOSPROBELISTENER_H

// ============================================================================
// bdQoSProbeListener — abstract; subclasses receive QoS probe callbacks.
// ============================================================================
class bdQoSProbeListener {
public:
    bdQoSProbeListener();                                        // @0x8B7400
    virtual ~bdQoSProbeListener();                               // @0x8B7410
    virtual void probeResultA(unsigned int result) = 0;          // purecall slot 1
    virtual void probeResultB(unsigned int result) = 0;          // purecall slot 2
};

#endif // COD3_BD_BDQOSPROBELISTENER_H

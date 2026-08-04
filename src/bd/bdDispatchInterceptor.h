// ============================================================================
// bdDispatchInterceptor — dispatch interception interface (2-slot vtable).
// Source: bdNet:bdDispatchInterceptor.obj
// ============================================================================
#ifndef COD3_BD_BDDISPATCHINTERCEPTOR_H
#define COD3_BD_BDDISPATCHINTERCEPTOR_H

// ============================================================================
// bdDispatchInterceptor — abstract; subclasses intercept dispatched packets.
// ============================================================================
class bdDispatchInterceptor {
public:
    bdDispatchInterceptor();                                     // @0x9ED130
    virtual ~bdDispatchInterceptor();                            // @0x9ED140
    virtual bool dispatchMessage(void* message) = 0;             // purecall slot
};

#endif // COD3_BD_BDDISPATCHINTERCEPTOR_H

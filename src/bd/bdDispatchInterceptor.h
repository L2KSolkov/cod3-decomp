// ============================================================================
// bdDispatchInterceptor - dispatch interception interface (2-slot vtable).
// Source: bdNet:bdDispatchInterceptor.obj
// vtable: [dtor, purecall]. The purecall (slot +4) is bdReceivedMessage
// acceptance, called by bdDispatcher::process.
// ============================================================================
#ifndef COD3_BD_BDDISPATCHINTERCEPTOR_H
#define COD3_BD_BDDISPATCHINTERCEPTOR_H

#include "bd/bd_types.h"

class bdDispatchInterceptor {
public:
    bdDispatchInterceptor();                                     // @0x9ED130
    virtual ~bdDispatchInterceptor();                            // @0x9ED140
    virtual bool accept(const bdReceivedMessage& message) = 0;   // purecall slot (+4)
};

static_assert(sizeof(bdDispatchInterceptor) == 0x4,
              "bdDispatchInterceptor size mismatch");

#endif // COD3_BD_BDDISPATCHINTERCEPTOR_H

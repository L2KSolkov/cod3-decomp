// ============================================================================
// bdSessionInterceptor.cpp — session connection-intercept interface (2 funcs).
// Source: bdPeer:bdSessionInterceptor.obj
// Verified against IDA:
//   ctor @0x8B5C60 (??0bdSessionInterceptor@@QAE@XZ)
//   dtor @0x8B5C70 (??1bdSessionInterceptor@@UAE@XZ)
// vtable: [dtor, purecall(onSessionConnectRequest)]
// ============================================================================
#include "bdSessionHandler.h"

// ============================================================================
// bdSessionInterceptor::bdSessionInterceptor — set the vtable.
// ea: 0x8B5C60
// ============================================================================
bdSessionInterceptor::bdSessionInterceptor() {
}

// ============================================================================
// bdSessionInterceptor::~bdSessionInterceptor — restore base vtable.
// ea: 0x8B5C70
// ============================================================================
bdSessionInterceptor::~bdSessionInterceptor() {
}

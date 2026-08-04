// ============================================================================
// bdSessionHandler.cpp — default session connect handler (3 non-inline funcs).
// Source: bdPeer:bdSessionHandler.obj
// Verified against IDA:
//   ctor @0x8B5CA0 (??0bdSessionHandler@@QAE@XZ)
//   dtor @0x8B5CB0 (??1bdSessionHandler@@UAE@XZ)
//   onSessionConnectRequest @0x8B5CE0
// ============================================================================
#include "bdSessionHandler.h"

// ============================================================================
// bdSessionHandler::bdSessionHandler — set the vtable.
// ea: 0x8B5CA0
// ============================================================================
bdSessionHandler::bdSessionHandler() {
}

// ============================================================================
// bdSessionHandler::~bdSessionHandler — ea: 0x8B5CB0
// ============================================================================
bdSessionHandler::~bdSessionHandler() {
}

// ============================================================================
// bdSessionHandler::onSessionConnectRequest — release the incoming buffer and
// accept the connect request.
// ea: 0x8B5CE0
// ============================================================================
bool bdSessionHandler::onSessionConnectRequest(bdReference<bdBitBuffer> buffer, bdBitBuffer* const result) {
    if (buffer.m_ptr != NULL) {
        if (buffer.m_ptr->releaseRef() == 0)
            delete buffer.m_ptr;
    }
    return true;
}

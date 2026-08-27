// ============================================================================
// bdConnectionListener.cpp - connection event listener (5 funcs).
// Source: bdConnection:bdConnectionListener.obj
// Reconstructed from Demonware 2.0 source + COD3 release disassembly.
// vtable: [dtor, onConnect, onConnectFailed, onDisconnect, onReconnect]
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// bdConnectionListener::bdConnectionListener
// ============================================================================
bdConnectionListener::bdConnectionListener() {
}

// ============================================================================
// bdConnectionListener::~bdConnectionListener - ea: 0x9ECFF0
// ============================================================================
bdConnectionListener::~bdConnectionListener() {
}

// ============================================================================
// bdConnectionListener::onConnect - ea: 0x9ED000
// ============================================================================
void bdConnectionListener::onConnect(bdReference<bdConnection> connection) {
    (void)connection;
}

// ============================================================================
// bdConnectionListener::onConnectFailed - ea: 0x9ED030
// ============================================================================
void bdConnectionListener::onConnectFailed(bdReference<bdConnection> connection) {
    (void)connection;
}

// ============================================================================
// bdConnectionListener::onDisconnect - ea: 0x9ED060
// ============================================================================
void bdConnectionListener::onDisconnect(bdReference<bdConnection> connection) {
    (void)connection;
}

// ============================================================================
// bdConnectionListener::onReconnect - ea: 0x9ED090
// ============================================================================
void bdConnectionListener::onReconnect(const bdReference<bdConnection>& connection) {
    onDisconnect(connection);
    onConnect(connection);
}

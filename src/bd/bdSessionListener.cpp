// ============================================================================
// bdSessionListener.cpp - peer-to-peer session event callbacks (10 funcs).
// Source: bdPeer:bdSessionListener.obj
// Verified against IDA (release decompilation).
// ============================================================================

#include "bdSessionListener.h"

// ============================================================================
// bdSessionListener::bdSessionListener - ea: 0x8B5B70
// ============================================================================
bdSessionListener::bdSessionListener() {
}

// ============================================================================
// bdSessionListener::~bdSessionListener - ea: 0x8B5B80
// ============================================================================
bdSessionListener::~bdSessionListener() {
}

// ============================================================================
// bdSessionListener::onSessionJoinAccepted - ea: 0x8B5B90
// ============================================================================
void bdSessionListener::onSessionJoinAccepted() {
}

// ============================================================================
// bdSessionListener::onSessionJoinRefused - ea: 0x8B5C00
// ============================================================================
void bdSessionListener::onSessionJoinRefused(bdReference<bdBitBuffer>) {
}

// ============================================================================
// bdSessionListener::onSessionConnectFail - ea: 0x8B5BA0
// ============================================================================
void bdSessionListener::onSessionConnectFail() {
}

// ============================================================================
// bdSessionListener::onSessionConnectSuccess - ea: 0x8B5BB0
// ============================================================================
void bdSessionListener::onSessionConnectSuccess() {
}

// ============================================================================
// bdSessionListener::onSessionConnect - ea: 0x8B5C20
// ============================================================================
void bdSessionListener::onSessionConnect(bdReference<bdConnection>) {
}

// ============================================================================
// bdSessionListener::onSessionDisconnect - ea: 0x8B5C40
// ============================================================================
void bdSessionListener::onSessionDisconnect(bdReference<bdConnection>) {
}

// ============================================================================
// bdSessionListener::onSessionStatusChange - ea: 0x8B5BC0
// ============================================================================
void bdSessionListener::onSessionStatusChange(bdSession::bdSessionStatus,
                                              bdSession::bdSessionStatus) {
}

// ============================================================================
// bdSessionListener::onSessionRoleUpdate - ea: 0x8B5BD0
// ============================================================================
void bdSessionListener::onSessionRoleUpdate(bdSession::bdSessionRole) {
}

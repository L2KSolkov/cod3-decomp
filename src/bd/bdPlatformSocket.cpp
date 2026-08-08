// ============================================================================
// bdPlatformSocket.cpp - platform socket stats + private ctor (5 funcs).
// Source: bdPlatform:bdPlatformSocket.obj
// Verified against IDA (release decompilation).
// ============================================================================

#include "bd/bd_types.h"

unsigned __int64 bdPlatformSocket::m_totalBytesSent = 0;
unsigned __int64 bdPlatformSocket::m_totalPacketsSent = 0;
unsigned __int64 bdPlatformSocket::m_totalBytesRecvd = 0;
unsigned __int64 bdPlatformSocket::m_totalPacketsRecvd = 0;

// ============================================================================
// bdPlatformSocket::bdPlatformSocket - ea: 0x8B5D40 (private)
// ============================================================================
bdPlatformSocket::bdPlatformSocket() {
}

// ============================================================================
// getBytesSent - ea: 0x8B5D00
// ============================================================================
unsigned __int64 bdPlatformSocket::getBytesSent() {
    return m_totalBytesSent;
}

// ============================================================================
// getBytesReceived - ea: 0x8B5D10
// ============================================================================
unsigned __int64 bdPlatformSocket::getBytesReceived() {
    return m_totalBytesRecvd;
}

// ============================================================================
// getPacketsSent - ea: 0x8B5D20
// ============================================================================
unsigned __int64 bdPlatformSocket::getPacketsSent() {
    return m_totalPacketsSent;
}

// ============================================================================
// getPacketsRecvd - ea: 0x8B5D30
// ============================================================================
unsigned __int64 bdPlatformSocket::getPacketsRecvd() {
    return m_totalPacketsRecvd;
}

// ============================================================================
// bdConnectionStatistics.cpp â€” connection statistics (16 funcs).
// Source: bdConnection:bdConnectionStatistics.obj
// Verified against IDA (bdConnection:bdConnectionStatistics.obj).
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// bdConnectionStatistics::bdConnectionStatistics â€” ea: 0x8A3A10
// ============================================================================
bdConnectionStatistics::bdConnectionStatistics() {
    this->m_bytesSent = 0;
    this->m_bytesSentPerSecond = 0;
    this->m_lastBytesSent = 0;
    this->m_avgBytesSent = 0;
    this->m_bytesRecv = 0;
    this->m_avgPacketSentSize = 0;
    this->m_avgPacketRecvSize = 0;
    this->m_packetsSent = 0;
    this->m_packetsRecv = 0;
    this->m_maxPacketSizeSent = 0;
    this->m_maxPacketSizeRecv = 0;
    this->m_minPacketSizeSent = -1;
    this->m_minPacketSizeRecv = -1;
    this->m_maxRTT = 0.0f;
    this->m_minRTT = -1.0f;
    this->m_avgRTT = 0.0f;
}

// ============================================================================
// bdConnectionStatistics::reset â€” ea: 0x8A37F0
// ============================================================================
unsigned int bdConnectionStatistics::reset() {
    this->m_bytesSent = 0;
    this->m_bytesSentPerSecond = 0;
    this->m_lastBytesSent = 0;
    this->m_avgBytesSent = 0;
    this->m_bytesRecv = 0;
    this->m_avgPacketSentSize = 0;
    this->m_avgPacketRecvSize = 0;
    this->m_packetsSent = 0;
    this->m_packetsRecv = 0;
    this->m_maxPacketSizeSent = 0;
    this->m_maxPacketSizeRecv = 0;
    this->m_minPacketSizeSent = -1;
    this->m_minPacketSizeRecv = -1;
    this->m_maxRTT = 0.0f;
    this->m_minRTT = -1.0f;
    this->m_avgRTT = 0.0f;
    return -1;
}

// ============================================================================
// addBytesSent â€” ea: 0x8A3840
// ============================================================================
unsigned int bdConnectionStatistics::addBytesSent(unsigned int bytes) {
    this->m_bytesSent += bytes;
    return bytes;
}

// ============================================================================
// addBytesRecv â€” ea: 0x8A3850
// ============================================================================
unsigned int bdConnectionStatistics::addBytesRecv(unsigned int bytes) {
    this->m_bytesRecv += bytes;
    return bytes;
}

// ============================================================================
// addPacketSizeSent â€” ea: 0x8A3860
// ============================================================================
unsigned int bdConnectionStatistics::addPacketSizeSent(unsigned int size) {
    unsigned int m_maxPacketSizeSent = this->m_maxPacketSizeSent;
    if (m_maxPacketSizeSent <= size)
        m_maxPacketSizeSent = size;
    this->m_maxPacketSizeSent = m_maxPacketSizeSent;
    unsigned int m_minPacketSizeSent = this->m_minPacketSizeSent;
    if (m_minPacketSizeSent >= size)
        m_minPacketSizeSent = size;
    this->m_minPacketSizeSent = m_minPacketSizeSent;
    unsigned int m_avgPacketSentSize = this->m_avgPacketSentSize;
    if (m_avgPacketSentSize != 0)
        this->m_avgPacketSentSize = (size + m_avgPacketSentSize) >> 1;
    else
        this->m_avgPacketSentSize = size;
    return size;
}

// ============================================================================
// addPacketSizeRecv â€” ea: 0x8A38A0
// ============================================================================
unsigned int bdConnectionStatistics::addPacketSizeRecv(unsigned int size) {
    unsigned int m_maxPacketSizeRecv = this->m_maxPacketSizeRecv;
    if (m_maxPacketSizeRecv <= size)
        m_maxPacketSizeRecv = size;
    this->m_maxPacketSizeRecv = m_maxPacketSizeRecv;
    unsigned int m_minPacketSizeRecv = this->m_minPacketSizeRecv;
    if (m_minPacketSizeRecv >= size)
        m_minPacketSizeRecv = size;
    this->m_minPacketSizeRecv = m_minPacketSizeRecv;
    unsigned int m_avgPacketRecvSize = this->m_avgPacketRecvSize;
    if (m_avgPacketRecvSize != 0)
        this->m_avgPacketRecvSize = (size + m_avgPacketRecvSize) >> 1;
    else
        this->m_avgPacketRecvSize = size;
    return size;
}

// ============================================================================
// addPacketsSent â€” ea: 0x8A38E0
// ============================================================================
unsigned int bdConnectionStatistics::addPacketsSent(unsigned int packets) {
    this->m_packetsSent += packets;
    return packets;
}

// ============================================================================
// addPacketsRecv â€” ea: 0x8A38F0
// ============================================================================
unsigned int bdConnectionStatistics::addPacketsRecv(unsigned int packets) {
    this->m_packetsRecv += packets;
    return packets;
}

// ============================================================================
// setLastRTT â€” ea: 0x8A3900
// ============================================================================
void bdConnectionStatistics::setLastRTT(float rtt) {
    float m_maxRTT = this->m_maxRTT;
    float v4 = rtt;
    if (m_maxRTT <= rtt)
        m_maxRTT = rtt;
    this->m_maxRTT = m_maxRTT;
    if (rtt <= this->m_minRTT)
        this->m_minRTT = rtt;
    float m_avgRTT = this->m_avgRTT;
    if (m_avgRTT != 0.0f)
        v4 = (rtt + m_avgRTT) * 0.5f;
    this->m_avgRTT = v4;
}

// ============================================================================
// getBytesSent â€” ea: 0x8A3960
// ============================================================================
unsigned int bdConnectionStatistics::getBytesSent() const {
    return this->m_bytesSent;
}

// ============================================================================
// getBytesSentPerSecond â€” ea: 0x8A3970
// ============================================================================
unsigned int bdConnectionStatistics::getBytesSentPerSecond() const {
    return this->m_bytesSentPerSecond;
}

// ============================================================================
// getBytesRecv â€” ea: 0x8A3980
// ============================================================================
unsigned int bdConnectionStatistics::getBytesRecv() const {
    return this->m_bytesRecv;
}

// ============================================================================
// getPacketsSent â€” ea: 0x8A3990
// ============================================================================
unsigned int bdConnectionStatistics::getPacketsSent() const {
    return this->m_packetsSent;
}

// ============================================================================
// getPacketsRecv â€” ea: 0x8A39A0
// ============================================================================
unsigned int bdConnectionStatistics::getPacketsRecv() const {
    return this->m_packetsRecv;
}

// ============================================================================
// getAvgRTT â€” ea: 0x8A39B0
// ============================================================================
float bdConnectionStatistics::getAvgRTT() const {
    return this->m_avgRTT;
}

// ============================================================================
// update â€” ea: 0x8A39C0
// ============================================================================
void bdConnectionStatistics::update(float dt) {
    if (dt > 0.0f) {
        unsigned int m_lastBytesSent = this->m_lastBytesSent;
        unsigned int m_bytesSent = this->m_bytesSent;
        this->m_lastBytesSent = this->m_bytesSent;
        int v5 = (m_bytesSent - m_lastBytesSent + this->m_avgBytesSent) >> 1;
        this->m_avgBytesSent = v5;
        this->m_bytesSentPerSecond = (unsigned int)(v5 / dt);
    }
}

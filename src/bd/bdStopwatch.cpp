// ============================================================================
// bdStopwatch.cpp - high-resolution stopwatch (7 funcs).
// Source: bdCore:bdStopwatch.obj
// Verified against IDA (release decompilation).
// ============================================================================

#include "bd/bdStopwatch.h"

// ============================================================================
// bdStopwatch::bdStopwatch (default) - ea: 0x89E700
// ============================================================================
bdStopwatch::bdStopwatch()
    : m_start(0), m_stop(0) {
}

// ============================================================================
// bdStopwatch::bdStopwatch (copy) - ea: 0x89E650
// ============================================================================
bdStopwatch::bdStopwatch(const bdStopwatch& other)
    : m_start(other.m_start), m_stop(other.m_stop) {
}

// ============================================================================
// bdStopwatch::start - ea: 0x89E670
// ============================================================================
void bdStopwatch::start() {
    m_start = bdPlatformTiming::getHiResTimeStamp();
}

// ============================================================================
// bdStopwatch::stop - ea: 0x89E680
// ============================================================================
void bdStopwatch::stop() {
    if (m_start != 0)
        m_stop = bdPlatformTiming::getHiResTimeStamp();
}

// ============================================================================
// bdStopwatch::reset - ea: 0x89E6A0
// ============================================================================
void bdStopwatch::reset() {
    m_start = 0;
    m_stop = 0;
}

// ============================================================================
// bdStopwatch::getTimeInSeconds - ea: 0x89E6B0
// ============================================================================
float bdStopwatch::getTimeInSeconds() const {
    return bdPlatformTiming::getElapsedTime(m_start, m_stop);
}

// ============================================================================
// bdStopwatch::getElapsedTimeInSeconds - ea: 0x89E6D0
// ============================================================================
float bdStopwatch::getElapsedTimeInSeconds() const {
    if (m_start == 0)
        return 0.0f;
    return bdPlatformTiming::getElapsedTime(m_start,
                                            bdPlatformTiming::getHiResTimeStamp());
}

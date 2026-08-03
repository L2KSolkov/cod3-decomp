// ============================================================================
// bdShortTimer — short-duration stopwatch (COD3 release)
// ea: 0x9EC390 (start), 0x9EC3B0 (reset), 0x9EC3C0 (getElapsedTimeInSeconds)
// ============================================================================

#pragma once

#include <math.h>

// Forward — defined in bdPlatform.cpp
struct bdPlatformTiming {
    static unsigned __int64 getHiResTimeStamp();
    static double getElapsedTime(unsigned __int64 t1, unsigned __int64 t2);
};

// COD3: m_start stores getHiResTimeStamp() / 100 (ea: 0x9EC3A3)
// getElapsedTimeInSeconds: fabs(getElapsedTime(m_start, getHiResTimeStamp()/100) * 100.0)
struct bdShortTimer {
    unsigned __int64 m_start;

    bdShortTimer() : m_start(0) {}

    void start() {
        m_start = bdPlatformTiming::getHiResTimeStamp() / 100;  // ea: 0x9EC390
    }

    void reset() {
        m_start = 0;
    }

    float getElapsedTimeInSeconds() const {
        // ea: 0x9EC3C0 — divides TS by 100, gets elapsed, multiplies by 100
        unsigned __int64 now100 = bdPlatformTiming::getHiResTimeStamp() / 100;
        double elapsed = bdPlatformTiming::getElapsedTime(m_start, now100) * 100.0;
        return (float)fabs(elapsed);
    }
};

// ============================================================================
// bdShortTimer - short-duration stopwatch (COD3 release)
// ea: 0x9EC390 (start), 0x9EC3B0 (reset), 0x9EC3C0 (getElapsedTimeInSeconds)
// ============================================================================

#pragma once

#include <math.h>
#include <stddef.h>

// COD3 bdShortTimer is a 4-byte timer: m_start stores only the low dword of
// getHiResTimeStamp() / 100 (start writes [esi], reset clears a dword, and
// getElapsedTimeInSeconds zero-extends m_start before the 64-bit compare).
// bdPlatformTiming::getElapsedTime returns float (SAM) in this binary.
struct bdPlatformTiming {
    static unsigned __int64 getHiResTimeStamp();
    static float getElapsedTime(unsigned __int64 t1, unsigned __int64 t2);
};

struct bdShortTimer {
    unsigned int m_start;   // +0x00

    bdShortTimer() : m_start(0) {}

    void start() {
        m_start = (unsigned int)(bdPlatformTiming::getHiResTimeStamp() / 100);
    }

    void reset() {
        m_start = 0;
    }

    float getElapsedTimeInSeconds() const {
        unsigned __int64 now100 = bdPlatformTiming::getHiResTimeStamp() / 100;
        float elapsed = bdPlatformTiming::getElapsedTime(m_start, now100) * 100.0f;
        return (float)fabs(elapsed);
    }
};
static_assert(sizeof(bdShortTimer) == 4, "bdShortTimer size mismatch");
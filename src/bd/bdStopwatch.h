// ============================================================================
// bdStopwatch - high-resolution stopwatch (16 bytes).
// Source: bdCore:bdStopwatch.obj (7 funcs).
// Verified against IDA: m_start +0, m_stop +8.
// ============================================================================

#pragma once

#include "bd/bdTiming/bdShortTimer.h"
#include <stddef.h>

struct bdStopwatch {
    unsigned __int64 m_start;   // +0x00
    unsigned __int64 m_stop;    // +0x08

    bdStopwatch();
    bdStopwatch(const bdStopwatch& other);
    void start();
    void stop();
    void reset();
    float getTimeInSeconds() const;
    float getElapsedTimeInSeconds() const;
};
static_assert(sizeof(bdStopwatch) == 0x10, "bdStopwatch size mismatch");

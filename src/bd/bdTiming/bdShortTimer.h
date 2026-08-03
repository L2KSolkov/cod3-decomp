// ============================================================================
// bdTimer — platform-independent high-resolution timer (Demonware 2.0)
// Verified against COD3 ea: 0x9EC3C0 (getElapsedTimeInSeconds), 0x9EC390 (start)
// ============================================================================

#pragma once

#ifdef _WIN32
  #include <windows.h>
#else
  #include <sys/time.h>
#endif

typedef unsigned long long bdUInt64;
typedef float bdFloat32;

class bdShortTimer {
    bdUInt64 m_start;
public:
    bdShortTimer() { reset(); }

    void start() {
#ifdef _WIN32
        LARGE_INTEGER t;
        QueryPerformanceCounter(&t);
        m_start = t.QuadPart;
#else
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        m_start = (bdUInt64)tv.tv_sec * 1000000ULL + tv.tv_usec;
#endif
    }

    void reset() { m_start = 0; }

    bdFloat32 getElapsedTimeInSeconds() const {
        if (m_start == 0) return 0.f;
#ifdef _WIN32
        LARGE_INTEGER now, freq;
        QueryPerformanceCounter(&now);
        QueryPerformanceFrequency(&freq);
        return (bdFloat32)((double)(now.QuadPart - m_start) / (double)freq.QuadPart);
#else
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        bdUInt64 now = (bdUInt64)tv.tv_sec * 1000000ULL + tv.tv_usec;
        return (bdFloat32)((now - m_start) / 1000000.0);
#endif
    }
};

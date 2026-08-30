// ============================================================================
// bdPlatformTiming - Win32 timing functions with the IDA release ABI.
// ============================================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "bd/bdTiming/bdShortTimer.h"

// ea: 0x008B5D50
unsigned __int64 bdPlatformTiming::getHiResTimeStamp()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return static_cast<unsigned __int64>(counter.QuadPart);
}

// ea: 0x008B5D70
float bdPlatformTiming::getElapsedTime(unsigned __int64 t1,
                                       unsigned __int64 t2)
{
    static LARGE_INTEGER frequency;
    static bool initialized = false;
    if (!initialized)
    {
        QueryPerformanceFrequency(&frequency);
        initialized = true;
    }
    return static_cast<float>(
        static_cast<double>(t2 - t1) / static_cast<double>(frequency.QuadPart));
}

// ea: 0x008B5DE0
void bdPlatformTiming::sleep(unsigned int ms)
{
    Sleep(ms);
}

// ea: 0x008B5DF0
unsigned int bdPlatformTiming::getLoResTimeStamp()
{
    return static_cast<unsigned int>(GetTickCount());
}

// ea: 0x008B5E00
unsigned int bdPlatformTiming::getLoResElapsedTime(unsigned int t1,
                                                   unsigned int t2)
{
    unsigned __int64 extended = t2;
    if (t2 < t1)
        extended += 0x100000000ULL;
    return static_cast<unsigned int>((extended - t1) / 1000);
}

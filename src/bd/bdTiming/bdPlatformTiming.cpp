// ============================================================================
// bdPlatformTiming - Win32 timing functions with the IDA release ABI.
// ============================================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "bd/bdTiming/bdShortTimer.h"

unsigned __int64 bdPlatformTiming::getHiResTimeStamp()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return static_cast<unsigned __int64>(counter.QuadPart);
}

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

void bdPlatformTiming::sleep(unsigned int ms)
{
    Sleep(ms);
}

unsigned int bdPlatformTiming::getLoResTimeStamp()
{
    return static_cast<unsigned int>(GetTickCount());
}

unsigned int bdPlatformTiming::getLoResElapsedTime(unsigned int t1,
                                                   unsigned int t2)
{
    unsigned __int64 extended = t2;
    if (t2 < t1)
        extended += 0x100000000ULL;
    return static_cast<unsigned int>((extended - t1) / 1000);
}

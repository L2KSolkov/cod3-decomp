// ============================================================================
// fogconfig.cpp - render.o FogConfig (FogConfig.cpp)
// ============================================================================

#include "game/logic/g_local.h"

#include <intrin.h>

// global ngl exports (outside the namespace so they mangle as ::ngl*)
extern void nglSetFogRange(float Near, float Far, float Min, float Max);
extern void nglSetFogColor(float r, float g, float b);

namespace FogConfig {
int   sEnabled;   // ?sEnabled@FogConfig@@3HA @ 0xDFA400
float sNear;      // ?sNear@FogConfig@@3MA @ 0xF743BC
float sFar;       // ?sFar@FogConfig@@3MA @ 0xDFA404
float sStart;     // ?sStart@FogConfig@@3MA @ 0xF743C0
float sEnd;       // ?sEnd@FogConfig@@3MA @ 0xDFA408
float sRed;       // ?sRed@FogConfig@@3MA @ 0xDFA40C
float sGreen;     // ?sGreen@FogConfig@@3MA @ 0xDFA410
float sBlue;      // ?sBlue@FogConfig@@3MA @ 0xDFA414

// ea: 0x006BC300
void SetEnabled(int enable)
{
    sEnabled = enable;
}

// ea: 0x006BC310
void SetRange(float n, float f)
{
    sNear = n;
    sFar = f;
}

// ea: 0x006BC330
void SetVal(float s, float e)
{
    sStart = s;
    sEnd = e;
}

// ea: 0x006BC350
void SetColor(float r, float g, float b)
{
    if (r < 0.0f || r > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FogConfig.cpp";
        AeAssert::gCurrentLine = 41;
        AeAssert::gCurrentExpr = "r >= 0 && r <= 1.0f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("fog red out of range"))
            __debugbreak();
    }
    if (g < 0.0f || g > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FogConfig.cpp";
        AeAssert::gCurrentLine = 42;
        AeAssert::gCurrentExpr = "g >= 0 && g <= 1.0f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("fog red out of range"))
            __debugbreak();
    }
    if (b < 0.0f || b > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FogConfig.cpp";
        AeAssert::gCurrentLine = 43;
        AeAssert::gCurrentExpr = "b >= 0 && b <= 1.0f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("fog red out of range"))
            __debugbreak();
    }
    sRed = r;
    sGreen = g;
    sBlue = b;
}

// ea: 0x006BC4A0
void SetColorInt(int r, int g, int b)
{
    SetColor(r * 0.0039215689f, g * 0.0039215689f, b * 0.0039215689f);
}

// ea: 0x006BC4F0
void Apply()
{
    if (sEnabled != 0)
    {
        ::nglSetFogRange(sNear, sFar, sStart, sEnd * 0.5f);
        ::nglSetFogColor(sRed, sGreen, sBlue);
    }
}

// ea: 0x006BC550
void GetEnabled(int& enable)
{
    enable = sEnabled;
}

// ea: 0x006BC560
void GetColor(float& red, float& green, float& blue)
{
    red = sRed;
    green = sGreen;
    blue = sBlue;
}

// ea: 0x006BC5A0
void GetColorInt(int& red, int& green, int& blue)
{
    red = (int)(sRed * 255.0f);
    green = (int)(sGreen * 255.0f);
    blue = (int)(sBlue * 255.0f);
}

// ea: 0x006BC5F0
void GetRange(float& n, float& f)
{
    n = sNear;
    f = sFar;
}

// ea: 0x006BC620
void GetVal(float& s, float& e)
{
    s = sStart;
    e = sEnd;
}
}  // namespace FogConfig

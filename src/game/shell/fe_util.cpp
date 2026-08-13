// ============================================================================
// fe_util.cpp - FE free helpers (shell.o)
// ============================================================================

#include "game/shell/shell_types.h"
#include "core/tlFixedString.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>

extern float sNaN;  // ?sNaN@@3MA @ 0x10F19D0
extern int cg_widescreen_integer;  // ?cg_widescreen@@3Ucvar_t@@A (cg.o)
extern ELanguage gLanguage;        // 0x012F03A4
extern int g_currentAsian;         // shell.o data
extern const char* const defaultFileName;  // 0xCD67AE

int SEH_GetCurrentLanguage();  // game.o stub

struct nglTexture;
void* cdGetResource(const tlFixedString& FileName, unsigned int FourCC,
                    bool ExtraSafety);  // streamer.o 0x678D40

// ============================================================================
// screensafe
// ============================================================================

// ea: 0x00565C80
float get_screensafe_left()
{
    return 50.0f;
}

// ea: 0x00565C90
float get_screensafe_right()
{
    return 590.0f;
}

// ea: 0x00565CA0
float get_screensafe_top()
{
    return 40.0f;
}

// ea: 0x00565CB0
float get_screensafe_bottom()
{
    return 440.0f;
}

// ============================================================================
// color helpers
// ============================================================================

// ea: 0x0056A9A0
color32 MultiplyColors(color32 c1, color32 c2)
{
    color32 result;
    result.c.b = (unsigned char)(c1.c.b * c2.c.b / 0xFF);
    result.c.g = (unsigned char)(c1.c.g * c2.c.g / 0xFF);
    result.c.r = (unsigned char)(c2.c.r * c1.c.r / 0xFF);
    result.c.a = (unsigned char)(c1.c.a * c2.c.a / 0xFF);
    return result;
}

// ea: 0x0056ADD0
color32 ConvertTextColor(unsigned int lng)
{
    color32 result;
    result.i = (lng & 0xFF00FF00) | (lng << 16) | ((lng >> 16) & 0xFF);
    return result;
}

// ea: 0x0056B400
color32 ConvertNGLColor(unsigned int color)
{
    color32 result;
    result.i = color;
    return result;
}

// ea: 0x0056AE00
color32 ReadColor(unsigned char* buffer, int& index)
{
    color32 result;
    unsigned char c0 = buffer[index];
    unsigned char c1 = buffer[index + 1];
    unsigned char c2 = buffer[index + 2];
    unsigned char c3 = buffer[index + 3];
    index += 4;
    result.c.b = c2;
    result.c.g = c1;
    result.c.r = c0;
    result.c.a = c3;
    return result;
}

// ============================================================================
// binary readers
// ============================================================================

// ea: 0x0056AE40
unsigned int ReadLong(unsigned char* buffer, int& index)
{
    unsigned int result = buffer[index]
                          | ((buffer[index + 1]
                              | (buffer[index + 2] << 8)) << 8);
    index += 4;
    return result;
}

// ea: 0x0056AE80
unsigned char ReadChar(unsigned char* buffer, int& index)
{
    return buffer[index++];
}

// ea: 0x0056AEA0
float ReadFloat(unsigned char* buffer, int& index)
{
    unsigned int bits = buffer[index]
                        | ((buffer[index + 1] | (buffer[index + 2] << 8)) << 8);
    index += 4;
    float result;
    memcpy(&result, &bits, 4);
    return result;
}

// ea: 0x0056AEE0
short ReadShort(unsigned char* buffer, int& index)
{
    short result = (signed char)buffer[index];
    index += 2;
    return result;
}

// ea: 0x0056AF50
char* ReadStringPointer(unsigned char* buffer, int& index)
{
    int v2 = index + 2;
    unsigned short v3 = buffer[index] | (buffer[index + 1] << 8);
    index = v2;
    if (v3 > 0)
    {
        do
        {
            buffer[index - 2] = buffer[index];
            --v3;
            ++index;
        }
        while (v3 != 0);
    }
    buffer[index - 2] = 0;
    buffer[index - 1] = 0;
    return (char*)&buffer[v2 - 2];
}

// ============================================================================
// math helpers
// ============================================================================

// ea: 0x0056AA40
int CountSharedVertices(short* didxs, int tri1, int tri2, int* ind)
{
    char used[3] = {0, 0, 0};
    int result = 0;
    for (int i = 0; i < 3; ++i)
    {
        bool found = false;
        for (int j = 0; j < 3; ++j)
        {
            if (used[j] == 0)
            {
                short v8 = didxs[2 * tri1 + tri1 + i];
                if (v8 == didxs[2 * tri2 + tri2 + j])
                {
                    ++result;
                    found = true;
                    used[j] = 1;
                    ind[result] = v8;
                }
            }
        }
        if (!found)
            ind[0] = didxs[2 * tri1 + tri1 + i];
    }
    if (used[0] == 0)
        ind[3] = didxs[3 * tri2];
    if (used[1] == 0)
        ind[3] = didxs[3 * tri2 + 1];
    if (used[2] == 0)
        ind[3] = didxs[3 * tri2 + 2];
    return result;
}

// ea: 0x0056AB10
bool evenly_divides(float total, float divider)
{
    return (total / divider) == (total / divider);
}

// ea: 0x0056AB40
int round_to_nearest_halfpi(float val)
{
    int result = (int)(val * 0.63661975f);
    float v2 = (float)(result + 1) - (val * 0.63661975f);
    if (v2 < 0.001f && v2 > -0.001f)
        ++result;
    return result;
}

// ea: 0x0056B3C0
bool eq_to_tolerance(float a, float b, float tol)
{
    if (a < b)
        return !(tol <= (b - a));
    return !(tol <= (a - b));
}

// ============================================================================
// window scaling
// ============================================================================

// ea: 0x0056B8A0
float GetXScalingForWindow(int window)
{
    float v1 = 0.75f;
    if (cg_widescreen_integer == 0)
        v1 = 1.0f;
    if (window < 5 || window > 8)
        return v1;
    return v1 * 0.60000002f;
}

// ea: 0x0056B8F0
float GetYScalingForWindow(int window)
{
    switch (window)
    {
    case 3:
    case 4:
        return 0.69999999f;
    case 5:
    case 6:
    case 7:
    case 8:
        return 0.60000002f;
    default:
        return 1.0f;
    }
}

// ============================================================================
// text / date
// ============================================================================

// ea: 0x0056F380
void AsciiToUnicode(char* ascii_buffer, unsigned short* string)
{
    char* v2 = ascii_buffer;
    char i = *ascii_buffer;
    for (; *v2 != 0; ++string)
    {
        ++v2;
        *string = (unsigned short)i;
        i = *v2;
    }
    *string = 0;
}

// ea: 0x00575690
system_time GetSystemDate()
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    system_time sd;
    sd.year = st.wYear;
    sd.month = st.wMonth;
    sd.day = st.wDay;
    sd.hour = st.wHour;
    sd.minute = st.wMinute;
    sd.second = st.wSecond;
    if (sd.month < 1 || sd.month > 12)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\game_data.cpp";
        AeAssert::gCurrentLine = 146;
        AeAssert::gCurrentExpr = "0 && \"Month value incorrect\"";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
        sd.month = sd.month < 1 ? 1 : 12;
    }
    if (sd.day < 1 || sd.day > 31)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\game_data.cpp";
        AeAssert::gCurrentLine = 148;
        AeAssert::gCurrentExpr = "0 && \"Day value incorrect\"";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
        sd.day = sd.day < 1 ? 1 : 31;
    }
    if (sd.hour < 0 || sd.hour > 23)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\game_data.cpp";
        AeAssert::gCurrentLine = 150;
        AeAssert::gCurrentExpr = "0 && \"Hour value incorrect\"";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
        sd.hour = sd.hour < 0 ? 0 : 23;
    }
    if (sd.minute < 0 || sd.minute > 59)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\game_data.cpp";
        AeAssert::gCurrentLine = 152;
        AeAssert::gCurrentExpr = "0 && \"Minute value incorrect\"";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
        sd.minute = sd.minute < 0 ? 0 : 59;
    }
    if (sd.second < 0 || sd.second > 59)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\game_data.cpp";
        AeAssert::gCurrentLine = 154;
        AeAssert::gCurrentExpr = "0 && \"Second value incorrect\"";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
        sd.second = sd.second < 0 ? 0 : 59;
    }
    return sd;
}

// ============================================================================
// CJK collation helpers
// ============================================================================

// ea: 0x00576910
int Korean_CollapseKSC5601HangulCode(unsigned int uiCode)
{
    if (((uiCode >> 8) & 0xFF) < 0xB0 || ((uiCode >> 8) & 0xFF) > 0xC8
        || uiCode <= 0xA0 || uiCode == 0xFF)
        return 0;
    return (uiCode + 96) + 96 * ((uiCode - 45216) >> 8);
}

// ea: 0x005769A0
int Taiwanese_CollapseBig5Code(unsigned int uiCode)
{
    if ((((uiCode >> 8) & 0xFF) < 0xA1 || ((uiCode >> 8) & 0xFF) > 0xC6)
            && (((uiCode >> 8) & 0xFF) < 0xC9
                || ((uiCode >> 8) & 0xFF) > 0xF9)
        || ((uiCode < 0x40 || uiCode > 0x7E) && (uiCode < 0xA1 || uiCode == 0xFF)))
    {
        return 0;
    }
    unsigned int v1 = uiCode - 41280;
    if ((uiCode - 64) >= 0x60)
        v1 = uiCode - 41312;
    return v1 + 160 * (v1 >> 8);
}

// ea: 0x00576A80
int Japanese_CollapseShiftJISCode(unsigned int uiCode)
{
    if ((((uiCode >> 8) & 0xFF) < 0x81 || ((uiCode >> 8) & 0xFF) > 0x9F)
            && (((uiCode >> 8) & 0xFF) < 0xE0
                || ((uiCode >> 8) & 0xFF) > 0xEF)
        || ((uiCode < 0x40 || uiCode > 0x7E) && uiCode > 0xFFFFFFFC))
    {
        return 0;
    }
    unsigned int v1 = uiCode - 33088;
    if ((uiCode - 64) >= 0x40)
        v1 = uiCode - 33089;
    if ((v1 & 0xFF00) >= 0x5F00)
        v1 -= 0x4000;
    return v1 + 188 * (v1 >> 8);
}

// ea: 0x00576B60
int Chinese_CollapseGBCode(unsigned int uiCode)
{
    if (((uiCode >> 8) & 0xFF) < 0xA1 || ((uiCode >> 8) & 0xFF) > 0xF7
        || uiCode <= 0xA0 || uiCode == 0xFF)
        return 0;
    return (uiCode + 96) + 95 * ((uiCode - 41376) >> 8);
}

// ============================================================================
// language queries
// ============================================================================

// ea: 0x00576BA0
int Language_IsAsian()
{
    return g_currentAsian;
}

// ea: 0x00576BB0
int Language_UsesSpaces()
{
    int CurrentLanguage = SEH_GetCurrentLanguage();
    return CurrentLanguage < 9 || CurrentLanguage > 11;
}

// ============================================================================
// localized texture
// ============================================================================

// ea: 0x0056A8A0
nglTexture* LocalizedGetTexture(const char* name)
{
    const char* v1 = "";
    switch (gLanguage)
    {
    case kLanguageEnglish:
        break;
    case kLanguageGerman:
        v1 = "de_";
        break;
    case kLanguageFrench:
        v1 = "fr_";
        break;
    case kLanguageSpanish:
        v1 = "es_";
        break;
    case kLanguageItalian:
        v1 = "it_";
        break;
    default:
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEPanel.cpp";
        AeAssert::gCurrentLine = 618;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Unknown language!"))
            __debugbreak();
        break;
    }
    char localized_name[256];
    sprintf(localized_name, "%s%s", v1, name);
    tlFixedString FileName(localized_name);
    nglTexture* result = (nglTexture*)cdGetResource(FileName, 0x584554,
                                                    false);
    if (result == nullptr)
    {
        tlFixedString FileName2(name);
        return (nglTexture*)cdGetResource(FileName2, 0x584554, false);
    }
    return result;
}

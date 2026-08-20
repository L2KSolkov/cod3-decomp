// ============================================================================
// g_q_shared.cpp - game.o q_shared string/endian/utility helpers
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Q_* character classification - ea: 0x6107E0..0x6108F0
// ============================================================================
// ea: 0x006107E0
int Q_isprint(int c) { return c >= 32 && c <= 126; }
// ea: 0x00610800
int Q_islower(int c) { return c >= 97 && c <= 122; }
// ea: 0x00610820
int Q_isupper(int c) { return c >= 65 && c <= 90; }
// ea: 0x00610840
int Q_isalpha(int c) { return (c >= 97 && c <= 122) || (c >= 65 && c <= 90); }
// ea: 0x00610870
int Q_isnumeric(int c) { return c >= 48 && c <= 57; }
// ea: 0x00610890
int Q_isalphanumeric(int c)
{
    return (c >= 97 && c <= 122) || (c >= 65 && c <= 90)
        || (c >= 48 && c <= 57);
}
// ea: 0x006108C0
int Q_isforfilename(int c)
{
    return (c >= 97 && c <= 122) || (c >= 65 && c <= 90)
        || (c >= 48 && c <= 57) || c == 95 || c == 45;
}

// ============================================================================
// Q_strrchr - ea: 0x610900
// ============================================================================
// ea: 0x00610900
char* Q_strrchr(const char* string, int c)
{
    const char* result = string;
    char v3 = *string;
    const char* i = nullptr;
    for (; v3 != 0; ++result)
    {
        bool v5 = v3 == c;
        v3 = result[1];
        if (v5)
            i = result;
    }
    if (c != 0)
        return const_cast<char*>(i);
    return const_cast<char*>(result);
}

// ============================================================================
// Q_strncpyz - ea: 0x610930
// ============================================================================
// ea: 0x00610930
void Q_strncpyz(char* dest, const char* src, int destsize)
{
    if (src == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
        AeAssert::gCurrentLine = 423;
        AeAssert::gCurrentExpr = "src";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (dest == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
        AeAssert::gCurrentLine = 424;
        AeAssert::gCurrentExpr = "dest";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (destsize < 1)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
        AeAssert::gCurrentLine = 425;
        AeAssert::gCurrentExpr = "destsize >= 1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", destsize))
            __debugbreak();
    }
    strncpy(dest, src, destsize - 1);
    dest[destsize - 1] = 0;
}

// ============================================================================
// Q_stricmpn / Q_strncmp / Q_stricmp / Q_strlwr / Q_strupr / Q_strcat
// ============================================================================
// ea: 0x00610A30
int Q_stricmpn(const char* s1, const char* s2, int n)
{
    return ae_stricmpn(s1, s2, n);
}

// ea: 0x00610A40
int Q_strncmp(const char* s1, const char* s2, int n)
{
    int v6, v7;
    for (;;)
    {
        v6 = *s1;
        v7 = *s2;
        ++s1;
        ++s2;
        int v8 = n--;
        if (v8 == 0)
            return 0;
        if (v6 != v7)
            break;
        if (v6 == 0)
            return 0;
    }
    return 2 * (v6 >= v7) - 1;
}

// ea: 0x00610A80
int Q_stricmp(const char* s1, const char* s2)
{
    if (s1 != nullptr && s2 != nullptr)
        return ae_stricmpn(s1, s2, 0x7FFFFFFF);
    return -1;
}

// ea: 0x00610AB0
char* Q_strlwr(char* s1)
{
    char* v1 = s1;
    char v2;
    if (*s1 != 0)
    {
        do
        {
            *v1 = (char)tolower(*v1);
            v2 = *++v1;
        } while (v2 != 0);
    }
    return s1;
}

// ea: 0x00610AE0
char* Q_strupr(char* s1)
{
    char* v1 = s1;
    char v2;
    if (*s1 != 0)
    {
        do
        {
            *v1 = (char)toupper(*v1);
            v2 = *++v1;
        } while (v2 != 0);
    }
    return s1;
}

// ea: 0x00610B10
void Q_strcat(char* dest, int size, const char* src)
{
    int v3 = (int)strlen(dest);
    if (v3 >= size)
        Com_Error(ERR_FATAL, "Q_strcat: already overflowed");
    Q_strncpyz(&dest[v3], src, size - v3);
}

// ============================================================================
// Q_DrawStrlen - ea: 0x610B60 (skips ^N color codes)
// ============================================================================
// ea: 0x00610B60
int Q_DrawStrlen(const char* str)
{
    const char* v1 = str;
    int result = 0;
    while (*v1 != 0)
    {
        if (*v1 == 94 && v1[1] != 0 && v1[1] != 94
            && v1[1] >= 48 && v1[1] <= 57)
        {
            v1 += 2;
        }
        else
        {
            ++result;
            ++v1;
        }
    }
    return result;
}

// ============================================================================
// Q_CleanStr - ea: 0x610BA0
// ============================================================================
// ea: 0x00610BA0
char* Q_CleanStr(char* string)
{
    char* result = string;
    char v2 = *string;
    char* v3 = string;
    char* i = string;
    for (; v2 != 0; ++v3)
    {
        if (*v3 == 94 && v3[1] != 0 && v3[1] != 94
            && v3[1] >= 48 && v3[1] <= 57)
        {
            ++v3;
        }
        else if (v2 >= 32 && v2 != 127)
        {
            *i++ = v2;
        }
        v2 = v3[1];
    }
    *i = 0;
    return result;
}

// ============================================================================
// Q_CleanCharacter - ea: 0x610BF0
// ============================================================================
// ea: 0x00610BF0
char Q_CleanCharacter(char cCharacter)
{
    return cCharacter;
}

// ============================================================================
// Com_sprintf - ea: 0x610C00
// ============================================================================
// ea: 0x00610C00
void Com_sprintf(char* dest, int size, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    const char* v3 = dest;
    int v4 = _vsnprintf(dest, size, fmt, ap);
    va_end(ap);
    dest[size - 1] = 0;
    if (v4 < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
        AeAssert::gCurrentLine = 559;
        AeAssert::gCurrentExpr = "len >= 0";
        if (!AeAssert::IsIgnored())
        {
            if (size > 1024)
                v3 = "<string too long>";
            if (AeAssert::Assert(v3))
                __debugbreak();
        }
    }
}

// ============================================================================
// Q_strncasecmp / Q_strcasecmp - ea: 0x610C80
// ============================================================================
// ea: 0x00610C80
int Q_strncasecmp(const char* s1, const char* s2, int n)
{
    for (;;)
    {
        int v6 = *s1;
        int v7 = *s2;
        ++s1;
        ++s2;
        int v8 = n--;
        if (v8 == 0)
            return 0;
        if (v6 != v7)
        {
            if (v6 >= 97 && v6 <= 122)
                v6 -= 32;
            if (v7 >= 97 && v7 <= 122)
                v7 -= 32;
            if (v6 != v7)
                break;
        }
        if (v6 == 0)
            return 0;
    }
    return -1;
}

// ea: 0x00610CE0
int Q_strcasecmp(const char* s1, const char* s2)
{
    return Q_strncasecmp(s1, s2, 0x7FFFFFFF);
}

// ============================================================================
// va - ea: 0x610D00 (ring of 2048-byte buffers)
// ============================================================================
static char va_string[2048];
static char va_temp_buffer[2048];
static int index_1;

// ea: 0x00610D00
char* va(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    unsigned int v1 = (unsigned int)_vsnprintf(va_temp_buffer, 0x800u,
                                               format, ap);
    va_end(ap);
    va_temp_buffer[2047] = 0;
    if (v1 >= 0x800)
        Com_Error(ERR_DROP, "overrun string in call to va(): Tell MikeA");
    int v2 = index_1;
    if ((index_1 + (int)v1) >= 2047)
    {
        v2 = 0;
        index_1 = 0;
    }
    char* result = &va_string[v2];
    memcpy(result, va_temp_buffer, v1 + 1);
    index_1 += v1 + 1;
    return result;
}

// ============================================================================
// tv - ea: 0x610D90 (ring of 8 vec3)
// ============================================================================
static float tv_array[8][3];
static int index_2;

// ea: 0x00610D90
float* tv(float x, float y, float z)
{
    float* result = tv_array[index_2];
    result[0] = x;
    result[1] = y;
    index_2 = (index_2 + 1) & 7;
    result[2] = z;
    return result;
}

// ============================================================================
// Com_* misc helpers
// ============================================================================
// ea: 0x00610470
unsigned char ColorIndex(unsigned char c)
{
    int result = c - 48;
    if (result >= 0xA)
        return 7;
    return result;
}

// ea: 0x00610490
const float Com_Clamp(float min, float max, float value)
{
    if (min > value)
        return min;
    if (value <= max)
        return value;
    return max;
}

// ea: 0x006104C0
char* Com_SkipPath(char* pathname)
{
    char* v1 = pathname;
    char v2 = *pathname;
    char* result = pathname;
    for (; v2 != 0; ++v1)
    {
        if (v2 == 47)
            result = v1 + 1;
        v2 = v1[1];
    }
    return result;
}

// ea: 0x006104F0
void Com_StripExtension(const char* in, char* out)
{
    const char* v2 = in;
    char v3 = *in;
    if (*in != 0)
    {
        char* v4 = out;
        do
        {
            if (v3 == 46)
                break;
            *v4 = v3;
            v3 = v2[1];
            ++v4;
            ++v2;
        } while (v3 != 0);
        *v4 = 0;
    }
    else
    {
        *out = 0;
    }
}

// ea: 0x00610520
int Com_BitCheck(const int* const array, int bitNum)
{
    return (array[bitNum >> 5] & (1 << (bitNum & 0x1F))) != 0;
}

// ea: 0x00610550
void Com_BitSet(int* const array, int bitNum)
{
    array[bitNum >> 5] |= 1 << (bitNum & 0x1F);
}

// ea: 0x00610570
void Com_BitClear(int* const array, int bitNum)
{
    array[bitNum >> 5] &= ~(1 << (bitNum & 0x1F));
}

// ============================================================================
// Com_Compress - ea: 0x6103D0 (strip // and /* */ comments, keep newlines)
// ============================================================================
// ea: 0x006103D0
int Com_Compress(char* data_p)
{
    char* v1 = data_p;
    int v2 = 0;
    char* v3 = data_p;
    if (data_p == nullptr)
        return 0;
    char v4 = *data_p;
    if (*data_p == 0)
    {
        *v3 = 0;
        return v2;
    }
    do
    {
        if (v4 == 47)
        {
            char v5 = v1[1];
            if (v5 == 47)
            {
                char i;
                for (i = *v1; i != 0; i = *++v1)
                {
                    if (i == 10)
                        break;
                }
                goto next_char;
            }
            if (v5 == 42)
            {
                char v7 = *v1;
                if (*v1 != 0)
                {
                    while (v7 != 42 || v1[1] != 47)
                    {
                        if (v7 == 10)
                        {
                            *v3++ = 10;
                            ++v2;
                        }
                        v7 = *++v1;
                        if (v7 == 0)
                            goto next_char;
                    }
                    if (*v1 != 0)
                        v1 += 2;
                }
                goto next_char;
            }
        }
        *v3++ = v4;
        ++v2;
        ++v1;
    next_char:
        v4 = *v1;
    } while (*v1 != 0);
    *v3 = 0;
    return v2;
}

// ============================================================================
// Lean fraction helpers - ea: 0x611690 / 0x6116C0
// ============================================================================
// ea: 0x00611690
float GetLeanFraction(float fFrac)
{
    return (2.0f - fabsf(fFrac)) * fFrac;
}

// ea: 0x006116C0
float UnGetLeanFraction(float fFrac)
{
    if (fFrac < 0.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
        AeAssert::gCurrentLine = 1120;
        AeAssert::gCurrentExpr = "fFrac >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (fFrac > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_shared.cpp";
        AeAssert::gCurrentLine = 1121;
        AeAssert::gCurrentExpr = "fFrac <= 1.f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return 1.0f - sqrtf(1.0f - fFrac);
}

// ============================================================================
// Orientation transforms - ea: 0x611780..0x6119E0
// ============================================================================
struct orientation_t {
    float origin[3];    // +0x00
    float axis[3][3];   // +0x0C
};

// ea: 0x00611780
void OrientationPosToWorldPos(const orientation_t* ori,
                              const float* const pos, float* const out)
{
    out[0] = (ori->axis[2][0] * pos[2]) + (ori->axis[0][0] * pos[0])
        + (ori->axis[1][0] * pos[1]) + ori->origin[0];
    out[1] = (ori->axis[2][1] * pos[2]) + (ori->axis[0][1] * pos[0])
        + (ori->axis[1][1] * pos[1]) + ori->origin[1];
    out[2] = (ori->axis[2][2] * pos[2]) + (ori->axis[0][2] * pos[0])
        + (ori->axis[1][2] * pos[1]) + ori->origin[2];
}

// ea: 0x00611820
void OrientationDirToWorldDir(const orientation_t* ori,
                              const float* const dir, float* const out)
{
    out[0] = (ori->axis[2][0] * dir[2]) + (ori->axis[0][0] * dir[0])
        + (ori->axis[1][0] * dir[1]);
    out[1] = (ori->axis[2][1] * dir[2]) + (ori->axis[0][1] * dir[0])
        + (ori->axis[1][1] * dir[1]);
    out[2] = (ori->axis[2][2] * dir[2]) + (ori->axis[0][2] * dir[0])
        + (ori->axis[1][2] * dir[1]);
}

// ea: 0x006118B0
void OrientationPosFromWorldPos(const orientation_t* ori,
                                const float* const pos, float* const out)
{
    float v3 = pos[1] - ori->origin[1];
    float v4 = pos[2] - ori->origin[2];
    float v5 = pos[0] - ori->origin[0];
    out[0] = (ori->axis[0][2] * v4) + (ori->axis[0][1] * v3)
        + (ori->axis[0][0] * v5);
    out[1] = (ori->axis[1][2] * v4) + (ori->axis[1][1] * v3)
        + (ori->axis[1][0] * v5);
    out[2] = (ori->axis[2][2] * v4) + (ori->axis[2][1] * v3)
        + (ori->axis[2][0] * v5);
}

// ea: 0x00611960
void OrientationDirFromWorldDir(const orientation_t* ori,
                                const float* const dir, float* const out)
{
    out[0] = (ori->axis[0][2] * dir[2]) + (ori->axis[0][0] * dir[0])
        + (ori->axis[0][1] * dir[1]);
    out[1] = (ori->axis[1][2] * dir[2]) + (ori->axis[1][0] * dir[0])
        + (ori->axis[1][1] * dir[1]);
    out[2] = (ori->axis[2][2] * dir[2]) + (ori->axis[2][0] * dir[0])
        + (ori->axis[2][1] * dir[1]);
}

// ============================================================================
// CGBankManager debug toggles - ea: 0x6119F0..0x611A80
// ============================================================================
struct CGBankManagerLocal {
    void* assetBase;                 // +0x00
    unsigned int mDebugRenderMode;   // +0x04 (bitmask)
    float scale;                     // +0x08 (perf graph zoom)
};

// ea: 0x006119F0
void* ToggleRenderGeom()
{
    CGBankManagerLocal* result = (CGBankManagerLocal*)CGBankManager::sInst;
    result->mDebugRenderMode ^= 1u;
    return result;
}

// ea: 0x00611A10
void* ToggleRenderPerf()
{
    CGBankManagerLocal* result = (CGBankManagerLocal*)CGBankManager::sInst;
    result->mDebugRenderMode ^= 2u;
    return result;
}

// ea: 0x00611A30
void* ToggleGraph()
{
    CGBankManagerLocal* result = (CGBankManagerLocal*)CGBankManager::sInst;
    result->mDebugRenderMode ^= 4u;
    return result;
}

// ea: 0x00611A50
void* ZoomIn()
{
    CGBankManagerLocal* result = (CGBankManagerLocal*)CGBankManager::sInst;
    result->scale -= 25.0f;
    return result;
}

// ea: 0x00611A70
void* ZoomOut()
{
    CGBankManagerLocal* result = (CGBankManagerLocal*)CGBankManager::sInst;
    result->scale += 25.0f;
    return result;
}

// ============================================================================
// Teleport - ea: 0x611A90
// ============================================================================
extern int cmd_argc;              // ?cmd_argc@@3HA (game.o)
extern char* Cmd_Argv(int arg);   // ?Cmd_Argv@@YAPADH@Z (game.o)
extern void TeleportPlayer(Entity* player, const float* origin,
                           const float* angles);  // ?TeleportPlayer (g.o)

// ea: 0x00611A90
void Teleport()
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player == nullptr)
        return;
    int v1 = cmd_argc;
    if (cmd_argc < 3)
        return;
    float pos[3];
    pos[0] = (float)atof(Cmd_Argv(1));
    pos[1] = (float)atof(Cmd_Argv(2));
    pos[2] = v1 <= 3 ? 0.0f : (float)atof(Cmd_Argv(3));
    float angles[3] = { 0.0f, 0.0f, 0.0f };
    TeleportPlayer(Player, pos, angles);
}

// ============================================================================
// Endian helpers - ea: 0x6105A0..0x610730
// The Big*/Little* symbols are C++ wrappers that call through C-linkage
// function pointers (assigned by Swap_Init).
// ============================================================================
struct qint64 {
    unsigned char b0, b1, b2, b3, b4, b5, b6, b7;
};

static short (*BigShortPtr)(short);
static short (*LittleShortPtr)(short);
static int (*BigLongPtr)(int);
static int (*LittleLongPtr)(int);
static qint64 (*BigLong64Ptr)(qint64);
static qint64 (*LittleLong64Ptr)(qint64);
static float (*BigFloatPtr)(float);
static float (*LittleFloatPtr)(float);

// ea: 0x006105A0
short BigShort(short l) { return BigShortPtr(l); }
short LittleShort(short l) { return LittleShortPtr(l); }

// ea: 0x00610610
short ShortSwap(short l)
{
    return (short)(((unsigned short)l << 8) | ((unsigned short)l >> 8));
}

// ea: 0x00610630
short ShortNoSwap(short l)
{
    return l;
}

// ea: 0x00610640
int LongSwap(int l)
{
    return ((unsigned char)l << 24)
        | (((unsigned char)((unsigned int)l >> 8)) << 16)
        | (((unsigned char)((unsigned int)l >> 16)) << 8)
        | ((unsigned int)l >> 24);
}

// ea: 0x00610670
int LongNoSwap(int l)
{
    return l;
}

// ea: 0x00610680
qint64 Long64Swap(qint64 ll)
{
    qint64 r;
    r.b0 = ll.b7;
    r.b1 = ll.b6;
    r.b2 = ll.b5;
    r.b3 = ll.b4;
    r.b4 = ll.b3;
    r.b5 = ll.b2;
    r.b6 = ll.b1;
    r.b7 = ll.b0;
    return r;
}

// ea: 0x006106D0
qint64 Long64NoSwap(qint64 ll)
{
    return ll;
}

// ea: 0x006106E0
float FloatSwap(float f)
{
    unsigned int bits;
    memcpy(&bits, &f, 4);
    bits = ((bits & 0x000000FFu) << 24)
        | ((bits & 0x0000FF00u) << 8)
        | ((bits & 0x00FF0000u) >> 8)
        | ((bits & 0xFF000000u) >> 24);
    memcpy(&f, &bits, 4);
    return f;
}

// ea: 0x00610710
float FloatNoSwap(float f)
{
    return f;
}

// ea: 0x006105B0
int BigLong(int l) { return BigLongPtr(l); }
int LittleLong(int l) { return LittleLongPtr(l); }
// ea: 0x006105C0
qint64 BigLong64(qint64 l) { return BigLong64Ptr(l); }
// ea: 0x006105E0
qint64 LittleLong64(qint64 l) { return LittleLong64Ptr(l); }
// ea: 0x00610600
float BigFloat(float l) { return BigFloatPtr(l); }
float LittleFloat(float l) { return LittleFloatPtr(l); }

// ea: 0x00610720
void Swap_Init()
{
    BigShortPtr = ShortSwap;
    LittleShortPtr = ShortNoSwap;
    BigLongPtr = LongSwap;
    LittleLongPtr = LongNoSwap;
    BigLong64Ptr = Long64Swap;
    LittleLong64Ptr = Long64NoSwap;
    BigFloatPtr = FloatSwap;
    LittleFloatPtr = FloatNoSwap;
}

// ============================================================================
// ParseConfigStringToStruct - ea: 0x629AD0 (q_shared.cpp)
// ============================================================================
// InplaceTree<InplaceString,InplaceString>::Find<const char*> (filesystem.o)
extern InplaceString* InplaceTree_FindStr(const void* tree,
                                          const char* const* key);

// ea: 0x00629AD0
int ParseConfigStringToStruct(
    unsigned char* pStruct, const cspField_t* pFieldList, int iNumFields,
    const ConfigString* pCfgStr, int iMaxFieldTypes,
    int (__cdecl* parseSpecialFieldType)(unsigned char*, const char*,
                                         const int),
    void (__cdecl* parseStrcpy)(unsigned char*, const char*))
{
    int v7 = iNumFields;
    int iField = 0;
    if (iNumFields > 0)
    {
        const InplaceTree<InplaceString, InplaceString>* p_mStringMap =
            &pCfgStr->mStringMap;
        const int* p_iFieldType = &pFieldList->iFieldType;
        do
        {
            const char* szName = *(const char**)(p_iFieldType - 2);
            InplaceString* v10 = InplaceTree_FindStr(p_mStringMap, &szName);
            if (v10 != nullptr)
            {
                const char* mStr = v10->mStr;
                if (mStr != nullptr && *mStr != 0)
                {
                    int v12 = *p_iFieldType;
                    if (*p_iFieldType >= 8)
                    {
                        if (iMaxFieldTypes <= 0 || v12 >= iMaxFieldTypes)
                        {
                            AeAssert::gCurrentAuthor = AeAssert::COD3;
                            AeAssert::gCurrentFile =
                                "c:\\cod\\code\\game\\q_shared.cpp";
                            AeAssert::gCurrentLine = 1079;
                            AeAssert::gCurrentExpr = nullptr;
                            if (!AeAssert::IsIgnored())
                            {
                                const char* v15 =
                                    va("Bad field type %i\n", *p_iFieldType);
                                if (AeAssert::Warning(v15))
                                    __debugbreak();
                            }
                            Com_Error(ERR_DROP, "Bad field type %i\n",
                                      *p_iFieldType);
                        }
                        else
                        {
                            if (parseSpecialFieldType == nullptr)
                            {
                                AeAssert::gCurrentAuthor = AeAssert::COD3;
                                AeAssert::gCurrentFile =
                                    "c:\\cod\\code\\game\\q_shared.cpp";
                                AeAssert::gCurrentLine = 1073;
                                AeAssert::gCurrentExpr =
                                    "parseSpecialFieldType != 0";
                                if (!AeAssert::IsIgnored()
                                    && AeAssert::Assert("old cod assert"))
                                    __debugbreak();
                            }
                            int result = parseSpecialFieldType(
                                pStruct, mStr, *p_iFieldType);
                            if (result == 0)
                                return result;
                        }
                    }
                    else
                    {
                        int offset = *(p_iFieldType - 1);
                        switch (v12)
                        {
                        case 0:
                            parseStrcpy(&pStruct[offset], mStr);
                            break;
                        case 1:
                            Q_strncpyz((char*)&pStruct[offset], mStr, 256);
                            break;
                        case 2:
                        case 3:
                            Q_strncpyz((char*)&pStruct[offset], mStr, 128);
                            break;
                        case 4:
                            *(int*)&pStruct[offset] = atoi(mStr);
                            break;
                        case 5:
                            *(int*)&pStruct[offset] = atoi(mStr) != 0;
                            break;
                        case 6:
                            *(float*)&pStruct[offset] = (float)atof(mStr);
                            break;
                        case 7:
                            *(float*)&pStruct[offset] =
                                (float)(atof(mStr) * 1000.0);
                            break;
                        default:
                            AeAssert::gCurrentAuthor = AeAssert::COD3;
                            AeAssert::gCurrentFile =
                                "c:\\cod\\code\\game\\q_shared.cpp";
                            AeAssert::gCurrentExpr = nullptr;
                            if (v12 >= 0)
                            {
                                AeAssert::gCurrentLine = 1067;
                                if (!AeAssert::IsIgnored()
                                    && AeAssert::Warning(
                                        "ParseConfigStringToStruct is out of sync with the csParseFieldType_t enum list\n"))
                                    __debugbreak();
                            }
                            else
                            {
                                AeAssert::gCurrentLine = 1063;
                                if (!AeAssert::IsIgnored())
                                {
                                    const char* v13 = va(
                                        "Negative field type %i given to ParseConfigStringToStruct\n",
                                        *p_iFieldType);
                                    if (AeAssert::Warning(v13))
                                        __debugbreak();
                                }
                            }
                            break;
                        }
                    }
                }
            }
            v7 = iNumFields;
            p_iFieldType += 3;
            ++iField;
        } while (iField < iNumFields);
    }
    return iField == v7;
}

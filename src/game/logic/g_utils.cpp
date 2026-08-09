// ============================================================================
// g_utils.cpp - game utilities (g.o: g_utils.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// zlib entry points (zlib_xboxr; modern zlib will provide these)
extern "C" int compress2(void* dest, int* destLen, void* src, int sourceLen, int level);
extern "C" int uncompress(void* dest, int* destLen, void* src, int sourceLen);

// core.o (q_math.cpp)
void AngleVectors(const float* angles, float* forward, float* right, float* up);
void VectorInverse(float* v);

// game.o / core.o command system
int  Cmd_Argc(void);
void Cmd_ArgvBuffer(int arg, char* buffer, int bufferLength);

// sv.o
void SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity> entityHandle,
                              const char* text);
void SV_GetConfigstring(int index, char* buffer, int bufferSize);
void SV_SetConfigstring(int index, const char* val);
const char* SV_GetConfigstringConst(int index);

// ea: 0x00460D70
void EntityHandleDb::Init()
{
    mDebugCallback = DebugDumpEnts;
}

// ea: 0x00460EC0
void EntityHandleDb::Find(int fieldOfs, HashString match, ae_sized_array<Entity*, 4096>* results)
{
    EntityHandleDb_Find<HashString>(fieldOfs, match, *results);
}

// ea: 0x00460EE0
void EntityHandleDb::Find(int fieldOfs, unsigned short match,
                          ae_sized_array<Entity*, 4096>* results)
{
    EntityHandleDb_Find<unsigned short>(fieldOfs, match, *results);
}

// ea: 0x00454C80
Entity** EntityHandleDb::Find(int fieldofs, unsigned short match,
                              Entity** begin, Entity** end)
{
    Entity** i = begin;
    for (; i != end; ++i)
    {
        if (*i != nullptr)
        {
            int16_t v7 = *(int16_t*)((char*)&(*i)->s.eType + fieldofs);
            if (v7 != 0 && v7 == match)
                break;
        }
    }
    return i;
}

// ea: 0x00454CC0
Entity** EntityHandleDb::Find(int fieldofs, HashString match,
                              Entity** begin, Entity** end)
{
    Entity** i = begin;
    for (; i != end; ++i)
    {
        if (*i != nullptr)
        {
            int v7 = *(int*)((char*)&(*i)->s.eType + fieldofs);
            if (v7 != 0 && v7 == (int)match.mHash)
                break;
        }
    }
    return i;
}

// ea: 0x00466460
void EntityHandleDb::Release(Entity* e)
{
    if (e->mHandle.mHandle.mVal != 0)
    {
        unsigned int idx = e->mHandle.mHandle.mVal & 0xFFF;
        if (idx < 0x540)
            EntityHandleDb::sInst.mElements[idx].mKey = 0;  // ReleaseHandle
        mActiveList.m_elements[e->mEntityArrayIndex] = nullptr;
    }
}

// ea: 0x00460D80
Entity* EntityHandleDb::Find(int fieldofs, unsigned short match)
{
    Entity** begin = mActiveList.m_elements;
    Entity** end = begin + mActiveList.m_size;
    Entity** result = Find(fieldofs, match, begin, end);
    if (result == end)
        return nullptr;
    return *result;
}

// ea: 0x00460DD0
Entity* EntityHandleDb::Find(int fieldofs, HashString match)
{
    Entity** begin = mActiveList.m_elements;
    Entity** end = begin + mActiveList.m_size;
    Entity** i = begin;
    for (; i != end; ++i)
    {
        if (*i != nullptr)
        {
            int v6 = *(int*)((char*)&(*i)->s.eType + fieldofs);
            if (v6 != 0 && v6 == (int)match.mHash)
                break;
        }
    }
    if (i == end)
        return nullptr;
    return *i;
}

// ea: 0x00460E30
Entity* EntityHandleDb::Find(int fieldofs, const Broc::string& match)
{
    Entity** begin = mActiveList.m_elements;
    Entity** end = begin + mActiveList.m_size;
    Entity** i = begin;
    for (; i != end; ++i)
    {
        if (*i != nullptr)
        {
            int v6 = *(int*)((char*)&(*i)->s.eType + fieldofs);
            if (v6 != 0 && v6 == (int)HashString::CalcHash(match.c_str()))
                break;
        }
    }
    if (i == end)
        return nullptr;
    return *i;
}

// ea: 0x00454BC0
Entity** EntityHandleDb::Find(int fieldofs, const Broc::string& match,
                              Entity** begin, Entity** end)
{
    Entity** m_ptr = begin;
    for (; m_ptr != end; ++m_ptr)
    {
        if (*m_ptr != nullptr)
        {
            Broc::string s = *(Broc::string*)((char*)&(*m_ptr)->s.eType + fieldofs);
            if (s.mBlock != nullptr
                && s.mBlock != (Broc::string::Block*)-12
                && s.c_str() != nullptr
                && s.c_str()[0] != 0
                && strcmp(s.c_str(), match.c_str()) == 0)
            {
                break;
            }
        }
    }
    return m_ptr;
}

// ea: 0x00454B00
void EntityHandleDb::Compact()
{
    unsigned int v1 = 0;
    if (mActiveList.m_size > 0)
    {
        do
        {
            if (v1 >= 0x1000)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 154;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            if (mActiveList.m_elements[v1] == nullptr)
            {
                int m_size = mActiveList.m_size;
                if (m_size != 0)
                    mActiveList.m_size = m_size - 1;
                Entity* v4 = mActiveList.m_elements[mActiveList.m_size];
                if (v4 != nullptr)
                {
                    v4->mEntityArrayIndex = (int16_t)v1;
                    mActiveList.m_elements[v1] = v4;
                }
                else
                {
                    --v1;
                }
            }
            ++v1;
        } while (v1 < (unsigned int)mActiveList.m_size);
    }
}

// ea: 0x00466350
void EntityHandleDb::Validate()
{
    for (int idx = 0; idx < 0x540; ++idx)
    {
        Entity* mObject = mElements[idx].mObject;
        if (mObject == nullptr)
            continue;
        bool found = false;
        for (int i = 0; i < mActiveList.m_size; ++i)
        {
            if (mActiveList.m_elements[i] == mObject)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EntityHandleDb.cpp";
            AeAssert::gCurrentLine = 85;
            AeAssert::gCurrentExpr = nullptr;
            if (AeAssert::Assert("unable to find entity in entity list\n"))
                __debugbreak();
        }
    }
}

// ea: 0x0044A240
int CheatsOk(Entity* ent)
{
    if (g_cheats->integer == 0)
    {
        SV_GameSendServerCommand(ent->mHandle, va("print \"GAME_CHEATSNOTENABLED\""));
        return 0;
    }
    if (ent->health <= 0)
    {
        SV_GameSendServerCommand(ent->mHandle, va("print \"GAME_MUSTBEALIVECOMMAND\""));
        return 0;
    }
    return 1;
}

// ea: 0x0044A290
char* ConcatArgs(int start)
{
    int v1 = 0;
    int v2 = start;
    int c = Cmd_Argc();
    if (start >= c)
    {
        line[0] = 0;
        return line;
    }
    do
    {
        char arg[256];
        Cmd_ArgvBuffer(v2, arg, 256);
        unsigned int v3 = (unsigned int)strlen(arg);
        int v4 = v3 + v1;
        if (v3 + v1 >= 255)
            break;
        memcpy(&line[v1], arg, v3);
        int v5 = c;
        v1 += v3;
        if (v2 != c - 1)
        {
            line[v4] = ' ';
            v1 = v4 + 1;
        }
        ++v2;
    } while (v2 < c);
    line[v1] = 0;
    return line;
}

// ea: 0x0044A340
void SanitizeString(char* in, char* out)
{
    char* v2 = in;
    char v3 = *in;
    if (*in != 0)
    {
        char* v4 = out;
        do
        {
            if (v3 == 27)
            {
                v2 += 2;
            }
            else
            {
                if (v3 >= 32)
                    *v4++ = (char)tolower(v3);
                ++v2;
            }
            v3 = *v2;
        } while (*v2 != 0);
        *v4 = 0;
    }
    else
    {
        *out = 0;
    }
}

// ea: 0x0044AF50
unsigned int G_GetHitLocationString(hitLocation_t hitLoc)
{
    if (hitLoc >= HITLOC_NUM)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_combat.cpp";
        AeAssert::gCurrentLine = 2153;
        AeAssert::gCurrentExpr = "(unsigned) hitLoc < HITLOC_NUM";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return g_HitLocConstNames[hitLoc];
}

// ea: 0x0044B760
void G_Printf(const char* fmt, ...)
{
    char text[1024];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);
    Com_Printf(text);
}

// ea: 0x0044B790
void G_DPrintf(const char* fmt, ...)
{
    char text[1024];
    va_list ap;
    va_start(ap, fmt);
    if (g_developer->integer != 0)
    {
        vsprintf(text, fmt, ap);
        Com_Printf(text);
    }
    va_end(ap);
}

// ea: 0x0044B7D0
void G_Error(const char* fmt, ...)
{
    char text[1024];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_main.cpp";
    AeAssert::gCurrentLine = 847;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored() && AeAssert::Warning(text))
        __debugbreak();
}

// ea: 0x0044B840
void G_Error_Localized(const char* fmt, ...)
{
    char text[1024];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_main.cpp";
    AeAssert::gCurrentLine = 860;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored() && AeAssert::Warning(text))
        __debugbreak();
}

// ea: 0x0044C6E0
void G_CreateRotationMatrix(const float* angles, float (*matrix)[3])
{
    AngleVectors(angles, *matrix, &(*matrix)[3], &(*matrix)[6]);
    VectorInverse(&(*matrix)[3]);
}

// ea: 0x0044C710
void G_TransposeMatrix(float (*matrix)[3], float (*transpose)[3])
{
    for (int i = 0; i < 3; ++i)
    {
        transpose[i][0] = matrix[0][i];
        transpose[i][1] = matrix[1][i];
        transpose[i][2] = matrix[2][i];
    }
}

// ea: 0x0044C750
void G_RotatePoint(math::Position3* point, float (*matrix)[3])
{
    float x = point->v.m128_f32[0];
    float y = point->v.m128_f32[1];
    float z = point->v.m128_f32[2];
    point->v.m128_f32[0] = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * z;
    point->v.m128_f32[1] = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * z;
    point->v.m128_f32[2] = matrix[2][0] * x + matrix[2][1] * y + matrix[2][2] * z;
}

// ea: 0x0044CE40
int G_GetHintStringIndex(int* piIndex, const char* pszString)
{
    int v2 = 0;
    while (1)
    {
        char szConfigString[256];
        SV_GetConfigstring(v2 + 628, szConfigString, 256);
        if (szConfigString[0] == 0)
        {
            SV_SetConfigstring(v2 + 628, pszString);
            *piIndex = v2;
            return 1;
        }
        if (strcmp(pszString, szConfigString) == 0)
            break;
        if (++v2 >= 32)
        {
            *piIndex = -1;
            return 0;
        }
    }
    *piIndex = v2;
    return 1;
}

// ea: 0x00450240
unsigned short G_NewString(const char* /*str*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 577;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("Dead Code CD"))
        __debugbreak();
    return 0;
}

// ea: 0x004508C0
int PASSFLOAT(float x)
{
    return *(int*)&x;
}

// ea: 0x00451420
int g_CompressZLIB(void* dest, int* destLen, void* src, int sourceLen)
{
    return compress2(dest, destLen, src, sourceLen, 1);
}

// ea: 0x00451440
int g_UnCompressZLIB(void* dest, int* destLen, void* src, int sourceLen)
{
    return uncompress(dest, destLen, src, sourceLen);
}

// ea: 0x00453700
int G_FindConfigstringIndex(const char* name, int start, int max, int create, const char* errormsg)
{
    if (g_debug_sound_aliases->integer == 2 && start == 161)
    {
        for (int i = 1; i < max; ++i)
        {
            const char* ConfigstringConst = SV_GetConfigstringConst(i + 161);
            if (*ConfigstringConst != 0)
                G_Printf("^1SOUND ALIAS %d %s\n", i, ConfigstringConst);
        }
        Cvar_Set("g_debug_sound_aliases", "1");
    }
    if (name == nullptr || *name == 0)
        return 0;
    int j;
    for (j = 1; j < max; ++j)
    {
        const char* v8 = SV_GetConfigstringConst(j + start);
        if (*v8 == 0)
            break;
        if (_stricmp(v8, name) == 0)
            return j;
    }
    if (g_debug_sound_aliases->integer == 1 && start == 161)
        G_Printf("^1SOUND ALIAS %d %s\n", j, name);
    if (create == 0)
    {
        if (errormsg != nullptr)
            Scr_Error(va("%s \"%s\" not precached", errormsg, name));
        return 0;
    }
    if (j == max)
    {
        G_Printf("^1=============================================================================\n");
        if (start > 561)
        {
            if (start == 660)
                G_Printf("^1", "LOCALIZED_STRINGS\n");
            else if (start == 724)
                G_Printf("^1SERVER_SHADERS\n");
        }
        else
        {
            switch (start)
            {
            case 561:
                G_Printf("^1SHELLSHOCKS\n");
                break;
            case 33:
                G_Printf("^1MODELS\n");
                break;
            case 161:
                G_Printf("^1SOUND ALIASES\n");
                break;
            case 225:
                G_Printf("^1EFFECT NAMES\n");
                break;
            default:
                break;
            }
        }
        G_Printf("^1 >>> G_FindConfigstringIndex: overflow %s failed <<<\n", name);
        G_Printf("^1=============================================================================\n");
        return 0;
    }
    else
    {
        SV_SetConfigstring(j + start, name);
        if (start == 33 || start == 225)
            level.newAssetLoaded = 1;
        return j;
    }
}

// ea: 0x004538F0
int G_LocalizedStringIndex(const char* string)
{
    if (string == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 181;
        AeAssert::gCurrentExpr = "string";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (*string != 0)
        return G_FindConfigstringIndex(string, 660, 64, 1, "localized string");
    else
        return 0;
}

// ea: 0x00453960
int G_ShaderIndex(const char* name)
{
    if (name == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 196;
        AeAssert::gCurrentExpr = "name";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (*name == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 197;
        AeAssert::gCurrentExpr = "name[0]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return G_FindConfigstringIndex(name, 724, 256, level.initializing, "shader");
}

// ea: 0x00453A20
int G_EffectIndex(const char* name)
{
    if (name == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 283;
        AeAssert::gCurrentExpr = "name";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return G_FindConfigstringIndex(name, 225, 80, level.initializing, "effect");
}

// ea: 0x00453A90
int G_ShellShockIndex(const char* name)
{
    if (name == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 288;
        AeAssert::gCurrentExpr = "name";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return G_FindConfigstringIndex(name, 561, 2, 1, nullptr);
}

// ea: 0x00453B00
unsigned char G_SoundAliasIndex(const char* name)
{
    if (name == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 294;
        AeAssert::gCurrentExpr = "name";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return (unsigned char)G_FindConfigstringIndex(name, 161, 64, 1, nullptr);
}

// ea: 0x00453B70
unsigned int G_GetGameId(Entity* ent)
{
    return ent->mHandle.mHandle.mVal;
}

static char  str[8][32];
static int   index_;
static char  str_0[8][32];
static int   index_0;

// ea: 0x00454080
char* vtos(const float* v)
{
    char* v1 = str[index_];
    index_ = (index_ + 1) & 7;
    Com_sprintf(v1, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);
    return v1;
}

// ea: 0x004540D0
char* vtos(const math::Position3* v)
{
    char* v1 = str_0[index_0];
    index_0 = (index_0 + 1) & 7;
    Com_sprintf(v1, 32, "(%i %i %i)", (int)v->v.m128_f32[0], (int)v->v.m128_f32[1], (int)v->v.m128_f32[2]);
    return v1;
}

// ea: 0x00454A00
void G_SetConstString(unsigned short* /*cs*/, const char* /*str*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
    AeAssert::gCurrentLine = 2272;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("Is this still used? (CD)"))
        __debugbreak();
}

// ea: 0x00454A50
const char* G_GetEntityTypeName(Entity* ent)
{
    if (ent->s.eType >= 0x12u)
        return "WARNING !! Entity Type Unknown WARNING !!!";
    else
        return entityTypeNames[ent->s.eType];
}

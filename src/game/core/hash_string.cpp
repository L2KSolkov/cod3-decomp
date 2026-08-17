// ============================================================================
// hash_string.cpp - HashString + language helpers (core.o)
// ============================================================================

#include "core/ae_fixed_string.h"
#include "game/core/core_systems.h"
#include "game/core/core_globals.h"

#include <string.h>

extern unsigned int AeHash(const char* str);

struct LanguageName {
    const char* first;
    ELanguage second;
};
LanguageName sLanguageNames[6] = {
    {"English", kLanguageEnglish}, {"German", kLanguageGerman},
    {"French", kLanguageFrench}, {"Spanish", kLanguageSpanish},
    {"Italian", kLanguageItalian}, {"Unlocalized", kLanguageJapanese},
};
const char* sLanguageIds[6] = {"en", "de", "fr", "sp", "it", "un"};
int dword_DD8B74[12] = {};

// ea: 0x004BD6B0
unsigned int CalcHash(const char* str)
{
    return AeHash(str);
}

// ea: 0x004BD6C0
const char* GetLanguageName(ELanguage l)
{
    return sLanguageNames[l].first;
}

// ea: 0x004BD6D0
const char* GetLanguageId(ELanguage l)
{
    return sLanguageIds[l];
}

// ea: 0x004C1490
void SetGameLanguage(
    const ae_fixed_string<1024, unsigned short>& sysCmdLine)
{
    int v1 = 0;
    while (strstr((const char*)sysCmdLine.mBuff, sLanguageNames[v1].first)
           == nullptr)
    {
        if (++v1 >= 6)
            return;
    }
    gLanguage = (ELanguage)dword_DD8B74[2 * v1];
}

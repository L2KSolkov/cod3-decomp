// ============================================================================
// stringed_hooks.cpp - localized string-ed language layer (shell.o)
// ============================================================================

#include "game/shell/shell_types.h"

#include <set>
#include <stdio.h>
#include <string.h>

extern char sString[128];
extern char sString_1[128];

extern cvar_t* Cvar_Get(const char* var_name, const char* var_value,
                        int flags);  // core.o
extern void Cvar_Set(const char* var_name, const char* value);  // core.o
extern void Com_Error(errorParm_t code, const char* fmt, ...);  // core.o
extern void Com_Printf(const char* fmt, ...);  // core.o
extern char* va(const char* fmt, ...);         // core.o
extern int Q_stricmp(const char* s1, const char* s2);  // core.o
extern void Q_strncpyz(char* dest, const char* src, int destsize);  // core.o
extern int FS_ReadFile(const char* qpath, void** buffer);  // core.o
extern void FS_FreeFile(void* buffer);                     // core.o
extern char** FS_ListFiles(const char* path, const char* extension,
                           int* numfiles);  // core.o
extern void FS_FreeFileList(char** list);   // core.o
extern int SEH_GetCurrentLanguage();                       // this file
extern const char* const defaultFileName;  // 0xCD67AE

namespace AeAssert {
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A
    const char* GetSTBString(const char* pszReference);   // core.o
    const char* GetSTBString(unsigned int hash);          // core.o
    const char* GetSTBString(TPakId pakId, unsigned int hash);  // core.o
};

enum msgLocErrType_t {
    LOCMSG_NOERR = 0,
    LOCMSG_ERR = 1,
};

// ============================================================================
// shell.o data (verified VAs from IDA)
// ============================================================================
struct languageInfo_t {
    const char* pszName;  // +0x00
    int         bPresent; // +0x04
};
static_assert(sizeof(languageInfo_t) == 8, "languageInfo_t size mismatch");

languageInfo_t g_languages[14] = {
    {"english", 1},  {"french", 0},  {"german", 0},   {"italian", 0},
    {"spanish", 0},  {"british", 0}, {"russian", 0},  {"polish", 0},
    {"korean", 0},   {"taiwanese", 0}, {"japanese", 0}, {"chinese", 0},
    {"thai", 0},     {"leet", 0},
};

cvar_t* cl_language = nullptr;              // 0xF15780
cvar_t* cl_languagesavailable = nullptr;    // 0xF0D1BC
cvar_t* cl_languagetranslate = nullptr;     // 0xF0CFCC
extern cvar_t* cl_languagewarnings;         // cl_console.cpp
extern cvar_t* cl_languagewarningsaserrors; // cl_console.cpp
int     g_currentAsian = 0;                 // 0xF30D50
int     giFilesFound = 0;                   // 0xF30AF0
char    szErrorString[256];                 // 0xF39F40
char    szStrings[2][256];                  // 0xF3A078
int     iCurrString = 0;                    // 0xF3A2E0
char    sTemp[128];                         // 0xF3A4D8
CStringEdPackage TheStringPackage;          // 0xF30D90
std::vector<std::string> gvLanguagesAvailable;  // 0xF3123C

// ============================================================================
// language cvar helpers
// ============================================================================

// ea: 0x00576100
void SEH_UpdateCurrentLanguage()
{
    if (cl_language == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\stringed_hooks.cpp";
        AeAssert::gCurrentLine = 71;
        AeAssert::gCurrentExpr = "cl_language";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Cvar_Get("cl_language", "0", 33);
    int result = cl_language->integer;
    if (result < 8 || result > 12)
        g_currentAsian = 0;
    else
        g_currentAsian = 1;
    (void)result;
}

// ea: 0x00576190
int SEH_GetCurrentLanguage()
{
    cvar_t* v0 = cl_language;
    if (cl_language != nullptr)
        return v0->integer;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\stringed_hooks.cpp";
    AeAssert::gCurrentLine = 101;
    AeAssert::gCurrentExpr = "cl_language";
    if (AeAssert::IsIgnored())
        return cl_language->integer;
    if (AeAssert::Assert("old cod assert"))
    {
        __debugbreak();
        v0 = cl_language;
        return v0->integer;
    }
    return cl_language->integer;
}

// ea: 0x00576200
const char* SEH_GetLanguageName(int iLanguage)
{
    if (iLanguage > 0xD)
        return g_languages[0].pszName;
    return g_languages[iLanguage].pszName;
}

// ea: 0x00576220
int SEH_GetLanguageIndexForName(const char* pszLanguageName,
                                int* piLanguageIndex)
{
    int v2 = 0;
    while (Q_stricmp(pszLanguageName, g_languages[v2].pszName) != 0)
    {
        if (++v2 >= 14)
        {
            *piLanguageIndex = 0;
            return 0;
        }
    }
    *piLanguageIndex = v2;
    return 1;
}

// ea: 0x00576270
void SEH_InitLanguage()
{
    cl_language = Cvar_Get("cl_language", "0", 33);
    cl_languagesavailable = Cvar_Get("cl_languagesavailable", "0", 0);
    cl_languagetranslate = Cvar_Get("cl_languagetranslate", "1", 32);
    cl_languagewarnings = Cvar_Get("cl_languagewarnings", "0", 0);
    cl_languagewarningsaserrors =
        Cvar_Get("cl_languagewarningsaserrors", "0", 0);
    SEH_UpdateCurrentLanguage();
}

// ea: 0x005762F0
int SEH_VerifyLanguageSelection(int iLanguageSelection)
{
    if (g_languages[iLanguageSelection].bPresent != 0)
        return iLanguageSelection;
    int v2 = 0;
    while (g_languages[(v2 + iLanguageSelection) % 14].bPresent == 0)
    {
        if (++v2 >= 14)
            return 0;
    }
    return (v2 + iLanguageSelection) % 14;
}

// ============================================================================
// string lookup
// ============================================================================

// ea: 0x00576350
const char* SEH_StringEd_GetString(const char* pszReference)
{
    if (cl_languagetranslate == nullptr || cl_languagetranslate->integer == 0)
        return pszReference;
    if (*pszReference != 0 && pszReference[1] != 0)
        return STBManager::sInst->GetSTBString(pszReference);
    return pszReference;
}

// ea: 0x00576390
const char* SEH_StringEd_GetString(unsigned int hash)
{
    return STBManager::sInst->GetSTBString(hash);
}

// ea: 0x005763B0
const char* SEH_StringEd_GetString(TPakId pakId, unsigned int hash)
{
    return STBManager::sInst->GetSTBString(pakId, hash);
}

// ea: 0x005763D0
const char* SEH_SafeTranslateString(const char* pszReference)
{
    const char* v1;
    const char* result;
    if (cl_languagetranslate != nullptr
        && cl_languagetranslate->integer != 0)
    {
        v1 = pszReference;
        if (*pszReference != 0 && pszReference[1] != 0)
        {
            result = STBManager::sInst->GetSTBString(pszReference);
            goto LABEL_8;
        }
    }
    else
    {
        v1 = pszReference;
    }
    result = v1;
LABEL_8:
    if (result == nullptr)
    {
        if (cl_languagewarnings->integer != 0)
        {
            if (cl_languagewarningsaserrors->integer != 0)
                Com_Error(ERR_LOCALIZATION,
                          "Could not translate exe string \"%s\"", v1);
            else
                Com_Printf("^3WARNING: Could not translate exe string \"%s\"\n",
                           v1);
            strcpy(szErrorString, "^1UNLOCALIZED(^7");
            strcat(szErrorString, v1);
            strcat(szErrorString, "^1)^7");
            return szErrorString;
        }
        else
        {
            strcpy(szErrorString, v1);
            return szErrorString;
        }
    }
    return result;
}

// ea: 0x00576500
int SEH_GetLocalizedTokenReference(char* pszToken, const char* pszReference,
                                   const char* pszMessageType,
                                   msgLocErrType_t errType)
{
    const char* v4;
    char* STBString;
    if (cl_languagetranslate == nullptr
        || cl_languagetranslate->integer == 0)
    {
        v4 = pszReference;
        STBString = (char*)v4;
    }
    else
    {
        v4 = pszReference;
        if (*pszReference == 0 || pszReference[1] == 0)
        {
            STBString = (char*)v4;
        }
        else
        {
            STBString = (char*)STBManager::sInst->GetSTBString(pszReference);
        }
    }
    if (STBString == nullptr)
    {
        if (cl_languagewarnings != nullptr
            && cl_languagewarnings->integer != 0)
        {
            if (cl_languagewarningsaserrors != nullptr
                && cl_languagewarningsaserrors->integer != 0
                && errType != LOCMSG_NOERR)
            {
                Com_Error(ERR_LOCALIZATION,
                          "Could not translate part of %s: \"%s\"",
                          pszMessageType, v4);
                STBString = va("^1UNLOCALIZED(^7%s^1)^7", v4);
            }
            else
            {
                Com_Printf("^3WARNING: Could not translate part of %s: \"%s\"\n",
                           pszMessageType, v4);
                STBString = va("^1UNLOCALIZED(^7%s^1)^7", v4);
            }
        }
        else
        {
            STBString = va("%s", v4);
        }
        if (errType == LOCMSG_NOERR)
            return 0;
    }
    strcpy(pszToken, STBString);
    return 1;
}

// ea: 0x005765D0
const char* SEH_LocalizeTextMessage(const char* pszInputBuffer,
                                    const char* pszMessageType,
                                    msgLocErrType_t errType)
{
    iCurrString = (iCurrString + 1) % 2;
    char* v3 = szStrings[iCurrString];
    memset(v3, 0, 0x100u);
    const char* v4 = pszInputBuffer;
    int v5 = 0;
    int v7 = 1;
    const char* v8 = pszInputBuffer;
    char* v33 = v3;
    int iLen = 0;
    int bLocOn = 1;
    int iInsertLevel = 0;
    int bLocSkipped = 0;
    const char* pszIn = pszInputBuffer;
    char szInsertBuf[256];
    char szTokenBuf[256];
    if (*pszInputBuffer == 0)
        return v3;
    while (1)
    {
        char v9 = *v8;
        if (!*v8 || v9 == 20 || v9 == 21 || v9 == 22)
            break;
        pszIn = ++v8;
    LABEL_57:
        if (!*v4)
        {
            if (bLocSkipped)
            {
                int v26 = iLen;
                for (int i = 0; i < v26; ++i)
                {
                    if (v3[i] == 22)
                        v3[i] = 37;
                }
            }
            return v3;
        }
    }
    if (v8 > v4)
    {
        int v10 = (int)(v8 - v4);
        Q_strncpyz(szTokenBuf, v4, v10 + 1);
        if (bLocOn)
        {
            if (!SEH_GetLocalizedTokenReference(szTokenBuf, szTokenBuf,
                                                pszMessageType, errType))
                return nullptr;
            v10 = (int)strlen(szTokenBuf);
        }
        if (v10 + iLen >= 0x100)
        {
            if (cl_languagewarnings
                && cl_languagewarnings->integer
                && cl_languagewarningsaserrors
                && cl_languagewarningsaserrors->integer
                && errType != LOCMSG_NOERR)
            {
                Com_Error(ERR_DROP,
                          "%s too long when translated: \"%s\"",
                          pszMessageType, pszInputBuffer);
            }
            Com_Printf("%s too long when translated: \"%s\"\n",
                       pszMessageType, pszInputBuffer);
        }
        for (int j = 0; j < (v10 - 1); ++j)
        {
            if (szTokenBuf[j] == 37 && szTokenBuf[j + 1] == 115)
            {
                if (!v7 || v5)
                {
                    szTokenBuf[j] = 22;
                    bLocSkipped = 1;
                }
                else
                {
                    ++iInsertLevel;
                    v5 = 1;
                }
            }
        }
        if (iInsertLevel == v5)
        {
            char* v22 = szTokenBuf;
            char v24;
            do
            {
                v24 = *v22;
                v22[(v33 + iLen) - v22] = *v22;
                ++v22;
            }
            while (v24);
            iLen = v10 + iLen;
        }
        else
        {
            int v12 = 0;
            char* v13;
            if (iLen - 1 <= 0)
            {
                v13 = v33;
            }
            else
            {
                while (1)
                {
                    v13 = v33;
                    if (v33[v12] == 37 && v33[v12 + 1] == 115)
                        break;
                    if (++v12 >= iLen - 1)
                        goto LABEL_36;
                }
                {
                    char* v14 = &v33[v12 + 2];
                    char v16;
                    char* dst = szInsertBuf;
                    do
                    {
                        v16 = *v14;
                        *dst++ = *v14;
                        ++v14;
                    }
                    while (v16);
                    v13[v12] = 0;
                }
            LABEL_36:
                if (v12 < 0)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\stringed_hooks.cpp";
                    AeAssert::gCurrentLine = 544;
                    AeAssert::gCurrentExpr = "i >= 0";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
            }
            strcpy(&v13[v12], szTokenBuf);
            strcat(&v13[v12 + v10], szInsertBuf);
            iLen -= 2;
            --iInsertLevel;
        }
        v3 = v33;
        v5 = 0;
        int v25 = v10 + iLen;
        v8 = pszIn;
        iLen = v25;
    }
    v7 = 1;
    if (*v8 == 20)
    {
        bLocOn = 1;
    }
    else
    {
        if (*v8 != 21)
        {
        LABEL_54:
            if (*v8 == 22)
            {
                v7 = 0;
                pszIn = ++v8;
            }
            v4 = v8;
            goto LABEL_57;
        }
        bLocOn = 0;
    }
    pszIn = ++v8;
    goto LABEL_54;
}

// ============================================================================
// file load helpers
// ============================================================================

// ea: 0x00576F10
unsigned char* SE_LoadFileData(const char* psFileName, int* piLoadedLength)
{
    if (piLoadedLength != nullptr)
        *piLoadedLength = 0;
    void* buffer = nullptr;
    int File = FS_ReadFile(psFileName, &buffer);
    if (File <= 0)
        return nullptr;
    unsigned char* result = (unsigned char*)buffer;
    if (piLoadedLength != nullptr)
        *piLoadedLength = File;
    return result;
}

// ea: 0x00576F50
void SE_FreeFileDataAfterLoad(unsigned char* psLoadedFile)
{
    if (psLoadedFile != nullptr)
        FS_FreeFile(psLoadedFile);
}

// ============================================================================
// CJK-aware text walkers
// ============================================================================

// shell.o 0x5768D0 (not in map; internal helpers)
static bool Korean_ValidKSC5601Hangul(unsigned char _iHi, unsigned char _iLo)
{
    return _iHi >= 0xB0 && _iHi <= 0xC8 && _iLo > 0xA0 && _iLo != 0xFF;
}
static bool Taiwanese_ValidBig5Code(unsigned short uiCode)
{
    return (((uiCode >> 8) & 0xFF) >= 0xA1 && ((uiCode >> 8) & 0xFF) <= 0xC6
            || ((uiCode >> 8) & 0xFF) >= 0xC9
                && ((uiCode >> 8) & 0xFF) <= 0xF9)
        && ((uiCode >= 0x40 && uiCode <= 0x7E)
            || (uiCode >= 0xA1 && uiCode != 0xFF));
}
static bool Taiwanese_IsTrailingPunctuation(unsigned int uiCode)
{
    return uiCode >= 0xA140 && uiCode < 0xA154;
}
static bool Japanese_ValidShiftJISCode(unsigned char _iHi, char _iLo)
{
    return (_iHi >= 0x81 && _iHi <= 0x9F || _iHi >= 0xE0 && _iHi <= 0xEF)
        && (_iLo >= 0x40 && _iLo <= 0x7E || _iLo <= -4);
}
static bool Japanese_IsTrailingPunctuation(unsigned int uiCode)
{
    return uiCode >= 0x8140 && uiCode < 0x8152;
}
static bool Chinese_ValidGBCode_UICode(unsigned short uiCode)
{
    return ((uiCode >> 8) & 0xFF) >= 0xA1 && ((uiCode >> 8) & 0xFF) <= 0xF7
        && uiCode > 0xA0 && uiCode != 0xFF;
}
static bool Chinese_IsTrailingPunctuation(unsigned int uiCode)
{
    return uiCode > 0xA1A0 && uiCode < 0xA1AE;
}

// ea: 0x005810A0
unsigned int SEH_ReadCharFromString(const char** ppsText,
                                    int* pbIsTrailingPunctuation)
{
    const unsigned char* v2 = (const unsigned char*)*ppsText;
    if (g_currentAsian != 0)
    {
        switch (SEH_GetCurrentLanguage())
        {
        case 8:
            if (!Korean_ValidKSC5601Hangul(v2[0], v2[1]))
                goto LABEL_18;
            {
                unsigned int result = v2[1] + (v2[0] << 8);
                *ppsText += 2;
                if (pbIsTrailingPunctuation != nullptr)
                    *pbIsTrailingPunctuation = 0;
                return result;
            }
        case 9:
            if (!Taiwanese_ValidBig5Code((unsigned short)((v2[0] << 8) + v2[1])))
                goto LABEL_18;
            {
                unsigned int result = (v2[0] << 8) + v2[1];
                *ppsText += 2;
                if (pbIsTrailingPunctuation != nullptr)
                    *pbIsTrailingPunctuation =
                        Taiwanese_IsTrailingPunctuation(result);
                return result;
            }
        case 10:
            if (!Japanese_ValidShiftJISCode(v2[0], (char)v2[1]))
                goto LABEL_18;
            {
                unsigned int result = v2[1] + (v2[0] << 8);
                *ppsText += 2;
                if (pbIsTrailingPunctuation != nullptr)
                    *pbIsTrailingPunctuation =
                        Japanese_IsTrailingPunctuation(result);
                return result;
            }
        case 11:
            if (!Chinese_ValidGBCode_UICode((unsigned short)(v2[1] + (v2[0] << 8))))
                goto LABEL_18;
            {
                unsigned int result = (v2[0] << 8) + v2[1];
                *ppsText += 2;
                if (pbIsTrailingPunctuation != nullptr)
                    *pbIsTrailingPunctuation =
                        Chinese_IsTrailingPunctuation(result);
                return result;
            }
        default:
            goto LABEL_18;
        }
    }
    else
    {
    LABEL_18:
        {
            unsigned int result = *v2;
            ++*ppsText;
            if (pbIsTrailingPunctuation != nullptr)
                *pbIsTrailingPunctuation =
                    result == '!' || result == '?' || result == ','
                    || result == '.' || result == ';' || result == ':';
            return result;
        }
    }
}

// ea: 0x00581200
int SEH_PrintStrlen(const char* string)
{
    const char* v1 = string;
    if (string == nullptr)
        return 0;
    int v3 = 0;
    unsigned int v6 = 0;
    if (*string != 0)
    {
        while (1)
        {
            if (g_currentAsian != 0)
            {
                switch (SEH_GetCurrentLanguage())
                {
                case 8:
                    if (*v1 < 0xB0 || (unsigned char)*v1 > 0xC8
                        || (unsigned char)v1[1] <= 0xA0
                        || (unsigned char)v1[1] == 0xFF)
                        goto LABEL_33;
                    v6 = (unsigned char)v1[1] + ((unsigned char)*v1 << 8);
                    v1 += 2;
                    break;
                case 9:
                    v6 = ((unsigned char)*v1 << 8) + (unsigned char)v1[1];
                    if ((((v6 >> 8) & 0xFF) < 0xA1 || ((v6 >> 8) & 0xFF) > 0xC6)
                            && (((v6 >> 8) & 0xFF) < 0xC9
                                || ((v6 >> 8) & 0xFF) > 0xF9)
                        || ((unsigned char)v1[1] < 0x40
                            || (unsigned char)v1[1] > 0x7E)
                            && ((unsigned char)v1[1] < 0xA1
                                || (unsigned char)v1[1] == 0xFF))
                    {
                        goto LABEL_33;
                    }
                    v1 += 2;
                    break;
                case 10:
                    if ((*v1 < 0x81 || (unsigned char)*v1 > 0x9F)
                            && ((unsigned char)*v1 < 0xE0
                                || (unsigned char)*v1 > 0xEF)
                        || ((signed char)v1[1] < 0x40
                            || (signed char)v1[1] > 0x7E)
                            && (signed char)v1[1] > -4)
                    {
                        goto LABEL_33;
                    }
                    v6 = (unsigned char)v1[1] + ((unsigned char)*v1 << 8);
                    v1 += 2;
                    break;
                case 11:
                    v6 = ((unsigned char)*v1 << 8) + (unsigned char)v1[1];
                    if (((v6 >> 8) & 0xFF) < 0xA1 || ((v6 >> 8) & 0xFF) > 0xF7
                        || (unsigned char)v1[1] <= 0xA0
                        || (unsigned char)v1[1] == 0xFF)
                    {
                        goto LABEL_33;
                    }
                    v1 += 2;
                    break;
                default:
                    goto LABEL_33;
                }
            }
            else
            {
            LABEL_33:
                v6 = (unsigned char)*v1++;
            }
            if (v6 != 94)
                break;
            if (v1 == nullptr)
                goto LABEL_42;
            char v9 = *v1;
            if (*v1 == 94 || v9 < 48 || v9 > 57)
                goto LABEL_42;
            ++v1;
        LABEL_43:
            if (*v1 == 0)
                return v3;
        }
        if (v6 == 10 || v6 == 13)
            goto LABEL_43;
    LABEL_42:
        ++v3;
        goto LABEL_43;
    }
    return v3;
}

// ============================================================================
// SE_* package wrappers
// ============================================================================

// ea: 0x00581370
int SE_GetNumFlags()
{
    return (int)TheStringPackage.m_vstrFlagNames.size();
}

// ea: 0x0058FE90
const char* SE_GetFlagName(int iFlagIndex)
{
    if (TheStringPackage.m_vstrFlagNames.empty()
        || iFlagIndex >= TheStringPackage.m_vstrFlagNames.size())
    {
        return defaultFileName;
    }
    return TheStringPackage.m_vstrFlagNames[iFlagIndex].c_str();
}

// ea: 0x0058FEF0
const char* SE_GetLanguageName(int iLangIndex)
{
    if (gvLanguagesAvailable.empty()
        || iLangIndex >= gvLanguagesAvailable.size())
    {
        return defaultFileName;
    }
    return gvLanguagesAvailable[iLangIndex].c_str();
}

// ea: 0x0058FF50
const char* SE_GetLanguageDir(int iLangIndex)
{
    if (gvLanguagesAvailable.empty()
        || iLangIndex >= gvLanguagesAvailable.size())
    {
        return defaultFileName;
    }
    return va("localizedstrings/%s",
              gvLanguagesAvailable[iLangIndex].c_str());
}

// ea: 0x0059B850
const char* SE_GetString(const char* psPackageAndStringReference, int bDebug)
{
    std::map<std::string, SE_Entry_s>::iterator it =
        TheStringPackage.m_StringEntries.find(
            std::string(psPackageAndStringReference));
    if (it == TheStringPackage.m_StringEntries.end())
        return nullptr;
    if (bDebug != 0 && TheStringPackage.m_bLoadDebug != 0)
        return it->second.m_strDebug.c_str();
    return it->second.m_strString.c_str();
}

// ea: 0x0059B950
int SE_GetFlags(const char* psPackageAndStringReference)
{
    std::map<std::string, SE_Entry_s>::iterator it =
        TheStringPackage.m_StringEntries.find(
            std::string(psPackageAndStringReference));
    if (it == TheStringPackage.m_StringEntries.end())
        return 0;
    return it->second.m_iFlags;
}

// ea: 0x0059BA00
int SE_GetFlagMask(const char* psFlagName)
{
    return TheStringPackage.GetFlagMask(psFlagName);
}

// ea: 0x0059CA00
const char* SE_GetString(const char* psPackageReference,
                         const char* psStringReference, int bDebug)
{
    char sReference[256];
    sprintf(sReference, "%s_%s", psPackageReference, psStringReference);
    return SE_GetString(sReference, bDebug);
}

// ea: 0x0059CA40
int SE_GetFlags(const char* psPackageReference,
                const char* psStringReference)
{
    char sReference[256];
    sprintf(sReference, "%s_%s", psPackageReference, psStringReference);
    return SE_GetFlags(sReference);
}

// ============================================================================
// package lifecycle
// ============================================================================

// ea: 0x0059D850
void SE_NewLanguage()
{
    TheStringPackage.m_StringEntries.clear();
    TheStringPackage.m_bEndMarkerFound_ParseOnly = 0;
    TheStringPackage.m_strCurrentEntryRef_ParseOnly.assign("", 0);
    TheStringPackage.m_strCurrentEntryEnglish_ParseOnly.assign("", 0);
}

// ea: 0x0059D8C0
void SE_Init()
{
    TheStringPackage.Clear(0);
}

// ea: 0x0059D8D0
void SE_ShutDown()
{
    TheStringPackage.Clear(0);
}

// ea: 0x0059D8E0
void SEH_UpdateLanguageInfo()
{
    if (cl_language == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\stringed_hooks.cpp";
        AeAssert::gCurrentLine = 178;
        AeAssert::gCurrentExpr = "cl_language";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    SEH_UpdateCurrentLanguage();
    Cvar_Set("cl_languagesavailable", "1");
    SE_NewLanguage();
}

// ea: 0x0059D950
void SEH_Init_StringEd()
{
    TheStringPackage.Clear(0);
}

// ea: 0x0059D960
void SEH_Shutdown_StringEd()
{
    TheStringPackage.Clear(0);
}

// ea: 0x0059D970
void SEH_Restart_StringEd()
{
    SE_NewLanguage();
}

// ============================================================================
// file listing / loading
// ============================================================================

// shell.o 0x59BA20 (helper; recursive .str directory listing)
void SE_R_ListFiles(const char* psExtension, const char* psDir,
                    std::string* strResults)
{
    int numdirs = 0;
    char** dirFiles = FS_ListFiles(psDir, "/", &numdirs);
    for (int i = 0; i < numdirs; ++i)
    {
        char v4 = *dirFiles[i];
        if (v4 != 0 && v4 != '.')
        {
            char sFilename[128];
            sprintf(sFilename, "%s/%s", psDir, dirFiles[i]);
            SE_R_ListFiles(psExtension, sFilename, strResults);
        }
    }
    int numSysFiles = 0;
    char** sysFiles = FS_ListFiles(psDir, psExtension, &numSysFiles);
    for (int j = 0; j < numSysFiles; ++giFilesFound)
    {
        char sFilename[128];
        sprintf(sFilename, "%s/%s", psDir, sysFiles[j]);
        strResults->append(sFilename, strlen(sFilename));
        strResults->append(1, ';');
        ++j;
    }
    FS_FreeFileList(sysFiles);
    FS_FreeFileList(dirFiles);
}

// shell.o 0x58FDF0 (helper; shared strResults splitter)
static const char* SE_GetFoundFile(std::string& strResult)
{
    if (strResult.empty())
        return nullptr;
    strncpy(sTemp, strResult.c_str(), 0x7F);
    sTemp[127] = 0;
    char* v5 = strchr(sTemp, ';');
    if (v5 != nullptr)
    {
        *v5 = 0;
        strResult.erase(0, (size_t)(v5 - sTemp) + 1);
    }
    else
    {
        strResult.erase(0, std::string::npos);
    }
    return sTemp;
}

// shell.o 0x59E100 (helper; reads + parses a .str file)
static const char* SE_Load_Actual(const char* psFileName, int bLoadDebug,
                                  int bSpeculativeLoad)
{
    const char* v3 = nullptr;
    void* fileBuf = nullptr;
    if (FS_ReadFile(psFileName, &fileBuf) > 0 && fileBuf != nullptr)
    {
        char* v4 = (char*)fileBuf;
        const char* psParsePos = v4;
        char sLineBuffer[16384];
        TheStringPackage.SetupNewFileParse(psFileName, bLoadDebug);
        do
        {
            if (TheStringPackage.ReadLine(psParsePos, sLineBuffer) == 0)
                break;
            unsigned int v7 = (unsigned int)strlen(sLineBuffer);
            if (v7 != 0)
                v3 = TheStringPackage.ParseLine(sLineBuffer);
        }
        while (v3 == nullptr);
        FS_FreeFile(v4);
        if (v3 == nullptr
            && TheStringPackage.m_bEndMarkerFound_ParseOnly == 0)
            return va("Truncated file, failed to find \"%s\" at file end!",
                      "ENDMARKER");
    }
    else if (bSpeculativeLoad == 0)
    {
        return va("Unable to load \"%s\"!", psFileName);
    }
    return v3;
}

// ea: 0x0059E1E0
const char* SE_Load(const char* psFileName, int bLoadDebug)
{
    const char* Actual = SE_Load_Actual(psFileName, bLoadDebug, 0);
    if (Actual != nullptr)
        return Actual;
    char sFileName[128];
    strncpy(sFileName, psFileName, 0x7F);
    sFileName[127] = 0;
    char* v3 = strrchr(sFileName, '.');
    if (v3 == nullptr || strlen(v3) != 4)
        return Actual;
    strcpy(v3, ".ste");
    return SE_Load_Actual(sFileName, bLoadDebug, 1);
}

// ea: 0x0059E270
int SE_GetNumLanguages()
{
    gvLanguagesAvailable.clear();
    std::string strResults;
    giFilesFound = 0;
    strResults.assign("", 0);
    SE_R_ListFiles(".str", "localizedstrings", &strResults);
    std::set<std::string> strUniqueStrings;
    for (const char* i = SE_GetFoundFile(strResults); i != nullptr;
         i = SE_GetFoundFile(strResults))
    {
        char* sStringPtr = sString;
        const char* src = i;
        do
        {
            *sStringPtr = *src;
            ++sStringPtr;
            ++src;
        }
        while (*src != 0);
        *sStringPtr = 0;
        char* v5 = strrchr(sString, '\\');
        char* v7 = strrchr(sString, '/');
        if (v5 > v7)
            v7 = v5;
        if (v7 != nullptr)
            *v7 = 0;
        char* v9 = sString;
        for (char* j = sString; *j != 0; ++j)
        {
            if (*j == '/' || *j == '\\')
                v9 = j + 1;
        }
        strcpy(sString_1, v9);
        std::string langName(sString_1);
        if (strUniqueStrings.insert(langName).second)
        {
            if (_stricmp(sString_1, "english") == 0)
                gvLanguagesAvailable.insert(gvLanguagesAvailable.begin(),
                                            langName);
            else
                gvLanguagesAvailable.push_back(langName);
        }
    }
    return (int)gvLanguagesAvailable.size();
}

// ea: 0x0059E670
const char* SE_LoadLanguage(const char* psLanguage, int bLoadDebug)
{
    const char* result = nullptr;
    if (psLanguage == nullptr || *psLanguage == 0)
        return result;
    SE_NewLanguage();
    std::string strResults;
    giFilesFound = 0;
    strResults.assign("", 0);
    SE_R_ListFiles(".str", "localizedstrings", &strResults);
    const char* FoundFile = SE_GetFoundFile(strResults);
    if (FoundFile != nullptr)
    {
        const char* psErrorMessage = nullptr;
        while (1)
        {
            if (psErrorMessage != nullptr)
                break;
            strcpy(sString, FoundFile);
            char* v6 = strrchr(sString, '\\');
            char* v7 = strrchr(sString, '/');
            if (v6 > v7)
                v7 = v6;
            if (v7 != nullptr)
                *v7 = 0;
            char* v9 = sString;
            for (char* i = sString; *i != 0; ++i)
            {
                if (*i == '/' || *i == '\\')
                    v9 = i + 1;
            }
            char* dst = sString_1;
            const char* src = v9;
            char v12;
            do
            {
                v12 = *src;
                *dst = *src;
                ++dst;
                ++src;
            }
            while (v12 != 0);
            *dst = 0;
            if (_stricmp(psLanguage, sString_1) == 0)
                psErrorMessage = SE_Load(FoundFile, bLoadDebug);
            FoundFile = SE_GetFoundFile(strResults);
            if (FoundFile == nullptr)
                break;
        }
        return psErrorMessage;
    }
    return result;
}

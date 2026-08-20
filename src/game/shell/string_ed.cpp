// ============================================================================
// string_ed.cpp - CStringEdPackage string-table parser (shell.o)
// ============================================================================

#include "game/shell/shell_types.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern const char defaultFileName[];  // 0xCD67AE
extern void mem_heap_free(void* ptr);      // core.o
extern char* va(const char* fmt, ...);     // core.o

// File-scope scratch buffers (shell.o data; shared with stringed_hooks.cpp)
char sString[128];
char sString_0[128];
char sString_1[128];

// ea: 0x00576BD0
char* CStringEdPackage::Filename_PathOnly(const char* psFilename)
{
    strcpy(sString, psFilename);
    char* v3 = strrchr(sString, '\\');
    char* v4 = strrchr(sString, '/');
    if (v3 > v4)
        v4 = v3;
    if (v4 != nullptr)
        *v4 = 0;
    return sString;
}

// ea: 0x00576C20
char* CStringEdPackage::Filename_WithoutExt(const char* psFilename)
{
    strcpy(sString_0, psFilename);
    char* v3 = strrchr(sString_0, '.');
    char* v5 = strrchr(sString_0, '\\');
    char* v6 = strrchr(sString_0, '/');
    if (v3 != nullptr && (v5 == 0 || v3 > v5) && (v6 == 0 || v3 > v6))
        *v3 = 0;
    return sString_0;
}

// ea: 0x00576C90
char* CStringEdPackage::Filename_WithoutPath(const char* psFilename)
{
    const char* v2 = psFilename;
    char v3 = *psFilename;
    const char* i = psFilename;
    char v6;
    for (; v3 != 0; ++v2)
    {
        if (v3 == '/' || v3 == '\\')
            i = v2 + 1;
        v3 = v2[1];
    }
    ptrdiff_t v5 = sString_1 - i;
    do
    {
        v6 = *i;
        *(sString_1 + (i - psFilename)) = *i;
        ++i;
    }
    while (v6 != 0);
    return sString_1;
}

// ea: 0x00576CE0
const char* CStringEdPackage::ExtractLanguageFromPath(const char* psFileName)
{
    const char* v3 = Filename_PathOnly(psFileName);
    return Filename_WithoutPath(v3);
}

// ea: 0x00576D00
int CStringEdPackage::CheckLineForKeyword(const char* psKeyword,
                                          const char*& psLine)
{
    if (_strnicmp(psKeyword, psLine, strlen(psKeyword)) != 0)
        return 0;
    for (const char* i = &psLine[strlen(psKeyword)]; ; ++i)
    {
        psLine = i;
        if (*i != '\t' && *i != ' ')
            break;
    }
    return 1;
}

// ea: 0x00576D70
void CStringEdPackage::REMKill(char* psBuffer)
{
    char* v2 = psBuffer;
    int v3 = 0;
    char* v4 = strstr(psBuffer, "//");
    if (v4 != nullptr)
    {
        while (1)
        {
            int v5 = v3;
            for (int i = 0; i < v4 - v2; ++i)
            {
                if (v2[i] == '"')
                    ++v5;
            }
            if ((v5 & 1) == 0)
                break;
            v2 = v4 + 1;
            v3 = v5;
            v4 = strstr(v4 + 1, "//");
            if (v4 == nullptr)
                return;
        }
        *v4 = 0;
        if (*v2 != 0)
        {
            for (int j = (int)strlen(v2) - 1; j >= 0; v2[j--] = 0)
            {
                if (!isspace((unsigned char)v2[j]))
                    break;
            }
        }
    }
}

// ea: 0x00576E10
int CStringEdPackage::ReadLine(const char*& psParsePos, char* psDest)
{
    if (*psParsePos == 0)
        return 0;
    char* v3 = strchr((char*)psParsePos, '\n');
    if (v3 != nullptr)
    {
        size_t v4 = v3 - psParsePos;
        strncpy(psDest, psParsePos, v4);
        psDest[v4] = 0;
        const char* v5 = &psParsePos[v4];
        psParsePos = v5;
        if (*v5 != 0)
        {
            const char* v6;
            do
            {
                if (strchr("\r\n", *psParsePos) == nullptr)
                    break;
                v6 = psParsePos + 1;
                psParsePos = v6;
            }
            while (*v6 != 0);
        }
    }
    else
    {
        strcpy(psDest, psParsePos);
        psParsePos += strlen(psParsePos);
    }
    if (*psDest != 0)
    {
        for (int i = (int)strlen(psDest) - 1; i >= 0; psDest[i--] = 0)
        {
            if (!isspace((unsigned char)psDest[i]))
                break;
        }
        REMKill(psDest);
    }
    return 1;
}

// ea: 0x005878C0
const char* CStringEdPackage::GetCurrentReference_ParseOnly()
{
    return m_strCurrentEntryRef_ParseOnly.c_str();
}

// ea: 0x0059B220
void CStringEdPackage::SetupNewFileParse(const char* psFileName, int bLoadDebug)
{
    const char* v4 = Filename_WithoutExt(psFileName);
    char sString[128];
    strcpy(sString, Filename_WithoutPath(v4));
    _strupr(sString);
    m_strCurrentFileRef_ParseOnly.assign(sString, strlen(sString));
    const char* v5 = Filename_PathOnly(psFileName);
    const char* v6 = Filename_WithoutPath(v5);
    m_strLoadingLanguage_ParseOnly.assign(v6, strlen(v6));
    m_bLoadingEnglish_ParseOnly =
        _stricmp(m_strLoadingLanguage_ParseOnly.c_str(), "english") == 0;
    m_bLoadDebug = bLoadDebug;
}

// ea: 0x0059B2E0
const char* CStringEdPackage::ConvertCRLiterals_Read(const char* psString)
{
    static std::string str_1;
    str_1.assign(psString, strlen(psString));
    for (size_t i = str_1.find("\\n", 0, 2); i != std::string::npos;
         i = str_1.find("\\n", 0, 2))
    {
        str_1[i] = '\n';
        str_1.erase(i + 1, 1);
    }
    return str_1.c_str();
}

// ea: 0x0059B3E0
const char* CStringEdPackage::InsideQuotes(const char* psLine)
{
    static std::string str_2;
    str_2.assign("", 0);
    while (*psLine == ' ' || *psLine == '\t')
        ++psLine;
    if (*psLine == '"')
        ++psLine;
    str_2.assign(psLine, strlen(psLine));
    if (*psLine == 0)
        return str_2.c_str();
    while (1)
    {
        size_t len = str_2.length();
        if (len == 0)
            break;
        char last = str_2[len - 1];
        if (last != ' ' && last != '\t')
            break;
        str_2.erase(len - 1, 1);
    }
    if (str_2.length() != 0 && str_2[str_2.length() - 1] == '"')
        str_2.erase(str_2.length() - 1, 1);
    return str_2.c_str();
}

// ea: 0x0059B5B0
int CStringEdPackage::GetFlagMask(const char* psFlagName)
{
    std::map<std::string, int>::iterator it =
        m_mapFlagMasks.find(std::string(psFlagName));
    if (it == m_mapFlagMasks.end())
        return 0;
    return it->second;
}

// ea: 0x0059B670
void CStringEdPackage::SetString(const char* psLocalReference,
                                 const char* psNewString, int bEnglishDebug)
{
    char buf[1024];
    _snprintf(buf, sizeof(buf), "%s_%s",
              m_strCurrentFileRef_ParseOnly.c_str(), psLocalReference);
    std::map<std::string, SE_Entry_s>::iterator it =
        m_StringEntries.find(std::string(buf));
    if (it == m_StringEntries.end())
        return;
    SE_Entry_s& entry = it->second;
    if (bEnglishDebug != 0 || m_bLoadingEnglish_ParseOnly != 0)
    {
        const char* v10 = psNewString;
        entry.m_strString.assign(psNewString, strlen(psNewString));
        if (m_bLoadDebug != 0)
        {
            entry.m_strDebug.assign("[", 1);
            entry.m_strDebug.append(v10, strlen(v10));
            entry.m_strDebug.append("]", 1);
        }
        m_strCurrentEntryEnglish_ParseOnly.assign(v10, strlen(v10));
    }
    else if (_stricmp(psNewString, "#same") == 0)
    {
        entry.m_strString.assign(m_strCurrentEntryEnglish_ParseOnly, 0,
                                 std::string::npos);
        if (m_bLoadDebug != 0)
        {
            entry.m_strDebug = "[";
            entry.m_strDebug += "#same";
            entry.m_strDebug += "]";
        }
    }
    else
    {
        entry.m_strString = psNewString;
    }
}

// ea: 0x0059D790
void CStringEdPackage::Clear(int bChangingLanguages)
{
    m_StringEntries.clear();
    if (bChangingLanguages == 0)
    {
        m_vstrFlagNames.clear();
        m_mapFlagMasks.clear();
    }
    m_bEndMarkerFound_ParseOnly = 0;
    m_strCurrentEntryRef_ParseOnly.assign("", 0);
    m_strCurrentEntryEnglish_ParseOnly.assign("", 0);
}

// ea: 0x0059D980
void CStringEdPackage::AddEntry(const char* psLocalReference)
{
    char buf[1024];
    _snprintf(buf, sizeof(buf), "%s_%s",
              m_strCurrentFileRef_ParseOnly.c_str(), psLocalReference);
    std::string key(buf);
    if (m_StringEntries.find(key) == m_StringEntries.end())
    {
        SE_Entry_s entry;
        entry.m_strString.assign("", 0);
        entry.m_strDebug.assign("", 0);
        entry.m_iFlags = 0;
        m_StringEntries[key] = entry;
    }
    m_strCurrentEntryRef_ParseOnly.assign(psLocalReference,
                                          strlen(psLocalReference));
}

// ea: 0x0059DB40
void CStringEdPackage::AddFlagReference(const char* psLocalReference,
                                        const char* psFlagName)
{
    int FlagMask = GetFlagMask(psFlagName);
    if (FlagMask == 0)
    {
        m_vstrFlagNames.push_back(std::string(psFlagName));
        FlagMask = 1 << ((int)m_vstrFlagNames.size() - 1);
        m_mapFlagMasks[std::string(psFlagName)] = FlagMask;
    }
    char buf[1024];
    _snprintf(buf, sizeof(buf), "%s_%s",
              m_strCurrentFileRef_ParseOnly.c_str(), psLocalReference);
    std::map<std::string, SE_Entry_s>::iterator it =
        m_StringEntries.find(std::string(buf));
    if (it != m_StringEntries.end())
        it->second.m_iFlags |= FlagMask;
}

// ea: 0x0059DD00
const char* CStringEdPackage::ParseLine(const char* psLine)
{
    const char* psErrorMessage = nullptr;
    if (psLine == nullptr)
        return psErrorMessage;
    const char* linePos = psLine;
    if (CheckLineForKeyword("VERSION", linePos) != 0)
    {
        int v4 = atoi(InsideQuotes(linePos));
        if (v4 != 1)
            return va("Unexpected version number %d, expecting %d!\n", v4, 1);
        return psErrorMessage;
    }
    if (CheckLineForKeyword("CONFIG", linePos) != 0
        || CheckLineForKeyword("FILENOTES", linePos) != 0
        || CheckLineForKeyword("NOTES", linePos) != 0)
    {
        return psErrorMessage;
    }
    if (CheckLineForKeyword("REFERENCE", linePos) != 0)
    {
        AddEntry(InsideQuotes(linePos));
        return psErrorMessage;
    }
    if (CheckLineForKeyword("FLAGS", linePos) != 0)
    {
        if (m_strCurrentEntryRef_ParseOnly.empty())
            return "Error parsing file: Unexpected \"FLAGS\"\n";
        char sThisLanguage[1024];
        memset(sThisLanguage, 0, sizeof(sThisLanguage));
        strncpy(sThisLanguage, linePos, 0x3FFu);
        char* v8 = strtok(sThisLanguage, " \t");
        if (v8 == nullptr)
            return psErrorMessage;
        do
        {
            _strupr(v8);
            AddFlagReference(m_strCurrentEntryRef_ParseOnly.c_str(), v8);
            v8 = strtok(nullptr, " \t");
        }
        while (v8 != nullptr);
        return psErrorMessage;
    }
    if (CheckLineForKeyword("ENDMARKER", linePos) != 0)
    {
        m_bEndMarkerFound_ParseOnly = 1;
        return psErrorMessage;
    }
    if (_strnicmp("LANG_", linePos, 5) != 0)
        return va("Unknown keyword at linestart: \"%s\"\n", linePos);
    if (m_strCurrentEntryRef_ParseOnly.empty())
        return "Error parsing file: Unexpected \"LANG_\"\n";
    const char* v11 = &linePos[5];
    const char* i = v11;
    while (*i != 0 && *i != ' ' && *i != '\t')
        ++i;
    char sThisLanguage[1024];
    memset(sThisLanguage, 0, sizeof(sThisLanguage));
    size_t v13 = i - v11;
    if (v13 > 0x3FF)
        v13 = 1023;
    strncpy(sThisLanguage, v11, v13);
    const char* v14 = InsideQuotes(&v11[strlen(sThisLanguage)]);
    const char* v15 = ConvertCRLiterals_Read(v14);
    if (m_bLoadingEnglish_ParseOnly != 0)
    {
        SetString(m_strCurrentEntryRef_ParseOnly.c_str(), v15, 0);
        return psErrorMessage;
    }
    int v16 = _stricmp(sThisLanguage, "english");
    int v17 = v16 == 0;
    if (v16 == 0
        || _stricmp(m_strLoadingLanguage_ParseOnly.c_str(), sThisLanguage)
               == 0)
    {
        SetString(m_strCurrentEntryRef_ParseOnly.c_str(), v15, v17);
        return psErrorMessage;
    }
    return psErrorMessage;
}

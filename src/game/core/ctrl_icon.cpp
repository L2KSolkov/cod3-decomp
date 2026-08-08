// ============================================================================
// ctrl_icon.cpp - CtrlIcon (core.o)
// ============================================================================

#include "game/core/core_systems.h"

#include <stdio.h>
#include <string.h>

extern char* g_ctrlIconInfo[2][0x13];
extern const char* off_DD8A5C[];

// ea: 0x004BD700
bool CtrlIcon::ContainsIconTag(const char* text)
{
    const char* v2 = strchr(text, 123);
    return v2 != nullptr && strchr(v2, 125) != nullptr;
}

// ea: 0x004BD730
CtrlIcon* CtrlIcon::TranslateIconTag(const char* text)
{
    if (_strnicmp(text, "t:", 2) == 0)
    {
        sprintf(mScratchBuffer, "%s", text + 2);
        return this;
    }
    int v4 = 0;
    while (_stricmp(text, g_ctrlIconInfo[v4][0]) != 0)
    {
        if (++v4 >= 0x13)
            return nullptr;
    }
    return (CtrlIcon*)off_DD8A5C[2 * v4];
}

// ea: 0x004BD7A0
char CtrlIcon::ExtractIconTag(const char* text, char* preTagString,
                             char** postTagString, char** tagString)
{
    if (text == nullptr)
        goto fail;
    char* v6 = (char*)strchr(text, 123);
    char* v7 = v6;
    if (v6 == nullptr)
        goto fail;
    char* v8 = (char*)strchr(v6, 125);
    char* tagEnd = v8;
    if (v8 != nullptr && (v8 - v7) > 1)
    {
        size_t v9 = v8 - v7 - 1;
        strncpy(preTagString, v7 + 1, v9);
        preTagString[v9] = 0;
        CtrlIcon* v10 = TranslateIconTag(preTagString);
        *tagString = (char*)v10;
        if (v10 != nullptr)
        {
            size_t v11 = v7 - text;
            strncpy(preTagString, text, v11);
            preTagString[v11] = 0;
            *postTagString = tagEnd + 1;
            return 1;
        }
    }
fail:
    *preTagString = 0;
    *postTagString = (char*)text;
    *tagString = nullptr;
    return 0;
}

// ============================================================================
// ctrl_icon.cpp - CtrlIcon (core.o)
// ============================================================================

#include "game/core/core_systems.h"

#include <stdio.h>
#include <string.h>

extern const char defaultFileName[];
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file,
                                 int line);

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

// ea: 0x004E2900
CtrlIcon* CtrlIcon::CreateInst()
{
    if (CtrlIcon::sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ctrlicon.h";
        AeAssert::gCurrentLine = 42;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }
    CtrlIcon::sInst = static_cast<CtrlIcon*>(
        mem_heap_malloc_ctx(0x800u, 1, "ui",
                            "c:\\cod\\code\\game\\ctrlicon.h", 42));
    return CtrlIcon::sInst;
}

char* g_ctrlIconInfo[20][2] = {
    {"BUTTON_1", "XBOX_B01"},
    {"BUTTON_2", "XBOX_B02"},
    {"BUTTON_3", "XBOX_B03"},
    {"BUTTON_4", "XBOX_B04"},
    {"TRIGGER_L1", "XBOX_B05"},
    {"TRIGGER_R1", "XBOX_B06"},
    {"DPAD", "XBOX_B07"},
    {"DPAD_UP", "XBOX_B08"},
    {"DPAD_DOWN", "XBOX_B09"},
    {"DPAD_LEFT", "XBOX_B10"},
    {"DPAD_RIGHT", "XBOX_B11"},
    {"DPAD_UPDOWN", "XBOX_B12"},
    {"DPAD_LEFTRIGHT", "XBOX_B13"},
    {"BUTTON_START", "XBOX_B14"},
    {"BUTTON_SELECT", "XBOX_B15"},
    {"ANALOG_LEFT", "XBOX_B16"},
    {"ANALOG_RIGHT", "XBOX_B17"},
    {"XBOX_CLEAR", "XBOX_B18"},
    {"XBOX_BLACK", "XBOX_B19"},
    {(char*)defaultFileName, (char*)defaultFileName},
};

const char* off_DD8A5C[37] = {
    "XBOX_B01", "BUTTON_2", "XBOX_B02", "BUTTON_3", "XBOX_B03",
    "BUTTON_4", "XBOX_B04", "TRIGGER_L1", "XBOX_B05", "TRIGGER_R1",
    "XBOX_B06", "DPAD", "XBOX_B07", "DPAD_UP", "XBOX_B08",
    "DPAD_DOWN", "XBOX_B09", "DPAD_LEFT", "XBOX_B10", "DPAD_RIGHT",
    "XBOX_B11", "DPAD_UPDOWN", "XBOX_B12", "DPAD_LEFTRIGHT", "XBOX_B13",
    "BUTTON_START", "XBOX_B14", "BUTTON_SELECT", "XBOX_B15", "ANALOG_LEFT",
    "XBOX_B16", "ANALOG_RIGHT", "XBOX_B17", "XBOX_CLEAR", "XBOX_B18",
    "XBOX_BLACK", "XBOX_B19",
};

// ea: 0x004BD700
bool CtrlIcon::ContainsIconTag(const char* text)
{
    const char* v2 = strchr(text, 123);
    return v2 != nullptr && strchr(v2, 125) != nullptr;
}

// ea: 0x004BD730
const char* CtrlIcon::TranslateIconTag(const char* text)
{
    if (_strnicmp(text, "t:", 2) == 0)
    {
        sprintf(mScratchBuffer, "%s", text + 2);
        return (const char*)this;
    }
    int v4 = 0;
    while (_stricmp(text, g_ctrlIconInfo[v4][0]) != 0)
    {
        if (++v4 >= 0x13)
            return nullptr;
    }
    return (const char*)off_DD8A5C[2 * v4];
}

// ea: 0x004BD7A0
bool CtrlIcon::ExtractIconTag(const char* text, char* preTagString,
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
        const char* v10 = TranslateIconTag(preTagString);
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

// ============================================================================
// g_cmd.cpp - game.o command buffer / command registration (cmd.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// Command globals (game.o data)
// ============================================================================
struct cmd_t {
    char* data;     // +0x00
    int maxsize;    // +0x04
    int cmdsize;    // +0x08
};
static cmd_t cmd_text;               // ?cmd_text@@3Ucmd_t@@A (game.o)
static unsigned char cmd_text_buf[8192];
static cmd_t sv_cmd_text;            // ?sv_cmd_text@@3Ucmd_t@@A (game.o)
static unsigned char sv_cmd_text_buf[8192];
static int cmd_argc;                 // ?cmd_argc@@3HA (game.o)
static char* cmd_argv[512];
static char cmd_tokenized[8192];

enum ECmdFuncType { CMD = 0, INPUT_CMD = 1 };

struct BaseCmdFuncInfo {
    int mFuncType;          // +0x00
    const char* mName;      // +0x04
    BaseCmdFuncInfo* mNext; // +0x08
    void* mFuncPtr;         // +0x0C
    static int DoesFunctionExist(BaseCmdFuncInfo* cmd);  // ?DoesFunctionExist@BaseCmdFuncInfo@@SAHPAU1@@Z
};

// ea: 0x0060E450
void Cbuf_Init()
{
    cmd_text.data = (char*)cmd_text_buf;
    cmd_text.maxsize = 0x2000;
    cmd_text.cmdsize = 0;
    sv_cmd_text.data = (char*)sv_cmd_text_buf;
    sv_cmd_text.maxsize = 0x2000;
    sv_cmd_text.cmdsize = 0;
}

// ea: 0x0060E420
int BaseCmdFuncInfo::DoesFunctionExist(BaseCmdFuncInfo* cmd)
{
    int mFuncType = cmd->mFuncType;
    return (mFuncType == CMD && cmd[1].mNext != nullptr)
        || (mFuncType == INPUT_CMD && cmd[1].mNext != nullptr);
}

// ea: 0x0060F230
void Link_Init()
{
}

// ea: 0x0060F240
void Link_Frame()
{
}
static BaseCmdFuncInfo* cmd_functions;      // ?cmd_functions (game.o)
static BaseCmdFuncInfo* sv_cmd_functions;   // ?sv_cmd_functions (game.o)

extern void _Z_FreeInternal(void* ptr);             // ?_Z_FreeInternal (hunk_mem)
extern int Com_Filter(char* filter, char* name, int casesensitive);  // core.o
extern void ButtonMgr_ClearBinding(const BaseCmdFuncInfo* boundCmd,
                                   int clnt);  // ?ClearBinding@ButtonMgr (game2.o)
extern void Com_Printf(const char* fmt, ...);

// ============================================================================
// Cbuf_AddText - ea: 0x60E490
// ============================================================================
// ea: 0x0060E490
void Cbuf_AddText(const char* text)
{
    unsigned int v1 = (unsigned int)strlen(text);
    if ((cmd_text.cmdsize + (int)v1) < cmd_text.maxsize)
    {
        memcpy(&cmd_text.data[cmd_text.cmdsize], text, v1);
        cmd_text.cmdsize += v1;
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
        AeAssert::gCurrentLine = 153;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Cbuf_AddText: overflow"))
            __debugbreak();
    }
}

// ============================================================================
// Cbuf_InsertText - ea: 0x60E530
// ============================================================================
// ea: 0x0060E530
void Cbuf_InsertText(const char* text)
{
    unsigned int v1 = (unsigned int)strlen(text) + 1;
    if ((cmd_text.cmdsize + (int)v1) <= cmd_text.maxsize)
    {
        for (int i = cmd_text.cmdsize - 1; i >= 0; --i)
            cmd_text.data[i + v1] = cmd_text.data[i];
        memcpy(cmd_text.data, text, v1 - 1);
        cmd_text.data[v1 - 1] = 10;
        cmd_text.cmdsize += v1;
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
        AeAssert::gCurrentLine = 177;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Cbuf_Insert overflow"))
            __debugbreak();
    }
}

// ============================================================================
// Cmd_CallCmdFunction - ea: 0x60E6A0
// ============================================================================
// ea: 0x0060E6A0
void Cmd_CallCmdFunction(const BaseCmdFuncInfo* cmd, int key, int time)
{
    if (cmd == nullptr)
        return;
    int mFuncType = cmd->mFuncType;
    if (mFuncType == INPUT_CMD && cmd[1].mNext != nullptr)
    {
        (*(void (**)(int, int))&cmd[1].mNext)(key, time);
    }
    else if (mFuncType == CMD)
    {
        void (*mNext)() = (void (*)())cmd[1].mNext;
        if (mNext != nullptr)
            mNext();
    }
}

// ============================================================================
// Cmd_Argc / Cmd_Argv / Cmd_Args - ea: 0x60E6E0..0x60E7D0
// ============================================================================
// ea: 0x0060E6E0
int Cmd_Argc()
{
    return cmd_argc;
}

// ea: 0x0060E6F0
char* Cmd_Argv(int arg)
{
    if (arg < (unsigned int)cmd_argc)
        return cmd_argv[arg];
    return (char*)"";
}

// ea: 0x0061F5E0
void Cmd_ArgvBuffer(int arg, char* buffer, int bufferLength)
{
    if (arg < (unsigned int)cmd_argc)
        Q_strncpyz(buffer, cmd_argv[arg], bufferLength);
    else
        Q_strncpyz(buffer, "", bufferLength);
}

static char cmd_args1[1024];

// ea: 0x0060E710
char* Cmd_Args(int start)
{
    if (start < 1)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
        AeAssert::gCurrentLine = 612;
        AeAssert::gCurrentExpr = "start >= 1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    cmd_args1[0] = 0;
    for (int i = start; i < cmd_argc; ++i)
    {
        strcat(cmd_args1, cmd_argv[i]);
        if (i != cmd_argc - 1)
            strcat(cmd_args1, " ");
    }
    return cmd_args1;
}

// ============================================================================
// Cmd_TokenizeString2 - ea: 0x60E7E0
// ============================================================================
// ea: 0x0060E7E0
void Cmd_TokenizeString2(const char* text_in, int max_tokens)
{
    cmd_argc = 0;
    if (text_in == nullptr)
        return;
    char* v4 = cmd_tokenized;
    const char* v2 = text_in;
    int v3 = 0;
    while (--max_tokens != 0)
    {
        // skip whitespace
        char v5 = *v2;
        if (v5 == 0)
            break;
        while (v5 <= 32)
        {
            v5 = *++v2;
            if (v5 == 0)
                return;
        }
        if (*v2 == 0)
            break;
        char v6 = *v2;
        if (v6 == 47)
        {
            char v7 = v2[1];
            if (v7 == 47)
                return;  // // comment to EOL
            if (v7 == 42)
            {
                // /* */ block comment
                char c = *v2;
                if (c != 0)
                {
                    while (c != 42 || v2[1] != 47)
                    {
                        c = *++v2;
                        if (c == 0)
                            return;
                    }
                    if (*v2 != 0)
                        v2 += 2;
                }
                continue;
            }
        }
        // token
        v8:
        cmd_argv[v3++] = v4;
        cmd_argc = v3;
        char v8 = *v2;
        if (v8 == 34)
        {
            // quoted string
            char v9 = v2[1];
            const char* i = v2 + 1;
            for (; v9 != 0; ++i)
            {
                if (v9 == 34)
                    break;
                *v4 = v9;
                v9 = i[1];
                ++v4;
            }
            *v4++ = 0;
            if (*i == 0)
                return;
            v2 = i + 1;
        }
        else
        {
            for (char j = *v2; j > 32; ++v2)
            {
                if (j == 34)
                    break;
                if (j == 47)
                {
                    char v12 = v2[1];
                    if (v12 == 47 || v12 == 42)
                        break;
                }
                *v4 = j;
                j = v2[1];
                ++v4;
            }
            *v4++ = 0;
        }
        if (*v2 != 0)
        {
            if (*v2 <= 32)
                ++v2;
            if (v3 != 512)
                continue;
        }
        return;
    }
    if (*v2 != 0)
    {
        cmd_argv[v3] = v4;
        char v13 = *v2;
        bool v14 = *v2 == 0;
        cmd_argc = v3 + 1;
        if (!v14)
        {
            do
            {
                *v4 = v13;
                ++v4;
                v13 = *++v2;
            } while (v13 != 0);
            *v4 = 0;
        }
    }
}

// ea: 0x0060E930
void Cmd_TokenizeString(const char* text_in)
{
    Cmd_TokenizeString2(text_in, 0);
}

extern char* CopyStringInternal(const char* in);  // ?CopyStringInternal (hunk_mem)

// ============================================================================
// Cmd_AddCommand - ea: 0x60E950
// ============================================================================
void Cmd_AddCommand(const char* cmd_name, void (*function)())
{
    if (cmd_name == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
        AeAssert::gCurrentLine = 793;
        AeAssert::gCurrentExpr = "cmd_name";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    BaseCmdFuncInfo* v2 = cmd_functions;
    if (cmd_functions != nullptr)
    {
        while (strcmp(cmd_name, v2->mName) != 0)
        {
            v2 = v2->mNext;
            if (v2 == nullptr)
                goto add_entry;
        }
        int mFuncType = v2->mFuncType;
        if (mFuncType == CMD && v2[1].mNext != nullptr
            || mFuncType == INPUT_CMD && v2[1].mNext != nullptr)
            Com_Printf("Cmd_AddCommand: %s already defined\n", cmd_name);
    }
    else
    {
    add_entry:
        BaseCmdFuncInfo* v3 = (BaseCmdFuncInfo*)_Z_MallocInternal(16);
        char* v4 = CopyStringInternal(cmd_name);
        BaseCmdFuncInfo* v5 = cmd_functions;
        v3->mName = v4;
        v3->mFuncType = CMD;
        v3->mNext = v5;
        v3[1].mNext = (BaseCmdFuncInfo*)function;
        cmd_functions = v3;
    }
}

// ============================================================================
// Cmd_AddInputCommand - ea: 0x60EA50
// ============================================================================
void Cmd_AddInputCommand(const char* cmd_name, void (*function)(int, int))
{
    if (cmd_name == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
        AeAssert::gCurrentLine = 824;
        AeAssert::gCurrentExpr = "cmd_name";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    BaseCmdFuncInfo* v2 = cmd_functions;
    if (cmd_functions != nullptr)
    {
        while (strcmp(cmd_name, v2->mName) != 0)
        {
            v2 = v2->mNext;
            if (v2 == nullptr)
                goto add_entry;
        }
        int mFuncType = v2->mFuncType;
        if (mFuncType == CMD && v2[1].mNext != nullptr
            || mFuncType == INPUT_CMD && v2[1].mNext != nullptr)
            Com_Printf("Cmd_AddCommand: %s already defined\n", cmd_name);
    }
    else
    {
    add_entry:
        BaseCmdFuncInfo* v3 = (BaseCmdFuncInfo*)_Z_MallocInternal(16);
        char* v4 = CopyStringInternal(cmd_name);
        BaseCmdFuncInfo* v5 = cmd_functions;
        v3->mName = v4;
        v3->mFuncType = INPUT_CMD;
        v3->mNext = v5;
        v3[1].mNext = (BaseCmdFuncInfo*)function;
        cmd_functions = v3;
    }
}

// ============================================================================
// Cmd_RemoveCommand - ea: 0x60EB50
// ============================================================================
// ea: 0x0060EB50
void Cmd_RemoveCommand(const char* cmd_name)
{
    BaseCmdFuncInfo* v1 = cmd_functions;
    BaseCmdFuncInfo** p_mNext = &cmd_functions;
    if (cmd_functions == nullptr)
        return;
    while (strcmp(cmd_name, v1->mName) != 0)
    {
        p_mNext = &v1->mNext;
        v1 = v1->mNext;
        if (v1 == nullptr)
            return;
    }
    *p_mNext = v1->mNext;
    if (v1->mName != nullptr)
        _Z_FreeInternal((void*)v1->mName);
    ButtonMgr_ClearBinding(v1, currCl);
    _Z_FreeInternal(v1);
}

// ============================================================================
// Cmd_Shutdown - ea: 0x60EBE0
// ============================================================================
// ea: 0x0060EBE0
void Cmd_Shutdown()
{
    for (BaseCmdFuncInfo* i = cmd_functions; cmd_functions != nullptr;
         i = cmd_functions)
    {
        cmd_functions = i->mNext;
        if (i->mName == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
            AeAssert::gCurrentLine = 884;
            AeAssert::gCurrentExpr = "cmd->mName";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        _Z_FreeInternal((void*)i->mName);
        ButtonMgr_ClearBinding(i, currCl);
        _Z_FreeInternal(i);
    }
    for (BaseCmdFuncInfo* j = sv_cmd_functions; sv_cmd_functions != nullptr;
         j = sv_cmd_functions)
    {
        sv_cmd_functions = j->mNext;
        if (j->mName == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
            AeAssert::gCurrentLine = 895;
            AeAssert::gCurrentExpr = "cmd->mName";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        _Z_FreeInternal((void*)j->mName);
        _Z_FreeInternal(j);
    }
}

// ============================================================================
// Cmd_CommandCompletion - ea: 0x60ECF0
// ============================================================================
// ea: 0x0060ECF0
void Cmd_CommandCompletion(void (*callback)(const char*))
{
    for (BaseCmdFuncInfo* i = cmd_functions; i != nullptr; i = i->mNext)
        callback(i->mName);
}

// ============================================================================
// Cmd_List_f - ea: 0x60ED20
// ============================================================================
// ea: 0x0060ED20
void Cmd_List_f()
{
    char* v0 = cmd_argc <= 1 ? nullptr : cmd_argv[1];
    BaseCmdFuncInfo* v1 = cmd_functions;
    int i = 0;
    for (; v1 != nullptr; v1 = v1->mNext)
    {
        if (v0 == nullptr
            || Com_Filter(v0, (char*)v1->mName, 0) != 0)
        {
            Com_Printf("%s\n", v1->mName);
            ++i;
        }
    }
    Com_Printf("%i commands\n", i);
}

// ============================================================================
// Sys_Time - ea: 0x60EE40
// ============================================================================
static bool first_time_0 = true;
static unsigned __int64 initial_time_0;

// ea: 0x0060EE40
double Sys_Time()
{
    unsigned int v0;
    unsigned int v1;
    if (first_time_0)
    {
        unsigned __int64 v3 = __rdtsc();
        v0 = (unsigned int)(v3 >> 32);
        v1 = (unsigned int)v3;
        initial_time_0 = v3;
        first_time_0 = false;
    }
    else
    {
        v0 = (unsigned int)(initial_time_0 >> 32);
        v1 = (unsigned int)initial_time_0;
    }
    unsigned __int64 now = __rdtsc();
    return (double)(now - (((unsigned __int64)v0 << 32) | v1))
        * 0.0000000013636364;
}

// ============================================================================
// DebugCurveRender / Sys_Print - empty stubs
// ============================================================================
// ea: 0x0060EEE0
void DebugCurveRender()
{
}

// ea: 0x0060F250
void Sys_Print()
{
}

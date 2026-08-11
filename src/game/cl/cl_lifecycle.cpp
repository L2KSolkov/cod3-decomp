// ============================================================================
// cl_lifecycle.cpp - client lifecycle + configstring + server-command (cl.o)
// 18 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"
#include "cl_console.h"

#include <string.h>

// ============================================================================
// Externs
// ============================================================================
extern void Com_Printf(const char* fmt, ...);
extern void Com_Error(int code, const char* fmt, ...);
extern void Cvar_Set(const char* var_name, const char* value);
extern struct cvar_t* Cvar_Get(const char* var_name, const char* var_value,
                               int flags);
extern void Cmd_RemoveCommand(const char* cmd_name);
extern void Cmd_TokenizeString(const char* text_in);
extern void Cmd_TokenizeString2(const char* text_in, int max_tokens);
extern char* Cmd_Argv(int arg);
extern int Cmd_Argc();
extern void Cbuf_AddText(const char* text);
extern void Cmd_AddCommand(const char* cmd_name, void (*function)());
extern void XModelEnforceExist(int bEnforce);
extern void CL_InitRenderer();
extern char CL_InitUI();
extern void Com_CvarDump(int type);
extern void Axis_Bind_f();
extern void Axis_Unbindall_f();
extern void Key_Bind_f();
extern void Key_Unbind_f();
extern void Key_Unbindall_f();
extern void Key_Bindlist_f();
extern void Field_CharEvent(field_t* edit, int ch);
extern int dword_F170E0;
extern int dword_F170E8;
extern int atoi(const char* nptr);
extern int com_sv_running;
extern struct cvar_t* com_cl_running;
extern int dword_F0F200[2];
extern int dword_F0F204[2];
extern char byte_F0F208[];
extern int dword_F170F8;
extern int dword_F6A28C;
extern int gSaveGameData_mHorizontalSensitivity[4];
extern int gSaveGameData_mVerticalSensitivity[4];
extern int unk_F6A298[4];
extern int unk_F6A29C[4];
extern int Sys_Milliseconds();
extern int VM_Call(struct vm_s* vm, int callnum, ...);
extern void VM_Free(struct vm_s* vm);
extern struct vm_s { int (__cdecl* systemCall)(int*); }* cgvm;
extern int CL_ShutdownDebugData();
extern void CL_ShutdownInput();
extern void CL_ShutdownUI();
extern int StatMon_Reset();
extern int CL_InitInput();
extern void CL_InitGamepadCommands();
extern void CL_InitKeyCommands();
extern void CL_ConfigstringModified();
extern void CL_InitCGame();
extern void Con_Init();
extern void Con_Close();
extern void Con_OneTimeInit();
extern void CL_ConsolePrint_AddLine(int type, const char* txt, int duration,
                                    int linewidth, int color, int flags);
extern int CL_RestoreMessageType(unsigned char* buffer, int used, int total,
                                 void* msgwnd, int type, int linewidth);
extern int ColorIndex(unsigned char c);
extern void Con_Linefeed(int type, int duration, int flags);
extern void SoundDevice_StopAllSounds(void* self);
extern void SoundDevice_FrameAdvance(void* self, float delta);
extern void* SoundDevice_sInst;

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)

// ============================================================================
// clc state (cl.o data; 0x4C48 bytes)
// ============================================================================
struct clc_t {
    unsigned char data[0x4C48];
};
extern clc_t clc;

// cls.configstrings - Broc::string[1024] (the "servername" field aliases it)
struct Broc_string_view {
    void* mBlock;
};
extern Broc_string_view cls_configstrings[1024];

// ============================================================================
// Lifecycle
// ============================================================================

// ea: 0x5327E0
int CL_ClearState()
{
    SoundDevice_StopAllSounds(SoundDevice_sInst);
    SoundDevice_FrameAdvance(SoundDevice_sInst, 0.0f);
    for (int i = 0; i < 1024; ++i)
    {
        if (cls_configstrings[i].mBlock != nullptr)
            cls_configstrings[i].mBlock = nullptr;
    }
    memset(cl, 0, sizeof(clientActive_t) * 2);
    memset(&clc, 0, sizeof(clc));
    return 0;
}

// ea: 0x532880
void CL_Disconnect()
{
    if (com_cl_running != nullptr && com_cl_running->integer != 0)
    {
        Cvar_Set("r_uiFullScreen", "1");
        CL_ClearState();
        cls.state = 0;  // CA_DISCONNECTED
    }
}

// ea: 0x532F40
void CL_ConfigstringModified()
{
    const char* v0 = Cmd_Argv(1);
    unsigned int v1 = (unsigned int)atoi(v0);
    int index = (int)v1;
    if (v1 >= 0x400)
        Com_Error(1, "configstring > MAX_CONFIGSTRINGS");
    const char* s = Cmd_Argv(2);
    if (cls_configstrings[v1].mBlock == nullptr)
    {
        ASSERT("cls.configstrings[index].IsDefined()",
               "c:\\cod\\code\\game\\cl_cgame.cpp", 329);
    }
    cls_configstrings[v1].mBlock = (void*)(s - 12);
}

// ea: 0x52F290
const char* CL_GetConfigString(int index)
{
    void* v1 = cls_configstrings[index].mBlock;
    if (v1 != nullptr)
        return (const char*)v1 + 12;
    return "";
}

// ea: 0x52F210
int CL_Restart()
{
    for (int i = 0; i < 1024; ++i)
    {
        if (cls_configstrings[i].mBlock != nullptr)
        {
            ASSERT("!cls.configstrings[i].IsDefined()",
                   "c:\\cod\\code\\game\\cl_cgame.cpp", 356);
        }
        cls_configstrings[i].mBlock = nullptr;
    }
    return VM_Call(cgvm, 19);
}

// ea: 0x533080
int CL_GetServerCommand(int serverCommandNumber)
{
    if (serverCommandNumber <= dword_F0F200[4882 * currCl] - 64)
    {
        ASSERT("0", "c:\\cod\\code\\game\\cl_cgame.cpp", 379);
    }
    if (serverCommandNumber > dword_F0F200[4882 * currCl])
    {
        ASSERT("0", "c:\\cod\\code\\game\\cl_cgame.cpp", 384);
    }
    const char* v1 = &byte_F0F208[19528 * currCl + 128 * (serverCommandNumber & 0x3F)];
    dword_F0F204[4882 * currCl] = serverCommandNumber;
    Cmd_TokenizeString(v1);
    const char* v2 = Cmd_Argv(0);
    if (strcmp(v2, "cs") == 0)
    {
        Cmd_TokenizeString2(v1, 3);
        CL_ConfigstringModified();
        Cmd_TokenizeString2(v1, 3);
        return 1;
    }
    if (strcmp(v2, "clientLevelShot") == 0)
    {
        if (com_sv_running == 0)
            return 0;
        Con_Close();
        Cbuf_AddText("wait ; wait ; wait ; wait ; screenshot levelshot\n");
    }
    return 1;
}

// ea: 0x533330
void CL_ConsoleFixPosition()
{
    if (cl_noprint != nullptr && cl_noprint->integer != 0)
    {
        con.display = con.current - 1;
    }
    else
    {
        if (con.initialized == 0)
        {
            Con_OneTimeInit();
            if (con.initialized == 0)
            {
                ASSERT("con.initialized", "c:\\cod\\code\\game\\cl_console.cpp", 1270);
            }
        }
        int v1 = ColorIndex(0x37);
        CL_ConsolePrint_AddLine(PMSG_CONSOLE, "\n", 0, 0, v1, 0);
        con.display = con.current - 1;
    }
}

// ea: 0x52F670
int CL_RestoreMessages(unsigned char* buffer, int bufSize)
{
    bool moveDisplayPos = con.display == con.current;
    if (con.x > 0)
        Con_Linefeed(con.prevType, 0, 0);
    if (bufSize < 4)
        Com_Error(1, "CL_RestoreMessages: buffer too small (%i)", bufSize);
    int v3 = *buffer;
    int v4 = CL_RestoreMessageType(buffer, 4, bufSize,
                                   &con.gamemsg_starttimes, PMSG_CONSOLE,
                                   *buffer);
    int v5 = CL_RestoreMessageType(buffer, v4, bufSize, &msgwnd, 0, v3);
    for (int i = 0; i < con.linewidth; ++i)
    {
        int v7 = ColorIndex(0x37);
        con.text[i + con.linewidth * (con.current % con.totallines)] =
            (short)((v7 << 8) | 0x20);
    }
    if (moveDisplayPos)
        con.display = con.current;
    return v5;
}

// ea: 0x532A70
void CL_Shutdown()
{
    Com_Printf("----- CL_Shutdown -----\n");
    CL_ShutdownDebugData();
    if (com_cl_running != nullptr && com_cl_running->integer != 0)
    {
        Cvar_Set("r_uiFullScreen", "1");
        CL_ClearState();
        cls.state = 0;
    }
    cls.keyCatchers &= ~8;
    if (cgvm != nullptr)
    {
        VM_Call(cgvm, 1);
        VM_Free(cgvm);
        cgvm = nullptr;
    }
    CL_ShutdownUI();
    CL_ShutdownInput();
    Cmd_RemoveCommand("cmd");
    Cmd_RemoveCommand("snd_restart");
    Cmd_RemoveCommand("vid_restart");
    Cmd_RemoveCommand("disconnect");
    Cmd_RemoveCommand("cinematic");
    Cmd_RemoveCommand("setenv");
    Cmd_RemoveCommand("fs_openedList");
    Cmd_RemoveCommand("fs_referencedList");
    Cmd_RemoveCommand("updatehunkusage");
    Cmd_RemoveCommand("updatescreen");
    Cmd_RemoveCommand("sl");
    Cmd_RemoveCommand("startMultiplayer");
    Cmd_RemoveCommand("shellExecute");
}

// ea: 0x5359E0
void CL_Init()
{
    Com_Printf("----- Client Initialization -----\n");
    Con_Init();
    cls.state = 0;
    dword_F170F8 = 0;
    CL_InitInput();
    cl_noprint = Cvar_Get("cl_noprint", "0", 0);
    cl_shownet = Cvar_Get("cl_shownet", "0", 256);
    cl_yawspeed = Cvar_Get("cl_yawspeed", "140", 1);
    cl_pitchspeed = Cvar_Get("cl_pitchspeed", "140", 1);
    cl_anglespeedkey = Cvar_Get("cl_anglespeedkey", "1.5", 0);
    cl_disable_ads = Cvar_Get("cl_disable_ads", "0", 256);
    cl_aADS[0] = 1;
    cl_mouseAccel = Cvar_Get("cl_mouseAccel", "0", 1);
    cl_freelook = Cvar_Get("cl_freelook", "1", 1);
    cl_showMouseRate = Cvar_Get("cl_showmouserate", "0", 0);
    CL_InitGamepadCommands();
    CL_InitKeyCommands();
}

// ea: 0x532700
void CL_InitKeyCommands()
{
    Cmd_AddCommand("bind", Key_Bind_f);
    Cmd_AddCommand("unbind", Key_Unbind_f);
    Cmd_AddCommand("unbindall", Key_Unbindall_f);
    Cmd_AddCommand("bindlist", Key_Bindlist_f);
}

// ea: 0x532F10
void CL_InitGamepadCommands()
{
    Cmd_AddCommand("bindaxis", Axis_Bind_f);
    Cmd_AddCommand("unbindallaxis", Axis_Unbindall_f);
}

// ea: 0x532740
void CL_CharEvent(int key)
{
    if (key != 96 && key != 126
        && ((cls.keyCatchers & 1) != 0 || cls.state == 0))
    {
        Field_CharEvent(&g_consoleField, key);
    }
}

// ea: 0x532780
void CL_ShutdownAll()
{
    cls.keyCatchers &= ~8;
    if (cgvm != nullptr)
    {
        VM_Call(cgvm, 1);
        VM_Free(cgvm);
        cgvm = nullptr;
    }
    CL_ShutdownUI();
    dword_F170E8 = 0;
    dword_F170E0 = 0;
}

// ea: 0x5328C0
void CL_StartHunkUsers()
{
    if (com_cl_running != nullptr && com_cl_running->integer != 0)
    {
        cvar_t* v0 = Cvar_Get("cl_xmodelcheck", "0", 33);
        XModelEnforceExist(v0->integer);
        if (!dword_F170E0)
        {
            dword_F170E0 = 1;
            CL_InitRenderer();
        }
        if (!dword_F170E8)
        {
            dword_F170E8 = 1;
            CL_InitUI();
        }
        Com_CvarDump(PMSG_LOGFILE);
    }
}

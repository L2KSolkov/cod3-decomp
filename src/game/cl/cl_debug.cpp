// ============================================================================
// cl_debug.cpp - client debug-draw + misc command helpers (cl.o)
// 18 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"
#include "cl_console.h"

#include <string.h>

struct netchan_t;
enum netsrc_t {
    NS_CLIENT = 0,
    NS_SERVER = 1,
};
struct netadr_t {
    int type;
    unsigned char ip[4];
    unsigned char ipx[10];
    unsigned short port;
};

// ============================================================================
// Externs
// ============================================================================
extern void Com_Printf(const char* fmt, ...);
extern void Com_DPrintf(const char* fmt, ...);
extern void Com_Error(int code, const char* fmt, ...);
extern int Cmd_Argc();
extern char* Cmd_Argv(int arg);
extern int Q_stricmp(const char* s1, const char* s2);
extern void Cmd_AddCommand(const char* cmd_name, void (*function)());
extern void Cvar_Set(const char* var_name, const char* value);
extern void Cvar_SetValue(const char* var_name, float value);
extern int VM_Call(struct vm_s* vm, int callnum, ...);
extern void VM_Free(struct vm_s* vm);
extern struct vm_s { int (__cdecl* systemCall)(int*); }* cgvm;
extern int com_timescale_value;
extern int dword_F170F0;
int com_skelTimeStamp = 0;  // ?com_skelTimeStamp@@3HA (core.o)
int bCL_AllowedAllocSkel = 0;  // cl.o BSS
extern struct cvar_t* cl_testAnimWeight;
extern void* _Z_MallocInternal(int size);
struct FEManager;
extern FEManager g_femanager;
extern void Netchan_Setup(netsrc_t sock, netchan_t* chan, netadr_t adr,
                          int qport);
extern void CL_AddReliableCommand(const char* cmd);

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
// Debug-data globals (cl.o)
// ============================================================================
int dword_F170E0 = 0;  // cl.o BSS
int dword_F171C0 = 0;  // cl.o BSS
int dword_F171B8 = 0;  // cl.o BSS
int dword_F171C4 = 0;  // cl.o BSS (string count)
int dword_F171C8 = 0;  // cl.o BSS (debug string buffer)
int dword_F171CC = 0;  // cl.o BSS (string source)
int dword_F171D0 = 0;  // cl.o BSS (line capacity)
int dword_F171D4 = 0;  // cl.o BSS (line count)
int ptr = 0;           // cl.o BSS (line buffer)
int dword_F171DC = 0;  // cl.o BSS (line source)
int dword_F171E0 = 0;  // cl.o BSS (line depth)
int dword_F171E4 = 0;  // cl.o BSS
char byte_F171E8 = 0;  // cl.o BSS (server string source)
extern void re_LocateDebugStrings(int a1, int a2);
extern void re_LocateDebugLines(int a1, int a2);
extern int CL_SaveMessageType(unsigned char* buffer, int used, int total,
                              void* msgwnd);

// ============================================================================
// Misc commands
// ============================================================================

// ea: 0x5288A0
void CL_AddCgameCommand(const char* cmdName, void (__cdecl* function)())
{
    Cmd_AddCommand(cmdName, function);
}

// ea: 0x5288B0
void CL_CM_LoadMap()
{
}

// ea: 0x5288C0
void CL_ShutdownCGame()
{
    cls.keyCatchers &= ~8;
    if (cgvm != nullptr)
    {
        VM_Call(cgvm, 1);
        VM_Free(cgvm);
        cgvm = nullptr;
    }
}

// ea: 0x528900
char CL_DObjInvalidateSkels()
{
    char result = (char)bCL_AllowedAllocSkel;
    if (bCL_AllowedAllocSkel == 0)
    {
        ASSERT("bCL_AllowedAllocSkel", "c:\\cod\\code\\game\\cl_cgame.cpp", 513);
    }
    if (++com_skelTimeStamp == 0)
        com_skelTimeStamp = 1;
    return result;
}

// ea: 0x52CBB0
void CL_EndgameMenu()
{
    cls.endgamemenu = 1;
}

// ea: 0x52CBC0
void CL_CDDialog()
{
    cls.cddialog = 1;
}

// ea: 0x52CDA0
void CL_ConnectResponse(netadr_t from)
{
    if (com_cl_running->integer == 0)
    {
        ASSERT("com_cl_running->integer", "c:\\cod\\code\\game\\cl_main.cpp", 609);
    }
    Netchan_Setup((netsrc_t)currCl,
                  (netchan_t*)((char*)0xF11208 + 19528 * currCl), from,
                  currCl);
    cls.state = 4 * (cgvm != nullptr) + 1;
}

// ea: 0x52CFE0
int CG_GetGameModel(short modelindex)
{
    return VM_Call(cgvm, 8, modelindex);
}

// ea: 0x52D000
void* CL_GetFontInfo(int font, float scale)
{
    extern void* FEManager_GetFont(void* self, int f, float scale);
    return FEManager_GetFont(&g_femanager, font, scale);
}

// ea: 0x52D080
void CL_IncAnimWeight_f()
{
    float value = (float)com_timescale_value;
    if (value == 0.0f)
        value = 1.0f;
    float test_anim_weight = ((float)dword_F170F0 / value) * 0.0020000001f
                             + cl_testAnimWeight->value;
    if (test_anim_weight > 1.0f)
        test_anim_weight = 1.0f;
    Cvar_SetValue("cl_testAnimWeight", test_anim_weight);
    Com_Printf("anim weight: %f\n", test_anim_weight);
}

// ea: 0x52D110
void CL_DecAnimWeight_f()
{
    float value = (float)com_timescale_value;
    if (value == 0.0f)
        value = 1.0f;
    float test_anim_weight = cl_testAnimWeight->value
                             - ((float)dword_F170F0 / value) * 0.0020000001f;
    if (test_anim_weight < 0.0f)
        test_anim_weight = 0.0f;
    Cvar_SetValue("cl_testAnimWeight", test_anim_weight);
    Com_Printf("anim weight: %f\n", test_anim_weight);
}

// ea: 0x52D190
void CL_DumpDXFFile_f()
{
    const char* v0 = Cmd_Argv(1);
    if (Cmd_Argc() != 2
        || Q_stricmp(v0, "full") != 0
            && Q_stricmp(v0, "local") != 0
            && Q_stricmp(v0, "view") != 0)
    {
        Com_Printf("Syntax: dumpDXF [local/full/view]\n");
    }
    else
    {
        Com_Printf("Preparing to write out DXF file...\n");
        Cvar_Set("r_showtris", "3");
        if (Q_stricmp(v0, "local") != 0)
        {
            if (Q_stricmp(v0, "view") != 0)
            {
                Cvar_Set("r_nocull", "1");
                Cvar_Set("r_novis", "1");
            }
            else
            {
                Cvar_Set("r_nocull", "0");
                Cvar_Set("r_novis", "0");
            }
        }
        else
        {
            Cvar_Set("r_nocull", "1");
            Cvar_Set("r_novis", "0");
        }
    }
}

// ============================================================================
// Debug-draw accumulation
// ============================================================================

// ea: 0x52D2A0
void CL_AddDebugString(float* xyz, float* color, float scale,
                       const char* pszText, int fromServer)
{
    if (dword_F170E0 != 0)
    {
        int v5 = dword_F171C4;
        dword_F171C0 = 256;
        if (dword_F171C4 + 1 <= 256)
        {
            char v6 = byte_F171E8;
            if (fromServer == 0)
                v6 = 2;
            if (dword_F171C8 == 0)
            {
                dword_F171C8 = (int)_Z_MallocInternal(33792);
                v5 = 0;
                dword_F171CC = (int)_Z_MallocInternal((unsigned int)dword_F171C0);
                dword_F171C4 = 0;
            }
            char* v7 = (char*)dword_F171C8 + 132 * v5;
            *(float*)v7 = xyz[0];
            *(float*)(v7 + 4) = xyz[1];
            *(float*)(v7 + 8) = xyz[2];
            *(float*)(v7 + 12) = color[0];
            *(float*)(v7 + 16) = color[1];
            *(float*)(v7 + 20) = color[2];
            *(float*)(v7 + 24) = color[3];
            *(float*)(v7 + 28) = scale;
            strncpy(v7 + 32, pszText, 0x5F);
            v7[127] = 0;
            v7[32] = 0;
            *(char*)(dword_F171CC + dword_F171C4++) = v6;
        }
    }
}

// ea: 0x52D3A0
void CL_AddDebugString2D(int x, int y, float* color, float scale,
                         const char* pszText, int fromServer)
{
    if (dword_F170E0 != 0)
    {
        int v6 = dword_F171C4;
        dword_F171C0 = 256;
        if (dword_F171C4 + 1 <= 256)
        {
            char v7 = byte_F171E8;
            if (fromServer == 0)
                v7 = 2;
            if (dword_F171C8 == 0)
            {
                dword_F171C8 = (int)_Z_MallocInternal(33792);
                v6 = 0;
                dword_F171CC = (int)_Z_MallocInternal((unsigned int)dword_F171C0);
                dword_F171C4 = 0;
            }
            char* v8 = (char*)dword_F171C8 + 132 * v6;
            *(int*)v8 = x;
            *(int*)(v8 + 4) = y;
            *(float*)(v8 + 12) = color[0];
            *(float*)(v8 + 16) = color[1];
            *(float*)(v8 + 20) = color[2];
            *(float*)(v8 + 24) = color[3];
            *(float*)(v8 + 28) = scale;
            strncpy(v8 + 32, pszText, 0x5F);
            v8[127] = 0;
            v8[32] = 1;
            *(char*)(dword_F171CC + dword_F171C4++) = v7;
        }
    }
}

// ea: 0x52D4A0
void CL_AddDebugLine(const float* start, const float* end, const float* color,
                     int depthTest, int duration, int fromServer, int fadeOut)
{
    if (dword_F170E0 != 0)
    {
        int v7 = dword_F171D4;
        dword_F171D0 = 0x2000;
        if (dword_F171D4 + 1 <= 0x2000)
        {
            char v8 = byte_F171E8;
            if (fromServer == 0)
                v8 = 2;
            if (ptr == 0)
            {
                ptr = (int)_Z_MallocInternal(0x58000);
                dword_F171DC = (int)_Z_MallocInternal((unsigned int)dword_F171D0);
                dword_F171E0 = (int)_Z_MallocInternal(4 * (unsigned int)dword_F171D0);
                dword_F171E4 = (int)_Z_MallocInternal(4 * (unsigned int)dword_F171D0);
                v7 = 0;
                dword_F171D4 = 0;
            }
            float* v9 = (float*)ptr + 11 * v7;
            v9[0] = start[0];
            v9[1] = start[1];
            v9[2] = start[2];
            v9[3] = end[0];
            v9[4] = end[1];
            v9[5] = end[2];
            v9[6] = color[0];
            v9[7] = color[1];
            v9[8] = color[2];
            *(int*)((char*)v9 + 36) = depthTest;
            *(int*)((char*)v9 + 40) = duration;
            *(char*)((char*)v9 + 43) = (char)((fadeOut != 0) | (v8 << 1));
            *(char*)(dword_F171DC + dword_F171D4++) = v8;
        }
    }
}

// ea: 0x52D600
void CL_FlushDebugData(int fromServer)
{
    if (dword_F170E0 != 0)
    {
        char v1;
        unsigned char source;
        if (fromServer != 0)
        {
            byte_F171E8 = byte_F171E8 == 0;
            v1 = byte_F171E8;
            source = (unsigned char)byte_F171E8;
        }
        else
        {
            source = 2;
            v1 = 2;
        }
        if (dword_F171C8 != 0)
        {
            if (dword_F171CC == 0)
            {
                ASSERT("cls.debug.stringSource", "c:\\cod\\code\\game\\cl_main.cpp", 1839);
            }
            // Render debug strings (re.LocateDebugStrings equivalent; the
            // actual painting happens in the renderer via UpdateDebugData).
            extern void re_DebugStrings(int count);
            re_DebugStrings(dword_F171C4);
            dword_F171C4 = 0;
        }
        if (ptr != 0)
        {
            if (dword_F171DC == 0)
            {
                ASSERT("cls.debug.lineSource", "c:\\cod\\code\\game\\cl_main.cpp", 1843);
            }
            extern void re_DebugLines(int count);
            re_DebugLines(dword_F171D4);
            dword_F171D4 = 0;
        }
    }
}

// ea: 0x52D840
void CL_UpdateDebugData()
{
    if (dword_F170E0 != 0)
    {
        if (dword_F171C8 != 0)
        {
            if (dword_F171CC == 0)
            {
                ASSERT("cls.debug.stringSource", "c:\\cod\\code\\game\\cl_main.cpp", 1905);
            }
            re_LocateDebugStrings(dword_F171C8, dword_F171C4);
        }
        if (ptr != 0)
        {
            if (dword_F171DC == 0)
            {
                ASSERT("cls.debug.lineSource", "c:\\cod\\code\\game\\cl_main.cpp", 1913);
            }
            re_LocateDebugLines(ptr, dword_F171D4);
        }
    }
}

// ea: 0x529550
int CL_SaveMessages(unsigned char* buffer, int bufSize)
{
    if (bufSize < 4)
        Com_Error(1, "CL_SaveMessages: buffer too small (%i)", bufSize);
    buffer[0] = (unsigned char)con.linewidth;
    int v2 = CL_SaveMessageType(buffer, 4, bufSize, &con.gamemsg_starttimes);
    return CL_SaveMessageType(buffer, v2, bufSize, &msgwnd);
}

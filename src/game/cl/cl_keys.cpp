// ============================================================================
// cl_keys.cpp - key binding + client command plumbing (cl.o cl_keys.cpp etc.)
// 26 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"
#include "cl_console.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// Externs
// ============================================================================
extern void Com_Printf(const char* fmt, ...);
extern void Com_DPrintf(const char* fmt, ...);
extern void Com_Error(int code, const char* fmt, ...);
struct nglTexture;  // render.o
extern int Cmd_Argc();
extern char* Cmd_Argv(int arg);
extern char* Cmd_Args(int start);
extern void CL_AddReliableCommand(const char* cmd);
extern int Q_stricmp(const char* s1, const char* s2);
extern int Q_isnumeric(int c);
extern int Sys_Milliseconds();
extern void* _Z_MallocInternal(int size);
extern void _Z_FreeInternal(void* ptr);
extern char* CopyStringInternal(const char* in);
extern char* va(const char* fmt, ...);
extern void Q_strncpyz(char* dest, const char* src, int destsize);
extern int SEH_GetCurrentLanguage();
extern int cvar_modifiedFlags;
extern int key_overstrikeMode;
extern int dword_F0D1F4[2];
extern int dword_F0D1F8[2];
extern char byte_F0D1FC[];
extern int dword_F171D0;
extern int dword_F171C0;
extern int dword_F171C4;
void* dword_F171C8 = nullptr;  // cl.o BSS (debug string buffer)
void* dword_F171CC = nullptr;  // cl.o BSS (string source)
void* ptr = nullptr;           // cl.o BSS (line buffer)
void* dword_F171DC = nullptr;  // cl.o BSS (line source)
void* dword_F171E0 = nullptr;  // cl.o BSS (line depth)
extern int dword_F171E4;
extern int dword_F171D4;
int dword_F17118;  // ?dword_F17118@@3HA (cl.o)
extern int dword_F171B8;
extern int dword_F171BC;
extern int dword_F170F0;
extern int animFrametime;
extern int g_console_field_width;
extern int g_console_char_width;
extern int g_console_char_height;
extern int dword_F1719C;
extern struct cvar_t* com_timescale;
extern struct cvar_t* cl_disable_ads;
extern struct vm_s { int (__cdecl* systemCall)(int*); }* cgvm;
extern int VM_Call(struct vm_s* vm, int callnum, ...);
int dword_CE8814[64];  // cl.o BSS
int dword_CE8818[64];  // cl.o BSS
int dword_CE881C[64];  // cl.o BSS

// ============================================================================
// Key binding data
// ============================================================================
struct keyname_t {
    const char* name;
    int keynum;
};
struct KeyInfoEntry {
    char* mBoundCmdName;
    void SetBinding(const char* boundCmdName);
};
keyname_t keynames[512];           // ?keynames@@3PAUkeyname_t@@A (cl.o @ 0x11DE9D8)
keyname_t keynames_localized[512]; // ?keynames_localized@@3PAUkeyname_t@@A (cl.o @ 0x11DEE38)
const char* off_DEFC78[10];  // cl.o
int re_Shutdown = 0;         // ?re_Shutdown@@3HA (cl.o)
static char tinystr[5];

// ============================================================================
// Key binding system
// ============================================================================

// ea: 0x52C590
void Field_Paste()
{
}

// ea: 0x52C7F0
int Key_GetOverstrikeMode()
{
    return key_overstrikeMode;
}

// ea: 0x52C800
void Key_SetOverstrikeMode(int state)
{
    key_overstrikeMode = state;
}

// ea: 0x52C810
void KeyInfoEntry::SetBinding(const char* boundCmdName)
{
    if (mBoundCmdName != nullptr)
    {
        _Z_FreeInternal(mBoundCmdName);
        mBoundCmdName = nullptr;
    }
    if (boundCmdName != nullptr)
    {
        mBoundCmdName = CopyStringInternal(boundCmdName);
        cvar_modifiedFlags |= 1;
    }
}

// ea: 0x52C860
int Key_StringToKeynum(char* str)
{
    if (str == nullptr)
        return -1;
    int result = (unsigned char)*str;
    if (*str == 0)
        return -1;
    char v2 = str[1];
    if (v2 == 0)
        return result;
    if (result == 48 && v2 == 120 && strlen(str) == 4)
    {
        int v3 = (unsigned char)str[2];
        int v4;
        if (Q_isnumeric(v3) != 0)
            v4 = v3 - 48;
        else if (v3 < 97 || v3 > 102)
            v4 = 0;
        else
            v4 = v3 - 87;
        int v5 = (unsigned char)str[3];
        if (Q_isnumeric(v5) != 0)
            return v5 - 48 + 16 * v4;
        if (v5 < 97 || v5 > 102)
            return 16 * v4;
        return v5 - 87 + 16 * v4;
    }
    keyname_t* v6 = keynames;
    if (keynames[0].name != nullptr)
    {
        while (Q_stricmp(str, v6->name) != 0)
        {
            const char* name = v6[1].name;
            ++v6;
            if (name == nullptr)
                return -1;
        }
        return v6->keynum;
    }
    return -1;
}

// ea: 0x52C960
char* Key_KeynumToString(int keynum, int bTranslate)
{
    if (keynum == -1)
        return "<KEY NOT FOUND>";
    if (keynum >= 0x100)
        return "<OUT OF RANGE>";
    if (bTranslate != 0 && SEH_GetCurrentLanguage() == 1
        && keynum >= 48 && keynum <= 57)
    {
        return (char*)off_DEFC78[keynum - 48];
    }
    keyname_t* v3;
    if (keynum <= 32 || keynum >= 127 || keynum == 34)
    {
        v3 = keynames_localized;
        if (bTranslate != 0)
            goto LABEL_17;
    }
    else
    {
        tinystr[0] = (char)toupper(keynum);
        tinystr[1] = 0;
        if (keynum != 59 || bTranslate != 0)
            return tinystr;
    }
    v3 = keynames;
LABEL_17:
    if (v3->name != nullptr)
    {
        while (keynum != v3->keynum)
        {
            const char* name = v3[1].name;
            ++v3;
            if (name == nullptr)
                goto LABEL_20;
        }
        return (char*)v3->name;
    }
LABEL_20:
    int v5 = keynum >> 4;
    int v6 = keynum & 0xF;
    tinystr[0] = 48;
    tinystr[1] = 120;
    tinystr[2] = (char)(v5 <= 9 ? v5 + 48 : v5 + 87);
    tinystr[3] = (char)(v6 <= 9 ? v6 + 48 : v6 + 87);
    tinystr[4] = 0;
    return tinystr;
}

// ea: 0x52E280
void Key_KeynumToStringBuf(int keynum, char* buf, int buflen)
{
    const char* v3 = Key_KeynumToString(keynum, 1);
    Q_strncpyz(buf, v3, buflen);
}

// ea: 0x52CA80
bool CL_PortInPlay(int port)
{
    int* v1 = (int*)0xF6A28C;
    while (v1[1] != 2 || *v1 != port)
    {
        v1 += 802;
        if (v1 >= (int*)0xF6A28C + 802 * 4)
            return 0;
    }
    return 1;
}

// ea: 0x52CAB0
int CL_GetKeyBinding(const char* pszBinding, const char** ppszKey1,
                     const char** ppszKey2)
{
    if (cgvm != nullptr)
        return VM_Call(cgvm, 12, pszBinding, ppszKey1, ppszKey2);
    *ppszKey1 = "KEY_UNBOUND";
    *ppszKey2 = "";
    return 0;
}

// ea: 0x52CAF0
int CL_PreAllocStrings()
{
    int result = 0x2000;
    dword_F171D0 = 0x2000;
    dword_F171C0 = 256;
    if (dword_F171C8 == nullptr)
    {
        dword_F171C8 = _Z_MallocInternal(33792);
        dword_F171CC = _Z_MallocInternal((unsigned int)dword_F171C0);
        result = dword_F171D0;
        dword_F171C4 = 0;
    }
    if (ptr == nullptr)
    {
        ptr = _Z_MallocInternal(44 * (unsigned int)result);
        dword_F171DC = _Z_MallocInternal((unsigned int)dword_F171D0);
        dword_F171E0 = _Z_MallocInternal(4 * (unsigned int)dword_F171D0);
        result = (int)_Z_MallocInternal(4 * (unsigned int)dword_F171D0);
        dword_F171E4 = result;
        dword_F171D4 = 0;
    }
    return result;
}

// ============================================================================
// Client command plumbing
// ============================================================================

// ea: 0x52CBD0
void CL_AddReliableCommand(const char* cmd)
{
    if (dword_F0D1F4[4882 * currCl] - dword_F0D1F8[4882 * currCl] > 64)
    {
        Com_Error(1, "EXE_ERR_CLIENT_CMD_OVERFLOW - Tell MikeA");
    }
    int v1 = 4882 * currCl;
    int v2 = dword_F0D1F4[4882 * currCl] + 1;
    dword_F0D1F4[v1] = v2;
    Q_strncpyz(&byte_F0D1FC[128 * (v2 & 0x3F) + v1 * 4], cmd, 128);
}

// ea: 0x52CC70
int CL_ChangeReliableCommand()
{
    int v0 = ((dword_F0D1F4[4882 * currCl] & 0x3F) << 7) + 19528 * currCl;
    int result = (int)strlen(&byte_F0D1FC[v0]);
    if (result >= 255)
        result = 254;
    byte_F0D1FC[v0 + result] = 10;
    byte_F0D1FC[v0 + result + 1] = 0;
    return result;
}

// ea: 0x52CCC0
void CL_ForwardCommandToServer(const char* string)
{
    const char* v1 = Cmd_Argv(0);
    if (*v1 != 45)
    {
        if (cls.state == 0 || *v1 == 43)
        {
            Com_Printf("Unknown command \"%s\"\n", v1);
        }
        else if (Cmd_Argc() <= 1)
        {
            CL_AddReliableCommand(v1);
        }
        else
        {
            CL_AddReliableCommand(string);
        }
    }
}

// ea: 0x52CD20
void CL_ForwardToServer_f()
{
    if (cls.state == 2)
    {
        if (Cmd_Argc() > 1)
            CL_AddReliableCommand(Cmd_Args(1));
    }
    else
    {
        Com_Printf("Not connected to a server.\n");
    }
}

// ea: 0x52CD60
void CL_Setenv_f()
{
}

// ea: 0x52CD70
void CL_Disconnect_f()
{
}

// ea: 0x52CD80
void CL_ResetPureClientAtServer()
{
    CL_AddReliableCommand(va("vdr"));
}

// ea: 0x52CE50
void CL_SetFrametime(int frametime, int animFrametime)
{
    dword_F170F0 = frametime;
    ::animFrametime = animFrametime;
}

// ea: 0x52CE70
void CL_UpdateFog()
{
}

// ea: 0x52CE80
void CL_RefPrintf(int print_level, const char* fmt, ...)
{
    char msg[4096];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(msg, fmt, ap);
    va_end(ap);
    if (print_level != 0)
    {
        if (print_level == 2)
            Com_Printf("^3%s", msg);
        else if (print_level == 1)
            Com_DPrintf("^1%s", msg);
    }
    else
    {
        Com_Printf("%s", msg);
    }
}

// ea: 0x52CF00
void CL_ShutdownRef()
{
    extern int re_Shutdown;
    if (re_Shutdown != 0)
    {
        // re.Shutdown(1)
        extern void re_ShutdownFn(int);
        re_ShutdownFn(1);
        extern int StatMon_Reset();
        StatMon_Reset();
    }
}

// ea: 0x52CF30
void CL_InitRenderer()
{
    extern void re_BeginRegistration(int*);
    extern nglTexture* GetTextureData(const char* name, int image_type,
                                      const char* fromPak);
    re_BeginRegistration(&dword_F17118);
    dword_F171B8 = (int)GetTextureData("nglWhite", 0, "mp_frontEnd");
    dword_F171BC = (int)GetTextureData("console", 0, "mp_frontEnd");
    g_consoleField.charWidth = (float)g_console_char_width;
    g_console_field_width = (int)((float)dword_F1719C - 32.0f);
    g_consoleField.widthInPixels = (int)((float)dword_F1719C - 32.0f);
    g_consoleField.charHeight = (float)g_console_char_height;
    g_consoleField.bFixedSize = 1;
    extern int StatMon_Reset();
    StatMon_Reset();
}

// ea: 0x52CFC0
int CL_ScaledMilliseconds()
{
    return (int)(Sys_Milliseconds() * com_timescale->value);
}

// ea: 0x52D020
void CL_ShellExecute_URL_f()
{
    Com_DPrintf("CL_ShellExecute_URL_f\n");
    const char* v0 = Cmd_Argv(1);
    if (Q_stricmp(v0, "open") != 0)
    {
        Com_DPrintf("invalid CL_ShellExecute_URL_f syntax (shellExecute \"open\" <url> <doExit>)\n");
    }
    else if (Cmd_Argc() >= 4)
    {
        atoi(Cmd_Argv(3));
    }
}

// ea: 0x529000
void CL_LookupColor(unsigned char c, float* color)
{
    unsigned char v2 = ColorIndex(c);
    if (v2 >= 8)
    {
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        color[3] = 1.0f;
    }
    else
    {
        unsigned int v3 = 4 * v2;
        color[0] = (float)dword_CE8814[v3] / 255.0f;
        color[1] = (float)dword_CE8818[v3] / 255.0f;
        color[2] = (float)dword_CE881C[v3] / 255.0f;
        color[3] = (float)g_color_table[v2][0] / 255.0f;
    }
}

// ea: 0x52BAE0
bool CL_IsADS(int client)
{
    int integer = cl_disable_ads->integer;
    if (integer == 1)
        return false;
    if (integer == 2)
        return true;
    if (cl[client].snap.ps.weapon != 0
        && BG_GetInfoForWeapon(cl[client].snap.ps.weapon)->bADSOnly != 0)
    {
        return true;
    }
    return kb[KB_SPEED].active == cl_aADS[client];
}

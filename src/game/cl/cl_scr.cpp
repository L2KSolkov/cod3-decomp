// ============================================================================
// cl_scr.cpp - screen draw helpers + cubemap + gamepad + UI (cl.o)
// 21 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"
#include "cl_console.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Externs
// ============================================================================
extern void Com_Printf(const char* fmt, ...);
extern void Com_Error(int code, const char* fmt, ...);
extern int Cmd_Argc();
extern const char* Cmd_Argv(int arg);
extern char* va(const char* fmt, ...);
extern int com_skelTimeStamp;
extern int bCL_AllowedAllocSkel;
extern int dword_F1719C;
extern int dword_F171A0;
extern int dword_F171B8;
extern struct vm_s { int (__cdecl* systemCall)(int*); }* cgvm;
extern int VM_Call(struct vm_s* vm, int callnum, ...);
extern void CL_CubemapShotUsage();

// re renderer externs
struct re_api2 {
    void Text_Paint(float a1, float a2, int a3, float a4, const float* a5,
                    const char* a6, float a7, int a8, int a9);
    void Text_ConsolePaint(float a1, float a2, int a3, float a4,
                           const float* a5, const short* a6, float a7,
                           int a8, int a9);
    void SetColor(const float* color);
    void DrawStretchPic(float a1, float a2, float a3, float a4, float a5,
                        float a6, float a7, float a8, int a9);
    void CubemapShot(const char* name, int size, int face, float n0,
                     float n1);
    void CubemapWaterShot(const char* name, int size, int face,
                          const float* rgb0, const float* rgb90);
    void BeginFrame();
    void EndFrame(void* a, void* b);
};
extern re_api2 re;

// ============================================================================
// SCR_* screen helpers
// ============================================================================

// ea: 0x52DAB0
void SCR_DrawNamedPic()
{
}

// ea: 0x52DAC0
void SCR_AdjustFrom640(float* x, float* y, float* w, float* h)
{
    float v4 = (float)dword_F1719C * 0.0015625f;
    float v5 = (float)dword_F171A0 * 0.0020833334f;
    if (x != nullptr)
        *x = *x * v4;
    if (y != nullptr)
        *y = *y * v5;
    if (w != nullptr)
        *w = *w * v4;
    if (h != nullptr)
        *h = *h * v5;
}

// ea: 0x52DB40
void SCR_FillRect(float x, float y, float width, float height,
                  const float* color)
{
    re.SetColor(color);
    re.DrawStretchPic((float)dword_F1719C * 0.0015625f * x,
                      (float)dword_F171A0 * 0.0020833334f * y,
                      (float)dword_F1719C * 0.0015625f * width,
                      (float)dword_F171A0 * 0.0020833334f * height,
                      0.0f, 0.0f, 0.0f, 0.0f, dword_F171B8);
    re.SetColor(nullptr);
}

// ea: 0x52DBD0
void SCR_DrawPic(float x, float y, float width, float height, void* tex)
{
    re.DrawStretchPic((float)dword_F1719C * 0.0015625f * x,
                      (float)dword_F171A0 * 0.0020833334f * y,
                      (float)dword_F1719C * 0.0015625f * width,
                      (float)dword_F171A0 * 0.0020833334f * height,
                      0.0f, 0.0f, 1.0f, 1.0f, (int)tex);
}

// ea: 0x52DC50
void SCR_AdjustTo640(float* pfX, float* pfY, float* pfFontScale,
                     float* pfCharWidth)
{
    if (pfX != nullptr)
        *pfX = (640.0f / (float)dword_F1719C) * *pfX;
    if (pfY != nullptr)
        *pfY = (480.0f / (float)dword_F171A0) * *pfY;
    if (pfFontScale != nullptr)
        *pfFontScale = (480.0f / (float)dword_F171A0) * *pfFontScale;
    if (pfCharWidth != nullptr)
        *pfCharWidth = (640.0f / (float)dword_F1719C) * *pfCharWidth;
}

// ea: 0x52DCE0
void SCR_DrawSmallChar(int x, int y, int ch)
{
    float setColor[4];
    setColor[0] = 1.0f;
    setColor[1] = 1.0f;
    setColor[2] = 1.0f;
    setColor[3] = 1.0f;
    float v3 = 640.0f / (float)dword_F1719C;
    float fX = (float)x * v3;
    float v4 = 480.0f / (float)dword_F171A0;
    const char* v5 = va("%c", ch);
    re.Text_Paint(fX, (float)(y + 12) * v4, 5, v4 * 0.5f, setColor, v5,
                  v3 * 8.0f, 0, 0);
}

// ea: 0x52DDB0
void SCR_DrawSmallStringExt(int x, int y, const char* string,
                            const float* setColor)
{
    re.Text_Paint((float)x * (640.0f / (float)dword_F1719C),
                  (float)(y + 12) * (480.0f / (float)dword_F171A0),
                  5, (480.0f / (float)dword_F171A0) * 0.5f, setColor,
                  string, (640.0f / (float)dword_F1719C) * 8.0f, 0, 0);
}

// ea: 0x52DE40
void SCR_DrawConsoleString(int x, int y, const short* string, int limit,
                           const float* setColor)
{
    re.Text_ConsolePaint((float)x * (640.0f / (float)dword_F1719C),
                         (float)(y + 12) * (480.0f / (float)dword_F171A0),
                         5, (480.0f / (float)dword_F171A0) * 0.5f, setColor,
                         string, (640.0f / (float)dword_F1719C) * 8.0f,
                         limit, 0);
}

// ea: 0x52DED0
void SCR_DebugGraph()
{
}

// ea: 0x52DEE0
void SCR_DrawDebugGraph()
{
}

// ea: 0x52DEF0
void SCR_Init()
{
    extern int scr_initialized;
    scr_initialized = 1;
}

// ============================================================================
// Cubemap shot command
// ============================================================================

// ea: 0x52DF80
void CL_CubemapShot_f()
{
    extern const char* szShotName[6];
    if (cgvm == nullptr)
    {
        Com_Printf("must be in a map to use this command\n");
        return;
    }
    if (Cmd_Argc() < 3 || strlen(Cmd_Argv(2)) > 0x68)
    {
        CL_CubemapShotUsage();
        return;
    }
    char szBaseName[128];
    strcpy(szBaseName, Cmd_Argv(2));
    int v0 = 1;
    int v2 = atoi(Cmd_Argv(1));
    int v3 = v2;
    if (v2 < 4 || v2 > 1024 || ((v2 - 1) & v2) != 0)
        goto LABEL_17;
    if (Cmd_Argc() == 10)
    {
        if (_stricmp(Cmd_Argv(3), "water") == 0)
        {
            float rgb0[3];
            float rgb90[3];
            rgb0[0] = (float)atof(Cmd_Argv(4));
            rgb0[1] = (float)atof(Cmd_Argv(5));
            rgb0[2] = (float)atof(Cmd_Argv(6));
            rgb90[0] = (float)atof(Cmd_Argv(7));
            rgb90[1] = (float)atof(Cmd_Argv(8));
            rgb90[2] = (float)atof(Cmd_Argv(9));
            int v11 = 1;
            const char** v12 = szShotName;
            do
            {
                re.CubemapWaterShot(va("env/%s%s.tga", szBaseName, *v12),
                                    v3, v11, rgb0, rgb90);
                ++v12;
                ++v11;
            }
            while (v12 <= &szShotName[5]);
            return;
        }
LABEL_17:
        CL_CubemapShotUsage();
        return;
    }
    float n0 = 1.0f;
    float n1 = 1.3329999f;
    if (Cmd_Argc() == 6)
    {
        if (_stricmp(Cmd_Argv(3), "fresnel") != 0)
            goto LABEL_17;
        n0 = (float)atof(Cmd_Argv(4));
        n1 = (float)atof(Cmd_Argv(5));
        if (n0 < 1.0f || n1 < 1.0f)
            goto LABEL_17;
    }
    else if (Cmd_Argc() != 3)
    {
        goto LABEL_17;
    }
    if (++com_skelTimeStamp == 0)
        com_skelTimeStamp = 1;
    bCL_AllowedAllocSkel = 1;
    const char** v17 = szShotName;
    do
    {
        re.BeginFrame();
        VM_Call(cgvm, 3, cl[currCl].serverTime, 0, v0, v3);
        re.EndFrame(nullptr, nullptr);
        re.CubemapShot(va("%s%s", szBaseName, *v17), v3, v0, n0, n1);
        ++v17;
        ++v0;
    }
    while (v17 <= &szShotName[5]);
    bCL_AllowedAllocSkel = 0;
}

// ea: 0x52E270
void GetClipboardDataUI(char* buf)
{
    *buf = 0;
}

// ============================================================================
// Gamepad
// ============================================================================
extern int dword_F13368[64];
extern int dword_F1336C[64];
struct GpadAxesGlob {
    int axesValues[6];
};
extern GpadAxesGlob gaGlobs[2];
extern const char* virtualAxisNames[6];
extern int axisSameStick[6];
enum { GPAD_PHYSAXIS_NONE = -1 };

// ea: 0x52E660
int* CL_InitGamepadAxisBindings()
{
    GpadAxesGlob* v0 = gaGlobs;
    int* result = &dword_F1336C[0];
    for (int i = 6; i != 0; --i)
    {
        *(result - 1) = -1;
        *result = 1;
        v0->axesValues[0] = 0;
        result += 2;
        ++v0;
    }
    return result;
}

// ea: 0x52EE00
void CL_GamepadEvent(unsigned int physicalAxis, int value)
{
    if (physicalAxis >= 6)
        Com_Error(1, "CL_GamepadEvent: bad axis %i", physicalAxis);
    gaGlobs[currCl].axesValues[physicalAxis] = value;
}

// ea: 0x52EE40
float CL_GamepadAxisValue(unsigned int virtualAxis)
{
    if (virtualAxis >= 6)
    {
        // assert virtualAxis < GPAD_VIRTAXIS_COUNT
    }
    int v1 = 2 * (currCl + (int)virtualAxis + 8 * currCl);
    int v2 = dword_F13368[v1];
    if (v2 == -1)
        return 0.0f;
    float axisDeflection = (float)gaGlobs[currCl].axesValues[v2] * 0.0078125f;
    if (dword_F1336C[v1] != 1)
        return axisDeflection;
    int v4 = axisSameStick[v2];
    float otherAxisDeflection;
    if (v4 == GPAD_PHYSAXIS_NONE)
        otherAxisDeflection = 0.0f;
    else
        otherAxisDeflection = (float)gaGlobs[currCl].axesValues[v4] * 0.0078125f;
    return (float)(sqrt(otherAxisDeflection * otherAxisDeflection
                        + axisDeflection * axisDeflection)
                   * axisDeflection);
}

// ea: 0x52EF30
float CL_GamepadPhysicalAxisValue(int physicalAxis)
{
    return (float)gaGlobs[currCl].axesValues[physicalAxis];
}

// ea: 0x52EBB0
void Gamepad_WriteBindings(int f)
{
    extern void FS_Printf(int h, const char* fmt, ...);
    FS_Printf(f, "unbindallaxis\n");
    for (int i = 0; i < 6; ++i)
    {
        int idx = 16 * currCl + 2 * currCl + 2 * i;
        if (dword_F13368[idx] != -1)
            FS_Printf(f, "bindaxis %s %i\n", virtualAxisNames[i],
                      dword_F13368[idx]);
    }
}

// ============================================================================
// UI syscalls
// ============================================================================

// ea: 0x52E3C0
int CL_UISystemCalls(int* args)
{
    Com_Error(1, "Bad UI system trap: %i", *args);
    return -1;
}

// ea: 0x52E3E0
void CL_ShutdownUI()
{
    cls.keyCatchers &= ~2;
    if (cls.state == 2)
    {
        cls.keyCatchers &= ~2;
    }
}

// ea: 0x52E500
char CL_InitUI()
{
    return 0;
}

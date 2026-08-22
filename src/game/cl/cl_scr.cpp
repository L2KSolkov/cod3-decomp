// ============================================================================
// cl_scr.cpp - screen draw helpers + cubemap + gamepad + UI (cl.o)
// 21 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"
#include "cl_console.h"
#include "game/game_types.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Minimal view of SoundDevice (full class in game/sv/sv_stubs.h).
class SoundDevice { public: static SoundDevice* sInst; };  // ?sInst@SoundDevice@@2PAV1@A

struct fe_menusys_view {
    uint8_t _pad[0x2A];
    bool is_active;
    uint8_t _pad2B;

    bool IsSystemActive() const { return is_active; }
};

struct IGOFrontEnd {
    virtual void Update(float time_inc);
};

class InGameMenuSystem {
public:
    virtual void ReturnToPreviousMenu(int fallback);
    bool IsSystemActive() const
    {
        return *reinterpret_cast<const bool*>(
            reinterpret_cast<const unsigned char*>(this) + 0x2A);
    }
};
class DialogMenuSystem {
public:
    virtual void MakeActive(int menu);
};

struct FEManager {
    uint8_t _pad00[0x14];
    IGOFrontEnd* IGO;                  // +0x14
    void* ControllerDisconnected;      // +0x18
    fe_menusys_view* fems;             // +0x1C
    uint8_t _pad20[0x13];
    bool skipFE;                       // +0x33
    bool enablePause;                  // +0x34
    bool IGO_active;                   // +0x35
    bool inGame;                       // +0x36
    bool menuMovieRunning;             // +0x37
    bool legalMoviesFinished;          // +0x38
    bool skipAllLegalMovies;           // +0x39
    bool skipAllMovies;                // +0x3A
    bool renderMovieOnly;              // +0x3B
    bool mDontDrawHud;                 // +0x3C
    char loadLevel[128];               // +0x3D
    uint8_t _padBD[3];
    float saveTime;                    // +0xC0
    DialogMenuSystem* mDMS[1];         // +0xC4
    InGameMenuSystem* mIGMS[1];        // +0xC8
    fe_menusys_view* mAARS;            // +0xCC

    void LoadFrontEnd();
    void UpdateFrontEnd(float time_inc);
    void UpdateAARMenus(float time_inc);
    void UpdateInGameMenus(float time_inc);
    void DrawFrontEnd();
    void DrawAARMenus();
    void DrawInGameMenus();
    InGameMenuSystem* GetIGMS(int client);
    DialogMenuSystem* GetDMS(int client);
};
extern FEManager g_femanager;


// Minimal view of InteractionController (full class in g_local.h).
class InteractionController {
public:
    uint8_t _pad[0x1B8];
    void*   mRenderText[5];  // +0x1B8 (InteractionRenderText*)

    static InteractionController* Inst(int instance);  // ?Inst@InteractionController@@SAPAV1@H@Z
    int DoRenderText(unsigned int index);  // ?DoRenderText@InteractionController@@QBEHH@Z (cl.o 0x5397F0)
};


// ?cl@@3PAUclientActive_t@@A (cl.o data @ 0xDF01C0)
clientActive_t cl[2];

// ?cgvm@@3PAUvm_s@@A (cl.o data @ 0xF1577C)
struct vm_s;
vm_s* cgvm = NULL;

// Xbox per-client controller port table (BSS @ 0xF6A28C; unnamed in binary)
int dword_F6A28C[4 * 802];

extern int dword_F6A290[4 * 802];  // Xbox dev/retail flag array @ 0xF6A290

// ============================================================================
// Externs
// ============================================================================
extern void Com_Printf(const char* fmt, ...);
enum errorParm_t;
extern void Com_Error(errorParm_t code, const char* fmt, ...);
extern int Cmd_Argc();
extern char* Cmd_Argv(int arg);
extern char* va(const char* fmt, ...);
extern int com_skelTimeStamp;
extern int bCL_AllowedAllocSkel;
extern int dword_F1719C;
extern int dword_F171A0;
extern int dword_F171B8;
extern struct vm_s { int (__cdecl* systemCall)(int*); }* cgvm;
extern int VM_Call(struct vm_s* vm, int callnum, ...);
extern void CL_CubemapShotUsage();
extern int com_skelTimeStamp;
extern int bCL_AllowedAllocSkel;
extern int lFirstLocalClientIndex;
extern int lLastLocalClientIndex;
extern int currCl;
int dword_F170F8;
extern int dword_F170F0;
extern int dword_F170FC;
extern int dword_F170EC;
extern int time_frontend;
extern int time_backend;
bool gDisableRendering;
extern bool gSkipFrontEnd;
extern float Com_GetScreenTimeDelta();
extern void nullsub_35();
int scr_initialized;
extern void Cmd_ExecuteServerString(const char* text);
extern void CL_CGameRendering();
extern void Con_DrawConsole();
extern void SoundDevice_UndampenAllSounds(void* self);
extern void Cvar_Set(const char* var_name, const char* value);
struct glconfig_t;
extern void CL_GetGlconfig(glconfig_t* glconfig);

namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1 };
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

struct PakInfoNode {
    uint8_t _pad00[0xB4];
    TPakId pakId;
};

class PakManager {
public:
    static PakManager* sInst;
    uint8_t _pad00[0x2C];
    void* mProgressCallback;
    const PakInfoNode* GetPakInfo(const char* long_name) const;
    bool IsLoaded(TPakId id) const;
    void SetUserDistance(const PakInfoNode* cpak, float dist);
    TPakId SyncLoadPak(const PakInfoNode* cpak);
};

const PakInfoNode* sFrontEndInfo = nullptr;
extern const PakInfoNode* sLoadingScreenInfo;
class GamePause {
public:
    static void SetGamePaused(int client, bool paused);
};
class FEMenu {
protected:
    void ClearAllButtons();
    friend class PauseMenu;
};

// PauseMenu (cl.o; menus live on the InGameMenuSystem at +0x04)
class PauseMenu : public FEMenu {
public:
    uint8_t _pad[0x4F];
    int mVersion;  // +0x50
    static PauseMenu* Me(int version);  // ?Me@PauseMenu@@SAPAV1@H@Z (cl.o 0x928DB0)
    void UnPause();
};
// ea: 0x928DB0
PauseMenu* PauseMenu::Me(int version)
{
    InGameMenuSystem* igms = g_femanager.GetIGMS(version);
    void** menus = *(void***)((char*)igms + 4);  // InGameMenuSystem::menus
    return (PauseMenu*)menus[0];
}

void PauseMenu::UnPause()
{
    InGameMenuSystem* IGMS = g_femanager.GetIGMS(mVersion);
    IGMS->ReturnToPreviousMenu(-1);
    DialogMenuSystem* DMS = g_femanager.GetDMS(mVersion);
    DMS->MakeActive(-1);
    GamePause::SetGamePaused(currCl, false);
    ClearAllButtons();
}

// ea: 0x5397F0
int InteractionController::DoRenderText(unsigned int index)
{
    if (index > 4)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InteractionController.h";
        AeAssert::gCurrentLine = 172;
        AeAssert::gCurrentExpr = "index>=0&&index<kNumInteractRenderTexts";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bad index"))
            __debugbreak();
    }
    // mRenderText elements expose GetText at vtable slot 0xBC/4 (shell.o)
    void* textObj = mRenderText[index];
    void** vt = *(void***)textObj;
    typedef Broc::string(__thiscall* GetTextFn)(void* self);
    Broc::string text = ((GetTextFn)vt[0xBC / 4])(textObj);
    bool empty = true;
    if (text.mBlock != nullptr)
    {
        const char* data = (const char*)(text.mBlock + 1);
        if (data != nullptr && data[0] != 0)
            empty = false;
    }
    return !empty;
}
int InteractionController_DoRenderText(void* self, int index)
{
    return ((InteractionController*)self)->DoRenderText((unsigned int)index);
}
void InteractionController_RenderText(void* self)
{
    (void)self;
}

// re renderer externs
// Pointer table matching core/common.cpp's `re` layout (cl.o re_export).
struct refexport_t {
    void (*Shutdown)(int);
    void (*BeginRegistration)(void*);
    void* (*RegisterModel)(void* result, const char*, int, int);
    int (*RegisterShader)(const char*, int);
    int (*RegisterShaderNoMip)(const char*, int);
    void (*LoadWorld)(const char*, int*);
    void (*SetFXImageMemory)(int);
    int (*GetFXImageMemory)();
    int (*GetImageMemory)();
    float (*GetFarPlaneDist)();
    void (*EndRegistration)();
    void (*ClearScene)();
    void (*AddPolyToScene)(void*, int, const void*);
    void (*AddLightToScene)(const float*, float, float, float, float);
    void (*SetCullDist)(float);
    void (*SetFog)(int, int, int, float, float, float, float);
    void (*RenderScene)(const void*);
    void (*ClearFlares)();
    void (*SetColor)(const float*);
    void (*DrawStretchPic)(float, float, float, float, float, float, float,
                           float, void*);
    void (*DrawStretchPicGradient)(float, float, float, float, float, float,
                                   float, float, void*, const float*, int);
    void (*DrawStretchPicRotate)(float, float, float, float, float, float,
                                 float, float, float, void*);
    void (*DrawQuadPic)(const float (*)[2], const float (*)[2], void*);
    void (*DrawStretchRaw)(int, int, int, int, int, int,
                           const unsigned char*, int, int);
    void (*UploadCinematic)(int, int, int, int, const unsigned char*, int, int);
    void (*BeginFrame)();
    void (*EndFrame)(int*, int*);
    void (*SaveScreen)();
    void (*TrackStatistics)(void*);
    int (*PickShader)(const float*, const float*, char*, char*, char*, int);
    void (*ResetImageAllocations)();
    void (*FreeImageAllocations)();
    void (*CubemapShot)(const char*, int, int, float, float);
    void (*CubemapWaterShot)(const char*, int, int, float*, float*);
    void (*LocateDebugStrings)(void*, int);
    void (*LocateDebugLines)(void*, int);
    int (*Text_Width)(const char*, int, float, float, int);
    int (*Text_Height)(int, float);
    void (*Text_Paint)(float, float, int, float, const float*, const char*,
                       float, int, int);
    int (*Text_ConsoleWidth)(const short*, int, float, float, int);
    void (*Text_ConsolePaint)(float, float, int, float, const float*,
                              const short*, float, int, int);
    void (*Text_PaintWithCursor)(float, float, int, float, const float*,
                                 const char*, int, char, float, int, int);
};
extern refexport_t re;

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
                      0.0f, 0.0f, 0.0f, 0.0f, (void*)dword_F171B8);
    re.SetColor(nullptr);
}

// ea: 0x52DBD0
void SCR_DrawPic(float x, float y, float width, float height, void* tex)
{
    re.DrawStretchPic((float)dword_F1719C * 0.0015625f * x,
                      (float)dword_F171A0 * 0.0020833334f * y,
                      (float)dword_F1719C * 0.0015625f * width,
                      (float)dword_F171A0 * 0.0020833334f * height,
                      0.0f, 0.0f, 1.0f, 1.0f, tex);
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

// ea: 0x533D60
void SCR_DrawScreenField()
{
    if (cls.state != 5)  // CA_MAP_RESTART
    {
        int v0 = currCl;
        if (currCl != lFirstLocalClientIndex)
        {
            if (g_femanager.mAARS != nullptr
                && g_femanager.mAARS->IsSystemActive())
                return;
            v0 = currCl;
        }
        if (cls.state != 0)  // CA_DISCONNECTED
        {
            if (cls.state == 1)  // CA_LOADING
            {
                if (g_femanager.mIGMS[v0] != nullptr
                    && g_femanager.mIGMS[v0]->IsSystemActive())
                {
                    g_femanager.DrawInGameMenus();
                }
                if (g_femanager.fems != nullptr
                    && g_femanager.fems->IsSystemActive())
                {
                    g_femanager.DrawFrontEnd();
                }
            }
            else if (cls.state == 2)  // CA_ACTIVE
            {
                if (*(&dword_F6A290[0] + 802 * v0) == 2)
                {
                    if (cgvm == nullptr)
                    {
                        ASSERT("cgvm", "c:\\cod\\code\\game\\cl_scrn.cpp", 378);
                    }
                    CL_CGameRendering();
                    void* v1 = InteractionController::Inst(currCl);
                    if (InteractionController_DoRenderText(v1, 0) != 0)
                    {
                        void* v2 = InteractionController::Inst(currCl);
                        InteractionController_RenderText(v2);
                    }
                }
            }
            else
            {
                Com_Error((errorParm_t)0, "SCR_DrawScreenField: bad cls.state");
            }
        }
        else
        {
            cls.keyCatchers = 2;
            SoundDevice_UndampenAllSounds(SoundDevice::sInst);
            Cvar_Set("g_reloading", "0");
        }
        if (g_femanager.mAARS != nullptr
            && g_femanager.mAARS->IsSystemActive())
        {
            g_femanager.DrawAARMenus();
        }
        else
        {
            if (g_femanager.mIGMS[currCl] != nullptr
                && g_femanager.mIGMS[currCl]->IsSystemActive())
            {
                g_femanager.DrawInGameMenus();
            }
        }
        if (g_femanager.fems != nullptr
            && g_femanager.fems->IsSystemActive())
        {
            g_femanager.DrawFrontEnd();
        }
        Con_DrawConsole();
    }
}

// ea: 0x533F20
void SCR_UpdateScreen(float screen_time_inc)
{
    if (cls.state == 1)  // CA_LOADING
        nullsub_35();
    if (scr_initialized != 0)
    {
        if (cls.state == 2)  // CA_ACTIVE
        {
            if (currCl == lFirstLocalClientIndex && ++com_skelTimeStamp == 0)
                com_skelTimeStamp = 1;
            if (bCL_AllowedAllocSkel != 0)
            {
                ASSERT("!bCL_AllowedAllocSkel",
                       "c:\\cod\\code\\game\\cl_scrn.cpp", 480);
            }
            bCL_AllowedAllocSkel = 1;
        }
        if (!gDisableRendering)
        {
            if (currCl == lFirstLocalClientIndex || cls.state != 2)
                re.BeginFrame();
            SCR_DrawScreenField();
            if (currCl == lLastLocalClientIndex || cls.state != 2)
                re.EndFrame(&time_frontend, &time_backend);
        }
        if (g_femanager.loadLevel[0] != 0)
        {
            char tmpstr[128];
            sprintf(tmpstr, "spmap %s", g_femanager.loadLevel);
            g_femanager.loadLevel[0] = 0;
            Cmd_ExecuteServerString(tmpstr);
        }
        if (g_femanager.fems != nullptr
            && g_femanager.fems->IsSystemActive())
        {
            if (lLastLocalClientIndex == currCl)
                g_femanager.UpdateFrontEnd(screen_time_inc);
            return;
        }
        if (g_femanager.mAARS != nullptr
            && g_femanager.mAARS->IsSystemActive())
        {
            g_femanager.UpdateAARMenus(screen_time_inc);
        }
        else
        {
            if (g_femanager.mIGMS[currCl] != nullptr
                && g_femanager.mIGMS[currCl]->IsSystemActive())
            {
                g_femanager.UpdateInGameMenus(screen_time_inc);
            }
            else
            {
                if (g_femanager.IGO_active && g_femanager.IGO != nullptr)
                {
                    g_femanager.IGO->Update(screen_time_inc);
                }
            }
        }
        if (cls.state == 2)  // CA_ACTIVE
            bCL_AllowedAllocSkel = 0;
    }
}

// ea: 0x534C20
void SCR_UpdateScreen()
{
    float screen_time_inc = Com_GetScreenTimeDelta();
    SCR_UpdateScreen(screen_time_inc);
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
int dword_F13368[64];  // cl.o BSS
int dword_F1336C[64];  // cl.o BSS
struct GpadAxesGlob {
    int axesValues[6];
};
GpadAxesGlob gaGlobs[2];  // ?gaGlobs@@3PAUGpadAxesGlob@@A (cl.o)
const char* virtualAxisNames[6];  // cl.o
const char* szShotName[6];        // cl.o
int axisSameStick[6];  // cl.o BSS
enum { GPAD_PHYSAXIS_NONE = -1 };

// ea: 0x52E660
void CL_InitGamepadAxisBindings()
{
    int* axisValues = gaGlobs[0].axesValues;
    int* result = &dword_F1336C[0];
    for (int i = 6; i != 0; --i)
    {
        *(result - 1) = -1;
        *result = 1;
        *axisValues = 0;
        result += 2;
        ++axisValues;
    }
}

// ea: 0x52EE00
void CL_GamepadEvent(unsigned int physicalAxis, int value)
{
    if (physicalAxis >= 6)
        Com_Error((errorParm_t)1, "CL_GamepadEvent: bad axis %i", physicalAxis);
    gaGlobs[currCl].axesValues[physicalAxis] = value;
}

// ea: 0x52EE40
float CL_GamepadAxisValue(unsigned int virtualAxis)
{
    if (virtualAxis >= 6)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cl_gamepad.cpp";
        AeAssert::gCurrentLine = 300;
        AeAssert::gCurrentExpr =
            "virtualAxis >= 0 && virtualAxis < GPAD_VIRTAXIS_COUNT";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", virtualAxis))
            __debugbreak();
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
    Com_Error((errorParm_t)1, "Bad UI system trap: %i", *args);
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
    char result = 0;
    if (g_femanager.fems != nullptr
        && g_femanager.fems->IsSystemActive())
        return 1;
    if (g_femanager.inGame)
        return 1;

    g_femanager.LoadFrontEnd();
    if (g_femanager.fems != nullptr)
        g_femanager.fems->is_active = true;
    g_femanager.skipFE = false;
    PakManager::sInst->mProgressCallback = nullptr;

    sFrontEndInfo = PakManager::sInst->GetPakInfo("mp_FrontEnd");
    const PakInfoNode* loadingInfo =
        PakManager::sInst->GetPakInfo("mp_loadingscreen");
    sLoadingScreenInfo = loadingInfo;
    if (loadingInfo != nullptr
        && !PakManager::sInst->IsLoaded(loadingInfo->pakId))
    {
        PakManager::sInst->SetUserDistance(loadingInfo, 0.0f);
        PakManager::sInst->SyncLoadPak(loadingInfo);
    }

    if (sFrontEndInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cl_ui.cpp";
        AeAssert::gCurrentLine = 245;
        AeAssert::gCurrentExpr = "sFrontEndInfo!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Can't find FrontEnd pak"))
            __debugbreak();
        return result;
    }
    if (PakManager::sInst->IsLoaded(sFrontEndInfo->pakId))
        return 1;
    if (gSkipFrontEnd)
        return 1;

    PakManager::sInst->SetUserDistance(sFrontEndInfo, 0.0f);
    result = (char)PakManager::sInst->SyncLoadPak(sFrontEndInfo);
    return result;
}

// ============================================================================
// cg_view.cpp - view/fov/compass/weapon-position helpers (cg.o cg_view.cpp)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/core/core_types.h"
#include "game/cvar_types.h"
#include "game/game_types.h"
#include "game/snapshot_types.h"
#include "game/trace_types.h"
#include "ngl/ngl_scene.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// cg_local.h carries the cg-side PlayerState mirror.  Keep the command's
// release 0x30-byte field layout here without importing player_types.h (which
// declares a separate full PlayerState type used by game.o).
struct usercmd_s {
    int serverTime;
    int buttons;
    int weapon;
    int angles[3];
    char forwardmove;
    char rightmove;
    char upmove;
    unsigned char _pad1B;
    float gunPitch;
    float gunYaw;
    float gunXOfs;
    float gunYOfs;
    float gunZOfs;
};
static_assert(sizeof(usercmd_s) == 0x30, "cg usercmd_s size mismatch");

struct cg_pmove_abi {
    PlayerState* ps;
    usercmd_s cmd;
    usercmd_s oldcmd;
    int tracemask;
    unsigned char _pad68[0xD0];
    int pmove_fixed;
    int pmove_msec;
    void (__cdecl* trace)(trace_t*, const math::Position3&,
                          const math::Position3&, const math::Position3&,
                          const math::Position3&,
                          const collision_context_t&);
    void (__cdecl* boxtrace)(trace_t*, const math::Position3&,
                             const math::Position3&, const math::Position3&,
                             const math::Position3&,
                             const collision_context_t&);
    void (__cdecl* capsuletrace)(trace_t*, const math::Position3&,
                                 const math::Position3&,
                                 const math::Position3&,
                                 const math::Position3&,
                                 const collision_context_t&);
    int (__cdecl* pointcontents)(const math::Position3&,
                                 const collision_context_t&);
};
static_assert(offsetof(cg_pmove_abi, pmove_fixed) == 0x138,
              "cg pmove fixed offset mismatch");
static_assert(offsetof(cg_pmove_abi, trace) == 0x140,
              "cg pmove trace offset mismatch");

enum errorParm_t;
extern void Com_Error(errorParm_t code, const char* fmt, ...);

// Minimal view of GamePause (full class in game/sv/sv_stubs.h).
struct GamePause { static bool IsGamePaused(int client); };

class SoundDevice {
public:
    static SoundDevice* sInst;
};


// Minimal view of RumbleManager (full class in core/core_systems.h).
class RumbleEffect;
class RumbleEffectInstanceHandle {
public:
    int mVal;  // +0x00
};
class RumbleManager {
public:
    static RumbleManager* Inst(int instance);  // ?Inst@RumbleManager@@SAPAV1@H@Z (g.o)
    RumbleEffectInstanceHandle Play(const RumbleEffect& effect,
                                    float intensity);
};


extern const char* CL_GetConfigString(int index);  // ?CL_GetConfigString@@YAPBDH@Z (cl.o)


// Minimal view of InteractionController (full class in g_local.h).
class InteractionController {
public:
    static InteractionController* Inst(int instance);  // ?Inst@InteractionController@@SAPAV1@H@Z
};


// Minimal view of EntityManager (full class in game/sv/sv_stubs.h).
class EntityManager {
public:
    static EntityManager* sInst;  // ?sInst@EntityManager@@2PAV1@A (game.o)
    Entity* GetPlayer(int idx);   // ?GetPlayer@EntityManager@@QAEPAVEntity@@H@Z
    Entity* mPlayers[16];         // +0x04
};


extern int currCl;
extern int dword_F64140[4 * 1580];
extern int dword_F64144[4 * 1580];
extern int dword_F64148[4 * 1580];
extern int dword_F6414C[4 * 1580];
extern int dword_F64150[4 * 1580];
extern int dword_F64154[4 * 1580];
extern int dword_F64158[4 * 1580];
extern int dword_F6415C[4 * 1580];
extern int dword_F64160[4 * 1580];
extern int dword_F63BB4[4 * 1580];
extern int dword_F63BAC[4 * 1580];
extern int dword_F63BA8[4 * 1580];
extern int dword_F63CF4[4 * 1580];
extern int dword_F641E0[4 * 1580];
extern int dword_F641E4[4 * 1580];
extern int dword_F641E8[4 * 1580];
extern int dword_F641EC[4 * 1580];
extern float dword_F63CB4[4 * 1580];
extern float* dword_F63B8C[4 * 1580];
extern int dword_F63B34[4 * 1580];
extern int dword_F6355C[4 * 1580];
extern int dword_F62964[4 * 1580];
extern float dword_F63560[4 * 1580];
extern float dword_F63F34[4 * 1580];
extern float dword_F63F38[4 * 1580];
extern float dword_F63F3C[4 * 1580];
extern float dword_F63F40[4 * 1580];
extern float dword_F63F44[4 * 1580];
extern float dword_F63F48[4 * 1580];
extern float dword_F63F4C[4 * 1580];
extern float dword_F63F50[4 * 1580];
extern float dword_F63F54[4 * 1580];
extern float dword_F63F58[4 * 1580];
extern float unk_F6A278[4 * 802];
extern float unk_F6A27C[4 * 802];
extern int iLastCompassTime_0;
extern int iLastCompassTime_1;
extern float lastChange[4];
extern int lastVehPos[4];
extern float color[4];
struct View_Window {
    float XPos;    // +0x00
    float YPos;    // +0x04
    float Width;   // +0x08
    float Height;  // +0x0C
    float FovX;    // +0x10
    float FovY;    // +0x14
    unsigned int Safety; // +0x18
};
struct cgs_t {
    unsigned char _pad[0x84];
    int vidWidth;  // +0x84
    int vidHeight; // +0x88
};
extern cgs_t cgs[2];
extern vmCvar_t cg_viewsize;
extern vmCvar_t cg_letterbox;
extern void Cvar_VMSet(vmCvar_t* vmCvar, const char* value);
extern struct vmCvar_t cg_fov;
extern vmCvar_t cg_altTankCam;
extern vmCvar_t cg_hudCompassSpringyPointers;
extern struct vmCvar_t cg_drawGun;
extern float dword_F63C50[4 * 1580];
extern float dword_F63C54[4 * 1580];
extern float dword_F63C58[4 * 1580];
extern float dword_F63C5C[4 * 1580];

extern const float AngleNormalize360(float angle);
extern const const float AngleNormalize180(float angle);
extern const float AngleSubtract(float a1, float a2);
extern void CL_SetViewAnglesAxis(int axis, float angle);
extern bool BG_AllowPlayerWeaponAtVehiclePos(int vehType, int vehPos);
extern int BG_IsAimDownSightWeapon(int iWeapon);
extern weaponFileInfo_t* BG_GetPlayerWeaponInfo();
extern void CG_DrawGameScreenFade();
extern void trap_R_SetColor(const float* rgba);
struct nglTexture;
extern void trap_R_DrawStretchPic(float x, float y, float w, float h, float s1,
                                  float t1, float s2, float t2, nglTexture* tex,
                                  float z);
extern void CG_FillRect(float x, float y, float width, float height,
                        const float* color, float z);
extern nglTexture* cgsGlobal_media_whiteShader;

// ea: 0x00688620
void CG_Fade(int r, int g, int b, int a, int time, int duration, int viewport)
{
    if (viewport != -1)
        currCl = viewport;
    int v7 = 1580 * currCl;
    float alpha = a * 0.0039215689f;
    memcpy(&dword_F64154[v7], &alpha, 4);
    dword_F6415C[v7] = time;
    dword_F64160[v7] = duration;
    if (duration + dword_F6415C[v7] <= cgGlobal_time)
        dword_F64158[v7] = dword_F64154[v7];
}

// ea: 0x006887E0
float CG_CalcPlayerHealth()
{
    float* v0 = (float*)(int)dword_F62964[1580 * currCl];
    int v1 = *(int*)((char*)v0 + 324);
    char* v2 = (char*)v0 + 16;
    if (v1 != 0 && *(int*)(v2 + 316) != 0 && *(int*)(v2 + 36) != 6)
    {
        float v4 = v1 / *(int*)(v2 + 316);
        if (v4 < 0.0f)
            return 0.0f;
        if (v4 > 1.0f)
            return 1.0f;
        return v4;
    }
    return 0.0f;
}

// ea: 0x00689340
float* CG_FadeColor(int startMsec, int totalMsec, int fadeMsec)
{
    if (startMsec == 0 || cgGlobal_time - startMsec >= totalMsec)
        return nullptr;
    int v4 = totalMsec - (cgGlobal_time - startMsec);
    if (v4 >= fadeMsec)
        *(int*)&color[3] = 1065353216;
    else
        color[3] = (float)v4 / (float)fadeMsec;
    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    return color;
}

// ea: 0x006893B0
float CG_GetNorthDirection()
{
    return *(float*)&dword_F64140[1580 * currCl];
}

// ea: 0x006893D0
int CG_UpdateCompassOrientation()
{
    float fTargetYaw =
        AngleNormalize360(*(float*)&dword_F63CB4[1580 * currCl]
                          - *(float*)&dword_F64140[1580 * currCl]);
    int v1;
    if (iLastCompassTime_0 > cgGlobal_time
        || (v1 = cgGlobal_time - iLastCompassTime_0,
            (cgGlobal_time - iLastCompassTime_0) > 500))
    {
        iLastCompassTime_0 = cgGlobal_time;
        int v13 = currCl;
        int result = 1580 * v13;
        *(float*)&dword_F64144[result] = fTargetYaw;
        dword_F64148[result] = 0;
        return result * 4;
    }
    float v2 = *(float*)&dword_F64144[1580 * currCl];
    iLastCompassTime_0 = cgGlobal_time;
    AngleSubtract(v2, fTargetYaw);
    float fYawOffset = v2;
    float v3 = fYawOffset;
    if (v1 > 0)
    {
        int v4 = currCl;
        while (true)
        {
            int v5;
            if (v1 <= 5)
            {
                v5 = v1;
                v1 = 0;
            }
            else
            {
                v5 = 5;
                v1 -= 5;
            }
            float fTimeStep = v5 * 0.001f;
            if (fabs((double)fYawOffset) < 0.25
                && fabs((double)*(float*)&dword_F64148[1580 * v4]) < 1.0)
                break;
            fYawOffset = AngleNormalize180(
                (*(float*)&dword_F64148[1580 * v4] * (v5 * 0.001f)) + v3);
            v4 = currCl;
            v3 = fYawOffset;
            if (fYawOffset <= 0.0f)
            {
                if (fYawOffset < 0.0f)
                    *(float*)&dword_F64148[1580 * currCl] =
                        (fTimeStep * 1000.0f)
                        + *(float*)&dword_F64148[1580 * currCl];
            }
            else
            {
                *(float*)&dword_F64148[1580 * currCl] =
                    *(float*)&dword_F64148[1580 * currCl]
                    - (fTimeStep * 1000.0f);
            }
            float* v6 = (float*)&dword_F64148[1580 * v4];
            float v7 = *v6 - ((*v6 * fTimeStep) * 2.0f);
            *v6 = v7;
            float v8;
            bool v9;
            if (v7 <= 0.0f)
            {
                if (fYawOffset < 0.0f)
                    *v6 = *v6 - ((*v6 * fTimeStep) * 3.5f);
                v8 = *v6 + fTimeStep;
                v9 = v8 <= 0.0f;
            }
            else
            {
                if (fYawOffset > 0.0f)
                    *v6 = *v6 - ((v7 * fTimeStep) * 3.5f);
                v8 = *v6 - fTimeStep;
                v9 = v8 >= 0.0f;
            }
            *v6 = v8;
            if (!v9)
                *v6 = 0.0f;
            float v10;
            if (*v6 > 30000.0f || (*v6 < -30000.0f))
                *v6 = *v6 > 30000.0f ? 1189765120 : -957718528;
            if (v1 <= 0)
            {
                float v11 = AngleNormalize360(v3 + fTargetYaw);
                int result = 1580 * currCl;
                *(float*)&dword_F64144[1580 * currCl] = v11;
                return result * 4;
            }
        }
        int v13 = v4;
        int result = 1580 * v13;
        *(float*)&dword_F64144[result] = fTargetYaw;
        dword_F64148[result] = 0;
        return result * 4;
    }
    float v11 = AngleNormalize360(v3 + fTargetYaw);
    int result = 1580 * currCl;
    *(float*)&dword_F64144[1580 * currCl] = v11;
    return result * 4;
}

// ea: 0x0068C970
void CG_ClampViewAngles(PlayerState* ps, const float* centerAngles,
                        const float* minClamp, const float* maxClamp)
{
    float viewAngles[3] = {ps->viewangles[0], ps->viewangles[1],
                           ps->viewangles[2]};
    float diffAngles[3];
    float newAngles[3];
    for (int v4 = 0; v4 < 3; ++v4)
    {
        if (minClamp[v4] != 0.0f || maxClamp[v4] != 0.0f)
        {
            float v18 = AngleNormalize180(viewAngles[v4]);
            viewAngles[v4] = v18;
            float angle = v18 - AngleNormalize180(centerAngles[v4]);
            float v6 = AngleNormalize180(angle);
            v18 = v6;
            diffAngles[v4] = v6;
            if (minClamp[v4] != 0.0f)
            {
                float v8 = minClamp[v4];
                if (v8 > v18)
                {
                    float v9 = v8 + 0.0099999998f;
                    newAngles[v4] = v9;
                    float angleb =
                        AngleNormalize180(centerAngles[v4]) + newAngles[v4];
                    float v11 = AngleNormalize180(angleb);
                    newAngles[v4] = v11;
                    CL_SetViewAnglesAxis(v4, v11);
                    ps->viewangles[v4] = v11;
                    continue;
                }
            }
            float v10 = maxClamp[v4];
            if (v10 != 0.0f && v18 > v10)
            {
                float v9 = v10 - 0.0099999998f;
                newAngles[v4] = v9;
                float angleb =
                    AngleNormalize180(centerAngles[v4]) + newAngles[v4];
                float v11 = AngleNormalize180(angleb);
                newAngles[v4] = v11;
                CL_SetViewAnglesAxis(v4, v11);
                ps->viewangles[v4] = v11;
            }
        }
    }
}

// ea: 0x0068D350
float CG_GetViewFov()
{
    Client* client =
        EntityManager::sInst->GetPlayer( currCl)->client;
    float fPosLerp = client->ps.fWeaponPosFrac;
    if (dword_F6355C[1580 * currCl] != 0 || client->ps.pm_type >= 6)
        fPosLerp = 0.0f;
    float fViewFov = cg_fov.value;
    if (cg_fov.value < 1.0f)
        fViewFov = 1.0f;
    else if (cg_fov.value > 160.0f)
        fViewFov = 160.0f;
    if (client->ps.pm_type >= 6)
        return cg_fov.value;
    bool vehicleBlocked = (client->ps.eFlags & 0x106000) != 0
                          && !BG_AllowPlayerWeaponAtVehiclePos(
                                 client->ps.vehType, client->ps.vehPos);
    if (!(cg_altTankCam.integer != 0 && client->ps.vehType == 2
          && (client->ps.eFlags & 0x106000) == 0)
        && vehicleBlocked)
    {
        goto LABEL_26;
    }
    if (BG_IsAimDownSightWeapon(client->ps.weapon) == 0)
        goto LABEL_26;
    if (fPosLerp == 1.0f)
    {
        fViewFov = dword_F63B8C[1580 * currCl][1600];
        goto LABEL_27;
    }
    if (fPosLerp == 0.0f)
        goto LABEL_26;
    {
        float* v4 = dword_F63B8C[1580 * currCl];
        if (dword_F63B34[1580 * currCl] != 0)
        {
            float v5 = fPosLerp - (1.0f - v4[402]);
            if (v5 > 0.0f)
            {
                float v6 = v5 / v4[402];
                if (v6 > 0.0f)
                    fViewFov = fViewFov - ((fViewFov - v4[400]) * v6);
            }
        }
        else
        {
            float v7 = fPosLerp - (1.0f - v4[403]);
            if (v7 > 0.0f)
            {
                float v6 = v7 / v4[403];
                if (v6 > 0.0f)
                    fViewFov = fViewFov - ((fViewFov - v4[400]) * v6);
            }
        }
    }
LABEL_26:
    currCl = currCl;
LABEL_27:
    return fViewFov;
}

// ea: 0x006968E0
int CG_NorthDirectionChanged()
{
    const char* ConfigString = CL_GetConfigString(11);
    float v1 = (float)atof(ConfigString);
    int result = 6320 * currCl;
    *(float*)&dword_F64140[1580 * currCl] = v1;
    return result;
}

// ea: 0x00696910
void CG_UpdateCompPointerOrientation(float a1)
{
    if (cg_hudCompassSpringyPointers.integer == 0)
    {
        *(float*)&dword_F6414C[1580 * currCl] =
            *(float*)&dword_F63CB4[1580 * currCl];
        return;
    }
    if (iLastCompassTime_1 != cgGlobal_time)
    {
        float fTargetYaw = *(float*)&dword_F63CB4[1580 * currCl];
        if (iLastCompassTime_1 > cgGlobal_time
            || (cgGlobal_time - iLastCompassTime_1) > 500)
        {
            iLastCompassTime_1 = cgGlobal_time;
            *(float*)&dword_F6414C[1580 * currCl] = fTargetYaw;
            dword_F64150[1580 * currCl] = 0;
            return;
        }
        int v3 = cgGlobal_time - iLastCompassTime_1;
        float v14 = *(float*)&dword_F6414C[1580 * currCl];
        iLastCompassTime_1 = cgGlobal_time;
        AngleSubtract(v14, fTargetYaw);
        float fYawOffset = a1;
        float v4;
        if (fabs((double)a1) <= 10.0f)
            v4 = a1;
        else if (fYawOffset >= 0.0f)
        {
            v4 = 10.0f;
            fYawOffset = 10.0f;
        }
        else
        {
            v4 = -10.0f;
            fYawOffset = -10.0f;
        }
        if (v3 > 0)
        {
            int v5 = currCl;
            while (true)
            {
                int v6;
                if (v3 <= 5)
                {
                    v6 = v3;
                    v3 = 0;
                }
                else
                {
                    v6 = 5;
                    v3 -= 5;
                }
                float fTimeStep = v6 * 0.001f;
                if (fabs((double)fYawOffset) < 0.5
                    && fabs((double)*(float*)&dword_F64150[1580 * v5]) < 2.0)
                    break;
                fYawOffset = AngleNormalize180(
                    (*(float*)&dword_F64150[1580 * v5] * (v6 * 0.001f)) + v4);
                v5 = currCl;
                v4 = fYawOffset;
                float v7 = fTimeStep;
                if (fYawOffset <= 0.0f)
                {
                    if (fYawOffset < 0.0f)
                    {
                        *(float*)&dword_F64150[1580 * currCl] =
                            (fTimeStep * 1500.0f)
                            + *(float*)&dword_F64150[1580 * currCl];
                        v7 = fTimeStep;
                    }
                }
                else
                {
                    *(float*)&dword_F64150[1580 * currCl] =
                        *(float*)&dword_F64150[1580 * currCl]
                        - (fTimeStep * 1500.0f);
                }
                float* v8 = (float*)&dword_F64150[1580 * v5];
                float v9 = *v8 - ((*v8 * v7) * 3.0f);
                *v8 = v9;
                float v12;
                bool v11;
                if (v9 <= 0.0f)
                {
                    if (fYawOffset < 0.0f)
                        *v8 = *v8 - ((*v8 * v7) * 5.0f);
                    v12 = (v7 * 2.0f) + *v8;
                    *v8 = v12;
                    v11 = v12 <= 0.0f;
                }
                else
                {
                    if (fYawOffset > 0.0f)
                        *v8 = *v8 - ((v9 * v7) * 5.0f);
                    v12 = *v8 - (v7 * 2.0f);
                    *v8 = v12;
                    v11 = v12 >= 0.0f;
                }
                if (!v11)
                    *v8 = 0.0f;
                if (*v8 > 2000.0f || *v8 < -2000.0f)
                    *v8 = *v8 > 2000.0f ? 1157234688 : -990248960;
                if (v3 <= 0)
                {
                    *(float*)&dword_F6414C[1580 * currCl] =
                        AngleNormalize360(v4 + fTargetYaw);
                    return;
                }
            }
            *(float*)&dword_F6414C[1580 * v5] = fTargetYaw;
            dword_F64150[1580 * v5] = 0;
            return;
        }
        *(float*)&dword_F6414C[1580 * currCl] =
            AngleNormalize360(v4 + fTargetYaw);
    }
}

// ea: 0x00697E30
int CG_Respawn()
{
    int v0 = 1580 * currCl;
    dword_F63BB4[v0] = -1;
    dword_F63BAC[v0] = -1;
    dword_F63CF4[v0] = 1;
    dword_F641E0[0] = -1;
    dword_F641E4[0] = 0;
    dword_F641E8[0] = 0;
    dword_F641EC[0] = 0;
    return 0;
}

// ea: 0x0069C190
void CG_ScreenFade()
{
    CG_DrawGameScreenFade();
    int v0 = 1580 * currCl;
    if (*(float*)&dword_F63F38[1580 * currCl] != 0.0f)
    {
        if (dword_F63F34[1580 * currCl] - cgGlobal_time > 0)
        {
            float v2 = (dword_F63F34[1580 * currCl] - cgGlobal_time)
                       * *(float*)&dword_F63F38[1580 * currCl];
            float color[4];
            color[0] = (*(float*)&dword_F63F4C[1580 * currCl] * (1.0f - v2))
                       + (*(float*)&dword_F63F3C[1580 * currCl] * v2);
            float v4 = (*(float*)&dword_F63F50[1580 * currCl] * (1.0f - v2))
                       + (*(float*)&dword_F63F40[1580 * currCl] * v2);
            color[1] = v4;
            color[2] = (*(float*)&dword_F63F54[1580 * currCl] * (1.0f - v2))
                       + (*(float*)&dword_F63F44[1580 * currCl] * v2);
            color[3] = (*(float*)&dword_F63F58[1580 * currCl] * (1.0f - v2))
                       + (*(float*)&dword_F63F48[1580 * currCl] * v2);
            if (color[3] != 0.0f)
            {
                trap_R_SetColor(color);
                trap_R_DrawStretchPic(
                    unk_F6A278[802 * currCl] * 0.0f,
                    unk_F6A27C[802 * currCl] * 0.0f,
                    unk_F6A278[802 * currCl] * 640.0f,
                    unk_F6A27C[802 * currCl] * 480.0f, 0.0f, 0.0f, 0.0f,
                    1.0f, cgsGlobal_media_whiteShader, 0.0f);
                trap_R_SetColor(nullptr);
            }
        }
        else
        {
            dword_F63F3C[1580 * currCl] = dword_F63F4C[1580 * currCl];
            dword_F63F40[v0] = dword_F63F50[v0];
            dword_F63F44[v0] = dword_F63F54[v0];
            int v1 = dword_F63F58[v0];
            dword_F63F48[v0] = v1;
            if (*(float*)&v1 == 0.0f)
                dword_F63F38[v0] = 0;
            else
                CG_FillRect(0.0f, 0.0f, 640.0f, 480.0f,
                            (float*)&dword_F63F3C[v0], 0.0f);
        }
    }
}

extern double VectorNormalize(float* const v);
extern void CrossProduct(const float* v1, const float* v2, float* cross);
extern void MatrixMultiply(const float (*const in1)[3],
                             const float (*const in2)[3],
                           float (*const out)[3]);
extern int dword_F64174[4 * 1580];
extern int dword_F64178[4 * 1580];
extern int dword_F6417C[4 * 1580];
extern int dword_F64170[4 * 1580];
extern int dword_F64184[4 * 1580];
extern int dword_F64188[4 * 1580];
extern float dword_F63550[4 * 1580];
extern float dword_F63C70[4 * 1580];
extern float dword_F63C74[4 * 1580];
extern float dword_F63C78[4 * 1580];
extern float dword_F63C80[4 * 1580];
extern float dword_F63C84[4 * 1580];
extern float dword_F63C88[4 * 1580];
extern float dword_F63C8C[4 * 1580];
extern float dword_F63C90[4 * 1580];
extern float dword_F63C94[4 * 1580];
extern float dword_F63C98[4 * 1580];
extern float dword_F63C9C[4 * 1580];
extern float dword_F63CA0[4 * 1580];
extern float angle[4 * 1580];
extern float dword_F63CB8[4 * 1580];
extern int dword_F64138[4 * 1580];
extern int dword_F6413C[4 * 1580];
extern float unk_F640A8[4 * 6320];
extern int dword_F640C4[4 * 1580];
extern int dword_F640E8[4 * 1580];
extern int dword_F6410C[4 * 1580];
extern int dword_F64130[4 * 1580];
extern vmCvar_t cg_camerashake;
extern vmCvar_t cg_hudDamageIconTime;
extern vmCvar_t cg_viewKickScale;
extern vmCvar_t cg_viewKickMax;
extern vmCvar_t cg_redFlashTime;
extern int dword_F64018[4 * 1580];
extern int dword_F6401C[4 * 1580];
extern int dword_F64020[4 * 1580];
extern int dword_F63FFC[4 * 1580];
extern int dword_F63F9C[4 * 1580 * 3];
extern int dword_F63FA0[4 * 1580 * 3];
extern int dword_F63FA8[4 * 1580 * 3];
float sRumIntensityMin;
float sRumIntensityMax;
float sRumIntensityFactor;
float sRumTimeMin;
float sRumTimeMax;
float sRumTimeFactor;
extern void AnglesToForward(const float* const angles, float* const forward);
extern const math::Position3 native_to_cdl_pos3(const float* v);
extern "C" int __fpclass(float value);

// cameraShake_t (cg.o, IDA type ordinal 6661; sizeof 0x24)
// Release layout is time/scale/length/radius/src/size/rumbleScale.
struct cameraShake_t {
    int time;
    float scale;
    float length;
    float radius;
    float src[3];
    float size;
    float rumbleScale;
};
static_assert(sizeof(cameraShake_t) == 0x24,
              "cameraShake_t layout mismatch");

// ea: 0x006959E0 (release cg.o)
int CG_UpdateCameraShake(cameraShake_t* shake, int client)
{
    if (cg_camerashake.integer == 0)
        return 0;

    const int elapsed = cgGlobal.time - shake->time;
    if (elapsed < 0 || elapsed >= shake->length)
        return 0;

    const math::Position3 source = native_to_cdl_pos3(shake->src);
    const int base = 1580 * client;
    const float dx = source.v.m128_f32[0] - dword_F63C70[base];
    const float dy = source.v.m128_f32[1] - dword_F63C74[base];
    const float dz = source.v.m128_f32[2] - dword_F63C78[base];
    const float distanceSq = dx * dx + dy * dy + dz * dz;
    const float falloff = 1.0f - sqrtf(distanceSq) / shake->radius;
    if ((__fpclass(falloff) & 0x297) != 0)
        CG_ASSERT("!IS_NAN(val)",
                  "c:\\cod\\code\\game\\cg_draw.cpp", 2393);

    const float scale = shake->scale;
    float value = (1.0f - ((float)elapsed / shake->length)) * scale;
    if (scale <= 0.0f)
    {
        CG_ASSERT("shake->scale > 0",
                  "c:\\cod\\code\\game\\cg_draw.cpp", 2396);
    }
    if (value <= 0.0f)
    {
        CG_ASSERT("x > 0", "c:\\cod\\code\\game\\cg_draw.cpp", 2397);
    }

    shake->size = falloff < 0.0f ? falloff / value : value * falloff;
    shake->rumbleScale = value;
    return 1;
}
extern void CG_EndShellShock(const void* parms, int time);
extern int CG_EndShellShockSound();
extern void CG_UpdateShellShockSound(const void* parms, int time,
                                     int duration);
extern "C" unsigned int CG_ShellShockPlaySound(const char* name);
extern "C" unsigned int CG_ShellShockQueueSound(const char* name);
extern "C" void CG_ShellShockSetVolume(unsigned int handle, float volume);
extern "C" bool CG_ShellShockSoundValid(unsigned int handle);
extern "C" void CG_ShellShockPlayQueued(unsigned int handle);
extern "C" void CG_ShellShockReleaseSound(unsigned int handle);
extern "C" void CG_ShellShockBusPitchFade(float pitch, float time);
extern "C" void CG_ShellShockBusVolumeFade(float volume, float time);

struct shellshock_parms_t {
    struct {
        int fadeTime;
        float kickRate;
        float kickRadius;
    } view;
    struct {
        int a;
        int b;
    } screenBlend;
    struct {
        int use;
        int fadeInTime;
        int fadeOutTime;
        int loopFadeTime;
        int loopEndDelay;
        char roomtype[16];
        float wetlevel;
        int modEndDelay;
        float channelvolume[10];
    } sound;
    struct {
        int use;
        int fadeTime;
        float maxPitchSpeed;
        float maxYawSpeed;
        float sensitivity;
    } mouse;
};
extern void CG_UpdateShellShockCamera(const void* parms, int time,
                                      int duration);
extern void CL_SetUserCmdInShellshock(int shocked);
extern void CL_CapTurnRate(float maxPitchSpeed, float maxYawSpeed);
extern void SoundDevice_UndampenAllSounds(void* self);
extern int g_doShellShock[16];
extern void nglSetSceneCallBack(nglSceneCallbackType Type,
                                void (*Fn)(void*), void* Data);
extern const float LerpAngle(float from, float to, float frac);

// ea: 0x00698650 (release cg.o)
void CG_EndShellShock(const void*, int)
{
    // SoundDevice::UndampenAllSounds is the existing Win32 sound bridge for
    // the release audio cleanup; the remaining state writes are direct IDA
    // matches from CG_EndShellShock.
    SoundDevice_UndampenAllSounds(SoundDevice::sInst);
    const int base = 1580 * currCl;
    *(float*)&dword_F64174[base] = 1.0f;
    CL_CapTurnRate(0.0f, 0.0f);
    nglSetSceneCallBack(NGLSCENE_POST, nullptr, nullptr);
    dword_F64178[base] = 0;
    dword_F6417C[base] = 0;
    CL_SetUserCmdInShellshock(0);
    g_doShellShock[currCl] = 0;
}

// ea: 0x006984D0 (release cg.o)
int CG_EndShellShockSound()
{
    SoundDevice_UndampenAllSounds(SoundDevice::sInst);
    const int base = 1580 * currCl;
    if (dword_F64170[base] != 0)
    {
        dword_F64170[base] = 0;
        CG_ShellShockBusPitchFade(1.0f, 1.0f);
        CG_ShellShockBusVolumeFade(1.0f, 1.0f);
    }
    if (dword_F64184[base] != 0)
        CG_ShellShockReleaseSound(static_cast<unsigned int>(dword_F64184[base]));
    dword_F64184[base] = 0;
    if (dword_F64188[base] != 0)
        CG_ShellShockReleaseSound(static_cast<unsigned int>(dword_F64188[base]));
    dword_F64188[base] = 0;
    return 6320 * currCl;
}

// ea: 0x006A3BC0 (release cg.o)
void CG_UpdateShellShockSound(const shellshock_parms_t* parms, int time,
                              int duration)
{
    if (parms == nullptr)
        CG_ASSERT("parms", "c:\\cod\\code\\game\\cg_shellshock.cpp", 561);
    if (time < 0)
        CG_ASSERT("time >= 0", "c:\\cod\\code\\game\\cg_shellshock.cpp", 562);
    if (duration < 0)
        CG_ASSERT("duration >= 0", "c:\\cod\\code\\game\\cg_shellshock.cpp", 563);
    if (parms->sound.use == 0)
    {
        CG_EndShellShockSound();
        return;
    }

    const int base = 1580 * currCl;
    const int fadeOutTime = parms->sound.fadeOutTime;
    const int fadeRemaining = duration + fadeOutTime
        + parms->sound.modEndDelay - time;
    float volume;
    if (fadeRemaining >= fadeOutTime)
    {
        if (time >= parms->sound.fadeInTime)
            volume = 1.0f;
        else
            volume = (float)time / (float)parms->sound.fadeInTime;
    }
    else
    {
        volume = (float)fadeRemaining / (float)fadeOutTime;
        if (volume < 0.0f)
            volume = 0.0f;
    }

    const int loopFadeTime = parms->sound.loopFadeTime;
    const int loopRemaining = parms->sound.loopEndDelay + duration
        + loopFadeTime - time;
    if (loopRemaining > 0)
    {
        if (loopFadeTime != 0)
        {
            volume = 1.0f - (float)loopRemaining / (float)loopFadeTime;
            if (volume < 0.0f)
                volume = 0.0f;
        }
        const float queuedVolume = 1.0f - volume;
        const unsigned int loopHandle =
            static_cast<unsigned int>(dword_F64184[base]);
        if (loopHandle != 0 && CG_ShellShockSoundValid(loopHandle))
        {
            CG_ShellShockSetVolume(loopHandle, queuedVolume);
        }
        else if (queuedVolume != 0.0f)
        {
            const unsigned int newHandle = CG_ShellShockPlaySound("shock_loop");
            dword_F64184[base] = static_cast<int>(newHandle);
            if (newHandle != 0 && CG_ShellShockSoundValid(newHandle))
                CG_ShellShockSetVolume(newHandle, queuedVolume);
        }
        if (dword_F64188[base] == 0)
            dword_F64188[base] = static_cast<int>(
                CG_ShellShockQueueSound("shock_end"));
    }

    const int triggerTime = parms->sound.loopEndDelay + duration
        + cgGlobal.time - time;
    if (cgGlobal.time >= triggerTime)
    {
        if (triggerTime != dword_F64170[base])
        {
            dword_F64170[base] = triggerTime;
            const unsigned int queuedHandle =
                static_cast<unsigned int>(dword_F64188[base]);
            if (queuedHandle != 0 && CG_ShellShockSoundValid(queuedHandle))
            {
                CG_ShellShockPlayQueued(queuedHandle);
                CG_ShellShockBusPitchFade(1.0f, 11.0f);
                CG_ShellShockBusVolumeFade(1.0f, 10.0f);
            }
        }
    }
    else if (dword_F64170[base] != 0)
    {
        dword_F64170[base] = 0;
    }
}

// ea: 0x006970B0
void CG_InterpolateEntityOrigin(Entity* cent)
{
    cent->s.SetLerpOrigin(cent->r.currentOrigin);
}

// ea: 0x006971E0
void CG_InterpolateEntityAngles(Entity* cent)
{
    const float frac = dword_F63550[1580 * currCl];
    const math::Position3 angles(
        LerpAngle(cent->s.apos.trBase[0], cent->s.apos.trDelta[0], frac),
        LerpAngle(cent->s.apos.trBase[1], cent->s.apos.trDelta[1], frac),
        LerpAngle(cent->s.apos.trBase[2], cent->s.apos.trDelta[2], frac));
    cent->s.SetLerpAngles(angles);
}
extern char* va(const char* fmt, ...);
enum fsMode_t;
extern int FS_FOpenFileByMode(const char* qpath, int* f, fsMode_t mode);
extern int FS_Write(const void* buffer, int len, int h);
extern int FS_Read(void* buffer, int len, int f);
extern void FS_FCloseFile(int f);
extern int Com_SaveCvarsToBuffer(const char** cvarnames, int numCvars,
                                 char* buffer, int bufsize);
extern int Com_LoadCvarsFromBuffer(const char** cvarnames, int numCvars,
                                   const char* buffer, const char* filename);
struct vmCvar_t;
extern void Cvar_Update(vmCvar_t* vmCvar);
extern "C" void* _Z_MallocInternal(int size);
extern "C" void _Z_FreeInternal(void* ptr);
extern void CG_Printf(const char* msg, ...);
extern void AxisCopy(const float (*const in)[3], float (*const out)[3]);
extern int dword_F62960[4 * 1580];
const char** cg_shock_cvar_names;  // ?cg_shock_cvar_names (cg.o)
void** cg_shock_cvar_ptrs = nullptr;  // cg.o (vmCvar_t*[])
extern vmCvar_t cg_shock_viewKickFadeTime;
extern vmCvar_t cg_shock_viewKickPeriod;
extern vmCvar_t cg_shock_viewKickRadius;
extern vmCvar_t cg_shock_sound;
extern vmCvar_t cg_shock_soundFadeInTime;
extern vmCvar_t cg_shock_soundFadeOutTime;
extern vmCvar_t cg_shock_soundLoopFadeTime;
extern vmCvar_t cg_shock_soundLoopEndDelay;
extern vmCvar_t cg_shock_soundRoomType;
extern vmCvar_t cg_shock_soundWetLevel;
extern vmCvar_t cg_shock_soundModEndDelay;
extern vmCvar_t cg_shock_volume_auto;
extern vmCvar_t cg_shock_volume_menu;
extern vmCvar_t cg_shock_volume_weapon;
extern vmCvar_t cg_shock_volume_voice;
extern vmCvar_t cg_shock_volume_item;
extern vmCvar_t cg_shock_volume_body;
extern vmCvar_t cg_shock_volume_local;
extern vmCvar_t cg_shock_volume_music;
extern vmCvar_t cg_shock_volume_announcer;
extern vmCvar_t cg_shock_volume_shellshock;
extern vmCvar_t cg_shock_mouse;
extern vmCvar_t cg_shock_mouse_fadeTime;
extern vmCvar_t cg_shock_mouse_maxpitchspeed;
extern vmCvar_t cg_shock_mouse_maxyawspeed;
extern vmCvar_t cg_shock_mouse_sensitivityscale;

// ea: 0x0068C3E0 (release cg.o)
void CG_UpdateShellShockMouse(const shellshock_parms_t* parms, int time,
                              int duration)
{
    if (parms == nullptr)
        CG_ASSERT("parms", "c:\\cod\\code\\game\\cg_shellshock.cpp", 694);
    if (time < 0)
        CG_ASSERT("time >= 0", "c:\\cod\\code\\game\\cg_shellshock.cpp", 695);
    if (duration < 0)
        CG_ASSERT("duration >= 0", "c:\\cod\\code\\game\\cg_shellshock.cpp", 696);

    if (parms->mouse.use == 0)
    {
        *(float*)&dword_F64174[1580 * currCl] = 1.0f;
        CL_CapTurnRate(0.0f, 0.0f);
        return;
    }

    const int fadeTime = parms->mouse.fadeTime;
    const int remaining = duration - time;
    if (remaining >= fadeTime)
    {
        *(float*)&dword_F64174[1580 * currCl] = parms->mouse.sensitivity;
        CL_CapTurnRate(parms->mouse.maxPitchSpeed,
                       parms->mouse.maxYawSpeed);
        return;
    }

    if (remaining > 0)
    {
        const float fraction = (float)remaining / (float)fadeTime;
        if (fraction == 1.0f)
        {
            *(float*)&dword_F64174[1580 * currCl] =
                parms->mouse.sensitivity;
            CL_CapTurnRate(parms->mouse.maxPitchSpeed,
                           parms->mouse.maxYawSpeed);
            return;
        }
        *(float*)&dword_F64174[1580 * currCl] =
            ((parms->mouse.sensitivity - 1.0f) * fraction) + 1.0f;
        CL_CapTurnRate(parms->mouse.maxPitchSpeed / fraction,
                       parms->mouse.maxYawSpeed / fraction);
        return;
    }

    *(float*)&dword_F64174[1580 * currCl] = 1.0f;
    CL_CapTurnRate(0.0f, 0.0f);
}

// ea: 0x0068BE90
void CG_PerturbCamera()
{
    if (*(float*)&dword_F64178[1580 * currCl] != 0.0f
        || *(float*)&dword_F6417C[1580 * currCl] != 0.0f)
    {
        float axis[3][3];
        float rot[3] = {1.0f,
                        *(float*)&dword_F64178[1580 * currCl],
                        *(float*)&dword_F6417C[1580 * currCl]};
        float cross[3];
        float v1[3] = {0.0f, 0.0f, 1.0f};
        VectorNormalize(rot);
        CrossProduct(v1, rot, cross);
        VectorNormalize(cross);
        CrossProduct(rot, cross, v1);
        AxisCopy((const float (*)[3])&dword_F63C80[1580 * currCl], axis);
        MatrixMultiply((const float (*)[3])rot, (const float (*)[3])axis,
                       (float (*)[3])&dword_F63C80[1580 * currCl]);
    }
}

// ea: 0x0068BF80
int CG_SaveShellShockCvars(const char* name)
{
    char filebuf[65536];
    int fh;
    if (Com_SaveCvarsToBuffer(cg_shock_cvar_names, 26, filebuf, 0x10000)
        == 0)
        return 0;
    const char* v1 = va("scripts/%s.shock", name);
    if (FS_FOpenFileByMode(v1, &fh, (fsMode_t)1 /* FS_WRITE */) < 0)
        return 0;
    FS_Write((char*)filebuf, (unsigned int)strlen((const char*)filebuf), fh);
    FS_FCloseFile(fh);
    return 1;
}

// ea: 0x0068C010
void CG_SetShellShockParmsFromCvars(shellshock_parms_t* parms)
{
    if (parms == nullptr)
        CG_ASSERT("parms", "c:\\cod\\code\\game\\cg_shellshock.cpp", 411);
    parms->view.fadeTime =
        (int)((*(float*)&cg_shock_viewKickFadeTime * 1000.0f) + 0.5f);
    if (parms->view.fadeTime < 1)
        parms->view.fadeTime = 1;
    float value = *(float*)&cg_shock_viewKickPeriod;
    if (value < 0.001f)
        value = 0.001f;
    parms->view.kickRate = 0.001f / value;
    parms->view.kickRadius = *(float*)&cg_shock_viewKickRadius;
    parms->sound.use = *(int*)&cg_shock_sound != 0;
    parms->sound.fadeInTime =
        (int)((*(float*)&cg_shock_soundFadeInTime * 1000.0f) + 0.5f);
    if (parms->sound.fadeInTime < 1)
        parms->sound.fadeInTime = 1;
    parms->sound.fadeOutTime =
        (int)((*(float*)&cg_shock_soundFadeOutTime * 1000.0f) + 0.5f);
    if (parms->sound.fadeOutTime < 1)
        parms->sound.fadeOutTime = 1;
    parms->sound.loopFadeTime =
        (int)((*(float*)&cg_shock_soundLoopFadeTime * 1000.0f) + 0.5f);
    if (parms->sound.loopFadeTime < 1)
        parms->sound.loopFadeTime = 1;
    parms->sound.loopEndDelay =
        (int)((*(float*)&cg_shock_soundLoopEndDelay * 1000.0f) + 0.5f);
    strncpy(parms->sound.roomtype, (const char*)&cg_shock_soundRoomType, 15);
    parms->sound.roomtype[15] = 0;
    float v7 = *(float*)&cg_shock_soundWetLevel;
    if (v7 < 0.0f)
        v7 = 0.0f;
    else if (v7 > 1.0f)
        v7 = 1.0f;
    parms->sound.wetlevel = v7;
    parms->sound.modEndDelay =
        (int)((*(float*)&cg_shock_soundModEndDelay * 1000.0f) + 0.5f);
    float v8 = *(float*)&cg_shock_volume_auto;
    if (v8 < 0.0f)
        v8 = 0.0f;
    else if (v8 > 1.0f)
        v8 = 1.0f;
    parms->sound.channelvolume[0] = v8;
    float v9 = *(float*)&cg_shock_volume_menu;
    if (v9 < 0.0f)
        v9 = 0.0f;
    else if (v9 > 1.0f)
        v9 = 1.0f;
    parms->sound.channelvolume[1] = v9;
    float v10 = *(float*)&cg_shock_volume_weapon;
    if (v10 < 0.0f)
        v10 = 0.0f;
    else if (v10 > 1.0f)
        v10 = 1.0f;
    parms->sound.channelvolume[4] = v10;
    float v11 = *(float*)&cg_shock_volume_voice;
    if (v11 < 0.0f)
        v11 = 0.0f;
    else if (v11 > 1.0f)
        v11 = 1.0f;
    parms->sound.channelvolume[5] = v11;
    float v12 = *(float*)&cg_shock_volume_item;
    if (v12 < 0.0f)
        v12 = 0.0f;
    else if (v12 > 1.0f)
        v12 = 1.0f;
    parms->sound.channelvolume[3] = v12;
    float v13 = *(float*)&cg_shock_volume_body;
    if (v13 < 0.0f)
        v13 = 0.0f;
    else if (v13 > 1.0f)
        v13 = 1.0f;
    parms->sound.channelvolume[2] = v13;
    float v14 = *(float*)&cg_shock_volume_local;
    if (v14 < 0.0f)
        v14 = 0.0f;
    else if (v14 > 1.0f)
        v14 = 1.0f;
    parms->sound.channelvolume[6] = v14;
    float v15 = *(float*)&cg_shock_volume_music;
    if (v15 < 0.0f)
        v15 = 0.0f;
    else if (v15 > 1.0f)
        v15 = 1.0f;
    parms->sound.channelvolume[7] = v15;
    float v16 = *(float*)&cg_shock_volume_announcer;
    if (v16 < 0.0f)
        v16 = 0.0f;
    else if (v16 > 1.0f)
        v16 = 1.0f;
    parms->sound.channelvolume[8] = v16;
    float v6 = *(float*)&cg_shock_volume_shellshock;
    if (v6 < 0.0f)
        v6 = 0.0f;
    else if (v6 > 1.0f)
        v6 = 1.0f;
    parms->sound.channelvolume[9] = v6;
    parms->mouse.use = *(int*)&cg_shock_mouse != 0;
    parms->mouse.fadeTime =
        (int)((*(float*)&cg_shock_mouse_fadeTime * 1000.0f) + 0.5f);
    if (parms->mouse.fadeTime < 1)
        parms->mouse.fadeTime = 1;
    parms->mouse.maxPitchSpeed = *(float*)&cg_shock_mouse_maxpitchspeed;
    parms->mouse.maxYawSpeed = *(float*)&cg_shock_mouse_maxyawspeed;
    parms->mouse.sensitivity = *(float*)&cg_shock_mouse_sensitivityscale;
}

// ea: 0x0068C590
void CG_ShellShockCamera()
{
}

// Release cg_shellshock.cpp table at 0xD0CDC8: 128 records, 8 IEEE-754 values each.
struct shellshock_perturbation_t { unsigned int bits[8]; };
static const shellshock_perturbation_t g_shellshock_perturbations[128] = {
    { 0xbf1044e1u, 0xbb912989u, 0xbe906a6eu, 0xbf4207e6u, 0x3ed37ae5u, 0x3e7a1ba0u, 0x3f072420u, 0xbf395150u },
    { 0xbea8d888u, 0x3f2b7803u, 0xbec9dae0u, 0xbf4359deu, 0x3e013c68u, 0x3efedb94u, 0x3ba36199u, 0xbc678184u },
    { 0x3f0f5675u, 0x3de710cbu, 0xbeaa8aa4u, 0xbf12c2adu, 0x3eabba13u, 0xbddb7f17u, 0xbf11adebu, 0xbe5a41a2u },
    { 0xbe2aad1du, 0x3f48fb44u, 0x3e99641fu, 0x3d19fb1eu, 0xbf045165u, 0x3f02c11au, 0x3e0d523bu, 0x3d0ea290u },
    { 0xbe1fea3eu, 0x3f543c7du, 0xbf7fdc7bu, 0x3ca66fd6u, 0x3e999d67u, 0x3e8181e0u, 0x3cf78573u, 0xbe976a2fu },
    { 0xbf6ad83cu, 0xbd4fb656u, 0x3d34f2f1u, 0xbe89e03fu, 0x3f16a2f5u, 0x3eb9a3b1u, 0xbec283f5u, 0x3f1e84cfu },
    { 0x3e51569fu, 0xbc9f1cfcu, 0x3c978b37u, 0x3eefa80du, 0x3f6a8b3bu, 0xbe7dd3bbu, 0x3b78f8a5u, 0x3ddd9d34u },
    { 0x3d6af577u, 0x3f1b328bu, 0x3ea63151u, 0x3e228ae7u, 0xbe05a965u, 0xbe3bca10u, 0x3f373648u, 0xbeba4b99u },
    { 0x3f7bf855u, 0x3dd948dcu, 0xbb591eebu, 0x3eb0e9f7u, 0xbea4050cu, 0xbf12ed78u, 0x3d81f4b2u, 0xbb544567u },
    { 0xbf11f6dcu, 0xbf426256u, 0x3dda059au, 0x3e914489u, 0xbf2b0cbbu, 0x3e11ce29u, 0xbf004956u, 0xbf385250u },
    { 0xbe81ae0cu, 0x3f0626f6u, 0xbd833e79u, 0xbe29ecf6u, 0xbe475819u, 0x3eddfa44u, 0xbe904817u, 0xbed5e28bu },
    { 0x3d3b8a1au, 0x3ece542eu, 0x3dd72bcbu, 0xbf0f167fu, 0x3e9fde72u, 0x3f30359cu, 0xbe86ce79u, 0xbe837cbbu },
    { 0x3f28c06au, 0x3d90bc7bu, 0x3dbfbe77u, 0xbd3fbdf1u, 0xbf600150u, 0x3e93b774u, 0x3ea8a1beu, 0x3dd8f798u },
    { 0xbe39a911u, 0x3e850d06u, 0x3e85f00bu, 0xbd97b203u, 0xbe97980fu, 0x3d027d89u, 0x3d1e0a42u, 0x3f10e1e7u },
    { 0xbe81c38bu, 0xbf37c600u, 0xbe58eb89u, 0x3eac4c7bu, 0x3f63df1au, 0x3ba2339cu, 0xbf7ad5d0u, 0xbe2ee393u },
    { 0x3d39bcbau, 0x3cb630a9u, 0xbeb10c2cu, 0x3f05d074u, 0x3dde425bu, 0x3e2964e9u, 0xbf12a2c2u, 0xbef2615bu },
    { 0x3ebcb9cbu, 0xbf5da7f4u, 0x3d9ac4f8u, 0xbea7c8b0u, 0xbeeec5d2u, 0xbf10cac5u, 0xbeb7b97cu, 0x3f1c3cc0u },
    { 0x3f1a9824u, 0x3ee14ab2u, 0x3b218bd6u, 0xbe13ea70u, 0xbe96ff19u, 0x3f4ccad5u, 0xbce837f8u, 0xbde5857bu },
    { 0xbc1b3073u, 0x3f2fa1b2u, 0x3d91b717u, 0x3ca31a4cu, 0x3f7672dau, 0x3ccc319cu, 0x3e9e5083u, 0x3f5f1dd6u },
    { 0xbdfd816bu, 0xbe9fe5ebu, 0xbeddb963u, 0xbf655f03u, 0x3f766613u, 0xbe870dc7u, 0xbf02ef0bu, 0xbeb80d80u },
    { 0xbd3446fau, 0x3ca58f71u, 0xbddfeda6u, 0xbf42dff8u, 0x3e2f1b69u, 0xbddc1483u, 0x3ed67ba2u, 0x3edededbu },
    { 0x3ee3cf2du, 0xbe0efe93u, 0x3f04c144u, 0x3ebb5fc4u, 0xbf01ca8eu, 0x3f27d534u, 0x3f02b1c4u, 0x3f024b45u },
    { 0xbe97a3fdu, 0xbf2d03a7u, 0x3f59f0e5u, 0x3e9d4845u, 0xbc0ad688u, 0xbe41461bu, 0x3f0d7df2u, 0x3edaab04u },
    { 0x3da48627u, 0xbb37d417u, 0x3d120c07u, 0x3f1c69e8u, 0x3f454595u, 0x3ecc3937u, 0xbf05aac5u, 0x3ea612c7u },
    { 0x3bc61523u, 0x3d2f4278u, 0x3ef70479u, 0x3f5957acu, 0x3e677bc0u, 0xbf05b9d8u, 0xbf2cb2fbu, 0xbf0c3d8au },
    { 0xbee24d90u, 0x3f194d94u, 0xbe3c5f39u, 0xbe8a5c1cu, 0x3f04d35bu, 0x3f228bd2u, 0x3edc5b8eu, 0x3e00435fu },
    { 0xbe3df2aau, 0xbe87672cu, 0x3cc21188u, 0x3ea03ea7u, 0xbee37996u, 0x3f5994e2u, 0x3e957e24u, 0xbf65ce4au },
    { 0xbd3bb40bu, 0xbd41094au, 0xbde9f9cfu, 0x3f0310cbu, 0x3f3cf649u, 0x3f1b9011u, 0xbf49718fu, 0xbec4a31eu },
    { 0x3e3b6284u, 0x3e87b95au, 0xbecc84b6u, 0xbe9e3950u, 0xbef73e03u, 0x3e8804dau, 0x3d746994u, 0x3dc83665u },
    { 0x3f4b0d74u, 0xbc82d7b6u, 0x3e4e7f6fu, 0x3efc21c0u, 0xbf351644u, 0xbcd68c69u, 0xbea44aa5u, 0x3ebe9b7cu },
    { 0x3f12a3dfu, 0xbf09898bu, 0x3eacdc66u, 0x3dee2b06u, 0xbf1b4602u, 0x3e3188b1u, 0xbe2a975bu, 0xbeab93cdu },
    { 0xbf158091u, 0x3e3b4e55u, 0xbf12d224u, 0xbf1f93bcu, 0xbec910e4u, 0x3ee62175u, 0x3e1b1c00u, 0x3f572485u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u },
    { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u }
};

static float CG_ShellShockPerturbation(int index, int component)
{
    union { unsigned int bits; float value; } v;
    v.bits = g_shellshock_perturbations[index & 127].bits[component & 7];
    return v.value;
}

// ea: 0x0068C5A0
void CG_UpdateShellShockCamera(const shellshock_parms_t* parms, int time,
                               int duration)
{
    const int remaining = duration - time;
    const int base = 1580 * currCl;
    if (remaining <= 0)
    {
        nglSetSceneCallBack(NGLSCENE_POST, nullptr, nullptr);
        dword_F64178[base] = 0;
        dword_F6417C[base] = 0;
        return;
    }

    if (parms == nullptr)
    {
        CG_ASSERT("parms", "c:\\cod\\code\\game\\cg_shellshock.cpp", 759);
        return;
    }

    float frac = 1.0f;
    if (remaining < parms->view.fadeTime)
        frac = (float)remaining / (float)parms->view.fadeTime;
    const float smooth = ((3.0f - (frac * 2.0f)) * frac) * frac;
    const float kickTime = (float)time * parms->view.kickRate;
    const float kickRadius = smooth * parms->view.kickRadius;
    const int index = ((int)kickTime + (61 * duration)) & 0x7f;
    const float p0 = CG_ShellShockPerturbation(index, 0);
    const float p1 = CG_ShellShockPerturbation(index, 1);
    const float p2 = CG_ShellShockPerturbation(index, 2);
    const float p3 = CG_ShellShockPerturbation(index, 3);
    const float p4 = CG_ShellShockPerturbation(index, 4);
    const float p5 = CG_ShellShockPerturbation(index, 5);
    const float p6 = CG_ShellShockPerturbation(index, 6);
    const float p7 = CG_ShellShockPerturbation(index, 7);
    const float phase = kickTime - (float)(int)kickTime;

    const float v16 = ((p6 - p4) + p2) - p0;
    const float v17 = ((p0 - p2) - v16) + (v16 * phase);
    const float v21 = (v17 * phase) + (p4 - p0);
    const float v23 = (v21 * phase) + p2;
    dword_F64178[base] = v23 * kickRadius;

    const float v18mix = ((p7 - p5) + p3) - p1;
    const float v19mix = ((p1 - p3) - v18mix) + (v18mix * phase);
    const float v21mix = (v19mix * phase) + (p5 - p1);
    const float v23mix = (v21mix * phase) + p3;
    dword_F6417C[base] = v23mix * kickRadius;
}
// ea: 0x0068C760
void CG_StartShellShock()
{
}

// ea: 0x006A3FB0
void CG_UpdateShellShock(const shellshock_parms_t* parms, int start,
                         int duration)
{
    int v3 = cgGlobal_time - start;
    int time = cgGlobal_time - start;
    if (start == 0 || v3 < 0 || v3 > duration)
        CG_EndShellShock(parms, time);
    else
    {
        CG_UpdateShellShockSound(parms, v3, duration);
        CG_UpdateShellShockMouse(parms, v3, duration);
        CG_UpdateShellShockCamera(parms, v3, duration);
        CL_SetUserCmdInShellshock(v3 < duration);
    }
}

// ea: 0x00698430
int CG_LoadShellShockCvars(const char* name)
{
    const char* v1 = va("scripts/%s.shock", name);
    int fh;
    int v2 = FS_FOpenFileByMode(v1, &fh, (fsMode_t)0 /* FS_READ */);
    int v3 = v2;
    if (v2 >= 0)
    {
        char* v5 = (char*)_Z_MallocInternal(v2 + 1);
        FS_Read((unsigned char*)v5, (unsigned int)v3, fh);
        v5[v3] = 0;
        FS_FCloseFile(fh);
        int CvarsFromBuffer =
            Com_LoadCvarsFromBuffer(cg_shock_cvar_names, 26, v5, v1);
        _Z_FreeInternal(v5);
        for (unsigned int i = 0; i < 26; ++i)
            Cvar_Update((vmCvar_t*)cg_shock_cvar_ptrs[i]);
        return CvarsFromBuffer;
    }
    CG_Printf("^1couldn't open '%s'\n", v1);
    return 0;
}

// ea: 0x00695BE0
void CG_StartShakeCamera(float p, int duration, const float* src, float radius,
                         int client)
{
    if (p > 0.0f && cg_camerashake.integer != 0)
    {
        cameraShake_t shake;
        shake.time = cgGlobal_time;
        shake.scale = p;
        shake.length = (float)duration;
        shake.radius = radius;
        shake.src[0] = src[0];
        shake.src[1] = src[1];
        shake.src[2] = src[2];
        CG_UpdateCameraShake(&shake, client);
        int v5 = 0;
        float* v6 = &unk_F640A8[6320 * client];
        while (v6[0] <= (float)cgGlobal_time
               && cgGlobal_time < (v6[0] + v6[2]))
        {
            ++v5;
            v6 += 9;
            if (v5 >= 4)
            {
                int minsize = *(int*)&shake.size;
                if (v5 != 4)
                    CG_ASSERT("i == 4", "c:\\cod\\code\\game\\cg_draw.cpp",
                              2446);
                if (shake.size > *(float*)&dword_F640C4[1580 * client])
                {
                    minsize = dword_F640C4[1580 * client];
                    v5 = 0;
                }
                if (*(float*)&minsize > *(float*)&dword_F640E8[1580 * client])
                {
                    minsize = dword_F640E8[1580 * client];
                    v5 = 1;
                }
                if (*(float*)&minsize > *(float*)&dword_F6410C[1580 * client])
                {
                    minsize = dword_F6410C[1580 * client];
                    v5 = 2;
                }
                if (*(float*)&minsize <= *(float*)&dword_F64130[1580 * client])
                {
                    if (v5 == 4)
                        return;
                }
                else
                {
                    v5 = 3;
                }
                break;
            }
        }
        *reinterpret_cast<cameraShake_t*>(
            &unk_F640A8[6320 * client + 36 * v5]) = shake;
    }
}

// ea: 0x00695DA0
void CG_ShakeCamera(int client)
{
    if (!GamePause::IsGamePaused(currCl) && cg_camerashake.integer != 0)
    {
        float scale = 0.0f;
        float sx = cgGlobal_time * 0.0016666667f;
        cameraShake_t* shake = reinterpret_cast<cameraShake_t*>(
            &unk_F640A8[6320 * client]);
        int intensity = 4;
        do
        {
            if (CG_UpdateCameraShake(shake, client) != 0
                && shake->scale > scale)
                scale = shake->size;
            ++shake;
            --intensity;
        } while (intensity != 0);
        float v2 = scale;
        if (*(float*)&dword_F6413C[1580 * client] > scale)
        {
            v2 = *(float*)&dword_F6413C[1580 * client];
            scale = v2;
        }
        if (v2 > 0.0f)
        {
            if (v2 > 1.0f)
            {
                scale = 1.0f;
                v2 = 1.0f;
            }
            float amp = v2 * 0.2f;
            float moveDir[3];
            for (int i = 0; i < 3; ++i)
                moveDir[i] = ((rand() * 0.000061035156f) - 1.0f) * amp;
            float v4 = sx * 25.132742f;
            dword_F63C70[1580 * client] += moveDir[0];
            dword_F63C74[1580 * client] += moveDir[1];
            dword_F63C78[1580 * client] += moveDir[2];
            angle[1580 * client] +=
                sinf(v4 + *(float*)&dword_F64138[1580 * client]) * scale
                * scale * 18.0f;
            dword_F63CB4[1580 * client] +=
                sinf(sx * 47.12389f + *(float*)&dword_F64138[1580 * client])
                * scale * scale * 16.0f;
            dword_F63CB8[1580 * client] +=
                sinf(sx * 37.699112f + *(float*)&dword_F64138[1580 * client])
                * scale * scale * 10.0f;
        }
        else
        {
            *(float*)&dword_F64138[1580 * client] =
                ((rand() * 0.000061035156f) - 1.0f) * 3.1415927f;
        }
    }
}

// ea: 0x0069D1B0
void CG_DamageFeedback(int yawByte, int pitchByte, float damage)
{
    float v3 = damage;
    float value = 5.0f;
    damage = damage * *(float*)&cg_viewKickScale;
    float v5 = damage;
    if (damage < 5.0f || damage > *(float*)&cg_viewKickMax)
    {
        damage = value;
        v5 = value;
    }
    int v6;
    if (yawByte == 255 && pitchByte == 255)
    {
        v6 = 1580 * currCl;
        dword_F64020[v6] = 0;
        float neg = -v5;
        memcpy(&dword_F6401C[v6], &neg, 4);
    }
    else
    {
        float angles[3] = {pitchByte * 1.4117647f, yawByte * 1.4117647f,
                           0.0f};
        float dir[3];
        AnglesToForward(angles, dir);
        int v7 = 1580 * currCl;
        float v8 = 0.0f - dir[0];
        float v9 = 0.0f - dir[1];
        float v10 = 0.0f - dir[2];
        float v11 = damage;
        float kickYaw =
            ((dword_F63C94[1580 * currCl] * (0.0f - dir[2])
              + dword_F63C90[1580 * currCl] * (0.0f - dir[1]))
             + dword_F63C8C[1580 * currCl] * (0.0f - dir[0]))
            * damage * 0.5f;
        memcpy(&dword_F64020[v7], &kickYaw, 4);
        float kickPitch =
            ((dword_F63C88[v7] * v10 + dword_F63C84[v7] * v9)
             + dword_F63C80[v7] * v8)
            * v11 * -0.5f;
        memcpy(&dword_F6401C[v7], &kickPitch, 4);
        float damageIcon = 0.0f;
        int v14 = 0;
        int v15 = 12;
        int* v16 = (int*)&dword_F63FA8[v7 * 4];
        do
        {
            if (v16[0] < dword_F63F9C[v14 + v7 * 4])
            {
                damageIcon = (float)(v15 / 12);
                v14 = v15;
            }
            v15 += 12;
            v16 += 3;
        } while (v15 < 96);
    int integer = cg_hudDamageIconTime.integer;
        v6 = 1580 * currCl;
        int v18 = 3 * (int)damageIcon;
        dword_F63F9C[v18 + v7] = *(int*)((char*)&dword_F62960[v7] + 4);
        dword_F63FA0[v18 + v7] = integer;
        float yaw = yawByte * 1.4117647f;
        *(float*)((char*)&dword_F63FA8[v18 * 4 + v6 * 4] + 4) =
            AngleNormalize360(((rand() * 0.000030517578f) - 0.5f) * 20.0f
                              + yaw);
        v3 = damage;
    }
    dword_F64018[v6] = (int)(cgGlobal_time + *(float*)&cg_redFlashTime);
    float v21 = v3 * sRumIntensityFactor;
    dword_F63FFC[v6] = *(int*)((char*)&dword_F62960[v6] + 4);
    damage = v21;
    if (sRumIntensityMax < sRumIntensityMin)
        CG_ASSERT("beg <= end", "c:\\cod\\code\\game\\com_math.h", 682);
    if (sRumIntensityMin <= v21)
    {
        damage = sRumIntensityMax;
        if (v21 <= sRumIntensityMax)
            damage = v21;
    }
    else
    {
        damage = sRumIntensityMin;
    }
    float v22 = v3 * sRumTimeFactor;
    if (sRumTimeMax < sRumTimeMin)
        CG_ASSERT("beg <= end", "c:\\cod\\code\\game\\com_math.h", 682);
    float yawa;
    if (sRumTimeMin <= v22)
    {
        yawa = sRumTimeMax;
        if (v22 <= sRumTimeMax)
            yawa = v22;
    }
    else
    {
        yawa = sRumTimeMin;
    }
    struct RumbleData {
        bool  enabled;
        unsigned char _pad[3];
        float delay;
        float intensity;
        float ramp_up_duration;
        float steady_duration;
        float ramp_down_duration;
        int   rumble_notes;
        int   m_flags;
    };
    struct RumbleEffectLocal {
        RumbleData mRumbleDataArray[2];
    } rumbleEffect;
    memset(&rumbleEffect, 0, sizeof(rumbleEffect));
    rumbleEffect.mRumbleDataArray[0].delay = 0.0f;
    rumbleEffect.mRumbleDataArray[0].intensity = 1.0f;
    rumbleEffect.mRumbleDataArray[0].ramp_up_duration = 0.0f;
    rumbleEffect.mRumbleDataArray[0].steady_duration = 1.0f;
    rumbleEffect.mRumbleDataArray[0].ramp_down_duration = 0.0f;
    rumbleEffect.mRumbleDataArray[1].delay = 0.0f;
    rumbleEffect.mRumbleDataArray[1].intensity = 1.0f;
    rumbleEffect.mRumbleDataArray[1].ramp_up_duration = 0.0f;
    rumbleEffect.mRumbleDataArray[1].steady_duration = 1.0f;
    rumbleEffect.mRumbleDataArray[1].ramp_down_duration = 0.0f;
    rumbleEffect.mRumbleDataArray[0].enabled = true;
    rumbleEffect.mRumbleDataArray[1].enabled = true;
    rumbleEffect.mRumbleDataArray[0].intensity = damage;
    rumbleEffect.mRumbleDataArray[0].steady_duration = yawa;
    rumbleEffect.mRumbleDataArray[1].intensity = damage;
    rumbleEffect.mRumbleDataArray[1].steady_duration = yawa;
    RumbleManager* v25 = RumbleManager::Inst(currCl);
    if (v25 != nullptr)
    {
        v25->Play(*(RumbleEffect*)&rumbleEffect, 1.0f);
    }
}

extern float dword_F641D8[4 * 1580];
extern float dword_F641DC[4 * 1580];
extern float dword_F64068[4 * 1580];
extern float dword_F6406C[4 * 1580];
extern float dword_F64070[4 * 1580];
extern float dword_F63B70[4 * 1580];
extern float dword_F63CC0[4 * 1580];
extern float dword_F63CC4[4 * 1580];
extern float dword_F63CC8[4 * 1580];
extern float dword_F63CD0[4 * 1580];
extern float dword_F63CD4[4 * 1580];
extern float dword_F63CE4[4 * 1580];
extern float dword_F63CE8[4 * 1580];
extern float dword_F63CC[4 * 1580];
extern int dword_F64164[4 * 1580];
extern int dword_F64168[4 * 1580];
extern int dword_F6416C[4 * 1580];
extern vmCvar_t cg_bobWeaponLag;
extern vmCvar_t cg_bobWeaponAmplitude;
extern vmCvar_t cg_bobWeaponMax;
extern vmCvar_t cg_bobWeaponRollAmplitude;
extern vmCvar_t cg_bobAmplitudeProne;
extern vmCvar_t cg_bobAmplitudeDucked;
extern vmCvar_t cg_bobAmplitudeStanding;
extern float CG_GetVerticalBobFactor(float a1, float a2, float a3);
extern float CG_GetHorizontalBobFactor(float a1, float a2, float a3);
extern void AngleVectors(const float* const angles, float* const forward,
                         float* const right, float* const up);
extern void AnglesSubtract(const math::Position3& v1,
                           const math::Position3& v2, math::Position3& v3);
float ServerTime_mTickDelta;
extern int BG_IsAimDownSightWeapon(int iWeapon);
extern const float AngleSubtract(float a1, float a2);

static float AngleSubtract2(float a, float b)
{
    return AngleSubtract(a, b);
}

// ea: 0x0068FA50
void CG_CalculateWeaponPosition_BobAngles(float* angles)
{
    angles[0] = dword_F64068[1580 * currCl] + angles[0];
    angles[1] = dword_F6406C[1580 * currCl] + angles[1];
    angles[2] = dword_F64070[1580 * currCl] + angles[2];
}

// ea: 0x0068F970
void CG_CalculateWeaponPosition_BobMovement(float* origin)
{
    float vOffset = origin[0];
    float v11 = origin[1];
    float v12 = origin[2];
    float vAxis[3], right[3], up[3];
    AngleVectors((float*)&dword_F64068[1580 * currCl], vAxis, right, up);
    origin[0] = ((0.0f - v11) * right[0]) + (up[0] * v12)
                + (vAxis[0] * vOffset);
    origin[1] = ((0.0f - v11) * right[1]) + (up[1] * v12)
                + (vAxis[1] * vOffset);
    origin[2] = ((0.0f - v11) * right[2]) + (up[2] * v12)
                + (vAxis[2] * vOffset);
}

// ea: 0x0068F710
void CG_CalculateWeaponPosition_BobOffset()
{
    float fCycle =
        *(float*)&cg_bobWeaponLag * 3.1415927f
        + *(float*)&dword_F641D8[1580 * currCl] + 6.2831855f;
    float fSpeed = *(float*)&cg_bobWeaponAmplitude
                   * *(float*)&dword_F641DC[1580 * currCl];
    CG_GetVerticalBobFactor(fCycle, fSpeed, *(float*)&cg_bobWeaponMax);
    float value = *(float*)&cg_bobWeaponMax;
    dword_F64068[1580 * currCl] = fSpeed * -1.0f;
    Client* client =
        EntityManager::sInst->GetPlayer( currCl)->client;
    int viewHeightTarget = client->ps.viewHeightTarget;
    float v4;
    if (viewHeightTarget == client->ps.proneViewHeight)
        v4 = *(float*)&cg_bobAmplitudeProne;
    else
    {
        v4 = *(float*)&cg_bobAmplitudeDucked;
        if (viewHeightTarget != client->ps.crouchViewHeight)
            v4 = *(float*)&cg_bobAmplitudeStanding;
    }
    float v28 = v4 * fSpeed;
    if (v28 > value)
        v28 = value;
    float v5 = sinf(fCycle);
    float fCyclea = fCycle - 0.47123894f;
    float fSpeeda = *(float*)&cg_bobWeaponRollAmplitude * fSpeed;
    float v26 = *(float*)&cg_bobWeaponMax;
    dword_F6406C[1580 * currCl] = v5 * v28 * -1.0f;
    Client* v7 =
        EntityManager::sInst->GetPlayer( currCl)->client;
    int v8 = v7->ps.viewHeightTarget;
    float v9;
    if (v8 == v7->ps.proneViewHeight)
        v9 = *(float*)&cg_bobAmplitudeProne;
    else
    {
        v9 = *(float*)&cg_bobAmplitudeDucked;
        if (v8 != v7->ps.crouchViewHeight)
            v9 = *(float*)&cg_bobAmplitudeStanding;
    }
    float v29 = v9 * fSpeeda;
    if (v29 > v26)
        v29 = v26;
    float v10 = sinf(fCyclea) * v29;
    int v11;
    if (v10 >= 0.0f)
        v11 = 0;
    else
    {
        CG_GetHorizontalBobFactor(fCyclea, fSpeeda, *(float*)&cg_bobWeaponMax);
        v11 = *(int*)&v10;
    }
    dword_F64070[1580 * currCl] = v11;
    Client* v20 =
        EntityManager::sInst->GetPlayer( currCl)->client;
    float fWeaponPosFrac = v20->ps.fWeaponPosFrac;
    if (fWeaponPosFrac != 0.0f)
    {
        float v21 = 1.0f
                    - ((1.0f - dword_F63B8C[1580 * currCl][1632])
                       * fWeaponPosFrac);
        dword_F64068[1580 * currCl] = dword_F64068[1580 * currCl] * v21;
        dword_F6406C[1580 * currCl] = v21 * dword_F6406C[1580 * currCl];
        dword_F64070[1580 * currCl] = v21 * dword_F64070[1580 * currCl];
    }
}

// ea: 0x006900F0
void CG_CalculateWeaponPosition_SwayMovement(float* origin)
{
    origin[1] = origin[1] - dword_F63CE4[1580 * currCl];
    origin[2] = dword_F63CE8[1580 * currCl] + origin[2];
}

// ea: 0x00690140
void CG_CalculateWeaponPosition_SwayAngles(float a1, float* angles)
{
    float v2 = angles[1];
    angles[0] = AngleSubtract2(angles[0], dword_F63CD0[1580 * currCl]);
    angles[1] = AngleSubtract2(v2, dword_F63CD4[1580 * currCl]);
}

// ea: 0x00690190
void CG_CalculateWeaponPosition_IdleAngles(float* angles)
{
    Client* client =
        EntityManager::sInst->GetPlayer( currCl)->client;
    Entity* Player = EntityManager::sInst->GetPlayer( currCl);
    int IsAimDownSightWeapon =
        BG_IsAimDownSightWeapon(Player->client->ps.weapon);
    int v4 = 1580 * currCl;
    float v5;
    if (IsAimDownSightWeapon != 0)
    {
        v5 = ((dword_F63B8C[1580 * currCl][1692]
               - dword_F63B8C[1580 * currCl][1696])
              * client->ps.fWeaponPosFrac)
             + dword_F63B8C[1580 * currCl][1696];
    }
    else
    {
        v5 = dword_F63B8C[1580 * currCl][1696];
        if (v5 == 0.0f)
            v5 = 80.0f;
    }
    int pm_flags = client->ps.pm_flags;
    float fTargScale = v5;
    float v7;
    if ((pm_flags & 1) != 0)
        v7 = dword_F63B8C[1580 * currCl][1704];
    else if ((pm_flags & 2) != 0)
        v7 = dword_F63B8C[1580 * currCl][1700];
    else
        v7 = 1.0f;
    if (v7 != dword_F63B70[1580 * currCl])
    {
        float v8;
        bool v9;
        if (v7 <= dword_F63B70[1580 * currCl])
        {
            v8 = dword_F63B70[1580 * currCl]
                 - (cgGlobal_frametime * 0.050000001f);
            v9 = v7 <= v8;
        }
        else
        {
            v8 = (cgGlobal_frametime * 0.050000001f)
                 + dword_F63B70[1580 * currCl];
            v9 = v8 <= v7;
        }
        dword_F63B70[1580 * currCl] = v8;
        if (!v9)
            dword_F63B70[v4] = v7;
    }
    float v10 = fTargScale * dword_F63B70[v4] * client->ps.mHoldBreathScale;
    angles[2] = sinf(cgGlobal_time * 0.00050000002f) * v10 * 0.039999999f
                + angles[2];
    angles[1] = sinf(cgGlobal_time * 0.00069999998f) * v10 * 0.0099999998f
                + angles[1];
    angles[0] = sinf(cgGlobal_time * 0.001f) * v10 * 0.0099999998f
                + angles[0];
}

extern weaponFileInfo_t* BG_GetInfoForWeapon(int weapon);
extern vmCvar_t cg_gun_move_minspeed;
extern vmCvar_t cg_gun_move_f;
extern vmCvar_t cg_gun_move_r;
extern vmCvar_t cg_gun_move_u;
extern vmCvar_t cg_gun_move_rate;
extern vmCvar_t cg_gun_ofs_f;
extern vmCvar_t cg_gun_ofs_r;
extern vmCvar_t cg_gun_ofs_u;
extern vmCvar_t cg_gun_rot_minspeed;
extern vmCvar_t cg_gun_rot_y;
extern vmCvar_t cg_gun_rot_p;
extern vmCvar_t cg_gun_rot_r;
extern vmCvar_t cg_gun_rot_rate;
extern vmCvar_t cg_viewKickDeflectTime;
extern vmCvar_t cg_viewKickReturnTime;
float vehicleOffsetRate;
float vehicleOffset;
extern float GetLeanFraction(float fFrac);
extern void AnglesToRight(const float* const angles, float* const right);
extern void AnglesToAxis(const float* const angles,
                         float (*const axis)[3]);
extern void AxisToAngles(const float (*const axis)[3], float* const angles);
extern const float AngleNormalize360(float angle);
extern const const float AngleNormalize180(float angle);
extern int dword_F64030[4 * 1580];
extern int dword_F64034[4 * 1580];
extern int dword_F64038[4 * 1580];
extern int dword_F6403C[4 * 1580];
extern int dword_F64040[4 * 1580];
extern int dword_F64044[4 * 1580];
extern int dword_F64048[4 * 1580];
extern int dword_F6404C[4 * 1580];
extern int dword_F64050[4 * 1580];
extern int dword_F64054[4 * 1580];
extern int dword_F64058[4 * 1580];
extern int dword_F6405C[4 * 1580];
extern int dword_F64060[4 * 1580];
extern int dword_F63FFC[4 * 1580];
extern int dword_F6401C[4 * 1580];
extern int dword_F64020[4 * 1580];
extern float dword_F63BB0[4 * 1580];
extern float dword_F63C58[4 * 1580];
extern float dword_F63C5C[4 * 1580];
extern int dword_F63C60[4 * 1580];
extern int dword_F63C64[4 * 1580];
extern float dword_F63CF0[4 * 1580];
extern int dword_F63CA8[4 * 1580];
extern bool Entity_IsLocalPlayer(const Entity* ent);
extern void CG_AdjustPositionForMover(const math::Position3* in,
                                      unsigned int mover, int fromTime,
                                      int toTime, math::Position3* out,
                                      float* outDeltaAngles);
struct trajectory_t;
extern void BG_EvaluateTrajectory(const trajectory_t* tr, int atTime,
                                  math::Position3& result);
float* unk_F63B30 = nullptr;  // ?unk_F63B30 (cg.o) playerEntity stream base
extern void CG_CalculateWeaponPosition_IdleAngles(float* angles);
extern void CG_CalculateWeaponPosition_BobMovement(float* origin);
extern bool IsPlayerFullySeatedInVehicle(Entity* player);
extern Entity* GetPlayer(int idx);

struct playerEntity_t {
    float fWeaponPosFrac;      // +0x00
    float fracDir;             // +0x04
    unsigned char _pad[0x28 - 0x08];
    float gunOfs[3];           // +0x28
    float gunAngOfs[3];        // +0x34
    unsigned char _pad2[0x44 - 0x40];
    float vLastMoveOrg[3];     // +0x44
    float vLastMoveAng[3];     // +0x50
};

static playerEntity_t* GetPlayerEntity(int client)
{
    return (playerEntity_t*)((char*)unk_F63B30 + 6320 * client);
}

// ea: 0x0068FB20
int CG_CalculateWeaponPosition_Sway()
{
    Entity* Player = EntityManager::sInst->GetPlayer( currCl);
    void* InfoForWeapon = (void*)BG_GetInfoForWeapon(Player->client->ps.weapon);
    float fWeaponPosFrac =
        EntityManager::sInst->GetPlayer( currCl)
            ->client->ps.fWeaponPosFrac;
    int v5 = *(int*)&dword_F64168[1580 * currCl];
    int v6 = v5 + *(int*)&dword_F6416C[1580 * currCl] - cgGlobal_time;
    bool v7 = v5 + *(int*)&dword_F6416C[1580 * currCl] == cgGlobal_time;
    float swayPitchScale = fWeaponPosFrac;
    float v10;
    if (v6 < 0 || v7)
        v10 = 0.0f;
    else
    {
        float swayVertScale = 1.0f;
        int v8 = *(int*)&dword_F64164[1580 * currCl];
        if (v8 == 0)
            CG_ASSERT("cg[currCl].shellshock.parms",
                      "c:\\cod\\code\\game\\cg_weapons.cpp", 1896);
        int v9 = *(int*)&dword_F64164[1580 * currCl];
        if (v6 < v9)
            swayVertScale = (float)v6 / (float)v9;
        v10 = ((3.0f - (swayVertScale * 2.0f)) * swayVertScale)
              * swayVertScale;
    }
    float swayYawScale =
        ((*(float*)((char*)InfoForWeapon + 0x6CC) - 1.0f) * v10) + 1.0f;
    float v17, v18, swayHorizScale, v20, swayVertScale2, swayMaxAngle,
        swayLerpSpeed;
    if (BG_IsAimDownSightWeapon(
            EntityManager::sInst->GetPlayer( currCl)
                ->client->ps.weapon)
        != 0)
    {
        float v12 = swayPitchScale;
        if (swayPitchScale > 0.0f
            && *(int*)((char*)InfoForWeapon + 0x654) != 0)
            return 0;
        v18 = ((*(float*)((char*)InfoForWeapon + 0x6D0)
                - *(float*)((char*)InfoForWeapon + 0x6B4))
                   * v12)
              + *(float*)((char*)InfoForWeapon + 0x6B4);
        swayHorizScale =
            ((*(float*)((char*)InfoForWeapon + 0x6DC)
              - *(float*)((char*)InfoForWeapon + 0x6C0))
                 * v12)
            + *(float*)((char*)InfoForWeapon + 0x6C0);
        v20 = ((*(float*)((char*)InfoForWeapon + 0x6E0)
                - *(float*)((char*)InfoForWeapon + 0x6C4))
                   * v12)
              + *(float*)((char*)InfoForWeapon + 0x6C4);
        swayVertScale2 =
            ((*(float*)((char*)InfoForWeapon + 0x6E4)
              - *(float*)((char*)InfoForWeapon + 0x6C8))
                 * swayPitchScale)
            + *(float*)((char*)InfoForWeapon + 0x6C8);
        swayLerpSpeed =
            ((*(float*)((char*)InfoForWeapon + 0x6D4)
              - *(float*)((char*)InfoForWeapon + 0x6B8))
                 * swayPitchScale)
            + *(float*)((char*)InfoForWeapon + 0x6B8);
        v17 = ((*(float*)((char*)InfoForWeapon + 0x6D8)
                - *(float*)((char*)InfoForWeapon + 0x6BC))
                   * v12)
              + *(float*)((char*)InfoForWeapon + 0x6BC);
        swayPitchScale = swayLerpSpeed;
    }
    else
    {
        v18 = *(float*)((char*)InfoForWeapon + 0x6B4);
        swayHorizScale = *(float*)((char*)InfoForWeapon + 0x6C0);
        v20 = *(float*)((char*)InfoForWeapon + 0x6C4);
        swayVertScale2 = *(float*)((char*)InfoForWeapon + 0x6C8);
        swayPitchScale = *(float*)((char*)InfoForWeapon + 0x6B8);
        v17 = *(float*)((char*)InfoForWeapon + 0x6BC);
    }
    float v21 = v20 * swayYawScale;
    float v51 = v17 * swayYawScale;
    float swayYawScale2 = swayHorizScale * swayYawScale;
    float v52 = v21;
    float v50[3] = {v18 * swayYawScale, v21, v17 * swayYawScale};
    AnglesSubtract(*(const math::Position3*)&angle[1580 * currCl],
                   *(const math::Position3*)&dword_F63CC0[1580 * currCl],
                   *(math::Position3*)v50);
    float mTickDelta = ServerTime_mTickDelta;
    if (mTickDelta == 0.0f)
        mTickDelta = 0.05f;
    float invFrame = 1.0f / (mTickDelta * 60.0f);
    v50[0] *= invFrame;
    v50[1] *= invFrame;
    v50[2] *= invFrame;
    float v24 = v50[0];
    if (0.0f - swayVertScale2 <= v50[0])
    {
        if (v50[0] > swayVertScale2)
            v24 = swayVertScale2;
    }
    else
        v24 = 0.0f - swayVertScale2;
    float v25 = v50[1];
    if (0.0f - swayVertScale2 <= v50[1])
    {
        if (v50[1] > swayVertScale2)
            v25 = swayVertScale2;
    }
    else
        v25 = 0.0f - swayVertScale2;
    int v26 = 1580 * currCl;
    float v27 = dword_F63CE4[1580 * currCl];
    float v28 = cgGlobal_frametime * 0.001f;
    float v29 = v25 * swayYawScale2;
    float v30 = v24 * v52;
    float sway = (v25 * swayYawScale2) - v27;
    float v31 = fabsf(sway);
    float v33;
    if (v31 <= 0.0049999999f
        || fabsf((v28 * sway) * swayPitchScale) > v31)
        v33 = v29;
    else
        v33 = ((v28 * sway) * swayPitchScale) + v27;
    dword_F63CE4[1580 * currCl] = v33;
    float v34 = dword_F63CE8[v26];
    float sway2 = v30 - v34;
    float v35 = fabsf(sway2);
    float v36;
    if (v35 <= 0.0049999999f
        || fabsf((v28 * sway2) * swayPitchScale) > v35)
        v36 = v30;
    else
        v36 = ((v28 * sway2) * swayPitchScale) + v34;
    float v37 = v24 * v51;
    float v38 = v25 * v50[2];
    dword_F63CE8[v26] = v36;
    float v39 = dword_F63CD0[v26];
    float v40 = v37;
    v50[0] = v37;
    v50[1] = v38;
    if ((v37 - v39) > 180.0f)
    {
        do
            v40 = v40 - 360.0f;
        while ((v40 - v39) > 180.0f);
        v50[0] = v40;
    }
    float v41 = dword_F63CD4[v26];
    if ((v38 - v41) > 180.0f)
    {
        do
            v38 = v38 - 360.0f;
        while ((v38 - v41) > 180.0f);
        v50[1] = v38;
    }
    float v42 = dword_F63CD0[v26];
    float sway3 = v40 - v42;
    float v43 = fabsf(sway3);
    if (v43 > 0.0049999999f && fabsf((v28 * sway3) * swayPitchScale) <= v43)
        v40 = ((v28 * sway3) * swayPitchScale) + v42;
    dword_F63CD0[v26] = v40;
    float v44 = dword_F63CD4[v26];
    float sway4 = v38 - v44;
    float v45 = fabsf(sway4);
    float v46 = (v28 * sway4) * swayPitchScale;
    float v47;
    if (v45 <= 0.0049999999f || fabsf(v46) > v45)
        v47 = v38;
    else
        v47 = v46 + v44;
    dword_F63CD4[v26] = v47;
    dword_F63CD0[v26] = AngleNormalize180(dword_F63CD0[v26]);
    dword_F63CD4[1580 * currCl] =
        AngleNormalize180(dword_F63CD4[1580 * currCl]);
    dword_F63CC0[1580 * currCl] = angle[1580 * currCl];
    dword_F63CC4[1580 * currCl] = dword_F63CB4[1580 * currCl];
    dword_F63CC8[1580 * currCl] = dword_F63CB8[1580 * currCl];
    return 0;
}

// ea: 0x00690340
void CG_CalculateWeaponPosition_BasePosition_movement(float* origin)
{
    float targetPos[3];
    float fWeaponPosFrac =
        EntityManager::sInst->GetPlayer( currCl)
            ->client->ps.fWeaponPosFrac;
    int pm_flags =
        EntityManager::sInst->GetPlayer( currCl)
            ->client->ps.pm_flags;
    float base;
    if ((pm_flags & 0x10000) != 0)
        base = dword_F63B8C[1580 * currCl][347];
    else if ((pm_flags & 1) != 0)
        base = dword_F63B8C[1580 * currCl][350];
    else if ((pm_flags & 2) != 0)
        base = dword_F63B8C[1580 * currCl][349];
    else
        base = dword_F63B8C[1580 * currCl][348];
    float fMin = base + *(float*)&cg_gun_move_minspeed;
    if (dword_F641DC[1580 * currCl] <= fMin
        || EntityManager::sInst->GetPlayer( currCl)
                   ->client->ps.weaponstate
               == 5)
    {
        targetPos[0] = 0.0f;
        targetPos[1] = 0.0f;
        targetPos[2] = 0.0f;
        goto apply;
    }
    float fFactor =
        (dword_F641DC[1580 * currCl] - fMin)
        / (*(float*)((char*)&EntityManager::sInst->GetPlayer(
                                                     currCl)
                          ->client->ps
                     + 0x31C)
           - fMin);
    if (fFactor < 0.0f)
        fFactor = 0.0f;
    else if (fFactor > 1.0f)
        fFactor = 1.0f;
    {
        int pm = EntityManager::sInst->GetPlayer( currCl)
                     ->client->ps.pm_flags;
        float v8, v9, v10;
        if ((pm & 0x10000) != 0)
        {
            v8 = dword_F63B8C[1580 * currCl][315] * fFactor;
            v9 = dword_F63B8C[1580 * currCl][316] * fFactor;
            v10 = dword_F63B8C[1580 * currCl][317] * fFactor;
        }
        else if ((pm & 1) != 0)
        {
            v8 = dword_F63B8C[1580 * currCl][339] * fFactor;
            v9 = dword_F63B8C[1580 * currCl][340] * fFactor;
            v10 = dword_F63B8C[1580 * currCl][341] * fFactor;
        }
        else if ((pm & 2) != 0)
        {
            v8 = dword_F63B8C[1580 * currCl][330] * fFactor;
            v9 = dword_F63B8C[1580 * currCl][331] * fFactor;
            v10 = dword_F63B8C[1580 * currCl][332] * fFactor;
        }
        else
        {
            v8 = dword_F63B8C[1580 * currCl][321] * fFactor;
            v9 = dword_F63B8C[1580 * currCl][322] * fFactor;
            v10 = dword_F63B8C[1580 * currCl][323] * fFactor;
        }
        targetPos[1] = (*(float*)&cg_gun_move_r * fFactor) + v9;
        targetPos[0] = (*(float*)&cg_gun_move_f * fFactor) + v8;
        targetPos[2] = (*(float*)&cg_gun_move_u * fFactor) + v10;
    }
apply:
    {
        Client* client =
            EntityManager::sInst->GetPlayer( currCl)->client;
        float v23, v24, v25;
        if (client->ps.viewHeightTarget
            == EntityManager::sInst->GetPlayer( currCl)
                   ->client->ps.crouchViewHeight)
        {
            v23 = dword_F63B8C[1580 * currCl][327];
            v24 = dword_F63B8C[1580 * currCl][328];
            v25 = dword_F63B8C[1580 * currCl][329];
        }
        else if (EntityManager::sInst->GetPlayer( currCl)
                         ->client->ps.viewHeightTarget
                 == EntityManager::sInst->GetPlayer( currCl)
                        ->client->ps.proneViewHeight)
        {
            v23 = dword_F63B8C[1580 * currCl][336];
            v24 = dword_F63B8C[1580 * currCl][337];
            v25 = dword_F63B8C[1580 * currCl][338];
        }
        else
        {
            v23 = v24 = v25 = 0.0f;
        }
        targetPos[1] = *(float*)&cg_gun_ofs_r + (v24 + targetPos[1]);
        targetPos[0] = *(float*)&cg_gun_ofs_f + (v23 + targetPos[0]);
        targetPos[2] = *(float*)&cg_gun_ofs_u + (v25 + targetPos[2]);
    }
    playerEntity_t* pe = GetPlayerEntity(currCl);
    for (int i = 0; i < 3; ++i)
    {
        if (pe->vLastMoveOrg[i] != targetPos[i])
        {
            float fWeaponPos =
                EntityManager::sInst->GetPlayer( currCl)
                    ->client->ps.viewHeightCurrent;
            float rate;
            if (fWeaponPos
                == EntityManager::sInst->GetPlayer( currCl)
                       ->client->ps.proneViewHeight)
                rate = dword_F63B8C[1580 * currCl][346];
            else
                rate = dword_F63B8C[1580 * currCl][345];
            float frametime = cgGlobal_frametime;
            float v37 =
                (((rate + *(float*)&cg_gun_move_rate)
                  * (targetPos[i] - pe->vLastMoveOrg[i]))
                 * frametime)
                * 0.001f;
            float v39;
            bool v40;
            if (targetPos[i] <= pe->vLastMoveOrg[i])
            {
                float v41 = frametime * -0.0001f;
                if (v37 > v41)
                    v37 = v41;
                v39 = pe->vLastMoveOrg[i] + v37;
                v40 = targetPos[i] <= v39;
            }
            else
            {
                float v38 = frametime * 0.0001f;
                if (v38 > v37)
                    v37 = v38;
                v39 = pe->vLastMoveOrg[i] + v37;
                v40 = v39 <= targetPos[i];
            }
            pe->vLastMoveOrg[i] = v39;
            if (!v40)
                pe->vLastMoveOrg[i] = targetPos[i];
        }
    }
    float fWeaponPosFrac2 =
        EntityManager::sInst->GetPlayer( currCl)
            ->client->ps.fWeaponPosFrac;
    if (fWeaponPosFrac2 == 0.0f)
    {
        origin[0] = pe->vLastMoveOrg[0] + origin[0];
        origin[1] = pe->vLastMoveOrg[1] + origin[1];
        origin[2] = pe->vLastMoveOrg[2] + origin[2];
    }
    else if (fWeaponPosFrac2 < 0.5f)
    {
        float v40 = 1.0f - (fWeaponPosFrac2 * 2.0f);
        origin[0] = (v40 * pe->vLastMoveOrg[0]) + origin[0];
        origin[1] = (v40 * pe->vLastMoveOrg[1]) + origin[1];
        origin[2] = (v40 * pe->vLastMoveOrg[2]) + origin[2];
    }
}

// ea: 0x00690AC0
void CG_CalculateWeaponPosition_BasePosition_angles(float* angles)
{
    float targetAng[3];
    int pm_flags =
        EntityManager::sInst->GetPlayer( currCl)
            ->client->ps.pm_flags;
    float base;
    if ((pm_flags & 1) != 0)
        base = dword_F63B8C[1580 * currCl][356];
    else if ((pm_flags & 2) != 0)
        base = dword_F63B8C[1580 * currCl][355];
    else
        base = dword_F63B8C[1580 * currCl][354];
    float fMin = base + *(float*)&cg_gun_rot_minspeed;
    if (dword_F641DC[1580 * currCl] <= fMin
        || EntityManager::sInst->GetPlayer( currCl)
                   ->client->ps.weaponstate
               == 5)
    {
        targetAng[0] = 0.0f;
        targetAng[1] = 0.0f;
        targetAng[2] = 0.0f;
        goto apply;
    }
    float fFactor =
        (dword_F641DC[1580 * currCl] - fMin)
        / (*(float*)((char*)&EntityManager::sInst->GetPlayer(
                                                     currCl)
                          ->client->ps
                     + 0x31C)
           - fMin);
    if (fFactor < 0.0f)
        fFactor = 0.0f;
    else if (fFactor > 1.0f)
        fFactor = 1.0f;
    {
        int pm = EntityManager::sInst->GetPlayer( currCl)
                     ->client->ps.pm_flags;
        float v9, v10, v11;
        if ((pm & 1) != 0)
        {
            v9 = dword_F63B8C[1580 * currCl][342] * fFactor;
            v10 = dword_F63B8C[1580 * currCl][343] * fFactor;
            v11 = dword_F63B8C[1580 * currCl][344] * fFactor;
        }
        else if ((pm & 2) != 0)
        {
            v9 = dword_F63B8C[1580 * currCl][333] * fFactor;
            v10 = dword_F63B8C[1580 * currCl][334] * fFactor;
            v11 = dword_F63B8C[1580 * currCl][335] * fFactor;
        }
        else
        {
            v9 = dword_F63B8C[1580 * currCl][324] * fFactor;
            v10 = dword_F63B8C[1580 * currCl][325] * fFactor;
            v11 = dword_F63B8C[1580 * currCl][326] * fFactor;
        }
        targetAng[1] = (*(float*)&cg_gun_rot_y * fFactor) + v10;
        targetAng[0] = (*(float*)&cg_gun_rot_p * fFactor) + v9;
        targetAng[2] = (*(float*)&cg_gun_rot_r * fFactor) + v11;
    }
apply:
    {
        float fWeaponPosFracApply =
            EntityManager::sInst->GetPlayer( currCl)
                ->client->ps.fWeaponPosFrac;
        if (fWeaponPosFracApply != 0.0f)
        {
            float v19 = 1.0f - fWeaponPosFracApply;
            targetAng[0] *= v19;
            targetAng[1] *= v19;
            targetAng[2] *= v19;
        }
    }
    playerEntity_t* pe = GetPlayerEntity(currCl);
    for (int i = 0; i < 3; ++i)
    {
        if (pe->vLastMoveAng[i] != targetAng[i])
        {
            float fWeaponPos =
                EntityManager::sInst->GetPlayer( currCl)
                    ->client->ps.viewHeightCurrent;
            float rate;
            if (fWeaponPos
                == EntityManager::sInst->GetPlayer( currCl)
                       ->client->ps.proneViewHeight)
                rate = dword_F63B8C[1580 * currCl][352];
            else
                rate = dword_F63B8C[1580 * currCl][351];
            float frametime = cgGlobal_frametime;
            float v29 =
                (((rate + *(float*)&cg_gun_rot_rate)
                  * (targetAng[i] - pe->vLastMoveAng[i]))
                 * frametime)
                * 0.001f;
            float v31;
            bool v32;
            if (targetAng[i] <= pe->vLastMoveAng[i])
            {
                float v33 = frametime * -0.0001f;
                if (v29 > v33)
                    v29 = v33;
                v31 = pe->vLastMoveAng[i] + v29;
                v32 = targetAng[i] <= v31;
            }
            else
            {
                float v30 = frametime * 0.0001f;
                if (v30 > v29)
                    v29 = v30;
                v31 = pe->vLastMoveAng[i] + v29;
                v32 = v31 <= targetAng[i];
            }
            pe->vLastMoveAng[i] = v31;
            if (!v32)
                pe->vLastMoveAng[i] = targetAng[i];
        }
    }
    float fWeaponPosFrac =
        EntityManager::sInst->GetPlayer( currCl)
            ->client->ps.fWeaponPosFrac;
    if (fWeaponPosFrac == 0.0f)
    {
        angles[0] = angles[0] + pe->vLastMoveAng[0];
        angles[1] = pe->vLastMoveAng[1] + angles[1];
        angles[2] = pe->vLastMoveAng[2] + angles[2];
    }
    else if (fWeaponPosFrac < 0.5f)
    {
        float v40 = 1.0f - (fWeaponPosFrac * 2.0f);
        angles[0] = (v40 * pe->vLastMoveAng[0]) + angles[0];
        angles[1] = (v40 * pe->vLastMoveAng[1]) + angles[1];
        angles[2] = (v40 * pe->vLastMoveAng[2]) + angles[2];
    }
}

// ea: 0x00691120
void CG_CalculateWeaponPosition_BasePosition(float* origin)
{
    Client* client =
        EntityManager::sInst->GetPlayer( currCl)->client;
    float v3;
    if ((0x100000 & client->ps.eFlags) != 0)
    {
        Entity* Player =
            EntityManager::sInst->GetPlayer( currCl);
        if (!IsPlayerFullySeatedInVehicle(Player)
            || ((client->ps.vehType != 1 || client->ps.vehPos != 2)
                && (*(unsigned char*)((void*)InteractionController::Inst(
                         currCl))
                    & 0x20)
                       == 0))
        {
            v3 = ((cgGlobal_frametime * 0.001f) * vehicleOffsetRate)
                 + vehicleOffset;
            vehicleOffset = v3;
            goto clamp;
        }
    }
    v3 = vehicleOffset;
    if (vehicleOffset > 0.0f)
    {
        v3 = vehicleOffset
             - ((cgGlobal_frametime * 0.001f) * vehicleOffsetRate);
        vehicleOffset = v3;
    }
clamp:
    if (v3 < 0.0f || v3 > 1.0f)
    {
        v3 = v3 < 0.0f ? 0.0f : 1.0f;
        vehicleOffset = v3;
    }
    float* v5 = dword_F63B8C[1580 * currCl];
    float vGunOfs[3];
    if (sqrtf(v5[315] * v5[315] + v5[316] * v5[316]
              + v5[317] * v5[317])
        <= 0.0099999998f)
    {
        vGunOfs[0] = 100.0f;
        vGunOfs[1] = 100.0f;
        vGunOfs[2] = 100.0f;
    }
    else
    {
        vGunOfs[0] = v5[315] * v3;
        vGunOfs[1] = v5[316] * v3;
        vGunOfs[2] = v5[317] * v3;
    }
    CG_CalculateWeaponPosition_BasePosition_movement(vGunOfs);
    playerEntity_t* pe = GetPlayerEntity(currCl);
    pe->gunOfs[0] = vGunOfs[0];
    pe->gunOfs[1] = vGunOfs[1];
    pe->gunOfs[2] = vGunOfs[2];
    origin[0] = *origin + vGunOfs[0];
    origin[1] = origin[1] + vGunOfs[1];
    origin[2] = origin[2] + vGunOfs[2];
}

// ea: 0x00691310
void CG_CalculateWeaponPosition_BaseAngles(float* angles)
{
    float vGunAngOfs[3] = {0.0f, 0.0f, 0.0f};
    playerEntity_t* pe = GetPlayerEntity(currCl);
    Entity* Player = EntityManager::sInst->GetPlayer( currCl);
    if (BG_IsAimDownSightWeapon(Player->client->ps.weapon) != 0)
    {
        float fWeaponPosFrac =
            EntityManager::sInst->GetPlayer( currCl)
                ->client->ps.fWeaponPosFrac;
        if (fWeaponPosFrac != 1.0f && fWeaponPosFrac != 0.0f)
        {
            int v3;
            if (fWeaponPosFrac == pe->fWeaponPosFrac)
                v3 = (int)pe->fracDir;
            else
                v3 = fWeaponPosFrac >= pe->fWeaponPosFrac;
            if (pe->fWeaponPosFrac == 1.0f
                || pe->fWeaponPosFrac == 0.0f)
            {
                pe->fracDir = (float)v3;
                pe->fWeaponPosFrac = 0.0f;
            }
        }
        pe->fWeaponPosFrac =
            EntityManager::sInst->GetPlayer( currCl)
                ->client->ps.fWeaponPosFrac;
        vGunAngOfs[0] =
            dword_F63B8C[1580 * currCl][498]
            * EntityManager::sInst->GetPlayer( currCl)
                  ->client->ps.fWeaponPosFrac;
    }
    CG_CalculateWeaponPosition_BasePosition_angles(vGunAngOfs);
    pe->gunAngOfs[0] = vGunAngOfs[0];
    pe->gunAngOfs[1] = vGunAngOfs[1];
    pe->gunAngOfs[2] = vGunAngOfs[2];
    angles[0] = *angles + vGunAngOfs[0];
    angles[1] = angles[1] + vGunAngOfs[1];
    angles[2] = angles[2] + vGunAngOfs[2];
}

// ea: 0x00691510
int CG_CalculateWeaponPosition_GunRecoil_SingleAngle(
    float* fOffset, float* fSpeed, float fTimeStep, float fOfsCap,
    float fGunKickAccel, float fGunKickSpeedMax, float fGunKickSpeedDecay,
    float fGunKickStaticDecay)
{
    if (fabsf(*fOffset) < 0.25f && fabsf(*fSpeed) < 1.0f)
    {
        *fOffset = 0.0f;
        *fSpeed = 0.0f;
        return 1;
    }
    float v9 = (*fSpeed * fTimeStep) + *fOffset;
    *fOffset = v9;
    if (v9 > fOfsCap)
    {
        *fOffset = fOfsCap;
        if (*fSpeed > 0.0f)
            *fSpeed = 0.0f;
    }
    else if ((0.0f - fOfsCap) > v9)
    {
        *fOffset = 0.0f - fOfsCap;
        if (*fSpeed < 0.0f)
            *fSpeed = 0.0f;
    }
    if (*fOffset <= 0.0f)
    {
        if (*fOffset < 0.0f)
            *fSpeed = (fTimeStep * fGunKickAccel) + *fSpeed;
    }
    else
    {
        *fSpeed = *fSpeed - (fTimeStep * fGunKickAccel);
    }
    float v11 = fTimeStep * fGunKickStaticDecay;
    float v12 = *fSpeed - ((*fSpeed * fTimeStep) * fGunKickSpeedDecay);
    *fSpeed = v12;
    bool v14;
    if (v12 <= 0.0f)
    {
        float v15 = v11 + v12;
        *fSpeed = v15;
        v14 = v15 <= 0.0f;
    }
    else
    {
        float v13 = v12 - v11;
        *fSpeed = v13;
        v14 = v13 >= 0.0f;
    }
    if (!v14)
        *fSpeed = 0.0f;
    if (*fSpeed <= fGunKickSpeedMax)
    {
        if ((0.0f - fGunKickSpeedMax) > *fSpeed)
            *fSpeed = 0.0f - fGunKickSpeedMax;
    }
    else
    {
        *fSpeed = fGunKickSpeedMax;
    }
    return 0;
}

// ea: 0x00691630
void CG_CalculateWeaponPosition_GunRecoil(float* angles)
{
    float fPosLerp =
        EntityManager::sInst->GetPlayer( currCl)
            ->client->ps.fWeaponPosFrac;
    Entity* Player = EntityManager::sInst->GetPlayer( currCl);
    if (BG_IsAimDownSightWeapon(Player->client->ps.weapon) != 0)
    {
        int v2 = 1580 * currCl;
        float* v3 = dword_F63B8C[1580 * currCl];
        int frametime = cgGlobal_frametime;
        float v5 = ((v3[505] - v3[523]) * fPosLerp) + v3[523];
        float fSpeedMax = ((v3[506] - v3[524]) * fPosLerp) + v3[524];
        float v7 = ((v3[508] - v3[526]) * fPosLerp) + v3[526];
        float fPosLerpa = ((v3[507] - v3[525]) * fPosLerp) + v3[525];
        float v9 = v3[427];
        if (cgGlobal_frametime > 0)
        {
            while (1)
            {
                int v8;
                if (frametime <= 5)
                {
                    v8 = frametime;
                    frametime = 0;
                }
                else
                {
                    v8 = 5;
                    frametime -= 5;
                }
                float v10 = v8 * 0.001f;
                int v11 = 0;
                if (fabsf(*(float*)&dword_F64050[v2]) < 0.25f
                    && fabsf(*(float*)&dword_F6405C[v2]) < 1.0f)
                {
                    dword_F64050[v2] = 0;
                    dword_F6405C[v2] = 0;
                    v11 = 1;
                    goto label_28;
                }
                float v12 = (*(float*)&dword_F6405C[v2] * v10)
                            + *(float*)&dword_F64050[v2];
                *(float*)&dword_F64050[v2] = v12;
                if (v12 > v9)
                    break;
                if ((0.0f - v9) > v12)
                {
                    *(float*)&dword_F64050[v2] = 0.0f - v9;
                    if (*(float*)&dword_F6405C[v2] < 0.0f)
                        dword_F6405C[v2] = 0;
                }
                if (*(float*)&dword_F64050[v2] <= 0.0f)
                {
                    if (*(float*)&dword_F64050[v2] < 0.0f)
                        *(float*)&dword_F6405C[v2] =
                            (v10 * v5) + *(float*)&dword_F6405C[v2];
                }
                else
                {
                    *(float*)&dword_F6405C[v2] =
                        *(float*)&dword_F6405C[v2] - (v10 * v5);
                }
                float v14 = *(float*)&dword_F6405C[v2]
                            - ((*(float*)&dword_F6405C[v2] * v10)
                               * fPosLerpa);
                *(float*)&dword_F6405C[v2] = v14;
                float v15 = v10 * v7;
                bool v17;
                if (v14 <= 0.0f)
                {
                    float v18 = v15 + v14;
                    *(float*)&dword_F6405C[v2] = v18;
                    v17 = v18 <= 0.0f;
                }
                else
                {
                    float v16 = v14 - v15;
                    *(float*)&dword_F6405C[v2] = v16;
                    v17 = v16 >= 0.0f;
                }
                if (!v17)
                    dword_F6405C[v2] = 0;
                if (*(float*)&dword_F6405C[v2] <= fSpeedMax)
                {
                    if ((0.0f - fSpeedMax) > *(float*)&dword_F6405C[v2])
                        *(float*)&dword_F6405C[v2] = 0.0f - fSpeedMax;
                }
                else
                {
                    *(float*)&dword_F6405C[v2] = fSpeedMax;
                }
            label_28:
                float v19 = v3[428];
                if (fabsf(*(float*)&dword_F64054[v2]) < 0.25f
                    && fabsf(*(float*)&dword_F64060[v2]) < 1.0f)
                {
                    dword_F64054[v2] = 0;
                    dword_F64060[v2] = 0;
                    if (v11 != 0)
                        goto label_32;
                    goto label_31;
                }
                float v20 = (*(float*)&dword_F64060[v2] * v10)
                            + *(float*)&dword_F64054[v2];
                *(float*)&dword_F64054[v2] = v20;
                if (v20 > v19)
                {
                    *(float*)&dword_F64054[v2] = v19;
                    if (*(float*)&dword_F64060[v2] > 0.0f)
                        dword_F64060[v2] = 0;
                    goto label_31;
                }
                if ((0.0f - v19) > v20)
                {
                    *(float*)&dword_F64054[v2] = 0.0f - v19;
                    if (*(float*)&dword_F64060[v2] < 0.0f)
                        dword_F64060[v2] = 0;
                }
                if (*(float*)&dword_F64054[v2] <= 0.0f)
                {
                    if (*(float*)&dword_F64054[v2] < 0.0f)
                        *(float*)&dword_F64060[v2] =
                            (v10 * v5) + *(float*)&dword_F64060[v2];
                }
                else
                {
                    *(float*)&dword_F64060[v2] =
                        *(float*)&dword_F64060[v2] - (v10 * v5);
                }
                float v22 = *(float*)&dword_F64060[v2]
                            - ((*(float*)&dword_F64060[v2] * v10)
                               * fPosLerpa);
                *(float*)&dword_F64060[v2] = v22;
                float v23 = v10 * v7;
                bool v25;
                if (v22 <= 0.0f)
                {
                    float v26 = v23 + v22;
                    *(float*)&dword_F64060[v2] = v26;
                    v25 = v26 <= 0.0f;
                }
                else
                {
                    float v24 = v22 - v23;
                    *(float*)&dword_F64060[v2] = v24;
                    v25 = v24 >= 0.0f;
                }
                if (!v25)
                    dword_F64060[v2] = 0;
                if (*(float*)&dword_F64060[v2] <= fSpeedMax)
                {
                    if ((0.0f - fSpeedMax) > *(float*)&dword_F64060[v2])
                        *(float*)&dword_F64060[v2] = 0.0f - fSpeedMax;
                }
                else
                {
                    *(float*)&dword_F64060[v2] = fSpeedMax;
                }
            label_31:
                if (frametime <= 0)
                    goto label_32;
            }
            *(float*)&dword_F64050[v2] = v9;
            if (*(float*)&dword_F6405C[v2] > 0.0f)
                dword_F6405C[v2] = 0;
            goto label_28;
        }
    label_32:
        angles[0] = *(float*)&dword_F64050[v2] + angles[0];
        angles[1] = *(float*)&dword_F64054[1580 * currCl] + angles[1];
        angles[2] = *(float*)&dword_F64058[1580 * currCl] + angles[2];
    }
}

// ea: 0x00691AB0
void CG_CalculateWeaponPosition_ToWorldPosition(float* origin)
{
    float vOffset[3] = {origin[0], origin[1], origin[2]};
    float vAxis[3], right[3], up[3];
    AngleVectors(&angle[1580 * currCl], vAxis, right, up);
    origin[0] = dword_F63C70[1580 * currCl];
    origin[1] = dword_F63C74[1580 * currCl];
    origin[2] = dword_F63C78[1580 * currCl];
    float v2 = 0.0f - vOffset[1];
    origin[0] = ((v2 * right[0]) + (up[0] * vOffset[2])
                 + (vAxis[0] * vOffset[0]))
                + origin[0];
    origin[1] = ((v2 * right[1]) + (up[1] * vOffset[2])
                 + (vAxis[1] * vOffset[0]))
                + origin[1];
    origin[2] = ((v2 * right[2]) + (up[2] * vOffset[2])
                 + (vAxis[2] * vOffset[0]))
                + origin[2];
}

// ea: 0x00691BE0
void CG_CalculateWeaponPosition_ToWorldAngles(float* angles)
{
    float vAxis[3][3], vAxis2[3][3], vAxis3[3][3];
    AnglesToAxis(angles, vAxis);
    AnglesToAxis(&angle[1580 * currCl], vAxis2);
    MatrixMultiply(vAxis, vAxis2, vAxis3);
    AxisToAngles(vAxis3, angles);
}

// ea: 0x00691C40
void CG_CalculateWeaponPosition_SaveOffsetMovement(float* origin)
{
    Entity* Player = EntityManager::sInst->GetPlayer( currCl);
    if (BG_IsAimDownSightWeapon(Player->client->ps.weapon) != 0)
    {
        float fWeaponPosFrac =
            EntityManager::sInst->GetPlayer( currCl)
                ->client->ps.fWeaponPosFrac;
        float v3 = 0.0f;
        int v4 = 1580 * currCl;
        if (fWeaponPosFrac == 0.0f)
        {
            dword_F64044[1580 * currCl] = 0;
            dword_F64048[v4] = 0;
        }
        else
        {
            *(float*)&dword_F64044[1580 * currCl] =
                (*origin - dword_F63C70[1580 * currCl]) * fWeaponPosFrac;
            *(float*)&dword_F64048[v4] =
                (origin[1] - dword_F63C74[v4]) * fWeaponPosFrac;
            v3 = (origin[2] - dword_F63C78[v4]) * fWeaponPosFrac;
        }
        *(float*)&dword_F6404C[v4] = v3;
    }
    else
    {
        int v5 = 1580 * currCl;
        dword_F64044[v5] = 0;
        dword_F64048[v5] = 0;
        dword_F6404C[v5] = 0;
    }
}

// ea: 0x00691D40
void CG_CalculateWeaponPosition_SaveOffsetAngles(float* angles)
{
    Entity* Player = EntityManager::sInst->GetPlayer( currCl);
    if (BG_IsAimDownSightWeapon(Player->client->ps.weapon) == 0
        || EntityManager::sInst->GetPlayer( currCl)
                   ->client->ps.fWeaponPosFrac
               == 0.0f)
    {
        int v2 = 1580 * currCl;
        dword_F6403C[v2] = *(int*)&angle[1580 * currCl];
        dword_F64040[v2] = *(int*)&dword_F63CB4[v2];
    }
    else
    {
        *(float*)&dword_F6403C[1580 * currCl] = AngleNormalize360(*angles);
        *(float*)&dword_F64040[1580 * currCl] =
            AngleNormalize360(angles[1]);
    }
}

// ea: 0x00691E00
void CG_CalculateWeaponAngles(float* angles)
{
    Entity* Player = EntityManager::sInst->GetPlayer( currCl);
    if (dword_F63B8C[1580 * currCl]
        != (float*)BG_GetInfoForWeapon(Player->client->ps.weapon))
    {
        CG_ASSERT("cg[currCl].pCurrentWeapInfo == "
                  "BG_GetInfoForWeapon(GetPlayerState().weapon)",
                  "c:\\cod\\code\\game\\cg_weapons.cpp", 2630);
    }
    angles[1] = 0.0f;
    angles[0] = 0.0f;
    if (EntityManager::sInst->GetPlayer( currCl)
            ->client->ps.leanf
        != 0.0f)
    {
        float LeanFraction = GetLeanFraction(
            EntityManager::sInst->GetPlayer( currCl)
                ->client->ps.leanf);
        angles[2] = angles[2] - (LeanFraction + LeanFraction);
    }
    CG_CalculateWeaponPosition_BaseAngles(angles);
    CG_CalculateWeaponPosition_IdleAngles(angles);
    angles[0] = *(float*)&dword_F64068[1580 * currCl] + angles[0];
    angles[1] = *(float*)&dword_F6406C[1580 * currCl] + angles[1];
    angles[2] = *(float*)&dword_F64070[1580 * currCl] + angles[2];
    Entity* v6 = EntityManager::sInst->GetPlayer( currCl);
    if (BG_IsAimDownSightWeapon(v6->client->ps.weapon) == 0)
    {
        angles[0] = angles[0] - *(float*)&dword_F64030[1580 * currCl];
        angles[1] = angles[1] - *(float*)&dword_F64034[1580 * currCl];
        angles[2] = angles[2] - *(float*)&dword_F64038[1580 * currCl];
    }
    if (dword_F63FFC[1580 * currCl] != 0)
    {
        float fWeaponPosFrac =
            (EntityManager::sInst->GetPlayer( currCl)
                 ->client->ps.fWeaponPosFrac
             + 1.0f)
            * 0.5f;
        float fDeflectTime = *(float*)&cg_viewKickDeflectTime * fWeaponPosFrac;
        float fFactor = fWeaponPosFrac;
        float fReturnTime = *(float*)&cg_viewKickReturnTime * fWeaponPosFrac;
        float fWeaponPosFrac2 =
            EntityManager::sInst->GetPlayer( currCl)
                ->client->ps.fWeaponPosFrac;
        if (fWeaponPosFrac2 != 0.0f
            && *(int*)((float*)dword_F63B8C[1580 * currCl] + 405) != 0)
        {
            float v10 =
                EntityManager::sInst->GetPlayer( currCl)
                    ->client->ps.fWeaponPosFrac;
            fFactor = (1.0f - (v10 * 0.75f)) * fFactor;
        }
        float v11 = (float)(cgGlobal_time - dword_F63FFC[1580 * currCl]);
        if (fDeflectTime > v11)
        {
            float v12 = GetLeanFraction(v11 / fDeflectTime);
            float v14 = v12 * fFactor;
            angles[0] = v14 * *(float*)&dword_F6401C[1580 * currCl] * 0.5f
                        + angles[0];
            angles[1] = angles[1] - v14 * *(float*)&dword_F64020[1580 * currCl];
            angles[2] = v14 * *(float*)&dword_F64020[1580 * currCl] * 0.5f
                        + angles[2];
        }
        else
        {
            float v13 = 1.0f - ((v11 - fDeflectTime) / fReturnTime);
            if (v13 > 0.0f)
            {
                float v12 = 1.0f - GetLeanFraction(1.0f - v13);
                float v14 = v12 * fFactor;
                angles[0] = v14 * *(float*)&dword_F6401C[1580 * currCl] * 0.5f
                            + angles[0];
                angles[1] =
                    angles[1] - v14 * *(float*)&dword_F64020[1580 * currCl];
                angles[2] =
                    v14 * *(float*)&dword_F64020[1580 * currCl] * 0.5f
                    + angles[2];
            }
        }
    }
    CG_CalculateWeaponPosition_GunRecoil(angles);
    angles[0] = AngleSubtract(angles[0], dword_F63CD0[1580 * currCl]);
    float v15 = angles[1];
    angles[1] = AngleSubtract(v15, dword_F63CD4[1580 * currCl]);
    CG_CalculateWeaponPosition_ToWorldAngles(angles);
    CG_CalculateWeaponPosition_SaveOffsetAngles(angles);
}

// ea: 0x00699D10
void CG_CalculateWeaponPosition(float* origin)
{
    Entity* Player = EntityManager::sInst->GetPlayer( currCl);
    if (dword_F63B8C[1580 * currCl]
        != (float*)BG_GetInfoForWeapon(Player->client->ps.weapon))
    {
        CG_ASSERT("cg[currCl].pCurrentWeapInfo == "
                  "BG_GetInfoForWeapon(GetPlayerState().weapon)",
                  "c:\\cod\\code\\game\\cg_weapons.cpp", 2580);
    }
    origin[1] = 0.0f;
    origin[0] = 0.0f;
    Client* client =
        EntityManager::sInst->GetPlayer( currCl)->client;
    if (client->ps.leanf != 0.0f
        && client->ps.fWeaponPosFrac < 1.0f)
    {
        float tempAngles[3] = {0.0f, 0.0f, 0.0f};
        float LeanFraction =
            GetLeanFraction(EntityManager::sInst->GetPlayer(
                                                    currCl)
                                ->client->ps.leanf);
        tempAngles[2] = LeanFraction * -2.0f;
        float fDist = LeanFraction;
        float fDista =
            ((1.0f - EntityManager::sInst->GetPlayer( currCl)
                         ->client->ps.fWeaponPosFrac)
             * fDist)
            * 1.6f;
        float right[3];
        AnglesToRight(tempAngles, right);
        origin[0] = (right[0] * fDista) + origin[0];
        origin[1] = (right[1] * fDista) + origin[1];
        origin[2] = (right[2] * fDista) + origin[2];
    }
    CG_CalculateWeaponPosition_BasePosition(origin);
    CG_CalculateWeaponPosition_BobMovement(origin);
    origin[1] = origin[1] - dword_F63CE4[1580 * currCl];
    origin[2] = dword_F63CE8[1580 * currCl] + origin[2];
    CG_CalculateWeaponPosition_ToWorldPosition(origin);
    int v5 = cgGlobal_time - dword_F63BB4[1580 * currCl];
    if (v5 < 150)
        origin[2] = (v5 * dword_F63BB0[1580 * currCl]) * 0.0016666667f
                    + origin[2];
    else if (v5 < 450)
        origin[2] = ((450 - v5) * dword_F63BB0[1580 * currCl])
                        * 0.00083333335f
                    + origin[2];
    CG_CalculateWeaponPosition_SaveOffsetMovement(origin);
}

extern void CG_InterpolateEntityOrigin(Entity* cent);
extern void CG_InterpolateEntityAngles(Entity* cent);

extern int dword_F63580[4 * 1580];
extern float dword_F63564[4 * 1580];
extern float dword_F63568[4 * 1580];
extern int dword_F63558[4 * 1580];
extern int dword_F63584[4 * 1580];
extern int dword_F635B8[4 * 1580];
extern unsigned int dword_F635C0[4 * 1580];
extern unsigned char unk_F63658[4 * 6320];
extern int dword_F63B90[4 * 1580];
extern int dword_F63B94[4 * 1580];
extern float dword_F63B98[4 * 1580];
extern float dword_F63B9C[4 * 1580];
extern float dword_F63BA0[4 * 1580];
extern int gDelayRenderForNFrames;
extern vmCvar_t cg_nopredict;
extern vmCvar_t pmove_fixed;
extern vmCvar_t pmove_msec;
extern vmCvar_t cg_norender;
extern vmCvar_t cg_showmiss;
extern vmCvar_t cg_errorDecay;
static cg_pmove_abi cg_pmove;

extern int CL_GetCurrentCmdNumber();
extern int CL_GetUserCmd(int cmdNumber, usercmd_s* ucmd);
extern int CG_PointContents(const math::Position3* point,
                            collision_context_t* context);
extern void CG_TraceCapsule(trace_t* result, const math::Position3* start,
                            const math::Position3* mins,
                            const math::Position3* maxs,
                            const math::Position3* end,
                            const collision_context_t* context);
extern void PM_UpdateViewAngles(
    PlayerState* ps, usercmd_s* cmd, usercmd_s* oldcmd, int msec,
    void (__cdecl* capsuleTrace)(trace_t*, const math::Position3&,
                                 const math::Position3&,
                                 const math::Position3&,
                                 const math::Position3&,
                                 const collision_context_t&));

// ea: 0x006A2650
void CG_InterpolatePlayerState(int grabAngles)
{
    const int base = 1580 * currCl;
    snapshot_t* current =
        reinterpret_cast<snapshot_t*>(dword_F62960[base]);
    snapshot_t* next =
        reinterpret_cast<snapshot_t*>(dword_F62964[base]);
    PlayerState* out =
        reinterpret_cast<PlayerState*>(&dword_F63560[base]);

    if (next == nullptr)
    {
        CG_ASSERT("next", "c:\\cod\\code\\game\\cg_predict.cpp", 273);
    }

    const unsigned char* currentPs = current->ps;
    const unsigned char* nextPs = next->ps;
    memcpy(out, nextPs, 0x5D0);

    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    dword_F63B8C[base] = reinterpret_cast<float*>(
        BG_GetInfoForWeapon(player->client->ps.weapon));

    if (grabAngles != 0)
    {
        usercmd_s cmd;
        const int currentCmdNumber = CL_GetCurrentCmdNumber();
        CL_GetUserCmd(currentCmdNumber, &cmd);
        using PMTrace = void (__cdecl*)(
            trace_t*, const math::Position3&, const math::Position3&,
            const math::Position3&, const math::Position3&,
            const collision_context_t&);
        PM_UpdateViewAngles(
            out, &cmd, &cmd, cgGlobal.frametime,
            reinterpret_cast<PMTrace>(CG_TraceCapsule));
    }

    if (next->serverTime <= current->serverTime)
        return;

    const float f = static_cast<float>(cgGlobal.time - current->serverTime)
                    / static_cast<float>(next->serverTime
                                         - current->serverTime);
    const int currentBobCycle =
        *reinterpret_cast<const int*>(currentPs + 0x28);
    int bobCycle = *reinterpret_cast<const int*>(nextPs + 0x28);
    if (bobCycle < currentBobCycle)
        bobCycle += 256;
    *reinterpret_cast<int*>(reinterpret_cast<unsigned char*>(out) + 0x28) =
        currentBobCycle
        + static_cast<int>((bobCycle - currentBobCycle) * f);
    const float currentAimSpread =
        *reinterpret_cast<const float*>(currentPs + 0x534);
    const float nextAimSpread =
        *reinterpret_cast<const float*>(nextPs + 0x534);
    *reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(out) + 0x534) =
        currentAimSpread + (nextAimSpread - currentAimSpread) * f;

    for (int i = 0; i < 3; ++i)
    {
        const float currentOrigin =
            *reinterpret_cast<const float*>(currentPs + 0x00 + i * 4);
        const float nextOrigin =
            *reinterpret_cast<const float*>(nextPs + 0x00 + i * 4);
        *reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(out)
                                  + 0x00 + i * 4) =
            currentOrigin + (nextOrigin - currentOrigin) * f;
        if (grabAngles == 0)
        {
            const float currentAngle =
                *reinterpret_cast<const float*>(currentPs + 0xD0 + i * 4);
            const float nextAngle =
                *reinterpret_cast<const float*>(nextPs + 0xD0 + i * 4);
            out->viewangles[i] = LerpAngle(currentAngle, nextAngle, f);
        }
        const float currentVelocity =
            *reinterpret_cast<const float*>(currentPs + 0x10 + i * 4);
        const float nextVelocity =
            *reinterpret_cast<const float*>(nextPs + 0x10 + i * 4);
        *reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(out)
                                  + 0x10 + i * 4) =
            currentVelocity + (nextVelocity - currentVelocity) * f;
    }
}

// ea: 0x006A2850
void CG_PredictPlayerState_Internal()
{
    const int base = 1580 * currCl;
    snapshot_t* current =
        reinterpret_cast<snapshot_t*>(dword_F62960[base]);
    snapshot_t* next =
        reinterpret_cast<snapshot_t*>(dword_F62964[base]);

    if (dword_F63B90[base] == 0)
    {
        dword_F63B90[base] = 1;
        memcpy(&dword_F63560[base],
               reinterpret_cast<const unsigned char*>(current) + 0x10,
               0x5D0);
        Entity* player = EntityManager::sInst->GetPlayer(currCl);
        dword_F63B8C[base] = reinterpret_cast<float*>(
            BG_GetInfoForWeapon(player->client->ps.weapon));
        Entity* playerEntity = EntityManager::sInst->GetPlayer(currCl);
        if (playerEntity != nullptr && next != nullptr)
        {
            memcpy(reinterpret_cast<unsigned char*>(next) + 0x10,
                   &playerEntity->r.currentOrigin, 0x10);
            memcpy(reinterpret_cast<unsigned char*>(next) + 0xE0,
                   &playerEntity->r.currentAngles, 0x0C);
        }
        dword_F64154[base] = 0x3F7D70A4;
        dword_F6415C[base] = cgGlobal.time - 1;
        gDelayRenderForNFrames = 0;
        dword_F64160[base] = 0;
        if (dword_F6415C[base] <= cgGlobal.time)
            dword_F64158[base] = dword_F64154[base];
        dword_F64154[base] = 0;
        dword_F6415C[base] = cgGlobal.time;
        dword_F64160[base] = 500;
        if (dword_F6415C[base] + 500 <= cgGlobal.time)
            dword_F64158[base] = dword_F64154[base];
    }

    if ((0x100000 & *reinterpret_cast<int*>(
                        reinterpret_cast<unsigned char*>(current) + 60)) != 0)
    {
        CG_InterpolatePlayerState(0);
        return;
    }
    if (cg_nopredict.integer != 0)
    {
        CG_InterpolatePlayerState(1);
        return;
    }

    cg_pmove.trace = reinterpret_cast<decltype(cg_pmove.trace)>(CG_TraceCapsule);
    cg_pmove.boxtrace = reinterpret_cast<decltype(cg_pmove.boxtrace)>(CG_TraceCapsule);
    cg_pmove.capsuletrace = reinterpret_cast<decltype(cg_pmove.capsuletrace)>(CG_TraceCapsule);
    cg_pmove.ps = reinterpret_cast<PlayerState*>(&dword_F63560[base]);
    cg_pmove.pointcontents = reinterpret_cast<decltype(cg_pmove.pointcontents)>(CG_PointContents);
    cg_pmove.tracemask = dword_F63560[base + 9] < 6 ? 0x02810011 : 0x00810011;
    using PMTrace = void (__cdecl*)(
        trace_t*, const math::Position3&, const math::Position3&,
        const math::Position3&, const math::Position3&,
        const collision_context_t&);
    const PMTrace cgTraceRef = reinterpret_cast<PMTrace>(CG_TraceCapsule);

    const int currentCmdNumber = CL_GetCurrentCmdNumber();
    usercmd_s latestCmd = {};
    if (CL_GetUserCmd(currentCmdNumber, &latestCmd) == 0)
        return;

    const int oldCommandTime = dword_F63560[base];
    const int currentOriginY = (int)dword_F63564[base];
    const int currentOriginZ = (int)dword_F63568[base];
    const int latestServerTime = latestCmd.serverTime;
    unsigned char oldEvents[0x3C];
    memcpy(oldEvents, unk_F63658 + 6320 * currCl, sizeof(oldEvents));
    if (next == nullptr)
        CG_ASSERT("cg[currCl].nextSnap", "c:\\cod\\code\\game\\cg_predict.cpp", 453);
    memcpy(&dword_F63560[base],
           reinterpret_cast<const unsigned char*>(next) + 0x10,
           0x5D0);
    dword_F63558[base] = *reinterpret_cast<int*>(
        reinterpret_cast<unsigned char*>(next) + 4);
    if (dword_F63584[base] == 1 || dword_F63584[base] == 7)
        CG_InterpolatePlayerState(0);
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    dword_F63B8C[base] = reinterpret_cast<float*>(
        BG_GetInfoForWeapon(player->client->ps.weapon));

    if (pmove_msec.integer < 8)
        Cvar_VMSet(&pmove_msec, "8");
    else if (pmove_msec.integer > 33)
        Cvar_VMSet(&pmove_msec, "33");
    cg_pmove.pmove_msec = pmove_msec.integer;
    cg_pmove.pmove_fixed = pmove_fixed.integer;

    bool ranPrediction = false;
    for (int cmdNumber = currentCmdNumber - 63;
         cmdNumber <= currentCmdNumber; ++cmdNumber)
    {
        if (CL_GetUserCmd(cmdNumber, &cg_pmove.cmd) == 0)
            break;
        if (cg_pmove.pmove_fixed != 0)
        {
            PM_UpdateViewAngles(cg_pmove.ps, &cg_pmove.cmd,
                                &cg_pmove.oldcmd, pmove_msec.integer,
                                cgTraceRef);
        }
        if (cg_pmove.cmd.serverTime <= dword_F63580[base]
            || cg_pmove.cmd.serverTime > latestServerTime
            || CL_GetUserCmd(cmdNumber - 1, &cg_pmove.oldcmd) == 0)
            break;

        if (dword_F63580[base] == oldCommandTime)
        {
            math::Position3 adjusted;
            float deltaAngles[3] = {};
            CG_AdjustPositionForMover(
                reinterpret_cast<const math::Position3*>(&dword_F63560[base]),
                dword_F635C0[base], dword_F63558[base], cgGlobal.oldTime,
                &adjusted, deltaAngles);
            dword_F635B8[base] += (int)(latestCmd.gunYOfs * 182.04445f);
            if (cg_showmiss.integer != 0
                && (oldCommandTime != (int)adjusted.v.m128_f32[0]
                    || currentOriginY != (int)adjusted.v.m128_f32[1]
                    || currentOriginZ != (int)adjusted.v.m128_f32[2]))
                CG_Printf("prediction error\n");
            const float dx = (float)oldCommandTime - adjusted.v.m128_f32[0];
            const float dy = (float)currentOriginY - adjusted.v.m128_f32[1];
            const float dz = (float)currentOriginZ - adjusted.v.m128_f32[2];
            const float error = sqrtf(dx * dx + dy * dy + dz * dz);
            if (error > 0.1f)
            {
                if (cg_showmiss.integer != 0)
                    CG_Printf("Prediction miss: %f\n", error);
                if (cg_errorDecay.integer == 0)
                {
                    dword_F63B98[base] = 0.0f;
                    dword_F63B9C[base] = 0.0f;
                    dword_F63BA0[base] = 0.0f;
                }
                else
                {
                    float decay = (cg_errorDecay.value
                                    - (cgGlobal.time - dword_F63B94[base]))
                                   / cg_errorDecay.value;
                    if (decay < 0.0f)
                        decay = 0.0f;
                    if (decay > 0.0f && cg_showmiss.integer != 0)
                        CG_Printf("Double prediction decay: %f\n", decay);
                    dword_F63B98[base] *= decay;
                    dword_F63B9C[base] *= decay;
                    dword_F63BA0[base] *= decay;
                }
                dword_F63B98[base] += dx;
                dword_F63B9C[base] += dy;
                dword_F63BA0[base] += dz;
                dword_F63B94[base] = cgGlobal.oldTime;
            }
        }

        if (cg_pmove.pmove_fixed != 0)
        {
            cg_pmove.cmd.serverTime = pmove_msec.integer
                * ((cg_pmove.cmd.serverTime + pmove_msec.integer - 1)
                   / pmove_msec.integer);
        }
        if (cg_norender.integer != 0)
        {
            cg_pmove.cmd.angles[0] = cg_pmove.oldcmd.angles[0];
            cg_pmove.cmd.angles[1] = cg_pmove.oldcmd.angles[1];
            cg_pmove.cmd.angles[2] = cg_pmove.oldcmd.angles[2];
            cg_pmove.cmd.forwardmove = 0;
            cg_pmove.cmd.rightmove = 0;
            cg_pmove.cmd.upmove = 0;
            cg_pmove.cmd.buttons = 0;
            if (cg_pmove.cmd.serverTime - dword_F63580[base] > 1)
                cg_pmove.cmd.serverTime = dword_F63580[base] + 1;
        }
        PM_UpdateViewAngles(cg_pmove.ps, &cg_pmove.cmd,
                            &cg_pmove.oldcmd, cg_pmove.pmove_msec,
                            cgTraceRef);
        ranPrediction = true;
    }

    if (cg_showmiss.integer > 1)
        CG_Printf("[%i : %i] ", cg_pmove.cmd.serverTime, cgGlobal.time);
    if (ranPrediction)
    {
        math::Position3 adjusted;
        float deltaAngles[3] = {};
        CG_AdjustPositionForMover(
            reinterpret_cast<const math::Position3*>(&dword_F63560[base]),
            dword_F635C0[base], dword_F63558[base], cgGlobal.time,
            &adjusted, deltaAngles);
        const unsigned char* eventBase = unk_F63658 + 6320 * currCl;
        if (*reinterpret_cast<const int*>(eventBase + 0x28)
            != *reinterpret_cast<const int*>(oldEvents + 0x28))
        {
            const int damage = *reinterpret_cast<const int*>(eventBase + 0x34);
            if (damage != 0)
                CG_DamageFeedback(*(reinterpret_cast<const int*>(eventBase + 0x2C)),
                                  *(reinterpret_cast<const int*>(eventBase + 0x30)),
                                  damage);
        }
        player = EntityManager::sInst->GetPlayer(currCl);
        dword_F63B8C[base] = reinterpret_cast<float*>(
            BG_GetInfoForWeapon(player->client->ps.weapon));
    }
    else if (cg_showmiss.integer != 0)
        CG_Printf("no prediction run\n");
}
extern int CG_PointContents(const math::Position3* point,
                            collision_context_t* context);
extern weaponFileInfo_t* BG_GetPlayerWeaponInfo();
extern vmCvar_t cg_fov;
extern vmCvar_t cg_widescreen;
float gZoomRatio;
float maxClamp[3];    // ?maxClamp (cg.o)
float maxClamp_0[3];  // ?maxClamp_0 (cg.o)
float minClamp[3];    // ?minClamp (cg.o)
float minClamp_0[3];  // ?minClamp_0 (cg.o)
class Camera;
extern Camera gCamera[];

// ea: 0x006A1900
void CG_CalcEntityLerpPositions(Entity* cent)
{
    if (cent->s.pos.trType != 0 /* TR_STATIONARY */)
    {
        if (cent->s.pos.trType == 5 /* TR_INTERPOLATE */)
        {
            CG_InterpolateEntityOrigin(cent);
        }
        else
        {
            math::Position3 v6;
    BG_EvaluateTrajectory(&cent->s.pos, cgGlobal_time, v6);
            cent->s.lerpOrigin.v = v6.v;
            if (!Entity_IsLocalPlayer(cent))
            {
                math::Position3 in = cent->s.lerpOrigin;
                CG_AdjustPositionForMover(
                    &in, cent->s.mGroundEntity.mHandle.mVal,
                    *(int*)((char*)&dword_F62960[1580 * currCl] + 4),
                    cgGlobal_time,
                    &v6, nullptr);
                cent->s.lerpOrigin.v = v6.v;
            }
        }
    }
    else
    {
        math::Position3 v6;
        v6.v = _mm_setr_ps(cent->s.pos.trBase[0], cent->s.pos.trBase[1],
                           cent->s.pos.trBase[2], 0.0f);
        cent->s.lerpOrigin.v = v6.v;
    }
    if (cent->s.apos.trType != 0 /* TR_STATIONARY */)
    {
        if (cent->s.apos.trType == 5 /* TR_INTERPOLATE */)
        {
            CG_InterpolateEntityAngles(cent);
        }
        else
        {
            math::Position3 v6;
    BG_EvaluateTrajectory(&cent->s.apos, cgGlobal_time, v6);
            cent->s.lerpAngles.v = v6.v;
        }
    }
    else
    {
        math::Position3 v6;
        v6.v = _mm_setr_ps(cent->s.apos.trBase[0], cent->s.apos.trBase[1],
                           cent->s.apos.trBase[2], 0.0f);
        cent->s.lerpAngles.v = v6.v;
    }
}

// ea: 0x006A30B0
void CG_PredictPlayerState()
{
    CG_PredictPlayerState_Internal();
}

static float AtanApprox(float x)
{
    if (x >= 0.5f)
    {
        float t = sqrtf(fabsf((1.0f - x) * 0.5f));
        float t2 = t * t;
        return t2 * t2 * t2 * -0.1079625f - t2 * t2 * 0.15000001f
               - t2 * t * 0.33333331f - t * 2.0f + 1.570796f;
    }
    float x2 = x * x;
    return x2 * x2 * x2 * 0.053981241f + x2 * x2 * 0.075000003f
           + x2 * x * 0.1666667f + x;
}

static float Atan2Approx(float y, float x)
{
    if (0.0f == fabsf(y) + fabsf(x))
        return 0.0f;
    float invLen = 1.0f / sqrtf(x * x + y * y);
    float result;
    if (fabsf(y) <= fabsf(x))
        result = AtanApprox(invLen * fabsf(y));
    else
        result = 1.5707964f - AtanApprox(invLen * fabsf(x));
    if (x < 0.0f)
        result = 3.1415927f - result;
    if (y < 0.0f)
        result = 0.0f - result;
    return result;
}

// ea: 0x0068D680 (release cg.o)
int CG_CalcCubemapViewValues()
{
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    Client* client = player->client;
    const int base = 1580 * currCl;
    const float cubemapSize = (float)cgGlobal.cubemapSize;
    const float cubemapSizePlusTwo = cubemapSize + 2.0f;

    dword_F63C50[base] = 0.0f;
    dword_F63C54[base] = 0.0f;
    dword_F63C58[base] = cubemapSizePlusTwo;
    dword_F63C5C[base] = cubemapSizePlusTwo;

    const float fovRadians =
        Atan2Approx(cubemapSizePlusTwo, cubemapSize);
    dword_F63C60[base] =
        (int)(fovRadians * 114.5915590261646f);
    dword_F63C64[base] = dword_F63C60[base];

    const char* ps = reinterpret_cast<const char*>(&client->ps);
    dword_F63C70[base] = *reinterpret_cast<const float*>(ps + 0x00);
    dword_F63C74[base] = *reinterpret_cast<const float*>(ps + 0x04);
    dword_F63C78[base] =
        *reinterpret_cast<const float*>(ps + 0x08)
        + *reinterpret_cast<const float*>(ps + 0xE0);

    const int face = cgGlobal.cubemapShot - 1;
    switch (face)
    {
    case 0:
        dword_F63C80[base] = 0.0f;
        dword_F63C84[base] = 0.0f;
        dword_F63C88[base] = 1.0f;
        dword_F63C8C[base] = 0.0f;
        dword_F63C90[base] = 1.0f;
        dword_F63C94[base] = 0.0f;
        dword_F63C98[base] = -1.0f;
        dword_F63C9C[base] = 0.0f;
        dword_F63CA0[base] = 0.0f;
        break;
    case 1:
        dword_F63C80[base] = 0.0f;
        dword_F63C84[base] = 0.0f;
        dword_F63C88[base] = -1.0f;
        dword_F63C8C[base] = 0.0f;
        dword_F63C90[base] = 1.0f;
        dword_F63C94[base] = 0.0f;
        dword_F63C98[base] = 1.0f;
        dword_F63C9C[base] = 0.0f;
        dword_F63CA0[base] = 0.0f;
        break;
    case 2:
        dword_F63C80[base] = -1.0f;
        dword_F63C84[base] = 0.0f;
        dword_F63C88[base] = 0.0f;
        dword_F63C8C[base] = 0.0f;
        dword_F63C90[base] = -1.0f;
        dword_F63C94[base] = 0.0f;
        dword_F63C98[base] = 0.0f;
        dword_F63C9C[base] = 0.0f;
        dword_F63CA0[base] = 1.0f;
        break;
    case 3:
        dword_F63C80[base] = 1.0f;
        dword_F63C84[base] = 0.0f;
        dword_F63C88[base] = 0.0f;
        dword_F63C8C[base] = 0.0f;
        dword_F63C90[base] = 1.0f;
        dword_F63C94[base] = 0.0f;
        dword_F63C98[base] = 0.0f;
        dword_F63C9C[base] = 0.0f;
        dword_F63CA0[base] = 1.0f;
        break;
    case 4:
        dword_F63C80[base] = 0.0f;
        dword_F63C84[base] = -1.0f;
        dword_F63C88[base] = 0.0f;
        dword_F63C8C[base] = 1.0f;
        dword_F63C90[base] = 0.0f;
        dword_F63C94[base] = 0.0f;
        dword_F63C98[base] = 0.0f;
        dword_F63C9C[base] = 0.0f;
        dword_F63CA0[base] = 1.0f;
        break;
    case 5:
        dword_F63C80[base] = 0.0f;
        dword_F63C84[base] = 1.0f;
        dword_F63C88[base] = 0.0f;
        dword_F63C8C[base] = -1.0f;
        dword_F63C90[base] = 0.0f;
        dword_F63C94[base] = 0.0f;
        dword_F63C98[base] = 0.0f;
        dword_F63C9C[base] = 0.0f;
        dword_F63CA0[base] = 1.0f;
        break;
    default:
        break;
    }
    return face;
}

// ea: 0x006A44D0
int CG_CalcFov()
{
    Client* client =
        EntityManager::sInst->GetPlayer( currCl)->client;
    float* cam = (float*)((char*)gCamera + 0x1F0 * currCl);
    float y = CG_GetViewFov();
    float v4 = y;
    if (*(float*)((char*)cam + 0x118) > *(float*)((char*)cam + 0x114))
    {
        v4 = (((y - *(float*)((char*)cam + 0x110))
               / *(float*)((char*)cam + 0x118))
              * *(float*)((char*)cam + 0x114))
             + *(float*)((char*)cam + 0x110);
        y = v4;
    }
    float v5 = (v4 - 20.0f) / (cg_fov.value - 20.0f);
    dword_F63CF0[1580 * currCl] = v5;
    if (v5 < 0.16f)
        dword_F63CF0[1580 * currCl] = 0.16f;
    if ((client->ps.eFlags & 0x200) != 0)
    {
        float v7 = dword_F63CF0[1580 * currCl];
        dword_F63CF0[1580 * currCl] =
            *(int*)((char*)client + 0x5A0) != 0 ? v7 * 0.89999998f
                                                : v7 * 0.69999999f;
    }
    void* PlayerWeaponInfo = BG_GetPlayerWeaponInfo();
    if (PlayerWeaponInfo != nullptr)
    {
        if (client->ps.fWeaponPosFrac == 1.0f)
        {
            if (*(float*)((char*)PlayerWeaponInfo + 0x644) > 0.0f)
                dword_F63CF0[1580 * currCl] =
                    dword_F63CF0[1580 * currCl]
                    * *(float*)((char*)PlayerWeaponInfo + 0x644);
        }
        else if (*(float*)((char*)PlayerWeaponInfo + 0x638) > 0.0f)
        {
            dword_F63CF0[1580 * currCl] =
                dword_F63CF0[1580 * currCl]
                * *(float*)((char*)PlayerWeaponInfo + 0x638);
        }
    }
    int v13 = 1580 * currCl;
    float aspectX = dword_F63C58[v13];
    float aspectY = dword_F63C5C[v13];
    float v14 = aspectX / tanf(y * 0.0087266462f);
    float fov_y = Atan2Approx(aspectY, v14) * 114.59155f;
    if (cg_widescreen.integer != 0)
    {
        aspectX = aspectX * 4.0f;
        aspectY = aspectY * 3.0f;
    }
    float v22 = aspectY / tanf(fov_y * 0.0087266462f);
    y = Atan2Approx(aspectX, v22) * 114.59155f;
    collision_context_t context;
    memset(&context, 0, sizeof(context));
    context.contentmask = 32;
    int v30;
    if (CG_PointContents((const math::Position3*)&dword_F63C70[v13],
                         &context)
        != 0)
    {
        float v29 = sinf(cgGlobal_time * 0.0025132743f);
        v30 = dword_F63CA8[1580 * currCl] | 0x20;
        y = y + v29;
        fov_y = fov_y - v29;
    }
    else
    {
        v30 = dword_F63CA8[1580 * currCl] & 0xFFFFFFDF;
    }
    dword_F63CA8[1580 * currCl] = v30;
    dword_F63C60[1580 * currCl] = *(int*)&y;
    dword_F63C64[1580 * currCl] = *(int*)&fov_y;
    const float fovX = *(float*)&dword_F63C60[1580 * currCl];
    if (fovX <= 0.0f || cg_fov.value <= 0.0f)
    {
        gZoomRatio = 1.0f;
    }
    else
    {
        float v32 = fovX / cg_fov.value;
        if (v32 < 1.0f)
            gZoomRatio = v32 * 0.75f;
        else
            gZoomRatio = 1.0f;
    }
    return 1580 * currCl * 4;
}

extern int CG_OffsetFirstPersonView();
extern void CG_Trace(trace_t* result, const math::Position3* start,
                     const math::Position3* mins, const math::Position3* maxs,
                     const math::Position3* end,
                     const collision_context_t* context);
extern void FastSinCos(float radians, float* psin, float* pcos);
extern void CG_ClampViewAngles(PlayerState* ps, const float* centerAngles,
                               const float* minClamp,
                               const float* maxClamp);
extern void AddLeanToPosition(float* const vPosition, float fViewYaw,
                              float fLeanFrac, float fViewRoll,
                              float fLeanDist);
extern void Com_DPrintf(const char* fmt, ...);
extern int G_DObjSetLocalTag(Entity* ent, int* const partBits,
                             unsigned int tag_name_hash,
                             const float* const trans, const float* const angles,
                             bool relative);
extern float flrand(float min, float max);
extern int CG_CalcCubemapViewValues();
extern void CG_CalcVrect(const View_Window* window);
extern void Camera_Update(void* self);
extern vmCvar_t bg_viewheight_prone;
extern vmCvar_t bg_viewheight_crouched;
extern vmCvar_t bg_viewheight_standing;
struct DObjSkelMat;
extern int G_DObjGetWorldTagMatrix(Entity* ent, unsigned int tag_name_hash,
                                   DObjSkelMat* tagMtx);
extern bool G_DObjGetWorldBoneIndexMatrix(Entity* ent, int boneIndex,
                                          DObjSkelMat* tagMtx);
struct vehicle_info_t;
extern vehicle_info_t* VEH_GetInfo(int idx);

extern float angle[4 * 1580];
extern int dword_F64024[4 * 1580];
extern vmCvar_t cg_bobMax;
extern vmCvar_t cg_thirdPersonAngle;
extern vmCvar_t cg_thirdPersonRange;
extern vmCvar_t cg_thirdPersonLock;

// ea: 0x0068CEA0
void CG_KickAngles()
{
    int frametime = cgGlobal_frametime;
    while (frametime > 0)
    {
        int step = frametime;
        if (step > 5)
            step = 5;
        else if (step == 0)
            CG_ASSERT("frametime", "c:\\cod\\code\\game\\cg_view.cpp",
                      472);
        const float dt = step * 0.001f;
        for (int axis = 0; axis < 3; ++axis)
        {
            const int index = axis + 1580 * currCl;
            float& speed = *reinterpret_cast<float*>(&dword_F64024[index]);
            float& kick = *reinterpret_cast<float*>(&dword_F64030[index]);
            if (speed == 0.0f && kick == 0.0f)
                continue;

            if (kick != 0.0f)
            {
                const float sign = kick > 0.0f ? -1.0f : 1.0f;
                float accel;
                Entity* player = EntityManager::sInst->GetPlayer(currCl);
                if (player->client->ps.weapon != 0)
                {
                    float* info = dword_F63B8C[1580 * currCl];
                    accel = player->client->ps.fWeaponPosFrac <= 0.5f
                                ? info[531]
                                : info[513];
                }
                else
                {
                    accel = 2400.0f;
                }
                speed += (accel * sign) * dt;
            }

            float delta = speed * dt;
            if (kick * delta < 0.0f)
                delta *= 0.06f;
            if ((kick + delta) * kick < 0.0f)
            {
                kick = 0.0f;
            }
            else
            {
                kick += delta;
                if (kick != 0.0f && fabsf(*reinterpret_cast<float*>(
                                      &dword_F64030[index])) > 10.0f)
                    kick = kick <= 0.0f ? -10.0f : 10.0f;
            }
            speed = 0.0f;
        }
        frametime -= 5;
        if (frametime <= 0)
            break;
    }
}

// ea: 0x0068D260
void CG_CalculateView_BobAngles(float* angles)
{
    const int index = 1580 * currCl;
    float* info = dword_F63B8C[index];
    if (info != nullptr && info[405] != 0.0f)
    {
        const float frac = EntityManager::sInst->GetPlayer(currCl)
                               ->client->ps.fWeaponPosFrac;
        angles[0] += dword_F64068[index] * frac;
        angles[1] += dword_F6406C[index] * frac;
        angles[2] += dword_F64070[index] * frac;
    }
}

// ea: 0x0068CE40
int CG_StepOffset()
{
    const int index = 1580 * currCl;
    int elapsed = cgGlobal_time - dword_F63BAC[index];
    if (elapsed < 0)
        dword_F63BAC[index] = cgGlobal_time;
    if (elapsed < 100)
        dword_F63C78[index] -=
            ((100 - elapsed) * *reinterpret_cast<float*>(&dword_F63BA8[index]))
            * 0.01f;
    return index * 4;
}

// ea: 0x006986D0
int CG_OffsetFirstPersonView()
{
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    Client* client = player->client;
    const int index = 1580 * currCl;
    float* origin = &dword_F63C70[index];
    float* viewAngles = &angle[index];

    if (dword_F62964[index] != 0
        && *reinterpret_cast<int*>(dword_F62964[index] + 52) >= 6)
    {
        viewAngles[2] = 40.0f;
        viewAngles[0] = -15.0f;
        int* deadSnap = reinterpret_cast<int*>(dword_F62964[6320 * currCl]);
        viewAngles[1] = *reinterpret_cast<float*>(deadSnap + 82);
        origin[2] += client->ps.viewHeightCurrent;
        return 6320 * currCl;
    }

    CG_KickAngles();
    viewAngles[0] += *reinterpret_cast<float*>(&dword_F64030[index]);
    viewAngles[1] += *reinterpret_cast<float*>(&dword_F64034[index]);
    viewAngles[2] += *reinterpret_cast<float*>(&dword_F64038[index]);
    CG_CalculateView_BobAngles(viewAngles);

    if (dword_F63FFC[index] != 0)
    {
        const float frac = client->ps.fWeaponPosFrac;
        float factor = 1.0f - frac * 0.5f;
        float* info = dword_F63B8C[index];
        if (frac != 0.0f && info != nullptr && info[405] != 0.0f)
            factor = (frac * 0.5f + 1.0f) * (1.0f - frac * 0.5f);
        const float elapsed = (float)(cgGlobal_time - dword_F63FFC[index]);
        float kickFrac;
        if (cg_viewKickDeflectTime.value <= elapsed)
        {
            const float ratio = 1.0f - ((elapsed - cg_viewKickDeflectTime.value)
                                        / cg_viewKickReturnTime.value);
            if (ratio <= 0.0f)
                kickFrac = 0.0f;
            else
                kickFrac = 1.0f - GetLeanFraction(1.0f - ratio);
        }
        else
        {
            kickFrac = GetLeanFraction(elapsed / cg_viewKickDeflectTime.value);
        }
        const float value = kickFrac * factor;
        viewAngles[0] += value * *reinterpret_cast<float*>(&dword_F6401C[index]);
        viewAngles[2] += value * *reinterpret_cast<float*>(&dword_F64020[index]);
    }

    origin[2] += client->ps.viewHeightCurrent;
    if ((client->ps.eFlags & 0x6000) == 0)
    {
        float* info = dword_F63B8C[index];
        if (client->ps.fWeaponPosFrac != 0.0f && info != nullptr
            && info[409] != 0.0f)
        {
            const float frac = client->ps.fWeaponPosFrac;
            viewAngles[0] -= info[409] * frac
                              * CG_GetVerticalBobFactor(
                                    dword_F641D8[index], dword_F641DC[index],
                                    45.0f);
            viewAngles[1] -= info[409] * frac
                              * CG_GetHorizontalBobFactor(
                                    dword_F641D8[index], dword_F641DC[index],
                                    45.0f);
        }
        origin[2] += CG_GetVerticalBobFactor(dword_F641D8[index],
                                             dword_F641DC[index],
                                             cg_bobMax.value);
        const float lateral = CG_GetHorizontalBobFactor(
            dword_F641D8[index], dword_F641DC[index], cg_bobMax.value);
        float right[3];
        AnglesToRight(viewAngles, right);
        origin[0] += right[0] * lateral;
        origin[1] += right[1] * lateral;
        origin[2] += right[2] * lateral;

        float elapsed = (float)(cgGlobal_time - dword_F63BB4[index]);
        if (elapsed < 0.0f)
        {
            dword_F63BB4[index] = cgGlobal_time - 450;
        }
        if (elapsed < 150.0f)
            origin[2] += elapsed * 0.0066666668f
                         * *reinterpret_cast<float*>(&dword_F63BB0[index]);
        else if (elapsed < 450.0f)
            origin[2] += (1.0f - (elapsed - 150.0f) * 0.0033333334f)
                         * *reinterpret_cast<float*>(&dword_F63BB0[index]);
        CG_StepOffset();

        float leaned[3] = {origin[0], origin[1], origin[2]};
        AddLeanToPosition(leaned, dword_F63CB4[index], client->ps.leanf,
                          16.0f, 20.0f);
        origin[0] = leaned[0];
        origin[1] = leaned[1];
        origin[2] = leaned[2];
        const float minZ = client->ps.origin.v.m128_f32[2] + 8.0f;
        if (origin[2] < minZ)
            origin[2] = minZ;
    }
    return 6320 * currCl;
}

static unsigned int s_thirdPersonInit;
static math::Position3 s_thirdPersonMins;
static math::Position3 s_thirdPersonMaxs;
static math::Position3 s_thirdPersonLockPos[4];
static math::Position3 s_thirdPersonLockAng[4];

// ea: 0x006A4020
void CG_OffsetThirdPersonView()
{
    if ((s_thirdPersonInit & 1) == 0)
    {
        s_thirdPersonMins.v = _mm_setr_ps(-4.0f, -4.0f, -4.0f, 0.0f);
        s_thirdPersonInit |= 1;
    }
    if ((s_thirdPersonInit & 2) == 0)
    {
        s_thirdPersonMaxs.v = _mm_setr_ps(4.0f, 4.0f, 4.0f, 0.0f);
        s_thirdPersonInit |= 2;
    }

    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    Client* client = player->client;
    const int index = 1580 * currCl;
    if (cg_thirdPersonLock.integer != 0)
    {
        const math::Position3& lockedPos = s_thirdPersonLockPos[currCl];
        const math::Position3& lockedAng = s_thirdPersonLockAng[currCl];
        dword_F63C70[index] = lockedPos.v.m128_f32[0];
        dword_F63C74[index] = lockedPos.v.m128_f32[1];
        dword_F63C78[index] = lockedPos.v.m128_f32[2];
        angle[index] = lockedAng.v.m128_f32[0];
        dword_F63CB4[index] = lockedAng.v.m128_f32[1];
        dword_F63CB8[index] = lockedAng.v.m128_f32[2];
        return;
    }

    dword_F63C78[index] += client->ps.viewHeightCurrent;
    angle[index] = 0.0f;
    dword_F63CB4[index] = (float)cg_thirdPersonAngle.integer;
    // The cg-local PlayerState mirror does not expose the release stats array;
    // normal and spectator views both use the registered camera angle here.
    if (angle[index] > 45.0f)
        angle[index] = 45.0f;

    float viewAngles[3] = {angle[index], dword_F63CB4[index],
                           dword_F63CB8[index]};
    float forward[3], right[3], up[3];
    AnglesToForward(viewAngles, forward);

    math::Position3 focus(
        dword_F63C70[index], dword_F63C74[index], dword_F63C78[index] + 8.0f);
    AngleVectors(viewAngles, forward, right, up);
    float sinAngle, cosAngle;
    FastSinCos(cg_thirdPersonAngle.value * 0.017453292f, &sinAngle,
               &cosAngle);
    focus.v.m128_f32[0] +=
        (-cg_thirdPersonRange.value * cosAngle) * right[0]
        + (-cg_thirdPersonRange.value * sinAngle) * forward[0];
    focus.v.m128_f32[1] +=
        (-cg_thirdPersonRange.value * cosAngle) * right[1]
        + (-cg_thirdPersonRange.value * sinAngle) * forward[1];
    focus.v.m128_f32[2] +=
        (-cg_thirdPersonRange.value * cosAngle) * right[2]
        + (-cg_thirdPersonRange.value * sinAngle) * forward[2];

    collision_context_t context(player->mHandle, 17);
    trace_t trace;
    CG_Trace(&trace,
             reinterpret_cast<const math::Position3*>(&dword_F63C70[index]),
             &s_thirdPersonMins, &s_thirdPersonMaxs, &focus, &context);
    if (trace.normal.v.m128_f32[1] != 1.0f)
    {
        focus = trace.endpos;
        focus.v.m128_f32[2] +=
            (1.0f - trace.normal.v.m128_f32[1]) * 32.0f;
        CG_Trace(&trace,
                 reinterpret_cast<const math::Position3*>(&dword_F63C70[index]),
                 &s_thirdPersonMins, &s_thirdPersonMaxs, &focus, &context);
        focus = trace.endpos;
    }

    dword_F63C70[index] = focus.v.m128_f32[0];
    dword_F63C74[index] = focus.v.m128_f32[1];
    dword_F63C78[index] = focus.v.m128_f32[2];
    angle[index] = 14.0f;
    dword_F63C78[index] = client->ps.origin.v.m128_f32[2]
                          + (client->ps.pm_type < 6 ? 68.0f : 34.0f);
    dword_F63CB4[index] -= cg_thirdPersonAngle.value;

    s_thirdPersonLockPos[currCl].v =
        _mm_setr_ps(dword_F63C70[index], dword_F63C74[index],
                    dword_F63C78[index], 0.0f);
    s_thirdPersonLockAng[currCl].v =
        _mm_setr_ps(angle[index], dword_F63CB4[index], dword_F63CB8[index],
                    0.0f);
}

static Entity* DbHandleToEntityLocal(unsigned int handle)
{
    unsigned int idx = handle & 0xFFF;
    if (idx < 0x540
        && (handle >> 12) == EntityHandleDb::sInst.mElements[idx].mKey)
        return EntityHandleDb::sInst.mElements[idx].mObject;
    return nullptr;
}

static float s_offset[4][12];
static float s_swayOffset[4][12];
static float s_swayAngles[4][12];
static float s_swayViewAngles[4][12];
static bool s_wasAnimating[4];
static float s_F73C20[4 * 1580];
static float s_F73C24[4 * 1580];
static float s_F73C30[4 * 1580];
static float s_F73C34[4 * 1580];
static float s_F73C40[4 * 1580];
static float s_F73C44[4 * 1580];
static float s_F73C50[4 * 1580];
static float s_F73C54[4 * 1580];
static unsigned int s_tagBarrelHash;
static bool s_tagBarrelHashInit;
static unsigned int s_tagPlayerHash;
static bool s_tagPlayerHashInit;
static unsigned int s_tagAimHash;
static bool s_tagAimHashInit;
static float s_horchOfs[3];
static float s_dodgeOfs[3];
static float s_shake;
static unsigned int s_turretTagPlayerHash;
static bool s_turretTagPlayerHashInit;

// ea: 0x006A5870 (release cg.o)
// Turret cameras follow the locked entity's tag_player bone while the
// player-state viewlocked flag is active.  Keep the player-state accesses at
// their release offsets: cg.o's local PlayerState view is intentionally only
// a partial declaration and its member order is not a wire-layout guarantee.
void CG_CalcTurretViewValues()
{
    Client* client = EntityManager::sInst->GetPlayer(currCl)->client;
    char* ps = reinterpret_cast<char*>(&client->ps);
    const int eFlags = *reinterpret_cast<const int*>(ps + 0xF4);
    if ((eFlags & 0x6000) == 0)
        return;

    const unsigned int viewLockedHandle =
        *reinterpret_cast<const unsigned int*>(ps + 0x4A0);
    Entity* lockedEntity = DbHandleToEntityLocal(viewLockedHandle);
    if (lockedEntity == nullptr
        || *reinterpret_cast<const int*>(ps + 0x49C) == 0)
        return;

    math::Position3* viewAngles =
        reinterpret_cast<math::Position3*>(&angle[1580 * currCl]);
    BG_EvaluateTrajectory(&lockedEntity->s.apos, cgGlobal_time, *viewAngles);

    if (!s_turretTagPlayerHashInit)
    {
        s_turretTagPlayerHashInit = true;
        s_turretTagPlayerHash = HashString::CalcHash("tag_player");
    }
    DObjSkelMat tagMtx;
    if (G_DObjGetWorldTagMatrix(lockedEntity, s_turretTagPlayerHash,
                                &tagMtx) == 0)
    {
        Com_Error((errorParm_t)1,
                  "\x15Turret has no bone: tag_player\n");
        return;
    }

    const int base = 1580 * currCl;
    angle[base] += lockedEntity->s.angles2.v.m128_f32[0];
    dword_F63CB4[base] += lockedEntity->s.angles2.v.m128_f32[1];
    dword_F63C70[base] = tagMtx.origin[0];
    dword_F63C74[base] = tagMtx.origin[1];
    dword_F63C78[base] =
        tagMtx.origin[2] - *reinterpret_cast<const float*>(ps + 0xE0);
}

// ea: 0x006A4BF0
void CG_CalcGunnerViewPos(bool crouched, unsigned int tag_gunner_barrel_hash)
{
    Client* client =
        EntityManager::sInst->GetPlayer( currCl)->client;
    Entity* ent = DbHandleToEntityLocal(client->ps.mViewLockedEntity);
    if (!s_tagBarrelHashInit)
    {
        s_tagBarrelHashInit = true;
        s_tagBarrelHash = HashString::CalcHash("tag_barrel");
    }
    if (ent == nullptr)
    {
        int v6 = 1580 * currCl;
        dword_F63C70[v6] = client->ps.origin.v.m128_f32[0];
        dword_F63C74[v6] = client->ps.origin.v.m128_f32[1];
        dword_F63C78[v6] = client->ps.origin.v.m128_f32[2];
        angle[1580 * currCl] = client->ps.viewangles[0];
        dword_F63CB4[v6] = client->ps.viewangles[1];
        dword_F63CB8[v6] = client->ps.viewangles[2];
        CG_OffsetFirstPersonView();
        return;
    }
    float tagMtx[16];
    if (G_DObjGetWorldTagMatrix(ent, tag_gunner_barrel_hash,
                                (DObjSkelMat*)tagMtx) == 0)
    {
        if (!s_tagPlayerHashInit)
        {
            s_tagPlayerHashInit = true;
            s_tagPlayerHash = HashString::CalcHash("tag_player");
        }
        if (G_DObjGetWorldTagMatrix(ent, s_tagPlayerHash,
                                    (DObjSkelMat*)tagMtx) == 0)
            return;
    }
    int v7 = 1580 * currCl;
    dword_F63C70[v7] = tagMtx[12];
    dword_F63C74[v7] = tagMtx[13];
    dword_F63C78[v7] = tagMtx[14];
    if (crouched)
    {
        void* Info = VEH_GetInfo(
            *(short*)((char*)ent->scr_vehicle + 0x178));
        float fwd[3] = {tagMtx[0], tagMtx[1], tagMtx[2]};
        float len = sqrtf(fwd[0] * fwd[0] + fwd[1] * fwd[1]
                          + fwd[2] * fwd[2]);
        fwd[0] /= len;
        fwd[1] /= len;
        fwd[2] /= len;
        dword_F63C70[v7] = dword_F63C70[v7] - fwd[0] * 25.0f;
        dword_F63C74[v7] = dword_F63C74[v7] - fwd[1] * 25.0f;
        dword_F63C78[v7] = dword_F63C78[v7] - fwd[2] * 25.0f;
        if (*(int*)((char*)Info + 0x4B4) == 1)
            dword_F63C78[v7] = dword_F63C78[v7] - 11.0f;
        else
            dword_F63C78[v7] = dword_F63C78[v7] - 4.0f;
    }
    else if (tag_gunner_barrel_hash == s_tagBarrelHash)
    {
        dword_F63C70[v7] = dword_F63C70[v7] - tagMtx[0] * 35.0f;
        dword_F63C74[v7] = dword_F63C74[v7] - tagMtx[1] * 35.0f;
        dword_F63C78[v7] = dword_F63C78[v7] - tagMtx[2] * 35.0f;
    }
    else
    {
        void* scr_vehicle = ent->scr_vehicle;
        float v19 = 10.0f;
        if (scr_vehicle != nullptr)
        {
            void* v21 =
                VEH_GetInfo(*(short*)((char*)scr_vehicle + 0x178));
            if (client->ps.vehType == 1
                && *(int*)((char*)v21 + 0x4B4) == 1)
            {
                dword_F63C70[v7] += s_horchOfs[0] * tagMtx[0];
                dword_F63C74[v7] += s_horchOfs[0] * tagMtx[1];
                dword_F63C78[v7] += s_horchOfs[0] * tagMtx[2];
                dword_F63C70[v7] += s_horchOfs[1] * tagMtx[4];
                dword_F63C74[v7] += s_horchOfs[1] * tagMtx[5];
                dword_F63C78[v7] += s_horchOfs[1] * tagMtx[6];
                v19 = s_horchOfs[2];
            }
            else
            {
                dword_F63C70[v7] += s_dodgeOfs[0] * tagMtx[0];
                dword_F63C74[v7] += s_dodgeOfs[0] * tagMtx[1];
                dword_F63C78[v7] += s_dodgeOfs[0] * tagMtx[2];
                dword_F63C70[v7] += s_dodgeOfs[1] * tagMtx[4];
                dword_F63C74[v7] += s_dodgeOfs[1] * tagMtx[5];
                dword_F63C78[v7] += s_dodgeOfs[1] * tagMtx[6];
                v19 = s_dodgeOfs[2];
            }
            dword_F63C70[v7] += v19 * tagMtx[8];
            dword_F63C74[v7] += v19 * tagMtx[9];
            dword_F63C78[v7] += v19 * tagMtx[10];
        }
        else
        {
            dword_F63C70[v7] += tagMtx[0] * 0.0f;
            dword_F63C74[v7] += tagMtx[1] * 0.0f;
            dword_F63C78[v7] += tagMtx[2] * 0.0f;
            dword_F63C70[v7] += tagMtx[8] * v19;
            dword_F63C74[v7] += tagMtx[9] * v19;
            dword_F63C78[v7] += tagMtx[10] * v19;
        }
    }
    float viewaxis[3][3] = {{tagMtx[0], tagMtx[4], tagMtx[8]},
                            {tagMtx[1], tagMtx[5], tagMtx[9]},
                            {tagMtx[2], tagMtx[6], tagMtx[10]}};
    float angoffset[3];
    AxisToAngles(viewaxis, angoffset);
    angoffset[0] = AngleNormalize180(angoffset[0]);
    angle[1580 * currCl] = angoffset[0];
    dword_F63CB4[1580 * currCl] = angoffset[1];
    dword_F63CB8[1580 * currCl] = 0.0f;
    if (crouched)
    {
        angle[1580 * currCl] = 0.0f;
    }
    else
    {
        void* v41 = ent->scr_vehicle;
        int v43;
        if (v41 != nullptr && *(int*)((char*)v41 + 0) != 0)
            v43 = *(int*)((char*)v41 + 0);
        else
        {
            if (ent->s.weapon == 0)
                return;
            v43 = ent->s.weapon;
        }
        Entity* Player =
            EntityManager::sInst->GetPlayer( currCl);
        if (*(bool*)((char*)Player->client + 0xAE8))
        {
            s_wasAnimating[currCl] = true;
        }
        else
        {
            if (s_wasAnimating[currCl])
            {
                s_swayViewAngles[currCl][0] = client->ps.viewangles[0];
                s_F73C50[1580 * currCl] = client->ps.viewangles[1];
                s_F73C54[1580 * currCl] = client->ps.viewangles[2];
                s_F73C34[1580 * currCl] = 0.0f;
                s_F73C30[1580 * currCl] = 0.0f;
                s_F73C40[1580 * currCl] = 0.0f;
                s_F73C44[1580 * currCl] = 0.0f;
                s_swayOffset[currCl][0] = 0.0f;
                s_swayAngles[currCl][0] = 0.0f;
                s_wasAnimating[currCl] = false;
            }
            CG_CalculateWeaponPosition_Sway();
        }
    }
    int v48 = cgGlobal_time % 100;
    if (cgGlobal_time % 100 > 50)
        v48 = 100 - v48;
    float v50 = v48 * 0.02f;
    s_F73C24[1580 * currCl] = 0.0f;
    s_F73C20[1580 * currCl] = 0.0f;
    s_offset[currCl][0] = 0.0f;
    bool overheating = false;
    if (!crouched && (client->ps.eFlags & 0x200) != 0)
    {
        void* v52 = ent->scr_vehicle;
        if (v52 != nullptr)
            overheating = *(bool*)((char*)v52 + 0x3BC);
        else
        {
            void* pTurretInfo = ent->pTurretInfo;
            if (pTurretInfo != nullptr)
                overheating = *(bool*)((char*)pTurretInfo + 0);
        }
        if (!overheating)
        {
            int v56 = 1580 * currCl;
            angle[1580 * currCl] -= v50 * 0.5f;
            float v57 = 0.0f - (v50 * 0.5f);
            s_offset[currCl][0] += v57 * tagMtx[0];
            s_F73C20[1580 * currCl] += v57 * tagMtx[1];
            s_F73C24[1580 * currCl] += v57 * tagMtx[2];
            float turretShake = cosf(0.020092782f * cgGlobal_time);
            angle[1580 * currCl] =
                sinf(cgGlobal_time * 0.014666426f) * turretShake * 0.25f
                + angle[1580 * currCl];
            s_shake = turretShake;
            if (!s_tagAimHashInit)
            {
                s_tagAimHashInit = true;
                s_tagAimHash = HashString::CalcHash("tag_aim");
            }
            unsigned int v64 = ent->pTurretInfo == nullptr
                                   ? tag_gunner_barrel_hash
                                   : s_tagAimHash;
            int partBits[4] = {0, 0, 0, 0};
            G_DObjSetLocalTag(ent, partBits, v64, &s_shake, nullptr, true);
        }
    }
    if (overheating || crouched || (client->ps.eFlags & 0x200) == 0)
    {
        s_offset[currCl][0] += tagMtx[0] * s_swayOffset[currCl][0];
        s_F73C20[1580 * currCl] += tagMtx[1] * s_swayOffset[currCl][0];
        s_F73C24[1580 * currCl] += tagMtx[2] * s_swayOffset[currCl][0];
        float v67 = 0.0f - s_F73C44[1580 * currCl];
        s_offset[currCl][0] += s_F73C40[1580 * currCl] * tagMtx[4];
        s_F73C20[1580 * currCl] +=
            s_F73C40[1580 * currCl] * tagMtx[5];
        s_F73C24[1580 * currCl] +=
            s_F73C40[1580 * currCl] * tagMtx[6];
        s_offset[currCl][0] += v67 * tagMtx[8];
        s_F73C20[1580 * currCl] += v67 * tagMtx[9];
        s_F73C24[1580 * currCl] += v67 * tagMtx[10];
        dword_F63C70[1580 * currCl] += s_offset[currCl][0];
        dword_F63C74[1580 * currCl] += s_F73C20[1580 * currCl];
        dword_F63C78[1580 * currCl] += s_F73C24[1580 * currCl];
    }
}

// ea: 0x006A5680
int CG_CalcPassengerViewPos()
{
    Client* client =
        EntityManager::sInst->GetPlayer( currCl)->client;
    Entity* mObject = DbHandleToEntityLocal(client->ps.mViewLockedEntity);
    Client* v3 = EntityManager::sInst->GetPlayer( currCl)->client;
    if (mObject->scr_vehicle == nullptr)
        CG_ASSERT("ent->scr_vehicle", "c:\\cod\\code\\game\\cg_view.cpp",
                  1421);
    float tagMtx[16];
    int v4 = G_DObjGetWorldBoneIndexMatrix(
        mObject,
        *(int*)((char*)mObject->scr_vehicle + 0x10 + 12 * client->ps.vehPos),
        (DObjSkelMat*)tagMtx);
    if (v4 != 0)
    {
        int v5 = 1580 * currCl;
        dword_F63C70[v5] = tagMtx[12] + tagMtx[8] * 25.0f;
        dword_F63C74[v5] = tagMtx[13] + tagMtx[9] * 25.0f;
        dword_F63C78[v5] = tagMtx[14] + tagMtx[10] * 25.0f;
        float tagAngles[3];
        AxisToAngles((const float(*)[3])tagMtx, tagAngles);
        if ((v3->ps.pm_flags & 0x20) != 0
            && *(int*)((char*)BG_GetInfoForWeapon(v3->ps.weapon) + 0x84)
                   == 10 /* WEAPCLASS_LMG */)
        {
            extern float minClamp_0[3];
            extern float maxClamp_0[3];
            CG_ClampViewAngles(&client->ps, tagAngles, minClamp_0,
                               maxClamp_0);
        }
        else
        {
            extern float minClamp[3];
            extern float maxClamp[3];
            CG_ClampViewAngles(&client->ps, tagAngles, minClamp, maxClamp);
        }
        angle[1580 * currCl] = client->ps.viewangles[0];
        dword_F63CB4[1580 * currCl] = client->ps.viewangles[1];
        dword_F63CB8[1580 * currCl] = client->ps.viewangles[2];
    }
    return v4;
}

// ea: 0x006AB800
int CG_CalcMuzzlePoint(unsigned int entity, float* muzzle, char* flashTag)
{
    int result = dword_F62960[1580 * currCl];
    if (result != 0)
    {
        unsigned int mVal = entity;
        Entity* mObject = nullptr;
        if ((*(int*)((char*)&dword_F62960[1580 * currCl] + 60)
             & 0x180000)
            == 0)
            goto label_8;
        mObject = DbHandleToEntityLocal(entity);
        if (mObject == EntityManager::sInst->GetPlayer( currCl))
        {
            muzzle[0] =
                *(float*)((char*)&dword_F62960[1580 * currCl] + 16);
            muzzle[1] =
                *(float*)((char*)&dword_F62960[1580 * currCl] + 20);
            muzzle[2] = *(float*)((char*)&dword_F62960[1580 * currCl] + 24)
                        + *(float*)((char*)&dword_F62960[1580 * currCl]
                                    + 240);
            AddLeanToPosition(muzzle, dword_F63CB4[1580 * currCl],
                              *(float*)((char*)&dword_F62960[1580 * currCl]
                                        + 92),
                              16.0f, 20.0f);
        }
        else
        {
        label_8:
            Entity* v10 = DbHandleToEntityLocal(entity);
            if (v10 != nullptr)
            {
                const char* v11 = flashTag;
                unsigned int flash_tag_hash = HashString::CalcHash(flashTag);
                float tagMat[16];
                if (G_DObjGetWorldTagMatrix(v10, flash_tag_hash,
                                            (DObjSkelMat*)tagMat)
                    != 0)
                {
                    muzzle[0] = tagMat[12];
                    muzzle[1] = tagMat[13];
                    muzzle[2] = tagMat[14];
                }
                else
                {
                    muzzle[0] = v10->s.pos.trBase[0];
                    muzzle[1] = v10->s.pos.trBase[1];
                    muzzle[2] = v10->s.pos.trBase[2];
                    float v15;
                    if (v10->client != nullptr)
                    {
                        Com_DPrintf("No %s in CG_CalcMuzzlePoint on "
                                    "entity.\n",
                                    v11);
                        int eFlags = v10->s.eFlags;
                        if ((eFlags & 0x40) != 0)
                            v15 = bg_viewheight_prone.integer + muzzle[2];
                        else if ((eFlags & 0x20) != 0)
                            v15 = bg_viewheight_crouched.integer + muzzle[2];
                        else
                            v15 = bg_viewheight_standing.integer + muzzle[2];
                    }
                    else
                    {
                        static unsigned int s_gunnerFlashHash;
                        static bool s_gunnerFlashHashInit;
                        unsigned int v16;
                        if (!s_gunnerFlashHashInit)
                        {
                            s_gunnerFlashHashInit = true;
                            s_gunnerFlashHash =
                                HashString::CalcHash("tag_gunner_flash");
                        }
                        v16 = s_gunnerFlashHash;
                        float tagMat2[16];
                        if (flash_tag_hash == v16
                            || G_DObjGetWorldTagMatrix(
                                   v10, v16, (DObjSkelMat*)tagMat2)
                                   == 0)
                            return 1;
                        muzzle[0] = tagMat2[12];
                        muzzle[1] = tagMat2[13];
                        v15 = tagMat2[14];
                    }
                    muzzle[2] = v15;
                }
            }
        }
        return 1;
    }
    return result;
}

// ea: 0x0068CCF0
void CG_CalcVrect(const View_Window* window)
{
    float viewScale;
    const int index = 1580 * currCl;
    if (*(int*)((char*)dword_F62964[index] + 0x34) == 5)
    {
        viewScale = 1.0f;
    }
    else if (cg_viewsize.integer >= 30)
    {
        if (cg_viewsize.integer <= 100)
            viewScale = (float)cg_viewsize.integer * 0.01f;
        else
        {
            Cvar_VMSet(&cg_viewsize, "100");
            viewScale = 1.0f;
        }
    }
    else
    {
        Cvar_VMSet(&cg_viewsize, "30");
        viewScale = 0.3f;
    }

    const float heightScale = cg_letterbox.integer != 0
                                  ? viewScale * 0.85f
                                  : viewScale;
    const float vidHeight = (float)cgs[currCl].vidHeight;
    const float vidWidth = (float)cgs[currCl].vidWidth;
    const float height = (window->Height * vidHeight) * heightScale;
    const float centerX = (window->XPos * vidWidth) * 0.5f + vidWidth * 0.5f;
    const float centerY = (window->YPos * vidHeight) * 0.5f + vidHeight * 0.5f;
    const int width = (int)(((window->Width * vidWidth) * viewScale) * 0.5f
                            + 0.5f) & ~1;
    const int heightPixels = (int)(height * 0.5f + 0.5f) & ~1;
    dword_F63C58[index] = (float)width;
    dword_F63C5C[index] = (float)heightPixels;
    dword_F63C50[index] = centerX;
    dword_F63C54[index] = centerY;
}

// ea: 0x006B05C0
void CG_CalcViewValues(const View_Window* window)
{
    if (cgGlobal_cubemapShot != 0 /* CUBEMAPSHOT_NONE */)
    {
        CG_CalcCubemapViewValues();
    }
    else
    {
        Camera_Update((char*)gCamera + 0x1F0 * currCl);
        CG_CalcVrect(window);
        CG_CalcFov();
    }
}

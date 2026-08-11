// ============================================================================
// cg_view.cpp - view/fov/compass/weapon-position helpers (cg.o cg_view.cpp)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"
#include "game/trace_types.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// Minimal view of GamePause (full class in game/sv/sv_stubs.h).
struct GamePause { static bool IsGamePaused(int client); };


// Minimal view of RumbleManager (full class in core/core_systems.h).
struct RumbleManager {
    static RumbleManager* Inst(int instance);  // ?Inst@RumbleManager@@SAPAV1@H@Z (g.o)
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
extern int cgGlobal_time;
extern int cgGlobal_oldTime;
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
extern int dword_F63CF4[4 * 1580];
extern int dword_F641E0[4 * 1580];
extern int dword_F641E4[4 * 1580];
extern int dword_F641E8[4 * 1580];
extern int dword_F641EC[4 * 1580];
extern float dword_F63CB4[4 * 1580];
extern float* dword_F63B8C[4 * 1580];
extern int dword_F63B34[4 * 1580];
extern int dword_F6355C[4 * 1580];
extern float dword_F62964[4 * 1580];
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
extern int cg_fov;
extern int cg_altTankCam;
extern int cg_hudCompassSpringyPointers;
extern int cg_drawGun;

extern float AngleNormalize360(float angle);
extern float AngleNormalize180(float angle);
extern float AngleSubtract(float a1, float a2);
extern void CL_SetViewAnglesAxis(int axis, float angle);
extern bool BG_AllowPlayerWeaponAtVehiclePos(int vehType, int vehPos);
extern int BG_IsAimDownSightWeapon(int iWeapon);
extern weaponFileInfo_t* BG_GetPlayerWeaponInfo();
extern void CG_DrawGameScreenFade();
extern void trap_R_SetColor(const float* rgba);
extern void trap_R_DrawStretchPic(float x, float y, float w, float h, float s1,
                                  float t1, float s2, float t2, void* tex,
                                  float z);
extern void CG_FillRect(float x, float y, float width, float height,
                        const float* color, float z);
extern void* cgsGlobal_media_whiteShader;

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
    float fViewFov = *(float*)&cg_fov;
    if (*(float*)&cg_fov < 1.0f || *(float*)&cg_fov > 160.0f)
        fViewFov = *(float*)&cg_fov;
    if (client->ps.pm_type >= 6)
        return *(float*)&cg_fov;
    bool vehicleBlocked = (client->ps.eFlags & 0x106000) != 0
                          && !BG_AllowPlayerWeaponAtVehiclePos(
                                 client->ps.vehType, client->ps.vehPos);
    if (!(cg_altTankCam != 0 && client->ps.vehType == 2
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
    if (cg_hudCompassSpringyPointers == 0)
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

extern float VectorNormalize(float* v);
extern void CrossProduct(const float* v1, const float* v2, float* cross);
extern void MatrixMultiply(const float (*in1)[3], const float (*in2)[3],
                           float (*out)[3]);
extern int dword_F64178[4 * 1580];
extern int dword_F6417C[4 * 1580];
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
struct vmCvar_t {
    int integer;  // +0x00
};
extern vmCvar_t cg_camerashake;
extern int cg_hudDamageIconTime;
extern int cg_viewKickScale;
extern int cg_viewKickMax;
extern int cg_redFlashTime;
extern int dword_F64018[4 * 1580];
extern int dword_F6401C[4 * 1580];
extern int dword_F64020[4 * 1580];
extern int dword_F63FFC[4 * 1580];
extern int dword_F63F9C[4 * 1580 * 3];
extern int dword_F63FA0[4 * 1580 * 3];
extern int dword_F63FA8[4 * 1580 * 3];
extern float sRumIntensityMin;
extern float sRumIntensityMax;
extern float sRumIntensityFactor;
extern float sRumTimeMin;
extern float sRumTimeMax;
extern float sRumTimeFactor;
extern void AnglesToForward(const float* const angles, float* const forward);
extern int CG_UpdateCameraShake(void* shake, int client);
extern void CG_EndShellShock(const void* parms, int time);
extern void CG_UpdateShellShockSound(const void* parms);
extern void CG_UpdateShellShockMouse(const void* parms, int time, int duration);
extern void CG_UpdateShellShockCamera(const void* parms, int time,
                                      int duration);
extern void CL_SetUserCmdInShellshock(int shocked);
extern char* va(const char* fmt, ...);
extern int FS_FOpenFileByMode(const char* qpath, int* f, int mode);
extern unsigned int FS_Write(char* buffer, unsigned int len, int h);
extern unsigned int FS_Read(unsigned char* buffer, unsigned int len, int f);
extern void FS_FCloseFile(int f);
extern int Com_SaveCvarsToBuffer(const char** cvarnames, int numCvars,
                                 char* buffer, unsigned int bufsize);
extern int Com_LoadCvarsFromBuffer(const char** cvarnames, int numCvars,
                                   const char* buffer, const char* filename);
struct vmCvar_t;
extern void Cvar_Update(vmCvar_t* vmCvar);
extern void* _Z_MallocInternal(int size);
extern void _Z_FreeInternal(void* ptr);
extern void CG_Printf(const char* msg, ...);
extern void RumbleManager_Play(void* mgr, void* result, const void* effect,
                               float intensity);
extern void AxisCopy(const float (*in)[3], float (*out)[3]);
extern float dword_F62960[4 * 1580];
extern const char** cg_shock_cvar_names;
extern void** cg_shock_cvar_ptrs;
extern int cg_shock_viewKickFadeTime;
extern int cg_shock_viewKickPeriod;
extern int cg_shock_viewKickRadius;
extern int cg_shock_sound;
extern int cg_shock_soundFadeInTime;
extern int cg_shock_soundFadeOutTime;
extern int cg_shock_soundLoopFadeTime;
extern int cg_shock_soundLoopEndDelay;
extern int cg_shock_soundRoomType;
extern int cg_shock_soundWetLevel;
extern int cg_shock_soundModEndDelay;
extern int cg_shock_volume_auto;
extern int cg_shock_volume_menu;
extern int cg_shock_volume_weapon;
extern int cg_shock_volume_voice;
extern int cg_shock_volume_item;
extern int cg_shock_volume_body;
extern int cg_shock_volume_local;
extern int cg_shock_volume_music;
extern int cg_shock_volume_announcer;
extern int cg_shock_volume_shellshock;
extern int cg_shock_mouse;
extern int cg_shock_mouse_fadeTime;
extern int cg_shock_mouse_maxpitchspeed;
extern int cg_shock_mouse_maxyawspeed;
extern int cg_shock_mouse_sensitivityscale;

struct shellshock_parms_t {
    struct {
        int fadeTime;
        float kickRate;
        float kickRadius;
    } view;  // +0x00
    struct {
        int a;
        int b;
    } screenBlend;  // +0x0C
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
    } sound;  // +0x14
    struct {
        int use;
        int fadeTime;
        float maxPitchSpeed;
        float maxYawSpeed;
        float sensitivity;
    } mouse;  // +0x68
};

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
    if (FS_FOpenFileByMode(v1, &fh, 1 /* FS_WRITE */) < 0)
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
        CG_UpdateShellShockSound(parms);
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
    int v2 = FS_FOpenFileByMode(v1, &fh, 0 /* FS_READ */);
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
        float shake[9];
        shake[0] = p;                    // scale
        shake[1] = (float)duration;      // length
        shake[2] = (float)cgGlobal_time; // time
        shake[3] = src[0];
        shake[4] = src[1];
        shake[5] = src[2];
        shake[6] = radius;
        shake[7] = 0.0f;  // size
        shake[8] = 0.0f;  // nextDelay?
        CG_UpdateCameraShake(shake, client);
        int v5 = 0;
        float* v6 = &unk_F640A8[6320 * client];
        while (v6[0] <= (float)cgGlobal_time
               && cgGlobal_time < (int)(v6[0] + v6[2]))
        {
            ++v5;
            v6 += 9;
            if (v5 >= 4)
            {
                int minsize = *(int*)&shake[7];
                if (v5 != 4)
                    CG_ASSERT("i == 4", "c:\\cod\\code\\game\\cg_draw.cpp",
                              2446);
                if (shake[7] > *(float*)&dword_F640C4[1580 * client])
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
        memcpy(&unk_F640A8[6320 * client + 36 * v5], shake, sizeof(shake));
    }
}

// ea: 0x00695DA0
void CG_ShakeCamera(int client)
{
    if (!GamePause::IsGamePaused(currCl) && cg_camerashake.integer != 0)
    {
        float scale = 0.0f;
        float sx = cgGlobal_time * 0.0016666667f;
        float* v1 = &unk_F640A8[6320 * client] + 1;
        int intensity = 4;
        do
        {
            if (CG_UpdateCameraShake(v1 - 1, client) != 0 && v1[0] > scale)
                scale = v1[6];
            v1 += 9;
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
        int integer = cg_hudDamageIconTime;
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
    void* v25 = RumbleManager::Inst(currCl);
    if (v25 != nullptr)
    {
        float result;
        RumbleManager_Play(v25, &result, &rumbleEffect, 1.0f);
    }
}

extern int dword_F641D8[4 * 1580];
extern int dword_F641DC[4 * 1580];
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
extern float dword_F64164[4 * 1580];
extern float dword_F64168[4 * 1580];
extern float dword_F6416C[4 * 1580];
extern int cg_bobWeaponLag;
extern int cg_bobWeaponAmplitude;
extern int cg_bobWeaponMax;
extern int cg_bobWeaponRollAmplitude;
extern int cg_bobAmplitudeProne;
extern int cg_bobAmplitudeDucked;
extern int cg_bobAmplitudeStanding;
extern int cgGlobal_frametime;
extern float CG_GetVerticalBobFactor(float a1, float a2, float a3);
extern float CG_GetHorizontalBobFactor(float a1, float a2, float a3);
extern void AngleVectors(const float* angles, float* forward, float* right,
                         float* up);
extern void AnglesSubtract(const math::Position3* v1,
                           const math::Position3* v2, math::Position3* v3);
extern float ServerTime_mTickDelta;
extern int BG_IsAimDownSightWeapon(int iWeapon);
extern float AngleSubtract(float a1, float a2);

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
extern int cg_gun_move_minspeed;
extern int cg_gun_move_f;
extern int cg_gun_move_r;
extern int cg_gun_move_u;
extern int cg_gun_move_rate;
extern int cg_gun_ofs_f;
extern int cg_gun_ofs_r;
extern int cg_gun_ofs_u;
extern int cg_gun_rot_minspeed;
extern int cg_gun_rot_y;
extern int cg_gun_rot_p;
extern int cg_gun_rot_r;
extern int cg_gun_rot_rate;
extern int cg_viewKickDeflectTime;
extern int cg_viewKickReturnTime;
extern float vehicleOffsetRate;
extern float vehicleOffset;
extern float GetLeanFraction(float fFrac);
extern void AnglesToRight(const float* const angles, float* const right);
extern void AnglesToAxis(const float* angles, float (*axis)[3]);
extern void AxisToAngles(const float (*axis)[3], float* angles);
extern float AngleNormalize360(float angle);
extern float AngleNormalize180(float angle);
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
extern float dword_F63C60[4 * 1580];
extern float dword_F63C64[4 * 1580];
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
extern float* unk_F63B30;
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
    AnglesSubtract((const math::Position3*)&angle[1580 * currCl],
                   (const math::Position3*)&dword_F63CC0[1580 * currCl],
                   (math::Position3*)v50);
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
extern int CG_PredictPlayerState_Internal();
extern int CG_PointContents(const math::Position3* point,
                            collision_context_t* context);
extern weaponFileInfo_t* BG_GetPlayerWeaponInfo();
extern int cg_fov;
extern int cg_widescreen;
extern float gZoomRatio;
extern float* gCamera;

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
int CG_PredictPlayerState()
{
    return CG_PredictPlayerState_Internal();
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
    float v5 = (v4 - 20.0f) / (*(float*)&cg_fov - 20.0f);
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
    if (cg_widescreen != 0)
    {
        aspectX = aspectX * 4.0f;
        aspectY = aspectY * 3.0f;
    }
    float v22 = aspectY / tanf(fov_y * 0.0087266462f);
    y = Atan2Approx(aspectX, v22) * 114.59155f;
    collision_context_t context;
    memset(&context, 0, sizeof(context));
    context.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
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
    if (dword_F63C60[1580 * currCl] <= 0 || *(float*)&cg_fov <= 0.0f)
    {
        gZoomRatio = 1.0f;
    }
    else
    {
        float v32 = dword_F63C60[1580 * currCl] / *(float*)&cg_fov;
        if (v32 < 1.0f)
            gZoomRatio = v32 * 0.75f;
        else
            gZoomRatio = 1.0f;
    }
    return 1580 * currCl * 4;
}

extern void CG_OffsetFirstPersonView();
extern void CG_ClampViewAngles(PlayerState* ps, const float* centerAngles,
                               const float* minClamp,
                               const float* maxClamp);
extern void AddLeanToPosition(float* const vPosition, float fViewYaw,
                              float fLeanFrac, float fViewRoll,
                              float fLeanDist);
extern void Com_DPrintf(const char* fmt, ...);
extern int G_DObjSetLocalTag(Entity* ent, int* partBits,
                             unsigned int tag_name_hash,
                             const float* trans, const float* angles,
                             bool relative);
extern float flrand(float min, float max);
extern void CG_CalcCubemapViewValues();
extern void CG_CalcVrect(const void* window);
extern void Camera_Update(void* self);
extern int cgGlobal_cubemapShot;
extern int bg_viewheight_prone;
extern int bg_viewheight_crouched;
extern int bg_viewheight_standing;
struct DObjSkelMat;
extern int G_DObjGetWorldTagMatrix(Entity* ent, unsigned int tag_name_hash,
                                   DObjSkelMat* tagMtx);
extern bool G_DObjGetWorldBoneIndexMatrix(Entity* ent, int boneIndex,
                                          DObjSkelMat* tagMtx);
struct vehicle_info_t;
extern vehicle_info_t* VEH_GetInfo(int idx);

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
                            v15 = bg_viewheight_prone + muzzle[2];
                        else if ((eFlags & 0x20) != 0)
                            v15 = bg_viewheight_crouched + muzzle[2];
                        else
                            v15 = bg_viewheight_standing + muzzle[2];
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

// ea: 0x006B05C0
void CG_CalcViewValues(const void* window)
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


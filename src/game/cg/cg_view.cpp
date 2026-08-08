// ============================================================================
// cg_view.cpp - view/fov/compass/weapon-position helpers (cg.o cg_view.cpp)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

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

extern Entity* EntityManager_GetPlayer(void* mgr, int idx);
extern void* EntityManager_sInst;
extern const char* CL_GetConfigStringC(int index);
extern float AngleNormalize360(float angle);
extern float AngleNormalize180(float angle);
extern float AngleSubtract(float a1, float a2);
extern void CL_SetViewAnglesAxis(int axis, float angle);
extern bool BG_AllowPlayerWeaponAtVehiclePos(int vehType, int vehPos);
extern int BG_IsAimDownSightWeapon(int iWeapon);
extern void* BG_GetPlayerWeaponInfo();
extern void CG_DrawGameScreenFade();
extern void trap_R_SetColor(const float* rgba);
extern void trap_R_DrawStretchPic(float x, float y, float w, float h, float s1,
                                  float t1, float s2, float t2, void* tex,
                                  float z);
extern void CG_FillRect(float x, float y, float width, float height,
                        float* color, float z);
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
        EntityManager_GetPlayer(EntityManager_sInst, currCl)->client;
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
    const char* ConfigString = CL_GetConfigStringC(11);
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

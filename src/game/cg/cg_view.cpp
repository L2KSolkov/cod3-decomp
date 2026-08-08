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

extern void VectorNormalize(float* v);
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
extern int cg_camerashake;
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
extern void AnglesToForward(const float* angles, float* forward);
extern int GamePause_IsGamePaused(int client);
extern int CG_UpdateCameraShake(void* shake, int client);
extern void CG_EndShellShock(const void* parms, int time);
extern void CG_UpdateShellShockSound(const void* parms);
extern void CG_UpdateShellShockMouse(const void* parms, int time, int duration);
extern void CG_UpdateShellShockCamera(const void* parms, int time,
                                      int duration);
extern void CL_SetUserCmdInShellshock(int shocked);
extern char* va(const char* fmt, ...);
extern int FS_FOpenFileByMode(const char* qpath, int* f, int mode);
extern int FS_Write(const void* buffer, int len, int h);
extern int FS_Read(void* buffer, int len, int f);
extern void FS_FCloseFile(int f);
extern int Com_SaveCvarsToBuffer(const char** cvarnames, int numCvars,
                                 char* buffer, int bufsize);
extern int Com_LoadCvarsFromBuffer(const char** cvarnames, int numCvars,
                                   const char* buffer, const char* filename);
extern void Cvar_Update(void* vmCvar);
extern void* _Z_MallocInternal(unsigned int size);
extern void _Z_FreeInternal(void* ptr);
extern void CG_Printf(const char* msg, ...);
extern void RumbleManager_Play(void* mgr, void* result, const void* effect,
                               float intensity);
extern void* RumbleManager_Inst(int instance);
extern void* RumbleManager_sInstHolder;
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
    FS_Write(filebuf, (int)strlen(filebuf), fh);
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
        FS_Read(v5, v3, fh);
        v5[v3] = 0;
        FS_FCloseFile(fh);
        int CvarsFromBuffer =
            Com_LoadCvarsFromBuffer(cg_shock_cvar_names, 26, v5, v1);
        _Z_FreeInternal(v5);
        for (unsigned int i = 0; i < 26; ++i)
            Cvar_Update(cg_shock_cvar_ptrs[i]);
        return CvarsFromBuffer;
    }
    CG_Printf("^1couldn't open '%s'\n", v1);
    return 0;
}

// ea: 0x00695BE0
void CG_StartShakeCamera(float p, int duration, const float* src, float radius,
                         int client)
{
    if (p > 0.0f && cg_camerashake != 0)
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
    if (!GamePause_IsGamePaused(currCl) && cg_camerashake != 0)
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
    void* v25 = RumbleManager_Inst(currCl);
    if (RumbleManager_sInstHolder != nullptr && v25 != nullptr)
    {
        float result;
        RumbleManager_Play(v25, &result, &rumbleEffect, 1.0f);
    }
}

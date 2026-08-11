// ============================================================================
// cg_misc.cpp - misc client game helpers: bars/rects, HUD state, camera (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"
#include "game/trace_types.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Minimal view of RumbleManager (full class in core/core_systems.h).
struct RumbleEffect;
class RumbleEffectInstanceHandle;
struct RumbleManager {
    static RumbleManager* Inst(int instance);  // ?Inst@RumbleManager@@SAPAV1@H@Z (g.o)
    RumbleEffectInstanceHandle Play(RumbleEffect* effect, float intensity);
};


extern const char* CL_GetConfigString(int index);  // ?CL_GetConfigString@@YAPBDH@Z (cl.o)


// Minimal view of InteractionController (full class in g_local.h).
class InteractionController {
public:
    static InteractionController* Inst(int instance);  // ?Inst@InteractionController@@SAPAV1@H@Z
};


// Minimal view of controller (full class in game/platform_xbox/XboxLiveMenus.h).
class controller { public:
    int locked_port;
    static controller* inst();  // ?inst@controller@@SAPAV1@XZ (controller_xbox.o)
};


// Minimal view of EntityManager (full class in game/sv/sv_stubs.h).
class EntityManager {
public:
    static EntityManager* sInst;  // ?sInst@EntityManager@@2PAV1@A (game.o)
    Entity* GetPlayer(int idx);   // ?GetPlayer@EntityManager@@QAEPAVEntity@@H@Z
    Entity* mPlayers[16];         // +0x04
};


extern int currCl;
extern float unk_F6A278[4 * 802];
extern float unk_F6A27C[4 * 802];
extern void* cgsGlobal_media_whiteShader;
struct FEManager; extern FEManager g_femanager;

extern void trap_R_SetColor(const float* rgba);
extern void trap_R_DrawStretchPic(float x, float y, float w, float h, float s1,
                                  float t1, float s2, float t2, void* tex,
                                  float z);
extern void CL_AddDebugLine(const float* start, const float* end,
                            const float* color, int depthTest, int duration,
                            int fromServer, int fadeOut);
extern void RE_AddRefEntityToScene(void* ent, int iCellNum);
extern void ByteToDir(unsigned int b, float* dir);
extern void PerpendicularVector(float* dst, float* src);
extern void CrossProduct(const float* v1, const float* v2, float* cross);

extern int controller_button_value(void* self, int i_controller_num,
                                   int i_button);
extern int controller_button_pressed(void* self, int i_controller_num,
                                     int i_button);
extern void* FEManager_GetIGMS(void* self, int client);
extern void InGameMenuSystem_ActivateMenu(void* self, int menu);

struct cgGlobal_t {
    int  time;       // +0x00
    int  teamGame;   // +0x04
};
extern cgGlobal_t cgGlobal;  // 0x00F5FE30

struct vmCvar_t {
    int   integer;  // +0x00
    float value;    // +0x04
};
extern vmCvar_t hud_healthOverlay_phaseEnd_toAlpha;  // 0x00F60258
extern vmCvar_t cg_hudAlpha;                          // 0x00F5FC80

extern int     dword_F641D0[4 * 1580];
extern int     dword_F641D4[4 * 1580];
extern int     dword_F641B0[4 * 1580];
extern int     dword_F641B8[4 * 1580];
extern int     dword_F641BC[4 * 1580];
extern int     dword_F641C8[4 * 1580];
extern int     dword_F641CC[4 * 1580];
extern unsigned char byte_F64194[4 * 6320];
extern unsigned char byte_F641C0[4 * 6320];
extern float   dword_F63C50[4 * 1580];
extern float   dword_F63C54[4 * 1580];
extern float   dword_F63C58[4 * 1580];
extern float   dword_F63C5C[4 * 1580];
extern int     dword_F62960[4 * 1580];
extern int     dword_F62964[4 * 1580];
extern float   dword_F63550[4 * 1580];
extern int     dword_F6355C[4 * 1580];

// ea: 0x00687BC0
void CG_ScoresDown_f()
{
    void* v0 = controller::inst();
    if (controller_button_value(v0, 0, 11 /* R3 */) <= 0)
    {
        if (dword_F641D0[1580 * currCl] == 0
            && byte_F64194[6320 * currCl] == 0)
        {
            dword_F641D0[1580 * currCl] = 1;
            void* IGMS = FEManager_GetIGMS(&g_femanager, currCl);
            InGameMenuSystem_ActivateMenu(IGMS, 10);
            dword_F641D4[1580 * currCl] = cgGlobal.time;
        }
    }
}

// ea: 0x00688790
void CG_ResetLowHealthOverlay(int client)
{
    int v1 = 1580 * client;
    dword_F641B0[v1] = *(int*)&hud_healthOverlay_phaseEnd_toAlpha.value;
    byte_F641C0[v1 * 4] = 0;
    dword_F641B8[v1] = 0;
    dword_F641BC[v1] = 0;
    dword_F641CC[v1] = 0;
    dword_F641C8[v1] = 1065353216;  // 1.0f
}

// ea: 0x00688880
void UpdateAmbientColourHelper()
{
}

// ea: 0x00688890
void CG_AdjustFrom640(float* x, float* y, float* w, float* h)
{
    *x = unk_F6A278[802 * currCl] * *x;
    *y = unk_F6A27C[802 * currCl] * *y;
    *w = unk_F6A278[802 * currCl] * *w;
    *h = unk_F6A27C[802 * currCl] * *h;
}

// ea: 0x00688910
void CG_AdjustFrom640_XYOnly(float* x, float* y)
{
    *x = unk_F6A278[802 * currCl] * *x;
    *y = unk_F6A27C[802 * currCl] * *y;
}

// ea: 0x00688960
void CG_GetCenterOfScreen(float* x, float* y)
{
    *x = (dword_F63C58[1580 * currCl] * 0.5f) + dword_F63C50[1580 * currCl]
         + *x;
    *y = (dword_F63C5C[1580 * currCl] * 0.5f) + dword_F63C54[1580 * currCl]
         + *y;
}

// ea: 0x006889D0
void CG_FillRect(float x, float y, float width, float height,
                 const float* color, float z)
{
    trap_R_SetColor(color);
    trap_R_DrawStretchPic(unk_F6A278[802 * currCl] * x,
                          unk_F6A27C[802 * currCl] * y,
                          unk_F6A278[802 * currCl] * width,
                          unk_F6A27C[802 * currCl] * height, 0.0f, 0.0f, 0.0f,
                          1.0f, cgsGlobal_media_whiteShader, z);
    trap_R_SetColor(nullptr);
}

// ea: 0x00688A60
void CG_FilledBar(float x, float y, float w, float h, float* startColorIn,
                  float* endColor, const float* bgColor, float frac,
                  short flags)
{
    float backgroundcolor[4] = {1.0f, 1.0f, 1.0f, 0.25f};
    float startColor[4] = {startColorIn[0], startColorIn[1], startColorIn[2],
                           startColorIn[3]};
    float colorAtPos[4];

    float v9 = 0.25f;
    if ((flags & 0x10) != 0 && bgColor != nullptr)
    {
        v9 = bgColor[3];
        backgroundcolor[0] = bgColor[0];
        backgroundcolor[1] = bgColor[1];
        backgroundcolor[2] = bgColor[2];
        backgroundcolor[3] = v9;
    }
    if ((flags & 8) == 0)
    {
        startColor[3] = cg_hudAlpha.value * startColor[3];
        if (endColor != nullptr)
            endColor[3] = endColor[3] * cg_hudAlpha.value;
        backgroundcolor[3] = cg_hudAlpha.value * v9;
    }
    float v15 = frac;
    if ((flags & 0x100) != 0)
    {
        colorAtPos[0] = (*endColor * frac) + ((1.0f - frac) * startColorIn[0]);
        colorAtPos[1] = (endColor[1] * frac)
                        + ((1.0f - frac) * startColorIn[1]);
        colorAtPos[2] = (endColor[2] * frac)
                        + ((1.0f - frac) * startColor[2]);
        colorAtPos[3] = (endColor[3] * frac)
                        + ((1.0f - frac) * startColor[3]);
    }

    float v21 = x, v18 = y, v20 = w, v19 = h;
    if ((flags & 0x10) != 0)
    {
        CG_FillRect(x, y, w, h, backgroundcolor, 0.0f);
        if ((flags & 0x40) == 0)
        {
            if ((flags & 0x20) != 0)
            {
                y += 6.0f;
                h -= 12.0f;
                v18 = y;
                v19 = h;
            }
            else
            {
                x += 2.0f;
                y += 2.0f;
                w -= 4.0f;
                h -= 4.0f;
                v21 = x;
                v18 = y;
                v20 = w;
                v19 = h;
            }
        }
    }

    if ((flags & 4) != 0)
    {
        if ((flags & 1) != 0)
            y = ((1.0f - v15) * v19) + v18;
        else if ((flags & 2) != 0)
            y = (((1.0f - v15) * v19) * 0.5f) + v18;
        float height = v19 * v15;
        if ((flags & 0x100) != 0)
            CG_FillRect(x, y, w, height, colorAtPos, 0.0f);
        else
            CG_FillRect(x, y, w, height, startColor, 0.0f);
    }
    else
    {
        if ((flags & 1) != 0)
            x = ((1.0f - v15) * v20) + v21;
        else if ((flags & 2) != 0)
            x = (((1.0f - v15) * v20) * 0.5f) + v21;
        float width = v20 * v15;
        if ((flags & 0x100) != 0)
            CG_FillRect(x, y, width, h, colorAtPos, 0.0f);
        else
            CG_FillRect(x, y, width, h, startColor, 0.0f);
    }
}

// ea: 0x00688D50
void CG_HorizontalPercentBar(float x, float y, float width, float height,
                             float percent)
{
    float color[4] = {0.5f, 0.5f, 0.5f, 0.30000001f};
    float v5[4] = {1.0f, 1.0f, 1.0f, 0.30000001f};
    CG_FillRect(x, y, width, height, color, 0.0f);
    CG_FillRect(x + 2.0f, y + 2.0f, (width - 4.0f) * percent, height - 4.0f,
                v5, 0.0f);
}

// ea: 0x00689680
void CG_DebugBox(const float* mins, const float* maxs, const float* color,
                 int depthTest, int duration)
{
    static const int edgePairs[24] = {
        0, 1, 0, 2, 0, 4, 1, 3, 1, 5, 2, 3,
        2, 6, 3, 7, 4, 5, 4, 6, 5, 7, 6, 7};
    float corners[8][3];
    for (int i = 0; i < 8; ++i)
    {
        corners[i][0] = (i & 1) ? maxs[0] : mins[0];
        corners[i][1] = (i & 2) ? maxs[1] : mins[1];
        corners[i][2] = (i & 4) ? maxs[2] : mins[2];
    }
    for (int i = 0; i < 12; ++i)
    {
        CL_AddDebugLine(corners[edgePairs[2 * i]], corners[edgePairs[2 * i + 1]],
                        color, depthTest, 0, 0, 0);
    }
}

// ea: 0x00689790
void CG_LockLightingOrigin(Entity* entity, refEntity_t* ent)
{
    if ((entity->s.eFlags & 0x8000) == 0)
    {
        if (entity->mRenderEntity != nullptr)
        {
            refEntity_t* ref = (refEntity_t*)&entity->GetRefEntity();
            ref->lightingOrigin[0] = 0.0f;
            ref->lightingOrigin[1] = 0.0f;
            ref->lightingOrigin[2] = 0.0f;
        }
    }
    else
    {
        refEntity_t* ref = (refEntity_t*)&entity->GetRefEntity();
        if (ref->lightingOrigin[0] == 0.0f && ref->lightingOrigin[1] == 0.0f
            && ref->lightingOrigin[2] == 0.0f)
        {
            ref->lightingOrigin[0] = entity->s.lerpOrigin.v.m128_f32[0];
            ref->lightingOrigin[1] = entity->s.lerpOrigin.v.m128_f32[1];
            ref->lightingOrigin[2] = entity->s.lerpOrigin.v.m128_f32[2];
        }
        ent->lightingOrigin[0] = ref->lightingOrigin[0];
        ent->lightingOrigin[1] = ref->lightingOrigin[1];
        ent->lightingOrigin[2] = ref->lightingOrigin[2];
        ent->renderfx |= 0x80;
    }
}

// ea: 0x0068A1B0
void CG_Portal(Entity* entity)
{
    refEntity_t* RefEntity = (refEntity_t*)&entity->GetRefEntity();
    RefEntity->origin[0] = entity->s.lerpOrigin.v.m128_f32[0];
    RefEntity->origin[1] = entity->s.lerpOrigin.v.m128_f32[1];
    RefEntity->origin[2] = entity->s.lerpOrigin.v.m128_f32[2];
    RefEntity->oldorigin[0] = entity->s.origin2.v.m128_f32[0];
    RefEntity->oldorigin[1] = entity->s.origin2.v.m128_f32[1];
    RefEntity->oldorigin[2] = entity->s.origin2.v.m128_f32[2];
    ByteToDir(entity->s.eventParm, RefEntity->axis[0]);
    PerpendicularVector(RefEntity->axis[1], RefEntity->axis[0]);
    RefEntity->axis[1][0] = 0.0f - RefEntity->axis[1][0];
    RefEntity->axis[1][1] = 0.0f - RefEntity->axis[1][1];
    RefEntity->axis[1][2] = 0.0f - RefEntity->axis[1][2];
    CrossProduct(RefEntity->axis[0], RefEntity->axis[1], RefEntity->axis[2]);
    RefEntity->reType = 11;  // RT_PORTALSURFACE
    RE_AddRefEntityToScene(RefEntity, -1);
}

// ea: 0x0068A2E0
int CG_SetFrameInterpolation()
{
    if (dword_F62964[1580 * currCl] == 0)
    {
        CG_ASSERT("cg[currCl].nextSnap", "c:\\cod\\code\\game\\cg_ent.cpp",
                  762);
    }
    int v1 = *(int*)(dword_F62960[1580 * currCl] + 4);
    int v2 = *(int*)(dword_F62964[1580 * currCl] + 4) - v1;
    if (v2 != 0)
    {
        float v3 = (float)(cgGlobal.time - v1) / (float)v2;
        dword_F63550[1580 * currCl] = v3;
        if (v3 < 0.0f)
            dword_F63550[1580 * currCl] = 0.0f;
    }
    else
    {
        dword_F63550[1580 * currCl] = 0.0f;
    }
    return 1580 * currCl * 4;
}

// ea: 0x0068A620
void CG_SoundBlend(Entity* entity)
{
}

extern const char* CG_SafeTranslateString_Internal(const char* pszReference,
                                                   const char* pszSystem);
extern const char* CG_ConfigString(unsigned int index);
extern const char* const defaultFileName;

struct game_hudelem_s {
    struct {
        int type;  // +0x00
    } elem;        // +0x00
    unsigned char _pad[0x7C - 0x04];
};
extern game_hudelem_s g_hudelems[16];  // 0x00EA5580

struct _objectiveInfo_t {
    int state;                    // +0x00
    float height;                 // +0x04
    int entity;                   // +0x08
    int state2;                   // +0x0C
    float vOrigin[3];             // +0x10
    int ringTime;                 // +0x1C
    int ring;                     // +0x20
    int displayOrder;             // +0x24
    _objectiveInfo_t* pChild;     // +0x28
    _objectiveInfo_t* pParent;    // +0x2C
    char szString[128];           // +0x30
};

struct localEntity_t {
    void*        next;  // +0x00
    void*        prev;  // +0x04
    unsigned char _pad[0x40 - 0x08];
    refEntity_t  refEntity;  // +0x40
};

extern float AngleNormalize180(float angle);
extern vmCvar_t cg_bobAmplitudeProne;     // 0x00F5ED68
extern vmCvar_t cg_bobAmplitudeDucked;    // 0x00F5B850
extern vmCvar_t cg_bobAmplitudeStanding;  // 0x00F5E978

// ea: 0x0068B270
int compare_hudelems(const void* pe0, const void* pe1)
{
    float v2 = *(float*)((char*)*(void* const*)pe0 + 108);
    float v3 = *(float*)((char*)*(void* const*)pe1 + 108);
    if (v2 == -777.0f)
        v2 = 0.0f;
    if (v3 == -777.0f)
        v3 = 0.0f;
    float v4 = v2 - v3;
    if (v4 >= 0.0f)
        return v4 > 0.0f;
    return -1;
}

// ea: 0x0068B300
void CG_ClearHudElems()
{
    for (int i = 0; i < 16; ++i)
        g_hudelems[i].elem.type = 0;  // HE_TYPE_FREE
}

// ea: 0x0068B380
void CG_AddFadeRGB(localEntity_t* le)
{
    RE_AddRefEntityToScene(&le->refEntity, -1);
}

// ea: 0x0068B890
const char* CG_SafeTranslateString(const char* pszReference)
{
    return CG_SafeTranslateString_Internal(pszReference, "cgame");
}

// ea: 0x0068B8B0
const char* CG_SafeTranslateHudElemString(int index)
{
    if (index == 0)
        return defaultFileName;
    return CG_SafeTranslateString_Internal(CG_ConfigString(index + 660),
                                           "hudelem");
}

// ea: 0x0068B8F0
int CG_ExportKeyBinding(const char* pszBinding, const char** ppszKey1,
                        const char** ppszKey2)
{
    return 1;
}

// ea: 0x0068BA60
int CountObjectiveChildren(_objectiveInfo_t* pObj)
{
    int result = 0;
    _objectiveInfo_t* pChild = pObj->pChild;
    for (; pChild != nullptr; ++result)
        pChild = pChild->pChild;
    return result;
}

// ea: 0x0068CAB0
void CG_ClampAngles(float* angles, const float* centerAngles,
                    const float* minClamp, const float* maxClamp)
{
    float viewAngles[3] = {angles[0], angles[1], angles[2]};
    for (int i = 0; i < 3; ++i)
    {
        if (minClamp[i] != 0.0f || maxClamp[i] != 0.0f)
        {
            float v18 = AngleNormalize180(viewAngles[i]);
            viewAngles[i] = v18;
            float angle = v18 - AngleNormalize180(centerAngles[i]);
            v18 = AngleNormalize180(angle);
            if (minClamp[i] != 0.0f)
            {
                if (minClamp[i] > v18)
                {
                    float v9 = minClamp[i] + 0.0099999998f;
                    angles[i] = AngleNormalize180(
                        AngleNormalize180(centerAngles[i]) + v9);
                    continue;
                }
            }
            float v10 = maxClamp[i];
            if (v10 != 0.0f && v18 > v10)
            {
                float v9 = v10 - 0.0099999998f;
                angles[i] = AngleNormalize180(
                    AngleNormalize180(centerAngles[i]) + v9);
            }
        }
    }
}

// ea: 0x0068CBD0
void CG_ClampAngles(math::Position3* angles, const float* centerAngles,
                    const float* minClamp, const float* maxClamp)
{
    float viewAngles[3] = {angles->v.m128_f32[0], angles->v.m128_f32[1],
                           angles->v.m128_f32[2]};
    for (int i = 0; i < 3; ++i)
    {
        if (minClamp[i] != 0.0f || maxClamp[i] != 0.0f)
        {
            float v18 = AngleNormalize180(viewAngles[i]);
            viewAngles[i] = v18;
            float angle = v18 - AngleNormalize180(centerAngles[i]);
            v18 = AngleNormalize180(angle);
            if (minClamp[i] != 0.0f)
            {
                if (minClamp[i] > v18)
                {
                    float v9 = minClamp[i] + 0.0099999998f;
                    angles->v.m128_f32[i] = AngleNormalize180(
                        AngleNormalize180(centerAngles[i]) + v9);
                    continue;
                }
            }
            float v10 = maxClamp[i];
            if (v10 != 0.0f && v18 > v10)
            {
                float v9 = v10 - 0.0099999998f;
                angles->v.m128_f32[i] = AngleNormalize180(
                    AngleNormalize180(centerAngles[i]) + v9);
            }
        }
    }
}

// ea: 0x0068D150
float CG_GetVerticalBobFactor(float fCycle, float fSpeed, float fMaxAmp)
{
    Client* client = EntityManager::sInst->GetPlayer( currCl)->client;
    int viewHeightTarget = client->ps.viewHeightTarget;
    float value;
    if (viewHeightTarget == client->ps.proneViewHeight)
        value = cg_bobAmplitudeProne.value;
    else
    {
        value = cg_bobAmplitudeDucked.value;
        if (viewHeightTarget != client->ps.crouchViewHeight)
            value = cg_bobAmplitudeStanding.value;
    }
    float fAmplitude = value * fSpeed;
    if (fAmplitude > fMaxAmp)
        fAmplitude = fMaxAmp;
    return (sinf(fCycle * 4.0f + 1.5707964f) * 0.2f
            + sinf(fCycle + fCycle))
           * fAmplitude * 0.75f;
}

// ea: 0x0068D1F0
float CG_GetHorizontalBobFactor(float fCycle, float fSpeed, float fMaxAmp)
{
    Client* client = EntityManager::sInst->GetPlayer( currCl)->client;
    int viewHeightTarget = client->ps.viewHeightTarget;
    float value;
    if (viewHeightTarget == client->ps.proneViewHeight)
        value = cg_bobAmplitudeProne.value;
    else
    {
        value = cg_bobAmplitudeDucked.value;
        if (viewHeightTarget != client->ps.crouchViewHeight)
            value = cg_bobAmplitudeStanding.value;
    }
    float fAmplitude = value * fSpeed;
    if (fAmplitude > fMaxAmp)
        fAmplitude = fMaxAmp;
    return sinf(fCycle) * fAmplitude;
}

// ea: 0x0068DB20
float SwayRand(float x, float y, float time)
{
    return cosf(y * time * 0.0062831859f) * sinf(x * time * 0.0062831859f);
}

// ============================================================================
// Camera (0x1F0 bytes; per-client, gCamera + 0x1F0 * client)
// ============================================================================

struct GlobalEffectNode {
    void* vftable;        // +0x00
    unsigned char _pad[0x0C - 0x04];
    struct {
        int on;      // +0x0C
        float r1;    // +0x10
        float r2;    // +0x14
        float r3;    // +0x18
        float r4;    // +0x1C
    } mDof;          // +0x0C
    float glowIntensity;  // +0x20
    int   ShockedClient;  // +0x24
    int   mClientIndex;   // +0x28
};

enum EVehicleCameraMode {
    VEH_MODE_FIRSTPERSON = 0,
    VEH_MODE_CHASECAM    = 1,
};

enum ECameraModes {
    CAM_NORMAL_FIRST = 0,
    CAM_NORMAL_THIRD = 1,
    CAM_VEHICLE_FIRST = 2,
    CAM_VEHICLE_THIRD = 3,
    CAM_VEHICLE_TANK = 4,
    CAM_VEHICLE_TANK_COMMANDER = 5,
    CAM_VEHICLE_GUNNER = 6,
    CAM_VEHICLE_GUNNER_CROUCHED = 7,  // CG_CalcGunnerViewPos(true)
    CAM_VEHICLE_GUNNER_STANDING = 8,  // CG_CalcGunnerViewPos(false)
    CAM_VEHICLE_PASSENGER = 9,        // CG_CalcPassengerViewPos
    CAM_VEHICLE_ANIM = 10,
    CAM_VEHICLE_ANIM_FIRST = 11,
    CAM_TURRET_FIRST = 12,            // first-person w/ turret sway
    CAM_TURRET = 13,
    CAM_INTERMISSION = 14,
    CAM_SCENE_ANIMATED = 15,
    CAM_INTERACTION_FREE = 16,
    CAM_INTERACTION_LOCKED = 17,
    CAM_TURRET_ANIMATED = 18,         // skips UpdateViewPO
    CAM_MP_DEATH_CAMERA = 19,
    CAM_MP_DEATH_CAMERA_NO_KILLER = 20,
    CAM_DEATH_CAMERA = 21,
};

// Camera::EVehInputState
enum {
    INPUT_NONE = 0,
    INPUT_STICK = 1,
    INPUT_LOOK_RIGHT = 2,
    INPUT_LOOK_LEFT = 3,
    INPUT_LOOK_BACK = 4,
};

class RumbleEffectInstanceHandle {
public:
    int mVal;  // +0x00
};

struct ServerTime_s {
    unsigned int mNumTicksElapsed;
    int mTickMSec;
    float mTickDelta;
    float mTickDeltaInv;
    float mElapsedTime;
};
extern ServerTime_s ServerTime_sInst;

extern void RumbleManager_Remove(void* self, RumbleEffectInstanceHandle handle);
extern float CG_GetViewFov();
extern int DObjGetBoneIndex(const DObj* obj, unsigned int boneNameHash);
extern void FastSinCos(float radians, float* psin, float* pcos);
extern int _fpclass(double x);
extern int dword_F6A2A0[4 * 802];
extern float angle[4 * 395];
extern float dword_F63C70[4 * 1580];
extern float dword_F63C74[4 * 1580];
extern float dword_F63C78[4 * 1580];
extern float dword_F63CB4[4 * 1580];
extern float dword_F63CB8[4 * 1580];
extern float dword_F63C80[4 * 1580];
extern float dword_F63C84[4 * 1580];
extern float dword_F63C88[4 * 1580];
extern int dword_F6A28C[4 * 802];
extern float dword_F641D8[4 * 1580];
extern float dword_F641DC[4 * 1580];
extern int dword_F64154[4 * 1580];
extern int dword_F64158[4 * 1580];
extern int dword_F6415C[4 * 1580];
extern int dword_F64160[4 * 1580];
extern int client;  // camera construction counter

static bool IsInvalidFloat(float v)
{
    // _fpclass mask 0x297: NaN, infinity and related non-finite classes
    return (_fpclass(v) & 0x297) != 0;
}

class Camera {
public:
    GlobalEffectNode mGlobalEffectNode;  // +0x00 (0x2C bytes)
    bool mDeathRumble;                   // +0x2C
    unsigned char _pad0[0x30 - 0x2D];
    math::Position3 mPrevViewPos;        // +0x30
    math::Position3 mPrevAngles;         // +0x40
    math::Position3 mPrevViewDir;        // +0x50
    float mPrevFOV;                      // +0x60
    unsigned char _pad1[0x70 - 0x64];
    math::Position3 mPrevAnimatedViewPos;    // +0x70
    math::Position3 mPrevAnimatedAngles;     // +0x80
    math::Position3 mVehPrevAngles;          // +0x90
    int mVehPrevAnglesTime;                  // +0xA0
    unsigned char _pad2[0xB0 - 0xA4];
    math::Position3 mVehPrevOrigin;          // +0xB0
    float mVehTimeSinceInput;                // +0xC0
    int mVehInputState;                      // +0xC4
    float mVehGasPressedTime;                // +0xC8
    float mSteerYawOffset;                   // +0xCC
    float mTankPrevious3rdFrac;              // +0xD0
    unsigned char _pad3[0xE0 - 0xD4];
    math::Position3 mTankRelativeAngles;     // +0xE0
    math::Position3 mTweenStartPos;          // +0xF0
    math::Position3 mTweenStartAngles;       // +0x100
    float mTweenStartFOV;                    // +0x110
    float mTweenTime;                        // +0x114
    float mTweenDuration;                    // +0x118
    unsigned short mTweenFlags;              // +0x11C
    unsigned char _pad4[0x120 - 0x11E];
    math::Position3 mTweenAnimatedStartPos;    // +0x120
    math::Position3 mTweenAnimatedStartAngles; // +0x130
    unsigned short mAnimFlags;                 // +0x140
    unsigned char _pad5[0x144 - 0x142];
    int mTagCameraIndex;                       // +0x144
    unsigned char _pad6[0x150 - 0x148];
    math::Mat43 mLastTagCamMat;                // +0x150
    int mCamMode;                              // +0x190
    int mVehicleCamMode;                       // +0x194
    unsigned char _pad7[0x1A0 - 0x198];
    math::Position3 mVehCamThirdAnglesOffset;  // +0x1A0
    math::Position3 mTweenParentPos;           // +0x1B0
    math::Position3 mTweenParentAngles;        // +0x1C0
    void* mShake;                              // +0x1D0
    int mRumbleEffect;                         // +0x1D4
    bool mDoingFadeOutIn;                      // +0x1D8
    unsigned char _pad8[0x1DC - 0x1D9];
    float mFadeTime;                           // +0x1DC
    int mClient;                               // +0x1E0
    unsigned char _pad9[0x1F0 - 0x1E4];

    Camera();

    float GetLastFOV();
    bool IsTweening();
    void StartCameraFade();
    void Restart();
    void StartTween(float tweenTime, bool anglesOnly);
    void StartAnimating(float minTweenTime);
    void StopAnimating(float minTweenTime);
    void UpdateAnimation();
    float UpdateFOV();
    void Update();
    void UpdatePostViewModels();

private:
    void SaveLastFOV();
    void SaveLastPO();
    void UpdateViewBob();
    void SetPlayerAngles(float* newAngles);
    void AdjustPlayerAngles(float* deltaAngles);
    void UpdateFade();
    void StopTween();
    void EndVehicleCam();
    void SetCameraTagIndex();
    void StartCircleTween(float tweenTime);
    void UpdateTween(math::Position3& tweenStartPos,
                     math::Position3& tweenStartAngles);
    void UpdateIntermissionCam();
    void UpdateSceneAnimCam();
    void UpdateReviveCam();
    void UpdateTankCam();
    void UpdateTankCamAngles(Entity* entity, PlayerState* ps);
    void UpdateTankCommanderCam();
    void UpdateMPDeathCamera();
    void UpdateMPDeathCameraNoKiller();
    void UpdateDeathCamera();
    void UpdateVehicleDriverCam(float fov);
    void UpdateVehicleDriverCamThird();
    void UpdateVehicleDriverCamAngles(Entity* entity, PlayerState* ps);
    void UpdateVehicleDriverCamAnglesInput(Entity* entity, PlayerState* ps);
    void UpdateVehicleDriverCamPos(Entity* entity, PlayerState* ps, float fov);
    void UpdateVehicleDriverSteerLookAhead(Entity* entity);
    void UpdateVehicleAnimCam();
    void UpdateViewPO();
    void BeginVehicleCam();
    math::Position3 GetVehicleViewAngles(Entity* entity, PlayerState* ps);
    void UpdateTankShakeRumble(Entity* entity, bool enable);
    float SetNewMode(ECameraModes newMode);
    ECameraModes CalcCamMode();
};

static unsigned int s_tagCameraHash;
static bool s_tagCameraHashInit;

// ea: 0x0069D9C0
Camera::Camera()
{
    int v1 = client;
    *(void**)this = (void*)0x00D0F648;  // &GlobalEffect::vftable
    mGlobalEffectNode.mClientIndex = v1;
    mGlobalEffectNode.glowIntensity = 0.0f;
    mGlobalEffectNode.ShockedClient = 0;
    mDeathRumble = false;
    memset(&mPrevViewPos, 0, sizeof(mPrevViewPos));
    memset(&mPrevAngles, 0, sizeof(mPrevAngles));
    mPrevFOV = 80.0f;
    memset(&mPrevAnimatedViewPos, 0, sizeof(mPrevAnimatedViewPos));
    memset(&mPrevAnimatedAngles, 0, sizeof(mPrevAnimatedAngles));
    memset(&mVehPrevAngles, 0, sizeof(mVehPrevAngles));
    mVehPrevAnglesTime = 0;
    memset(&mVehPrevOrigin, 0, sizeof(mVehPrevOrigin));
    mVehTimeSinceInput = 0.0f;
    mVehInputState = 0;  // INPUT_NONE
    mVehGasPressedTime = 0.0f;
    mSteerYawOffset = 0.0f;
    mTankPrevious3rdFrac = 0.0f;
    memset(&mTankRelativeAngles, 0, sizeof(mTankRelativeAngles));
    memset(&mTweenStartPos, 0, sizeof(mTweenStartPos));
    memset(&mTweenStartAngles, 0, sizeof(mTweenStartAngles));
    mTweenTime = 0.0f;
    mTweenDuration = 0.0f;
    mTweenFlags = 0;
    memset(&mTweenAnimatedStartPos, 0, sizeof(mTweenAnimatedStartPos));
    memset(&mTweenAnimatedStartAngles, 0, sizeof(mTweenAnimatedStartAngles));
    mAnimFlags = 0;
    mTagCameraIndex = -1;
    mCamMode = CAM_NORMAL_FIRST;
    mVehicleCamMode = VEH_MODE_FIRSTPERSON;
    mShake = nullptr;
    mRumbleEffect = 0;
    mClient = v1;
    mDoingFadeOutIn = false;
    client = v1 + 1;
}

// ea: 0x0068E750
void Camera::SaveLastFOV()
{
    if (mPrevViewPos.v.m128_f32[0] != 0.0f)
        mPrevFOV = CG_GetViewFov();
}

// ea: 0x0068E770
float Camera::GetLastFOV()
{
    return mPrevFOV;
}

// ea: 0x0068E780
void Camera::SaveLastPO()
{
    if (mPrevViewPos.v.m128_f32[0] == 0.0f)
    {
        mPrevViewPos.v.m128_f32[0] = 1.0f;
    }
    else
    {
        mPrevViewPos.v.m128_f32[0] = dword_F63C70[1580 * mClient];
        mPrevViewPos.v.m128_f32[1] = dword_F63C74[1580 * mClient];
        mPrevViewPos.v.m128_f32[2] = dword_F63C78[1580 * mClient];
        mPrevAngles.v.m128_f32[0] = angle[1580 * mClient];
        mPrevAngles.v.m128_f32[1] = dword_F63CB4[1580 * mClient];
        mPrevAngles.v.m128_f32[2] = dword_F63CB8[1580 * mClient];
        mPrevViewDir.v.m128_f32[0] = dword_F63C80[1580 * mClient];
        mPrevViewDir.v.m128_f32[1] = dword_F63C84[1580 * mClient];
        mPrevViewDir.v.m128_f32[2] = dword_F63C88[1580 * mClient];
    }
}

// ea: 0x0068EBB0
bool Camera::IsTweening()
{
    return mTweenDuration > mTweenTime;
}

// ea: 0x0068EBD0
void Camera::StopTween()
{
}

// ea: 0x0068EB70
void Camera::Restart()
{
    int mVal = mRumbleEffect;
    mCamMode = CAM_NORMAL_FIRST;
    mDeathRumble = false;
    if (mVal != 0)
    {
        RumbleEffectInstanceHandle v4 = {mVal};
        RumbleManager_Remove(RumbleManager::Inst(mClient), v4);
        mRumbleEffect = 0;
    }
}

// ea: 0x0068EA70
void Camera::StartCameraFade()
{
    mFadeTime = 0.5f;
    mDoingFadeOutIn = true;
    int time = cgGlobal.time;
    int v2 = 1580 * currCl;
    dword_F64154[v2] = 1065353216;  // 1.0f
    dword_F6415C[v2] = time - 10;
    dword_F64160[v2] = 1;
    if (dword_F6415C[v2] + 1 <= time)
        dword_F64158[v2] = dword_F64154[v2];
}

// ea: 0x0068EAE0
void Camera::UpdateFade()
{
    bool mDoingFadeOutIn = this->mDoingFadeOutIn;
    mFadeTime -= ServerTime_sInst.mTickDelta;
    if (mDoingFadeOutIn && mFadeTime <= 0.40000001f)
    {
        int time = cgGlobal.time;
        int v4 = 1580 * currCl;
        dword_F64154[v4] = 0;
        dword_F6415C[v4] = time - 1;
        dword_F64160[v4] = 400;
        if (dword_F6415C[v4] + 400 <= time)
            dword_F64158[v4] = dword_F64154[v4];
    }
    if (mFadeTime <= 0.0f)
        mDoingFadeOutIn = false;
}

// ea: 0x0068F040
void Camera::SetCameraTagIndex()
{
    if (mTagCameraIndex == -1)
    {
        const DObj* v2 = (const DObj*)dword_F6A2A0[802 * mClient];
        if (v2 != nullptr)
        {
            if (!s_tagCameraHashInit)
            {
                s_tagCameraHashInit = true;
                s_tagCameraHash = HashString::CalcHash("tag_camera");
            }
            mTagCameraIndex = DObjGetBoneIndex(v2, s_tagCameraHash);
        }
    }
}

// ea: 0x0068E730
bool IsVehicleCameraFadeMode(ECameraModes mode)
{
    return mode >= CAM_VEHICLE_FIRST && mode <= CAM_VEHICLE_TANK_COMMANDER;
}

// ea: 0x0068E840
void Camera::UpdateViewBob()
{
    Client* client = EntityManager::sInst->GetPlayer( mClient)->client;
    dword_F641D8[1580 * mClient] = (float)(client->ps.bobCycle & 0xFF)
                                       * 0.024639944f
                                   + 6.2831855f;
    if ((client->ps.pm_flags & 0x10) != 0)
    {
        if (cgGlobal.time - client->ps.jumpTime >= 500)
            *(int*)&dword_F641DC[1580 * mClient] =
                *(int*)&client->ps.velocity.v.m128_f32[2];
        else
            dword_F641DC[1580 * mClient] = 0.0f;
    }
    else
    {
        dword_F641DC[1580 * mClient] =
            sqrtf(client->ps.velocity.v.m128_f32[0]
                      * client->ps.velocity.v.m128_f32[0]
                  + client->ps.velocity.v.m128_f32[1]
                        * client->ps.velocity.v.m128_f32[1]);
    }
}

// ea: 0x0068E900
void Camera::SetPlayerAngles(float* newAngles)
{
    Client* client = EntityManager::sInst->GetPlayer( mClient)->client;
    client->ps.delta_angles[0] +=
        (int)((*newAngles - client->ps.viewangles[0]) * 182.04445f) & 0xFFFF;
    client->ps.delta_angles[1] +=
        (int)((newAngles[1] - client->ps.viewangles[1]) * 182.04445f) & 0xFFFF;
    client->ps.delta_angles[2] +=
        (int)((newAngles[2] - client->ps.viewangles[2]) * 182.04445f) & 0xFFFF;
    client->ps.viewangles[0] = newAngles[0];
    client->ps.viewangles[1] = newAngles[1];
    client->ps.viewangles[2] = newAngles[2];
}

// ea: 0x0068E9B0
void Camera::AdjustPlayerAngles(float* deltaAngles)
{
    Client* client = EntityManager::sInst->GetPlayer( mClient)->client;
    client->ps.delta_angles[0] += (int)(*deltaAngles * 182.04445f) & 0xFFFF;
    client->ps.delta_angles[1] +=
        (int)(deltaAngles[1] * 182.04445f) & 0xFFFF;
    client->ps.delta_angles[2] +=
        (int)(deltaAngles[2] * 182.04445f) & 0xFFFF;
    client->ps.viewangles[0] += *deltaAngles;
    client->ps.viewangles[1] += deltaAngles[1];
    client->ps.viewangles[2] += deltaAngles[2];
}

// ea: 0x00698FC0
float Camera::UpdateFOV()
{
    float fov = CG_GetViewFov();
    if (mTweenDuration > mTweenTime)
    {
        return ((((fov - mTweenStartFOV) / mTweenDuration) * mTweenTime)
                + mTweenStartFOV);
    }
    return fov;
}

// ea: 0x0069E580
void Camera::StartAnimating(float minTweenTime)
{
    if ((mAnimFlags & 1) == 0)
    {
        SetCameraTagIndex();
        if (mTagCameraIndex != -1)
        {
            mAnimFlags = (mAnimFlags | 1u) & ~4u;
            if (minTweenTime > 0.0f
                && minTweenTime > (mTweenDuration - mTweenTime))
                StartTween(minTweenTime, false);
        }
    }
}

// ea: 0x0069E5E0
void Camera::StopAnimating(float minTweenTime)
{
    if ((mAnimFlags & 1) != 0)
    {
        mAnimFlags &= 0xFAu;
        if (minTweenTime > 0.0f
            && minTweenTime > (mTweenDuration - mTweenTime))
            StartTween(minTweenTime, false);
    }
}

// ea: 0x0069DC00
void Camera::StartTween(float tweenTime, bool anglesOnly)
{
    if (IsInvalidFloat(mPrevAngles.v.m128_f32[0])
        || IsInvalidFloat(mPrevAngles.v.m128_f32[1])
        || IsInvalidFloat(mPrevAngles.v.m128_f32[2]))
    {
        CG_ASSERT("!IS_NAN((mPrevAngles)[0]) && !IS_NAN((mPrevAngles)[1]) "
                  "&& !IS_NAN((mPrevAngles)[2])",
                  "c:\\cod\\code\\game\\Camera.cpp", 1041);
    }
    if (IsInvalidFloat(mPrevViewPos.v.m128_f32[0])
        || IsInvalidFloat(mPrevViewPos.v.m128_f32[1])
        || IsInvalidFloat(mPrevViewPos.v.m128_f32[2]))
    {
        CG_ASSERT("!IS_NAN((mPrevViewPos)[0]) && !IS_NAN((mPrevViewPos)[1]) "
                  "&& !IS_NAN((mPrevViewPos)[2])",
                  "c:\\cod\\code\\game\\Camera.cpp", 1042);
    }
    mTweenStartAngles.v.m128_f32[0] = mPrevAngles.v.m128_f32[0];
    mTweenStartAngles.v.m128_f32[1] = mPrevAngles.v.m128_f32[1];
    mTweenStartAngles.v.m128_f32[2] = mPrevAngles.v.m128_f32[2];
    mTweenStartAngles.v.m128_f32[3] = mPrevAngles.v.m128_f32[3];
    mTweenStartPos.v.m128_f32[0] = mPrevViewPos.v.m128_f32[0];
    mTweenStartPos.v.m128_f32[1] = mPrevViewPos.v.m128_f32[1];
    mTweenStartPos.v.m128_f32[2] = mPrevViewPos.v.m128_f32[2];
    mTweenStartPos.v.m128_f32[3] = mPrevViewPos.v.m128_f32[3];
    mTweenStartFOV = mPrevFOV;
    mTweenDuration = tweenTime;
    mTweenTime = 0.0f;
    if (anglesOnly)
        mTweenFlags = (mTweenFlags | 1u) & ~2u;
    else
        mTweenFlags &= 0xFCu;
}

// ea: 0x0069DDA0
void Camera::StartCircleTween(float tweenTime)
{
    mTweenStartAngles.v.m128_f32[0] = mPrevAngles.v.m128_f32[0];
    mTweenStartAngles.v.m128_f32[1] = mPrevAngles.v.m128_f32[1];
    mTweenStartAngles.v.m128_f32[2] = mPrevAngles.v.m128_f32[2];
    mTweenStartAngles.v.m128_f32[3] = mPrevAngles.v.m128_f32[3];
    mTweenDuration = tweenTime;
    mTweenTime = 0.0f;
    mTweenStartFOV = mPrevFOV;

    float sinY, cosY, sinX, cosX;
    FastSinCos(mTweenStartAngles.v.m128_f32[1] * 0.017453292f, &sinY, &cosY);
    FastSinCos(mTweenStartAngles.v.m128_f32[0] * 0.017453292f, &sinX, &cosX);
    float dir[3] = {cosX * cosY, cosX * sinY, -sinX};
    mTweenStartPos.v = mPrevViewPos.v;
    mTweenStartPos.v = _mm_add_ps(
        mTweenStartPos.v,
        _mm_mul_ps(_mm_set1_ps(250.0f), _mm_loadu_ps(dir)));
    mTweenFlags = (mTweenFlags & ~1u) | 2u;

    if (IsInvalidFloat(mTweenStartPos.v.m128_f32[0])
        || IsInvalidFloat(mTweenStartPos.v.m128_f32[1])
        || IsInvalidFloat(mTweenStartPos.v.m128_f32[2]))
    {
        CG_ASSERT("!IS_NAN((mTweenStartPos)[0]) && !IS_NAN((mTweenStartPos)[1]) "
                  "&& !IS_NAN((mTweenStartPos)[2])",
                  "c:\\cod\\code\\game\\Camera.cpp", 1067);
    }
    if (IsInvalidFloat(mTweenStartAngles.v.m128_f32[0])
        || IsInvalidFloat(mTweenStartAngles.v.m128_f32[1])
        || IsInvalidFloat(mTweenStartAngles.v.m128_f32[2]))
    {
        CG_ASSERT("!IS_NAN((mTweenStartAngles)[0]) && "
                  "!IS_NAN((mTweenStartAngles)[1]) && "
                  "!IS_NAN((mTweenStartAngles)[2])",
                  "c:\\cod\\code\\game\\Camera.cpp", 1068);
    }
}

struct CameraShakeInstance {
    unsigned char _pad[0x3C];
    int m_active;  // +0x3C
};
struct CameraShake {
    unsigned char _data[0x14C];
};
extern CameraShake g_cameraShake[4];
extern CameraShakeInstance* CameraShake_StartCameraShake(
    CameraShake* self, int type, math::Position3* worldPos, float size,
    float timeOverride, float nextDelay);
extern void CameraShakeInstance_SetTime(CameraShakeInstance* self, float time);
extern void CameraShakeInstance_OverrideSettings(CameraShakeInstance* self,
                                                 float frequency,
                                                 float movement);
float rumbleFullIntensity;
extern void RumbleManager_SetIntensity(void* self, int handle,
                                       float intensity);
float tweenTime;  // 0x00DFA37C
struct DObjSkelMat;
extern int G_DObjGetWorldTagMatrix(Entity* ent, unsigned int tag_name_hash,
                                   DObjSkelMat* tagMtx);
extern void AnglesToAxis(const float* angles, float (*axis)[3]);
extern void AxisToAngles(const float (*axis)[3], float* angles);
extern float AngleDelta(float angle1, float angle2);

static unsigned int s_headHash;
static bool s_headHashInit;
static unsigned int s_spineHash;
static bool s_spineHashInit;

// ea: 0x0068EBE0
void Camera::EndVehicleCam()
{
    float newAngles[4] = {mPrevAngles.v.m128_f32[0],
                          mPrevAngles.v.m128_f32[1], 0.0f,
                          mPrevAngles.v.m128_f32[3]};
    SetPlayerAngles(newAngles);
}

// ea: 0x0068E180
void Camera::UpdateIntermissionCam()
{
    Client* client = EntityManager::sInst->GetPlayer( mClient)->client;
    dword_F63C70[1580 * mClient] = client->ps.origin.v.m128_f32[0];
    dword_F63C74[1580 * mClient] = client->ps.origin.v.m128_f32[1];
    dword_F63C78[1580 * mClient] = client->ps.origin.v.m128_f32[2];
    angle[1580 * mClient] = client->ps.viewangles[0];
    dword_F63CB4[1580 * mClient] = client->ps.viewangles[1];
    dword_F63CB8[1580 * mClient] = client->ps.viewangles[2];
}

// ea: 0x006AE710
void Camera::UpdateViewPO()
{
    UpdateTween(mTweenStartPos, mTweenStartAngles);
    AnglesToAxis((const float*)&angle[1580 * mClient],
                 (float(*)[3])&dword_F63C80[1580 * mClient]);
    SaveLastPO();
}

// ea: 0x0068EFC0
math::Position3 Camera::GetVehicleViewAngles(Entity* veh, PlayerState* ps)
{
    math::Position3 result;
    result.v = _mm_setzero_ps();
    if (ps->vehSubType == 2 && veh->scr_vehicle != nullptr
        && *(void**)((char*)veh->scr_vehicle + 0x518) != nullptr)
    {
        result.v.m128_f32[0] = 7.0f;
    }
    return result;
}

// ea: 0x006A6750
void Camera::BeginVehicleCam()
{
    Client* client = EntityManager::sInst->GetPlayer( mClient)->client;
    unsigned int v4 = client->ps.mViewLockedEntity & 0xFFF;
    Entity* mObject = nullptr;
    if (v4 < 0x540
        && (client->ps.mViewLockedEntity >> 12)
               == EntityHandleDb::sInst.mElements[v4].mKey)
    {
        mObject = EntityHandleDb::sInst.mElements[v4].mObject;
    }
    math::Position3 v15 = GetVehicleViewAngles(mObject, &client->ps);
    SetPlayerAngles(v15.v.m128_f32);
    mVehPrevAngles.v = mObject->r.currentAngles.v;
    mVehPrevOrigin.v = mObject->r.currentOrigin.v;
    bool mDoingFadeOutIn = this->mDoingFadeOutIn;
    mVehPrevAnglesTime = cgGlobal.time;
    mVehInputState = 0;  // INPUT_NONE
    mSteerYawOffset = 0.0f;
    if (mDoingFadeOutIn)
    {
        mPrevViewPos.v = _mm_add_ps(
            mObject->r.currentOrigin.v,
            _mm_mul_ps(mObject->r.currentMat.x.v, _mm_set1_ps(-300.0f)));
        mPrevAngles.v = mVehPrevAngles.v;
    }
}

// ea: 0x0069E2C0
void Camera::UpdateReviveCam()
{
    StartTween(tweenTime, false);
    if (!s_headHashInit)
    {
        s_headHashInit = true;
        s_headHash = HashString::CalcHash("bip01 head");
    }
    if (!s_spineHashInit)
    {
        s_spineHashInit = true;
        s_spineHash = HashString::CalcHash("bip01 spine1");
    }
    float tagMtx[31];
    G_DObjGetWorldTagMatrix(EntityManager::sInst->GetPlayer(
                                                    mClient),
                            s_headHash, (DObjSkelMat*)tagMtx);
    float angTagMtx[31];
    G_DObjGetWorldTagMatrix(EntityManager::sInst->GetPlayer(
                                                    mClient),
                            s_spineHash, (DObjSkelMat*)angTagMtx);
    dword_F63C70[1580 * mClient] = tagMtx[12];
    dword_F63C74[1580 * mClient] = tagMtx[13];
    dword_F63C78[1580 * mClient] = tagMtx[14];
    float axis[3][3] = {{angTagMtx[4], angTagMtx[5], angTagMtx[6]},
                        {angTagMtx[0], angTagMtx[1], angTagMtx[2]},
                        {angTagMtx[8], angTagMtx[9], angTagMtx[10]}};
    float angles[3];
    AxisToAngles(axis, angles);
    angle[1580 * mClient] = angles[0];
    dword_F63CB4[1580 * mClient] = angles[1];
    dword_F63CB8[1580 * mClient] = angles[2];
    float rotScale[3] = {0.3f, 1.0f, 0.1f};
    for (int i = 0; i < 3; ++i)
    {
        dword_F63C70[1580 * mClient + i] -= angTagMtx[i + 4] * 10.0f;
        float v6;
        if (rotScale[i] >= 1.0f)
            v6 = AngleNormalize180(angle[1580 * mClient + i]);
        else
            v6 = AngleNormalize180(angle[1580 * mClient + i]) * rotScale[i];
        angle[1580 * mClient + i] = AngleNormalize180(v6);
    }
}

// ea: 0x0068F0E0
void Camera::UpdateTankShakeRumble(Entity* veh, bool firstPerson)
{
    CameraShakeInstance* mShake = (CameraShakeInstance*)this->mShake;
    if (mShake == nullptr || mShake->m_active == 0)
    {
        this->mShake = CameraShake_StartCameraShake(
            &g_cameraShake[mClient], 11, nullptr, 1.0f, 1.0f, -1.0f);
    }
    CameraShakeInstance* v5 = (CameraShakeInstance*)this->mShake;
    if (v5 != nullptr)
    {
        CameraShakeInstance_SetTime(v5, 0.1f);
        float speed =
            fabsf(*(float*)((char*)veh->scr_vehicle + 0x118) * 180.0f
                  * 0.31830987f * 2.5f)
            + veh->speed;
        if (speed >= 220.0f)
            speed = 1.0f;
        else
            speed = speed * 0.0045454544f;
        float movement = speed * 1.5f;
        if (!firstPerson)
            movement = movement * 0.30000001f;
        CameraShakeInstance_OverrideSettings(
            (CameraShakeInstance*)this->mShake, 5.0f, movement);
        float v9 = rumbleFullIntensity * speed;
        float rumbleIntensity;
        if (v9 >= 0.0f)
        {
            if (v9 > 1.0f)
                rumbleIntensity = 1.0f;
            else
            {
                rumbleIntensity = v9;
                if (v9 >= 0.1f)
                    goto set_intensity;
            }
        }
        rumbleIntensity = 0.0f;
    set_intensity:
        RumbleManager_SetIntensity(RumbleManager::Inst(mClient), mRumbleEffect,
                                   rumbleIntensity);
    }
}

struct vehicle_info_t {
    unsigned char _pad0[0x16C];
    float turretVertSpanUp;      // +0x16C
    float turretVertSpanDown;    // +0x170
    unsigned char _pad1[0x1E8 - 0x174];
    float camLinkedPitchFactor;  // +0x1E8
    float pitchBasedCamOffsetX;  // +0x1EC
    float pitchBasedCamOffsetZ;  // +0x1F0
};

float maxAngle;  // 0x00DFA294
bool special_tween_bool;  // 0x00F6171D
bool gCameraSwayOnTurrets;  // 0x00DF9D84
extern void controller_stick_value(void* self, int index, int stick,
                                   int* outX, int* outY);
extern int RecalibrateInput(int val);
extern math::Dir3 rb_vehicle_get_velocity(void* self);
extern PlayerState& GetPlayerState(int idx);
extern void CG_OffsetFirstPersonView();
extern void CG_OffsetThirdPersonView();
extern void CG_CalcGunnerViewPos(bool crouched, unsigned int tag_hash);
extern int CG_CalcPassengerViewPos();
extern void CG_CalcTurretViewValues();
struct scr_vehicle_t;
extern vehicle_info_t* G_GetVehicleInfo(scr_vehicle_t* scr_vehicle);
extern void vectosignedangles(float* vec, float* angles);
extern void vectoangles(float* vec, float* angles);
extern float LerpAngle(float a1, float a2, float a3);
extern void InterpolateAngles(math::Position3* curAngles,
                              const math::Position3* initialAngles,
                              const math::Position3* targetAngles, float t);
extern void AnglesToForward(const math::Position3& angles,
                            math::Dir3& forward);
extern Entity* GetPlayer(int idx);

static unsigned int s_tagGunnerBarrelHash;
static bool s_tagGunnerBarrelHashInit;
static unsigned int s_tagBarrelHash;
static bool s_tagBarrelHashInit;

static Entity* DbHandleToEntity(unsigned int handle)
{
    unsigned int idx = handle & 0xFFF;
    if (idx < 0x540
        && (handle >> 12) == EntityHandleDb::sInst.mElements[idx].mKey)
        return EntityHandleDb::sInst.mElements[idx].mObject;
    return nullptr;
}

// ea: 0x0068EC30
void Camera::UpdateVehicleDriverSteerLookAhead(Entity* veh)
{
    float v2 = 10.0f;
    float m_steer_factor = 0.0f;
    float steerFactor = 0.0f;
    if (veh->speed > 50.0f)
    {
        void* mRBVeh = *(void**)((char*)veh->scr_vehicle + 0x518);
        m_steer_factor = *(float*)((char*)mRBVeh + 0x264);
        steerFactor = m_steer_factor;
    }
    if (fabsf(steerFactor) < 0.80000001f)
        goto relax;
    if (m_steer_factor > 0.0f)
    {
        if (mSteerYawOffset < 0.0f)
            v2 = 60.0f;
        mSteerYawOffset =
            ((ServerTime_sInst.mTickDelta * m_steer_factor) * v2)
            + mSteerYawOffset;
        goto clamp;
    }
    if (m_steer_factor >= 0.0f)
    {
    relax:
        float cur = mSteerYawOffset;
        float v6;
        bool v7;
        if (cur <= 0.0f)
        {
            if (mSteerYawOffset >= 0.0f)
                goto clamp;
            v6 = (ServerTime_sInst.mTickDelta * 40.0f) + mSteerYawOffset;
            v7 = v6 <= 0.0f;
        }
        else
        {
            v6 = cur - (ServerTime_sInst.mTickDelta * 40.0f);
            v7 = v6 >= 0.0f;
        }
        mSteerYawOffset = v6;
        if (!v7)
            mSteerYawOffset = 0.0f;
    }
    else
    {
        if (mSteerYawOffset > 0.0f)
            v2 = 60.0f;
        mSteerYawOffset = mSteerYawOffset
                          - ((fabsf(steerFactor) * ServerTime_sInst.mTickDelta)
                             * v2);
    }
clamp:
    float v8 = 0.0f - maxAngle;
    if (v8 <= mSteerYawOffset)
    {
        if (mSteerYawOffset > maxAngle)
        {
            mSteerYawOffset = maxAngle;
            return;
        }
        v8 = mSteerYawOffset;
    }
    mSteerYawOffset = v8;
}

// ea: 0x0069DFE0
void Camera::UpdateVehicleDriverCamAnglesInput(Entity* veh, PlayerState* ps)
{
    mVehTimeSinceInput = ServerTime_sInst.mTickDelta + mVehTimeSinceInput;
    int v5 = dword_F6A28C[802 * currCl];
    int prevVehInputState;
    int stickY;
    controller_stick_value(controller::inst(), v5, 1 /* RIGHTSTICK */,
                           &prevVehInputState, &stickY);
    prevVehInputState = RecalibrateInput(prevVehInputState);
    stickY = RecalibrateInput(stickY);
    int prevState = mVehInputState;
    void* mRBVeh = *(void**)((char*)veh->scr_vehicle + 0x518);
    if (mRBVeh != nullptr)
    {
        bool v27 = prevVehInputState != INPUT_NONE || stickY != 0;
        bool v28 = *(float*)((char*)mRBVeh + 0x254) > 0.0f;  // m_throttle
        math::Dir3 velocity = rb_vehicle_get_velocity(mRBVeh);
        float v24 = velocity.v.m128_f32[0] * velocity.v.m128_f32[0]
                    + velocity.v.m128_f32[1] * velocity.v.m128_f32[1]
                    + velocity.v.m128_f32[2] * velocity.v.m128_f32[2];
        if (v28)
            mVehGasPressedTime = mVehGasPressedTime
                                 + ServerTime_sInst.mTickDelta;
        else
            mVehGasPressedTime = 0.0f;
        if (v27)
        {
            mVehTimeSinceInput = 0.0f;
            mVehInputState = INPUT_STICK;
        }
        else if (mVehTimeSinceInput >= 0.30000001f)
        {
            if (controller_button_value(controller::inst(), v5, 7 /* R2 */) != 0
                && controller_button_value(controller::inst(), v5, 6 /* L2 */)
                       != 0)
                mVehInputState = INPUT_LOOK_BACK;
            else if (controller_button_value(controller::inst(), v5, 7) != 0)
                mVehInputState = INPUT_LOOK_RIGHT;
            else if (controller_button_value(controller::inst(), v5, 6) != 0)
                mVehInputState = INPUT_LOOK_LEFT;
            else if (mVehGasPressedTime > 0.30000001f || v24 > 250000.0f
                     || mVehInputState != INPUT_STICK)
                mVehInputState = INPUT_NONE;
        }
        else
        {
            mVehInputState = INPUT_STICK;
        }
        int v19 = mVehInputState;
        if ((v19 != INPUT_STICK || mTweenDuration <= mTweenTime)
            && prevState != v19)
        {
            if (v19 != INPUT_STICK)
            {
                if (mVehicleCamMode != VEH_MODE_FIRSTPERSON)
                    StartCircleTween(0.60000002f);
                else
                    StartTween(0.40000001f, true);
            }
            math::Position3 v21 = GetVehicleViewAngles(veh, ps);
            float v20;
            switch (mVehInputState)
            {
            case INPUT_STICK:
                v20 = mPrevAngles.v.m128_f32[1]
                      - veh->r.currentAngles.v.m128_f32[1];
                break;
            case INPUT_LOOK_RIGHT:
                v20 = v21.v.m128_f32[1] - 90.0f;
                break;
            case INPUT_LOOK_LEFT:
                v20 = v21.v.m128_f32[1] + 90.0f;
                break;
            case INPUT_LOOK_BACK:
                v20 = v21.v.m128_f32[1] + 180.0f;
                break;
            default:
                break;
            }
            v21.v.m128_f32[1] = v20;
            SetPlayerAngles(v21.v.m128_f32);
        }
    }
}

// ea: 0x006AEF30
void Camera::UpdateVehicleDriverCam(float extra_height_offset)
{
    Client* client = EntityManager::sInst->GetPlayer( mClient)->client;
    Entity* mObject = DbHandleToEntity(client->ps.mViewLockedEntity);
    UpdateVehicleDriverCamAngles(mObject, &client->ps);
    UpdateVehicleDriverCamAnglesInput(mObject, &client->ps);
    if (special_tween_bool)
    {
        if (mTweenTime < 0.69999999f)
        {
            UpdateVehicleDriverCamPos(mObject, &client->ps, 20.0f);
            return;
        }
        StartTween(0.30000001f, false);
        special_tween_bool = false;
    }
    UpdateVehicleDriverCamPos(mObject, &client->ps, extra_height_offset);
}

// ea: 0x006AF640
void Camera::Update()
{
    if (mPrevViewPos.v.m128_f32[0] != 0.0f)
        mPrevFOV = CG_GetViewFov();
    memset(&dword_F63C50[1580 * mClient], 0, 0x60u);
    ECameraModes v2 = CalcCamMode();
    ECameraModes newMode = v2;
    float adjX = SetNewMode(v2);
    if (IsInvalidFloat(mTweenStartPos.v.m128_f32[0])
        || IsInvalidFloat(mTweenStartPos.v.m128_f32[1])
        || IsInvalidFloat(mTweenStartPos.v.m128_f32[2]))
    {
        CG_ASSERT("!IS_NAN((mTweenStartPos)[0]) && !IS_NAN((mTweenStartPos)[1]) "
                  "&& !IS_NAN((mTweenStartPos)[2])",
                  "c:\\cod\\code\\game\\Camera.cpp", 96);
    }
    if (IsInvalidFloat(mTweenStartAngles.v.m128_f32[0])
        || IsInvalidFloat(mTweenStartAngles.v.m128_f32[1])
        || IsInvalidFloat(mTweenStartAngles.v.m128_f32[2]))
    {
        CG_ASSERT("!IS_NAN((mTweenStartAngles)[0]) && "
                  "!IS_NAN((mTweenStartAngles)[1]) && "
                  "!IS_NAN((mTweenStartAngles)[2])",
                  "c:\\cod\\code\\game\\Camera.cpp", 97);
    }
    if (adjX > 0.001f)
    {
        StartTween(adjX, false);
        if (v2 == CAM_VEHICLE_THIRD)
        {
            mVehCamThirdAnglesOffset.v.m128_f32[1] =
                EntityManager::sInst->GetPlayer( mClient)
                    ->client->ps.viewangles[1];
        }
    }
    if (!s_tagGunnerBarrelHashInit)
    {
        s_tagGunnerBarrelHashInit = true;
        s_tagGunnerBarrelHash = HashString::CalcHash("tag_gunner_barrel");
    }
    if (!s_tagBarrelHashInit)
    {
        s_tagBarrelHashInit = true;
        s_tagBarrelHash = HashString::CalcHash("tag_barrel");
    }
    unsigned int v3 = s_tagGunnerBarrelHash;
    switch (newMode)
    {
    case 0:
        UpdateViewBob();
        {
            Client* client = GetPlayer(mClient)->client;
            dword_F63C70[1580 * mClient] = client->ps.origin.v.m128_f32[0];
            dword_F63C74[1580 * mClient] = client->ps.origin.v.m128_f32[1];
            dword_F63C78[1580 * mClient] = client->ps.origin.v.m128_f32[2];
            angle[1580 * mClient] = client->ps.viewangles[0];
            dword_F63CB4[1580 * mClient] = client->ps.viewangles[1];
            dword_F63CB8[1580 * mClient] = client->ps.viewangles[2];
        }
        CG_OffsetFirstPersonView();
        break;
    case 1:
        {
            Client* v6 = GetPlayer(mClient)->client;
            dword_F63C70[1580 * mClient] = v6->ps.origin.v.m128_f32[0];
            dword_F63C74[1580 * mClient] = v6->ps.origin.v.m128_f32[1];
            dword_F63C78[1580 * mClient] = v6->ps.origin.v.m128_f32[2];
            angle[1580 * mClient] = v6->ps.viewangles[0];
            dword_F63CB4[1580 * mClient] = v6->ps.viewangles[1];
            dword_F63CB8[1580 * mClient] = v6->ps.viewangles[2];
        }
        CG_OffsetThirdPersonView();
        break;
    case 2:
        UpdateVehicleDriverCam(0.0f);
        break;
    case 3:
        UpdateVehicleDriverCamThird();
        break;
    case 4:
        UpdateTankCam();
        break;
    case 5:
        UpdateTankCommanderCam();
        break;
    case 6:
        CG_CalcGunnerViewPos(false, v3);
        break;
    case 7:
        CG_CalcGunnerViewPos(true, v3);
        break;
    case 8:
        CG_CalcGunnerViewPos(false, s_tagBarrelHash);
        break;
    case 9:
        CG_CalcPassengerViewPos();
        break;
    case 10:
        UpdateVehicleAnimCam();
        break;
    case 11:
        {
            Client* v4 = GetPlayer(mClient)->client;
            dword_F63C70[1580 * mClient] =
                GetPlayer(mClient)->r.currentOrigin.v.m128_f32[0];
            dword_F63C74[1580 * mClient] =
                GetPlayer(mClient)->r.currentOrigin.v.m128_f32[1];
            dword_F63C78[1580 * mClient] =
                GetPlayer(mClient)->r.currentOrigin.v.m128_f32[2];
            dword_F63C78[1580 * mClient] += v4->ps.viewHeightCurrent;
            angle[1580 * mClient] = mPrevAngles.v.m128_f32[0];
            dword_F63CB4[1580 * mClient] = mPrevAngles.v.m128_f32[1];
            dword_F63CB8[1580 * mClient] = mPrevAngles.v.m128_f32[2];
        }
        break;
    case 12:
        {
            Client* v8 = GetPlayer(mClient)->client;
            if ((gCameraSwayOnTurrets && v8->ps.vehType != 5
                 && GetPlayer(mClient)->client->ps.vehPos == 1
                 && (0x100000 & GetPlayerState(mClient).eFlags) != 0)
                || (GetPlayerState(mClient).eFlags & 0x6000) != 0)
            {
                angle[1580 * mClient] = v8->ps.viewangles[0];
                dword_F63CB4[1580 * mClient] = v8->ps.viewangles[1];
                dword_F63CB8[1580 * mClient] = v8->ps.viewangles[2];
                CG_CalcGunnerViewPos(false, v3);
                CG_OffsetFirstPersonView();
                if (IsInvalidFloat(v8->ps.origin.v.m128_f32[0])
                    || IsInvalidFloat(v8->ps.origin.v.m128_f32[1])
                    || IsInvalidFloat(v8->ps.origin.v.m128_f32[2]))
                {
                    CG_ASSERT("!IS_NAN((ps->origin)[0]) && "
                              "!IS_NAN((ps->origin)[1]) && "
                              "!IS_NAN((ps->origin)[2])",
                              "c:\\cod\\code\\game\\Camera.cpp", 198);
                }
                if (IsInvalidFloat(v8->ps.viewangles[0])
                    || IsInvalidFloat(v8->ps.viewangles[1])
                    || IsInvalidFloat(v8->ps.viewangles[2]))
                {
                    CG_ASSERT("!IS_NAN((ps->viewangles)[0]) && "
                              "!IS_NAN((ps->viewangles)[1]) && "
                              "!IS_NAN((ps->viewangles)[2])",
                              "c:\\cod\\code\\game\\Camera.cpp", 199);
                }
                if (IsInvalidFloat(dword_F63C70[1580 * mClient])
                    || IsInvalidFloat(dword_F63C74[1580 * mClient])
                    || IsInvalidFloat(dword_F63C78[1580 * mClient]))
                {
                    CG_ASSERT("!IS_NAN((cg[mClient].refdef.vieworg)[0]) && "
                              "!IS_NAN((cg[mClient].refdef.vieworg)[1]) && "
                              "!IS_NAN((cg[mClient].refdef.vieworg)[2])",
                              "c:\\cod\\code\\game\\Camera.cpp", 200);
                }
                if (IsInvalidFloat(angle[1580 * mClient])
                    || IsInvalidFloat(dword_F63CB4[1580 * mClient])
                    || IsInvalidFloat(dword_F63CB8[1580 * mClient]))
                {
                    CG_ASSERT("!IS_NAN((cg[mClient].refdefViewAngles)[0]) && "
                              "!IS_NAN((cg[mClient].refdefViewAngles)[1]) && "
                              "!IS_NAN((cg[mClient].refdefViewAngles)[2])",
                              "c:\\cod\\code\\game\\Camera.cpp", 201);
                }
            }
            else
            {
                if (IsInvalidFloat(v8->ps.origin.v.m128_f32[0])
                    || IsInvalidFloat(v8->ps.origin.v.m128_f32[1])
                    || IsInvalidFloat(v8->ps.origin.v.m128_f32[2]))
                {
                    CG_ASSERT("!IS_NAN((ps->origin)[0]) && "
                              "!IS_NAN((ps->origin)[1]) && "
                              "!IS_NAN((ps->origin)[2])",
                              "c:\\cod\\code\\game\\Camera.cpp", 205);
                }
                if (IsInvalidFloat(v8->ps.viewangles[0])
                    || IsInvalidFloat(v8->ps.viewangles[1])
                    || IsInvalidFloat(v8->ps.viewangles[2]))
                {
                    CG_ASSERT("!IS_NAN((ps->viewangles)[0]) && "
                              "!IS_NAN((ps->viewangles)[1]) && "
                              "!IS_NAN((ps->viewangles)[2])",
                              "c:\\cod\\code\\game\\Camera.cpp", 206);
                }
                dword_F63C70[1580 * mClient] = v8->ps.origin.v.m128_f32[0];
                dword_F63C74[1580 * mClient] = v8->ps.origin.v.m128_f32[1];
                dword_F63C78[1580 * mClient] = v8->ps.origin.v.m128_f32[2];
                angle[1580 * mClient] = v8->ps.viewangles[0];
                dword_F63CB4[1580 * mClient] = v8->ps.viewangles[1];
                dword_F63CB8[1580 * mClient] = v8->ps.viewangles[2];
                if ((0x100000 & v8->ps.eFlags) != 0)
                {
                    Entity* v9 = DbHandleToEntity(v8->ps.mViewLockedEntity);
                    if (v9 != nullptr)
                    {
                        void* scr_vehicle = v9->scr_vehicle;
                        if (scr_vehicle != nullptr)
                        {
                            vehicle_info_t* VehicleInfo =
                                G_GetVehicleInfo(
                                    (scr_vehicle_t*)scr_vehicle);
                            angle[1580 * mClient] =
                                VehicleInfo->camLinkedPitchFactor
                                * angle[1580 * mClient];
                            if (VehicleInfo->pitchBasedCamOffsetX > 0.001f
                                || VehicleInfo->pitchBasedCamOffsetZ > 0.001f)
                            {
                                float v12 = 0.0f;
                                float ratio =
                                    (VehicleInfo->turretVertSpanUp
                                     + *(float*)((char*)scr_vehicle + 0x430))
                                    / (VehicleInfo->turretVertSpanUp
                                       + VehicleInfo->turretVertSpanDown);
                                if (ratio >= 0.0f)
                                {
                                    v12 = 1.0f;
                                    if (ratio <= 1.0f)
                                        v12 = ratio;
                                }
                                float adjX_ = v12
                                              * VehicleInfo
                                                    ->pitchBasedCamOffsetX;
                                float adjZ = VehicleInfo->pitchBasedCamOffsetZ
                                             * v12;
                                float viewAxes[9];
                                AnglesToAxis(
                                    (const float*)&angle[1580 * mClient],
                                    (float(*)[3])viewAxes);
                                dword_F63C70[1580 * mClient] +=
                                    viewAxes[0] * adjX_;
                                dword_F63C74[1580 * mClient] +=
                                    viewAxes[1] * adjX_;
                                dword_F63C78[1580 * mClient] +=
                                    viewAxes[2] * adjX_;
                                dword_F63C70[1580 * mClient] +=
                                    viewAxes[6] * adjZ;
                                dword_F63C74[1580 * mClient] +=
                                    viewAxes[7] * adjZ;
                                dword_F63C78[1580 * mClient] +=
                                    viewAxes[8] * adjZ;
                            }
                        }
                    }
                }
                CG_OffsetFirstPersonView();
            }
        }
        break;
    case 13:
        if (gCameraSwayOnTurrets)
            CG_CalcGunnerViewPos(false, v3);
        else
        {
            CG_CalcTurretViewValues();
            CG_OffsetFirstPersonView();
        }
        break;
    case 14:
        UpdateIntermissionCam();
        break;
    case 15:
        UpdateSceneAnimCam();
        break;
    case 17:
        {
            Client* v16 = GetPlayer(mClient)->client;
            dword_F63C70[1580 * mClient] = v16->ps.origin.v.m128_f32[0];
            dword_F63C74[1580 * mClient] = v16->ps.origin.v.m128_f32[1];
            dword_F63C78[1580 * mClient] = v16->ps.origin.v.m128_f32[2];
            angle[1580 * mClient] = v16->ps.viewangles[0];
            dword_F63CB4[1580 * mClient] = v16->ps.viewangles[1];
            dword_F63CB8[1580 * mClient] = v16->ps.viewangles[2];
        }
        CG_OffsetFirstPersonView();
        break;
    case 19:
        UpdateMPDeathCamera();
        break;
    case 21:
        UpdateDeathCamera();
        break;
    default:
        break;
    }
    UpdateFade();
    if (newMode != 16 && newMode != 18)
        UpdateViewPO();
}

// ============================================================================
// View namespace - split-screen viewport/window helpers (cg.o View.cpp)
// ============================================================================

struct View_Window {
    float XPos;    // +0x00
    float YPos;    // +0x04
    float Width;   // +0x08
    float Height;  // +0x0C
    unsigned char _pad[0x1C - 0x10];
};

struct View_Setup {
    int Windows[5];  // +0x00
};

namespace View {
int lNumViewports = 0;  // ?lNumViewports@View@@3HA (cg.o @ 0xF61728)
}
View_Setup Setups[8];               // ?Setups@@3PAUView_Setup@@A (cg.o @ 0xDF9DB8)
View_Window Windows[8];             // ?Windows@@3PAUView_Window@@A (cg.o @ 0xDF9E58)
int ViewSetupConfigurations[4];  // ?ViewSetupConfigurations (cg.o @ 0xD0D200)
float scalar2View;           // 0x00DF9E44
float scalar4View;           // 0x00DF9E48
extern float unk_F6A284[4 * 802];
extern float unk_F6A288[4 * 802];
extern int dword_F6A290[4 * 802];
extern vmCvar_t cg_widescreen;      // 0x00F5CC88
extern float get_screensafe_left();
extern float get_screensafe_top();
extern void tlPrintf(const char* fmt, ...);
extern void FEManager_UpdateSplitScreen(void* self);
extern void nglSetView(float x1, float y1, float x2, float y2);
extern void nglSetScissor(float x1, float y1, float x2, float y2);

namespace View {

// ea: 0x00693C10
bool IsSplitScreen()
{
    return lNumViewports > 1;
}

// ea: 0x00693C20
int GetNumViewports()
{
    return lNumViewports;
}

// ea: 0x00693C30
void UpdateViewports()
{
    unk_F6A284[0] = 0.0f;
}

// ea: 0x00693C40
const View_Setup* GetCurrentSetup()
{
    return &Setups[ViewSetupConfigurations[lNumViewports]];
}

// ea: 0x00693C60
const View_Window* GetCurrentWindow(int clientIndex)
{
    return &Windows[Setups[ViewSetupConfigurations[lNumViewports]]
                        .Windows[(int)unk_F6A288[802 * clientIndex]]];
}

// ea: 0x00693CA0
void SetViewportClipping(int clientIndex)
{
    if (lNumViewports > 1)
    {
        const View_Window* v1 =
            &Windows[Setups[ViewSetupConfigurations[lNumViewports]]
                         .Windows[(int)unk_F6A288[802 * clientIndex]]];
        nglSetView(v1->XPos, v1->YPos, v1->XPos + v1->Width,
                   v1->YPos + v1->Height);
        nglSetScissor(v1->XPos, v1->YPos, v1->XPos + v1->Width,
                      v1->YPos + v1->Height);
    }
}

// ea: 0x00693DB0
float GetScalingForWindow(int window)
{
    switch (window)
    {
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        return 0.5f;
    default:
        return 1.0f;
    }
}

// ea: 0x00693DF0
float GetXScalingForWindow(int window, bool normal_aspect)
{
    float v2 = 0.75f;
    if (cg_widescreen.integer == 0)
        v2 = 1.0f;
    switch (window)
    {
    case 3:
    case 4:
        if (!normal_aspect)
            return v2;
        return v2 * 0.5f;
    case 5:
    case 6:
    case 7:
    case 8:
        return v2 * 0.5f;
    default:
        return v2;
    }
}

// ea: 0x00693E60
float GetYScalingForWindow(int window)
{
    switch (window)
    {
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        return 0.5f;
    default:
        return 1.0f;
    }
}

// ea: 0x00693EA0
float GetScalingForHUD(int window)
{
    float v1 = 0.75f;
    if (cg_widescreen.integer == 0)
        v1 = 1.0f;
    float scale = v1;
    switch (window)
    {
    case 3:
    case 4:
        return scalar2View * v1;
    case 5:
    case 6:
    case 7:
    case 8:
        scale = scalar4View * v1;
        return scale;
    default:
        return scale;
    }
}

// ea: 0x00693F20
float GetXScalingForHUD(int window)
{
    float v1 = 0.75f;
    if (cg_widescreen.integer == 0)
        v1 = 1.0f;
    float scale = v1;
    switch (window)
    {
    case 3:
    case 4:
        return scalar2View * v1;
    case 5:
    case 6:
    case 7:
    case 8:
        scale = scalar4View * v1;
        return scale;
    default:
        return scale;
    }
}

// ea: 0x00693FA0
float GetYScalingForHUD(int window)
{
    float scale = 1.0f;
    switch (window)
    {
    case 3:
    case 4:
        return scalar2View;
    case 5:
    case 6:
    case 7:
    case 8:
        scale = scalar4View;
        return scale;
    default:
        return scale;
    }
}

// ea: 0x00694010
float GetPreviousHUDXPos(float pos, int window, char justification,
                         float width)
{
    float screensafe_size = get_screensafe_left();
    float v4 = screensafe_size * 0.5f;
    float v5 = 320.0f - (screensafe_size * 0.5f);
    float v6 = v5 * 0.003125f;
    float pos_x = pos;
    switch (window)
    {
    case 0:
    case 3:
    case 4:
        return pos;
    case 5:
    case 7:
        pos_x = ((pos - v5) / v6) + v5;
        if ((justification & 2) == 0)
            return (pos_x - screensafe_size - scalar4View * width)
                       / ((320.0f - screensafe_size) * 0.0015625f)
                   + screensafe_size;
        {
            float v8 = (((pos - v5) / v6) + v5) - v4;
            pos_x = v8;
        }
        return (pos_x - screensafe_size - scalar4View * width)
                   / ((320.0f - screensafe_size) * 0.0015625f)
               + screensafe_size;
    case 6:
    case 8:
        {
            float v9 = ((pos - (v4 + 320.0f)) / v6) + (v4 + 320.0f);
            if ((justification & 1) != 0)
                v9 = v4 + v9;
            pos_x = v9 - 320.0f;
        }
        return (pos_x - screensafe_size - scalar4View * width)
                   / ((320.0f - screensafe_size) * 0.0015625f)
               + screensafe_size;
    default:
        return (pos_x - screensafe_size - scalar4View * width)
                   / ((320.0f - screensafe_size) * 0.0015625f)
               + screensafe_size;
    }
}

// ea: 0x00694100
float GetCurrentHUDXPos(float pos, int window, char justification, float width)
{
    float screensafe_size = get_screensafe_left();
    float v4 = screensafe_size * 0.5f;
    float v5 = 320.0f - (screensafe_size * 0.5f);
    float v6 = v5 * 0.003125f;
    float pos_x = (320.0f - screensafe_size) * 0.0015625f
                      * (pos - screensafe_size)
                  + scalar4View * width + screensafe_size;
    switch (window)
    {
    case 0:
    case 3:
    case 4:
        return pos;
    case 5:
    case 7:
        {
            float v8 = v4 + pos_x;
            if ((justification & 2) == 0)
                v8 = pos_x;
            return (((v8 - v5) * v6) + v5);
        }
    case 6:
    case 8:
        {
            float v9 = pos_x + 320.0f;
            if ((justification & 1) != 0)
                v9 = v9 - v4;
            pos_x = ((v9 - (v4 + 320.0f)) * v6) + (v4 + 320.0f);
        }
        return pos_x;
    default:
        return pos_x;
    }
}

// ea: 0x006941F0
float GetCurrentHUDYPos(float pos, int window, char justification, float height)
{
    float screensafe_size = get_screensafe_top();
    float v5 = scalar4View;
    if (window == 4 || window == 3)
        v5 = 1.0f;
    float v6 = (((pos - screensafe_size)
                 * ((240.0f - screensafe_size) * 0.0020833334f))
                + (v5 * height))
               + screensafe_size;
    float pos_x = v6;
    switch (window)
    {
    case 0:
        return pos;
    case 3:
    case 5:
    case 6:
        if ((justification & 8) == 0)
            return pos_x;
        return (screensafe_size * 0.5f) + v6;
    case 4:
    case 7:
    case 8:
        pos_x = v6 + (240.0f - screensafe_size);
        return pos_x;
    default:
        return pos_x;
    }
}

// ea: 0x006942C0
float GetPreviousHUDYPos(float pos, int window, char justification,
                         float height)
{
    float screensafe_size = get_screensafe_top();
    float pos_x = pos;
    switch (window)
    {
    case 0:
        return pos;
    case 3:
    case 5:
    case 6:
        if ((justification & 8) == 0)
            break;
        pos_x = pos - (screensafe_size * 0.5f);
        break;
    case 4:
    case 7:
    case 8:
        pos_x = pos - (240.0f - screensafe_size);
        break;
    default:
        break;
    }
    float height_scalar = scalar4View;
    if (window == 4 || window == 3)
        height_scalar = 1.0f;
    return (pos_x - screensafe_size - height_scalar * height)
               / ((240.0f - screensafe_size) * 0.0020833334f)
           + screensafe_size;
}

// ea: 0x00694390
float GetCurrentXPos(float pos, int window)
{
    switch (window)
    {
    case 5:
    case 7:
        return (pos * 0.0015625f) * 320.0f;
    case 6:
    case 8:
        pos = ((pos * 0.0015625f) + 1.0f) * 320.0f;
        return pos;
    default:
        return pos;
    }
}

// ea: 0x00694400
float GetCurrentYPos(float pos, int window)
{
    switch (window)
    {
    case 3:
    case 5:
    case 6:
        return (pos * 0.0020833334f) * 240.0f;
    case 4:
    case 7:
    case 8:
        pos = ((pos * 0.0020833334f) + 1.0f) * 240.0f;
        return pos;
    default:
        return pos;
    }
}

// ea: 0x0069B0B0
void SetNumViewports(int num)
{
    tlPrintf("SetNumViewports: set to %i\n", num);
    unk_F6A284[0] = 0.0f;
    if (num <= lNumViewports)
    {
        lNumViewports = num + 1;
        FEManager_UpdateSplitScreen(&g_femanager);
        --lNumViewports;
    }
    else
    {
        lNumViewports = num;
        FEManager_UpdateSplitScreen(&g_femanager);
    }
}

// ea: 0x0069B100
void UpdateNumViewports()
{
    int v1 = dword_F6A290 != 0;
    tlPrintf("SetNumViewports: set to %i\n", v1);
    unk_F6A284[0] = 0.0f;
    if (v1 <= lNumViewports)
    {
        lNumViewports = v1 + 1;
        FEManager_UpdateSplitScreen(&g_femanager);
        --lNumViewports;
    }
    else
    {
        lNumViewports = v1;
        FEManager_UpdateSplitScreen(&g_femanager);
    }
}

}  // namespace View

// ============================================================================
// Weapon selection helpers (cg.o cg_misc.cpp)
// ============================================================================

extern int cg_aWeaponSelect[4];       // 0x00F5D078
extern int cg_aWeaponSelectTime[4];   // 0x00F610D8
extern vmCvar_t cg_weaponCycleDelay;  // 0x00F5EF18
extern bool Entity_IsLocalPlayer(const Entity* ent);
extern bool CG_WeaponSelectable(int i);
extern int BG_SelectWeaponIndex(int iWeaponIndex, int client);
extern void CG_GameMessage(const char* msg, int flags);
extern const char* SEH_LocalizeTextMessage(const char* pszMessage,
                                           const char* pszMsgType);
extern int Com_BitCheck(const int* const array, int bitNum);
extern int BG_WeaponAmmo(const PlayerState* pPS, int iWeapon);
extern int BG_GetNumWeapons();
enum weapSlot_t : int;
extern weapSlot_t BG_IsPlayerWeaponInSlot(const PlayerState* pPS,
                                          int iWeaponIndex, int bAnyMode);
extern int BG_GetStackSlotForWeapon(const PlayerState* pPS, int iWeaponIndex,
                                    weapSlot_t preferedSlot);
extern int BG_IsPlayerWeaponAnAlt(int iWeaponIndex, int iAltIndex);
extern void PM_KillQueuedReloadSound(PlayerState* ps);
extern bool BG_AllowPlayerWeaponAtVehiclePos(int vehType, int vehPos);
extern weaponFileInfo_t* BG_GetInfoForWeapon(int weapon);
extern void* gpBrocAPI;  // 0x00F3ABDC
struct weaponParms;
extern void CalcMuzzlePoints(Entity* ent, weaponParms* wp);
extern bool Weapon_Revive_Test(Entity* ent, weaponParms* wp,
                               Entity** traceEnt);
extern int Weapon_Mine_Test(Entity* ent, weaponParms* wp,
                            math::Position3* position, math::Dir3* normal);

// weaponFileInfo_t extra fields (offsets verified from disassembly)
struct weaponInfoCam {
    unsigned char _pad0[0x84];
    int weapClass;        // +0x84
    int slot;             // +0x88
    unsigned char _pad1[0xAC - 0x8C];
    int type;             // +0xAC (weapType_t)
    unsigned char _pad3[0x764 - 0xB4];
    int iAltWeaponIndex;  // +0x764
};

struct weaponParms {
    void* pWeapInfo;  // +0x00
};

// ea: 0x00692600
void CG_AltWeapon_f()
{
    Entity* Player = EntityManager::sInst->GetPlayer( currCl);
    if (Player && Player->client
        && (EntityManager::sInst->GetPlayer( currCl)
                ->client->ps.pm_flags
            & 0x4000)
               == 0
        && (0x80000
            & EntityManager::sInst->GetPlayer( currCl)
                  ->client->ps.pm_flags)
               != 0
        && (0x106000
            & EntityManager::sInst->GetPlayer( currCl)
                  ->client->ps.eFlags)
               == 0
        && cgGlobal.time - cg_aWeaponSelectTime[currCl]
               >= cg_weaponCycleDelay.integer)
    {
        weaponInfoCam* InfoForWeapon =
            (weaponInfoCam*)BG_GetInfoForWeapon(cg_aWeaponSelect[currCl]);
        int iAltWeaponIndex = InfoForWeapon->iAltWeaponIndex;
        if (iAltWeaponIndex)
        {
            if (CG_WeaponSelectable(iAltWeaponIndex))
                BG_SelectWeaponIndex(iAltWeaponIndex, currCl);
        }
        else
        {
            const char* v3 = SEH_LocalizeTextMessage(
                "CGAME_THIS_WEAPON_HAS_NO_ALTERNATE", "game message");
            CG_GameMessage(v3, 0);
        }
    }
}

// ea: 0x00692C70
bool TestSpecialWeapon(Entity* player, int weapon)
{
    weaponParms wp;
    wp.pWeapInfo = BG_GetInfoForWeapon(weapon);
    CalcMuzzlePoints(player, &wp);
    int type = *(int*)((char*)wp.pWeapInfo + 0xAC);
    if (type == 4 /* WEAPTYPE_ITEM */)
    {
        if (*(int*)((char*)wp.pWeapInfo + 0xB0) == 0xB /* WEAPCLASS_REVIVE */
            && !Weapon_Revive_Test(player, &wp, nullptr))
        {
            if (*(void**)((char*)gpBrocAPI + 0xCC4) != nullptr)
            {
                (*(void(**)(int))((char*)gpBrocAPI + 0xCC4))(
                    *(int*)((char*)player + 0x234));
            }
            return false;
        }
    }
    else if (type == 7 /* WEAPTYPE_NUM */
             && !Weapon_Mine_Test(player, &wp, nullptr, nullptr))
    {
        if (*(void**)((char*)gpBrocAPI + 0xCC0) != nullptr)
        {
            (*(void(**)(int))((char*)gpBrocAPI + 0xCC0))(
                *(int*)((char*)player + 0x234));
            return false;
        }
        return false;
    }
    return true;
}

// ea: 0x00692FF0
int CG_SelectFirstWeaponInSlotWithLocalIndex(int bNext, int bIgnoreEmpty,
                                             int localIdx)
{
    int v3 = bNext != 0 ? 1 : 9;
    int iStep = bNext != 0 ? 1 : -1;
    while (1)
    {
        if (localIdx >= 16)
            CG_ASSERT("idx<16", "c:\\cod\\code\\game\\EntityManager.h", 19);
        if (((Entity*)EntityManager::sInst->mPlayers[localIdx])->client->ps.weaponslots
                    [v3]
                != 0
            && (v3 == 1 || v3 == 2))
        {
            if (localIdx >= 16)
                CG_ASSERT("idx<16", "c:\\cod\\code\\game\\EntityManager.h",
                          19);
            if (Entity_IsLocalPlayer(
                    (const Entity*)EntityManager::sInst->mPlayers[localIdx]))
            {
                if (bIgnoreEmpty == 0)
                    break;
                Client* client =
                    EntityManager::sInst->GetPlayer( localIdx)
                        ->client;
                Entity* Player =
                    EntityManager::sInst->GetPlayer( localIdx);
                if (BG_WeaponAmmo(&Player->client->ps,
                                  client->ps.weaponslots[v3])
                    != 0)
                    break;
            }
        }
        v3 += iStep;
        if (v3 == 0 || v3 == 10)
            return 0;
    }
    Entity* v9 = EntityManager::sInst->GetPlayer( localIdx);
    BG_SelectWeaponIndex(v9->client->ps.weaponslots[v3], localIdx);
    return 1;
}

// ea: 0x00693180
int CG_SelectFirstWeaponInSlot(int bNext, int bIgnoreEmpty)
{
    return CG_SelectFirstWeaponInSlotWithLocalIndex(bNext, bIgnoreEmpty,
                                                    currCl);
}

// ea: 0x006931A0
int CG_SelectFirstWeaponNotInSlotWithLocalIndex(int bNext, int bIgnoreEmpty,
                                                int localIdx)
{
    int v3 = bNext ? 1 : -1;
    int NumWeapons = bNext ? 1 : BG_GetNumWeapons();
    while (NumWeapons < BG_GetNumWeapons())
    {
        weaponInfoCam* InfoForWeapon =
            (weaponInfoCam*)BG_GetInfoForWeapon(NumWeapons);
        if (InfoForWeapon)
        {
            int slot = InfoForWeapon->slot;
            if (slot == 7 /* WEAPSLOT_BINOCS */)
                NumWeapons += v3;
            else if (slot == 4 /* WEAPSLOT_GRENADE */)
                NumWeapons += v3;
            else
            {
                Entity* Player =
                    EntityManager::sInst->GetPlayer( localIdx);
                Entity* v8 =
                    EntityManager::sInst->GetPlayer( localIdx);
                Entity* v9 =
                    EntityManager::sInst->GetPlayer( localIdx);
                Entity* v10 =
                    EntityManager::sInst->GetPlayer( localIdx);
                if (!Com_BitCheck(Player->client->ps.weapons, NumWeapons)
                    || BG_IsPlayerWeaponInSlot(&v8->client->ps, NumWeapons,
                                               1)
                    || BG_GetStackSlotForWeapon(&v9->client->ps, NumWeapons,
                                                (weapSlot_t)0)
                    || (bIgnoreEmpty
                        && !BG_WeaponAmmo(&v10->client->ps, NumWeapons)))
                {
                    NumWeapons += v3;
                    if (!NumWeapons || NumWeapons == BG_GetNumWeapons())
                        return 0;
                }
                else
                {
                    Entity* v11 =
                        EntityManager::sInst->GetPlayer( localIdx);
                    if (Entity_IsLocalPlayer(v11))
                    {
                        BG_SelectWeaponIndex(NumWeapons, localIdx);
                        return 1;
                    }
                }
            }
        }
        else
        {
            CG_ASSERT("0", "c:\\cod\\code\\game\\cg_weapons.cpp", 3912);
            NumWeapons += v3;
        }
    }
    return 0;
}

// ea: 0x00693320
int CG_SelectFirstWeaponNotInSlot(int bNext, int bIgnoreEmpty)
{
    return CG_SelectFirstWeaponNotInSlotWithLocalIndex(bNext, bIgnoreEmpty,
                                                       currCl);
}

// ea: 0x00693340
void CG_CycleWeap(int bNext, int bIgnoreEmpty)
{
    if (dword_F62960[1580 * currCl] != 0
        && (0x80000
            & EntityManager::sInst->GetPlayer( currCl)
                  ->client->ps.pm_flags)
               != 0)
    {
        Entity* Player =
            EntityManager::sInst->GetPlayer( currCl);
        if (Entity_IsLocalPlayer(Player))
        {
            int v3;
            int iStep;
            int iWeaponLooped;
            if (bNext != 0)
            {
                v3 = 1;
                iStep = 1;
                iWeaponLooped = 1;
            }
            else
            {
                iStep = -1;
                v3 = -1;
                iWeaponLooped = BG_GetNumWeapons();
            }
            Entity* v4 = EntityManager::sInst->GetPlayer( currCl);
            int StackSlotForWeapon =
                BG_IsPlayerWeaponInSlot(&v4->client->ps,
                                        cg_aWeaponSelect[currCl], 1);
            if (StackSlotForWeapon == 0 /* WEAPSLOT_NONE */)
            {
                Entity* v6 =
                    EntityManager::sInst->GetPlayer( currCl);
                StackSlotForWeapon = BG_GetStackSlotForWeapon(
                    &v6->client->ps, cg_aWeaponSelect[currCl],
                    (weapSlot_t)0 /* WEAPSLOT_NONE */);
            }
            Entity* v7 = EntityManager::sInst->GetPlayer( currCl);
            PM_KillQueuedReloadSound(&v7->client->ps);
            if (StackSlotForWeapon != 0)
            {
                for (int i = (StackSlotForWeapon + v3 + 8) % 9 + 1;
                     i != StackSlotForWeapon; i = (i + v3 + 8) % 9 + 1)
                {
                    if (EntityManager::sInst->GetPlayer( currCl)
                                ->client->ps.weaponslots[i]
                            != 0
                        && (i == 1 || i == 2))
                    {
                        Client* client =
                            EntityManager::sInst->GetPlayer(
                                                    currCl)
                                ->client;
                        Entity* v10 = EntityManager::sInst->GetPlayer(
                            currCl);
                        if (bIgnoreEmpty == 0
                            || BG_WeaponAmmo(&v10->client->ps,
                                             client->ps.weaponslots[i])
                                   != 0)
                        {
                            PlayerState* ps =
                                &GetPlayerState(currCl);
                            BG_SelectWeaponIndex(
                                ps->weaponslots[i], currCl);
                            return;
                        }
                        v3 = iStep;
                    }
                }
                if (CG_SelectFirstWeaponNotInSlot(bNext, bIgnoreEmpty) == 0)
                {
                    if (StackSlotForWeapon != 1 /* WEAPSLOT_PRIMARY */
                        && StackSlotForWeapon != 2 /* WEAPSLOT_PRIMARYB */)
                        CG_SelectFirstWeaponInSlot(bNext, 0);
                done:
                    Entity* v18 =
                        EntityManager::sInst->GetPlayer( currCl);
                    if (!Com_BitCheck(v18->client->ps.weapons,
                                      cg_aWeaponSelect[currCl]))
                        BG_SelectWeaponIndex(0, currCl);
                }
            }
            else
            {
                int v12 = cg_aWeaponSelect[currCl];
                while (1)
                {
                    v12 = (v12 + v3 - 1 + BG_GetNumWeapons())
                              % BG_GetNumWeapons()
                          + 1;
                    if (v12 == iWeaponLooped)
                        break;
                    Entity* v13 =
                        EntityManager::sInst->GetPlayer( currCl);
                    if (Com_BitCheck(v13->client->ps.weapons, v12) != 0
                        && !BG_IsPlayerWeaponAnAlt(v12,
                                                   cg_aWeaponSelect[currCl]))
                    {
                        Entity* v14 =
                            EntityManager::sInst->GetPlayer(
                                                    currCl);
                        if (BG_IsPlayerWeaponInSlot(&v14->client->ps, v12, 1)
                            == 0 /* WEAPSLOT_NONE */)
                        {
                            Entity* v15 =
                                EntityManager::sInst->GetPlayer(
                                                        currCl);
                            if (BG_GetStackSlotForWeapon(&v15->client->ps,
                                                         v12,
                                                         (weapSlot_t)0)
                                == 0)
                            {
                                Entity* v16 =
                                    EntityManager::sInst->GetPlayer(currCl);
                                if (bIgnoreEmpty == 0
                                    || BG_WeaponAmmo(&v16->client->ps, v12)
                                           != 0)
                                {
                                    int slot =
                                        ((weaponInfoCam*)BG_GetInfoForWeapon(
                                             v12))
                                            ->slot;
                                    if (slot == 1 /* WEAPSLOT_PRIMARY */
                                        || slot == 2 /* WEAPSLOT_PRIMARYB */)
                                    {
                                        BG_SelectWeaponIndex(v12, currCl);
                                        return;
                                    }
                                }
                            }
                        }
                    }
                }
                if (CG_SelectFirstWeaponInSlotWithLocalIndex(
                        bNext, bIgnoreEmpty, currCl)
                    == 0)
                {
                    if (CG_SelectFirstWeaponNotInSlotWithLocalIndex(
                            bNext, bIgnoreEmpty, currCl)
                        == 0)
                        CG_SelectFirstWeaponInSlotWithLocalIndex(bNext, 0,
                                                                 currCl);
                    goto done;
                }
            }
        }
    }
}

// ea: 0x0069A3B0
void CG_NextWeapon_f()
{
    Entity* Player = EntityManager::sInst->GetPlayer( currCl);
    if (Player != nullptr && Player->client != nullptr
        && (EntityManager::sInst->GetPlayer( currCl)
                ->client->ps.pm_flags
            & 0x4000)
               == 0
        && (EntityManager::sInst->GetPlayer( currCl)
                ->client->ps.eFlags
            & 0x6000)
               == 0)
    {
        Client* client =
            EntityManager::sInst->GetPlayer( currCl)->client;
        Entity* v2 = EntityManager::sInst->GetPlayer( currCl);
        if ((0x100000
             & EntityManager::sInst->GetPlayer( currCl)
                   ->client->ps.eFlags)
                == 0
            || BG_AllowPlayerWeaponAtVehiclePos(v2->client->ps.vehType,
                                                client->ps.vehPos))
        {
            Entity* v3 =
                EntityManager::sInst->GetPlayer( currCl);
            weaponInfoCam* InfoForWeapon =
                (weaponInfoCam*)BG_GetInfoForWeapon(v3->client->ps.weapon);
            weaponInfoCam* v5 = InfoForWeapon;
            if ((InfoForWeapon == nullptr
                 || ((InfoForWeapon->weapClass != 10 /* WEAPCLASS_LMG */
                      || (GetPlayerState(currCl).pm_flags & 0x20) == 0)
                     && v5->weapClass != 16))
                && (0x80000
                    & EntityManager::sInst->GetPlayer( currCl)
                          ->client->ps.pm_flags)
                       != 0
                && cgGlobal.time - cg_aWeaponSelectTime[currCl]
                       >= cg_weaponCycleDelay.integer)
            {
                cg_aWeaponSelectTime[currCl] = cgGlobal.time;
                CG_CycleWeap(1, 0);
            }
        }
    }
}

// ea: 0x0069A540
void CG_PrevWeapon_f()
{
    Entity* Player = EntityManager::sInst->GetPlayer( currCl);
    if (Player != nullptr && Player->client != nullptr
        && (EntityManager::sInst->GetPlayer( currCl)
                ->client->ps.pm_flags
            & 0x4000)
               == 0
        && (0x106000
            & EntityManager::sInst->GetPlayer( currCl)
                  ->client->ps.eFlags)
               == 0)
    {
        Entity* v1 = EntityManager::sInst->GetPlayer( currCl);
        weaponInfoCam* InfoForWeapon =
            (weaponInfoCam*)BG_GetInfoForWeapon(v1->client->ps.weapon);
        weaponInfoCam* v3 = InfoForWeapon;
        if ((InfoForWeapon == nullptr
             || ((InfoForWeapon->weapClass != 10 /* WEAPCLASS_LMG */
                  || (GetPlayerState(currCl).pm_flags & 0x20) == 0)
                 && v3->weapClass != 16))
            && (0x80000
                & EntityManager::sInst->GetPlayer( currCl)
                      ->client->ps.pm_flags)
                   != 0
            && cgGlobal.time - cg_aWeaponSelectTime[currCl]
                   >= cg_weaponCycleDelay.integer)
        {
            cg_aWeaponSelectTime[currCl] = cgGlobal.time;
            CG_CycleWeap(0, 0);
        }
    }
}

// ============================================================================
// Misc draw/parse/debug helpers (cg.o cg_misc.cpp)
// ============================================================================

float gFireHeatBlur;   // 0x00F616EC
float maxBlurScale;    // 0x00DFA314
float r_1;             // 0x00DFA310
float g;               // 0x00DFA30C
float b_1;             // 0x00DFA308
float a_0;             // 0x00DFA304
extern int g_blendType;       // 0x00DD9254
int fireBlendType;     // 0x00DFA300
float YOfs;            // 0x00DFA2FC
float XOfs;            // 0x00DFA2F8
char buffer_0[256];    // ?buffer_0@@3PADA (cg.o @ 0x00F73890)
extern _objectiveInfo_t objectives[4][17];  // 0x00F6A2B0
extern const float vec3_origin[3];
struct nglQuad;
extern void nglInitQuad(nglQuad* quad);
extern void nglSetQuadColor(nglQuad* quad, unsigned int c);
extern void nglSetQuadBlend(nglQuad* quad, unsigned int blend);
extern int nglGetScreenHeight();
extern int nglGetScreenWidth();
extern void nglSetQuadRect(nglQuad* quad, float x1, float y1, float x2,
                           float y2);
struct nglTexture;
extern nglTexture* nglGetFrontBufferTex();
extern void nglSetQuadTex(nglQuad* quad, nglTexture* tex);
extern void nglListAddQuad(nglQuad* quad);
extern void CG_PerturbationPoint(const float* prev, float* out, float mindist);
extern float VectorNormalize2(const float* v, float* out);
extern void Q_strncpyz(char* dest, const char* src, int destsize);
extern char* va(const char* fmt, ...);
extern void Com_Printf(const char* fmt, ...);
extern const char* Info_ValueForKey(const char* s, const char* key);
extern void Cmd_ArgvBuffer(int arg, char* buffer, int bufferLength);

// ea: 0x0068C920
bool CG_FxTest()
{
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cg_view.cpp";
    AeAssert::gCurrentLine = 63;
    AeAssert::gCurrentExpr = "0 && \"CG_FxTest Gone\"";
    bool result = AeAssert::IsIgnored();
    if (!result)
    {
        result = AeAssert::Assert("old cod assert");
        if (result)
            __debugbreak();
    }
    return result;
}

// ea: 0x006960D0
void CheckAndRunOverHeatBlur()
{
    gFireHeatBlur = gFireHeatBlur - ServerTime_sInst.mTickDelta;
    if (gFireHeatBlur < 0.0f)
        gFireHeatBlur = 0.0f;
    else if (gFireHeatBlur > maxBlurScale)
        gFireHeatBlur = maxBlurScale;
    if (gFireHeatBlur > 0.0f)
    {
        unsigned char r = (unsigned char)(r_1 * 255.0f);
        unsigned char green = (unsigned char)(g * 255.0f);
        unsigned char blue = (unsigned char)(b_1 * 255.0f);
        int beg = (int)(a_0 * 255.0f);
        unsigned char alpha = (unsigned char)(beg * gFireHeatBlur);
        if (g_blendType == 5 || g_blendType == 6)
        {
            r = (unsigned char)(r * gFireHeatBlur);
            green = (unsigned char)(green * gFireHeatBlur);
            blue = (unsigned char)(blue * gFireHeatBlur);
        }
        unsigned char q[0x60];
        memset(q, 0, sizeof(q));
        void* quad = q;
        nglInitQuad((nglQuad*)quad);
        int v2 = alpha;
        nglSetQuadColor((nglQuad*)quad,
                        (unsigned int)((alpha << 24) | (r << 16)
                                       | (green << 8) | blue));
        int blendconvert[7];
        blendconvert[5] = v2 | 0x87128600;
        blendconvert[1] = 0x10000 | v2;
        blendconvert[0] = 0;
        blendconvert[2] = 1691321856;
        blendconvert[3] = 1678214656;
        blendconvert[4] = 1678215936;
        blendconvert[6] = v2 | 0x86068600;
        nglSetQuadBlend((nglQuad*)quad,
                        (unsigned int)blendconvert[fireBlendType]);
        float y2 = (float)nglGetScreenHeight() + YOfs;
        int ScreenWidth = nglGetScreenWidth();
        nglSetQuadRect((nglQuad*)quad, XOfs, YOfs,
                       (float)ScreenWidth + XOfs, y2);
        nglSetQuadTex((nglQuad*)quad, nglGetFrontBufferTex());
        nglListAddQuad((nglQuad*)quad);
    }
}

// ea: 0x00696840
void CG_FillRectGradient(float x, float y, float width, float height,
                         const float* color, const float* gradcolor,
                         int gradientType)
{
    trap_R_SetColor(color);
    re.DrawStretchPicGradient(
        unk_F6A278[802 * currCl] * x, unk_F6A27C[802 * currCl] * y,
        unk_F6A278[802 * currCl] * width, unk_F6A27C[802 * currCl] * height,
        0.0f, 0.0f, 0.0f, 0.0f, cgsGlobal_media_whiteShader, gradcolor,
        gradientType);
    trap_R_SetColor(nullptr);
}

// ea: 0x00697E80
void CG_ClearObjective(_objectiveInfo_t* pObjective)
{
    if (pObjective == nullptr)
        return;
    _objectiveInfo_t* v1 = pObjective;
    _objectiveInfo_t* pParent = pObjective->pParent;
    pObjective->state = 0;  // OBJST_EMPTY
    if (pParent != nullptr)
    {
        pObjective->ringTime = -1;
        pObjective->displayOrder = -1;
        pObjective->vOrigin[0] = 0.0f;
        pObjective->vOrigin[1] = 0.0f;
        pObjective->vOrigin[2] = 0.0f;
        pObjective->szString[0] = 0;
        pObjective->ring = 0;
        pObjective->entity = 0;
        pObjective->pParent->pChild = pObjective->pChild;
        _objectiveInfo_t* pChild = pObjective->pChild;
        if (pChild != nullptr)
        {
            pChild->pParent = pObjective->pParent;
            pObjective->pChild = nullptr;
        }
        pObjective->pParent = nullptr;
    }
    else
    {
        do
        {
            v1->vOrigin[0] = 0.0f;
            v1->vOrigin[1] = 0.0f;
            v1->vOrigin[2] = 0.0f;
            v1->szString[0] = 0;
            v1->ringTime = -1;
            v1->ring = 0;
            v1->displayOrder = -1;
            v1->entity = 0;
            v1->pParent = nullptr;
            _objectiveInfo_t* next = v1->pChild;
            v1->pChild = nullptr;
            v1 = next;
        } while (v1 != nullptr);
    }
}

// ea: 0x00697F10
void CG_ParseCullDist()
{
    const char* ConfigString = CL_GetConfigString(9);
    float dist = (float)atof(ConfigString);
    re.SetCullDist(dist);
}

// ea: 0x00697F40
void CG_ParseFog()
{
    float ne, fa, r, g, b, density;
    Cmd_ArgvBuffer(1, buffer_0, 256);
    ne = (float)atof(buffer_0);
    Cmd_ArgvBuffer(2, buffer_0, 256);
    if (buffer_0[0] != 0)
    {
        fa = (float)atof(buffer_0);
        Cmd_ArgvBuffer(3, buffer_0, 256);
        density = (float)atof(buffer_0);
        Cmd_ArgvBuffer(4, buffer_0, 256);
        r = (float)atof(buffer_0);
        Cmd_ArgvBuffer(5, buffer_0, 256);
        g = (float)atof(buffer_0);
        Cmd_ArgvBuffer(6, buffer_0, 256);
        b = (float)atof(buffer_0);
        Cmd_ArgvBuffer(7, buffer_0, 256);
        int v0 = atoi(buffer_0);
        re.SetFog(4, (int)ne, (int)fa, r, g, b, density);
        re.SetFog(8, 4, v0, 0.0f, 0.0f, 0.0f, 0.0f);
    }
    else
    {
        re.SetFog(8, 3, (int)ne, 0.0f, 0.0f, 0.0f, 0.0f);
    }
}

// ea: 0x00698330
void CG_PrintPerturbationPoints()
{
    float perturb[128][2];
    CG_PerturbationPoint(vec3_origin, perturb[0], 0.5f);
    for (int i = 1; i < 128; ++i)
        CG_PerturbationPoint(perturb[i - 1], perturb[i], 0.5f);
    for (int j = 0; j < 128; ++j)
        Com_Printf("\t{%f, %f},\n", perturb[j][0], perturb[j][1]);
}

// ea: 0x006A0370
Entity* GetPlayerTarget()
{
    Client* client = EntityManager::sInst->GetPlayer( currCl)->client;
    if (client->ps.mTargetTime + 1000 <= cgGlobal.time)
        return nullptr;
    return DbHandleToEntity(client->ps.mTarget);
}

// ea: 0x006A30C0
void CG_ParseObjectiveChange(int iNum)
{
    int client = -1;
    int v1 = iNum - 16;
    const char* v2 = CG_ConfigString(iNum);
    if (v1 < 0 || v1 >= 17)
    {
        CG_ASSERT("(iObjective >= 0) && (iObjective < (16 + 1))",
                  "c:\\cod\\code\\game\\cg_scoreboard.cpp", 656);
        va("iObjective = %i\n", v1);
    }
    const char* v4 = Info_ValueForKey(v2, "clid");
    _objectiveInfo_t* v6;
    if (*v4 != 0 && (client = atoi(v4), client >= 0))
        v6 = &objectives[client][v1];
    else
        v6 = &objectives[0][v1];
    const char* v7 = Info_ValueForKey(v2, "delete");
    if (*v2 == 0 || *v7 != 0)
    {
        v6->state2 = 0;
    }
    else
    {
        int v8 = v6->state2;
        const char* v9 = Info_ValueForKey(v2, "state");
        if (*v9 != 0)
            v6->state2 = atoi(v9);
        else
            v6->state2 = 0;
        if (v6->state2 > 0x1A)
        {
            CG_ASSERT("(pObjective->state >= IGOCompassWidget::i_guy_bad_c) "
                      "&& (pObjective->state < "
                      "IGOCompassWidget::MAX_OBJECTIVE_ICONS)",
                      "c:\\cod\\code\\game\\cg_scoreboard.cpp", 695);
            va("pObjective->state = %i\n", v6->state2);
        }
        int v11 = v6->state2;
        v6->state = v11;
        if (v8 != 4 && v11 == 4)
            v6->ringTime = cgGlobal.time;
    }
    if (v6->state2 != 0)
    {
        const char* v12 = Info_ValueForKey(v2, "str");
        if (*v12 != 0)
            Q_strncpyz(v6->szString, v12, 128);
        else
            v6->szString[0] = 0;
        const char* v13 = Info_ValueForKey(v2, "org");
        if (*v13 != 0)
            sscanf(v13, "%f %f %f", &v6->vOrigin[0], &v6->vOrigin[1],
                   &v6->vOrigin[2]);
        else
        {
            v6->vOrigin[1] = 0.0f;
            v6->vOrigin[0] = 0.0f;
        }
        int v14 = v6->displayOrder;
        const char* v15 = Info_ValueForKey(v2, "ring");
        int v16 = *v15 != 0 ? atoi(v15) : 0;
        v6->ring = v16;
        if (v14 != v16)
            v6->ringTime = cgGlobal.time;
        const char* v17 = Info_ValueForKey(v2, "wstate");
        if (*v17 != 0)
            v6->state = atoi(v17);
        const char* v18 = Info_ValueForKey(v2, "height");
        v6->height = *v18 != 0 ? (float)atof(v18) : 0.0f;
        const char* v20 = Info_ValueForKey(v2, "ent");
        if (*v20 != 0)
        {
            unsigned int v21 = atoi(v20);
            unsigned int v22 = v21 & 0xFFF;
            Entity* mObject;
            if (v22 < 0x540
                && (v21 >> 12) == EntityHandleDb::sInst.mElements[v22].mKey
                && (mObject = EntityHandleDb::sInst.mElements[v22].mObject)
                       != nullptr)
            {
                v6->entity = *(int*)((char*)mObject + 0x234);
            }
            else
            {
                CG_ASSERT("pEnt",
                          "c:\\cod\\code\\game\\cg_scoreboard.cpp", 766);
            }
        }
        const char* v24 = Info_ValueForKey(v2, "pobj");
        if (v6->pParent == 0 && *v24 != 0)
        {
            int v25 = atoi(v24);
            _objectiveInfo_t* v26 =
                client < 0 ? &objectives[0][v25] : &objectives[client][v25];
            const char* v27 = Info_ValueForKey(v2, "order");
            v6->displayOrder = atoi(v27);
            if (v26->pChild != nullptr)
            {
                v26 = v26->pChild;
                if (v6->pChild != nullptr)
                {
                    CG_ASSERT("pObjective->pChild == 0",
                              "c:\\cod\\code\\game\\cg_scoreboard.cpp", 806);
                }
                while (v6->displayOrder > v26->displayOrder)
                {
                    if (v26->pChild == nullptr)
                        goto link_as_child;
                    v26 = v26->pChild;
                }
                v6->pChild = v26;
                _objectiveInfo_t* v28 = v26->pParent;
                v6->pParent = v28;
                v28->pChild = v6;
                v26->pParent = v6;
            }
            else
            {
            link_as_child:
                v26->pChild = v6;
                v6->pParent = v26;
            }
        }
    }
    else
    {
        CG_ClearObjective(v6);
    }
}

// ea: 0x0069C6E0
void CG_DebugCircleEx(const float* center, float radius, const float* dir,
                      const float* color, int depthTest, int duration)
{
    float norm[3];
    VectorNormalize2(dir, norm);
    float up[3], cross[3];
    PerpendicularVector(up, norm);
    CrossProduct(norm, up, cross);
    float points[16][3];
    for (int i = 0; i < 16; ++i)
    {
        float ang = i * 0.39269909f;
        float s = sinf(ang), c = cosf(ang);
        points[i][0] = up[0] * (s * radius) + cross[0] * (c * radius)
                       + center[0];
        points[i][1] = up[1] * (s * radius) + cross[1] * (c * radius)
                       + center[1];
        points[i][2] = up[2] * (s * radius) + cross[2] * (c * radius)
                       + center[2];
    }
    for (int i = 0; i < 16; ++i)
    {
        CL_AddDebugLine(points[i], points[(i + 1) & 0xF], color, depthTest,
                        duration, 0, 0);
    }
}

// ea: 0x0069C860
void CG_DebugCircle2Ex(const float* center, float radius, const float* dir,
                       const float* color, int depthTest, int duration)
{
    float norm[3];
    VectorNormalize2(dir, norm);
    float up[3], cross[3];
    PerpendicularVector(up, norm);
    CrossProduct(norm, up, cross);
    float points[16][3];
    for (int i = 0; i < 16; ++i)
    {
        float ang = i * 0.39269909f;
        float s = sinf(ang), c = cosf(ang);
        points[i][0] = up[0] * (s * radius) + cross[0] * (c * radius)
                       + center[0];
        points[i][1] = up[1] * (s * radius) + cross[1] * (c * radius)
                       + center[1];
        points[i][2] = up[2] * (s * radius) + cross[2] * (c * radius)
                       + center[2];
    }
    for (int i = 0; i < 16; ++i)
    {
        CL_AddDebugLine(points[i], points[(i + 1) & 0xF], color, depthTest,
                        duration, 0, 0);
    }
    for (int i = 0; i < 16; ++i)
    {
        CL_AddDebugLine(center, points[(i + 1) & 0xF], color, depthTest,
                        duration, 0, 0);
    }
}

// ea: 0x0069CA20
void CG_DebugArc(const float* center, float radius, float angle0, float angle1,
                 const float* color, int depthTest, int duration)
{
    float step = (angle1 - angle0) * 0.06666667f;
    if (step < 0.0f)
    {
        angle0 = angle0 - 360.0f;
        step = (angle1 - angle0) * 0.06666667f;
    }
    float points[16][3];
    for (int i = 0; i < 16; ++i)
    {
        float rad = ((i * step) + angle0) * 3.1415927f * 0.0055555557f;
        float s = sinf(rad), c = cosf(rad);
        points[i][0] = (s * radius) + center[0];
        points[i][1] = (c * radius) + center[1];
        points[i][2] = center[2];
    }
    for (int i = 0; i < 15; ++i)
    {
        CL_AddDebugLine(points[i], points[i + 1], color, depthTest, duration,
                        0, 0);
    }
}

// ============================================================================
// ADS meta-anim player (cg.o cg_misc.cpp)
// ============================================================================

struct nalAnyPoseAnim {
    unsigned char _pad[0x30];
    void* Skeleton;          // +0x30
    unsigned char Flags;     // +0x34
    float Duration;          // +0x38
};

struct tlFixedString {
    unsigned int hash;  // +0x00
    char str[28];       // +0x04
};

struct ADSMetaAnimData {
    void* vftable;           // +0x00
    tlFixedString mName;     // +0x04
    nalAnyPoseAnim* mAnimPtr;  // +0x24
    nalAnyPoseAnim* mRevPtr;   // +0x28

    virtual int IsAnimLooping() const;
    virtual int IsAnimTrajRelative() const;
    virtual float GetAnimDuration() const;
    virtual const void* GetSkeleton() const;
    virtual void* CreateAnimInst(void* theSkel, void* theAnim);
    virtual void DelayCreate(void** animArray, int numAnims);
};

struct ADSMetaAnimPlayer {
    void* mMetaNalBaseAnimPtr;      // +0x00
    ADSMetaAnimData* mADSMetaAnimDataPtr;  // +0x04

    void DeleteMetaAnim();
    void CreateMetaAnim(XAnimTree* pAnimTree);
    int Update(XAnimTree* pAnimTree, weaponInfo_s* weaponInfo);
};

extern void* tlMemAlloc(unsigned int size, unsigned int align,
                        unsigned int flags);
extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern void* MetaNalBaseAnim_Ctor(void* self);
extern void MetaNalBaseAnim_Create(void* self, void* metaAnimData);
extern void MetaNalBaseAnim_DelayCreate(void* self, void** animArray,
                                        int numAnims);
extern void XAnimEntry_Create(XAnimEntry* self);
extern void* ADSMetaAnimInstance_Ctor(void* self, void* forwardAnim,
                                      void* reverseAnim, void* theSkel,
                                      float* interpValue);
extern void AnimationPlayer_Play(void* self, void* anim, bool forceRestart,
                                 float fadeIn, void* playMethod,
                                 float callbackTime, void* callback,
                                 float speed, float startTimeSec);
extern float AnimationPlayer_GetAnimTime(void* self, void* anim);
extern void CG_StartWeaponAnim(int weaponNum, DObj* dobj, int animIndex,
                               float fadeInTime, float startTimeInSec,
                               int forceRestart);
float kADSAnimFadeInTime;  // 0x00DFA380
char gMetaAnimPlayMethod;  // 0x00F05108
char sWeaponAnimCallback;  // 0x00DF9E4C
extern tlFixedString tlFixedString_ctor(void* self, const char* s);

static XAnimEntry* AnimTreeEntry(void* pAnimTree, unsigned int index)
{
    void* anims = *(void**)((char*)pAnimTree + 8);
    unsigned int mSize = *(unsigned int*)((char*)anims + 4);
    if (index >= mSize)
    {
        CG_ASSERT("index < mSize", "../ae\\inplace/InplaceVector.h", 81);
        index = 0;
    }
    return &((XAnimEntry*)*(void**)((char*)anims + 8))[index];
}

// ea: 0x0068F240
int ADSMetaAnimData::IsAnimLooping() const
{
    if (mAnimPtr != nullptr)
        return mAnimPtr->Flags & 1;
    return 0;
}

// ea: 0x0068F260
int ADSMetaAnimData::IsAnimTrajRelative() const
{
    return mAnimPtr == nullptr || (mAnimPtr->Flags & 2) == 0;
}

// ea: 0x0068F280
float ADSMetaAnimData::GetAnimDuration() const
{
    if (mAnimPtr != nullptr)
        return mAnimPtr->Duration;
    return 1.0f;
}

// ea: 0x0068F2A0
const void* ADSMetaAnimData::GetSkeleton() const
{
    if (mAnimPtr != nullptr)
        return mAnimPtr->Skeleton;
    return nullptr;
}

// ea: 0x0068F2B0
void* ADSMetaAnimData::CreateAnimInst(void* theSkel, void* theAnim)
{
    void* v4 = tlMemAlloc(0x24, 8, 0);
    if (v4 == nullptr)
        return nullptr;
    Entity* Player = EntityManager::sInst->GetPlayer( currCl);
    return ADSMetaAnimInstance_Ctor(v4, mAnimPtr, mRevPtr, theSkel,
                                    &Player->client->ps.fWeaponPosFrac);
}

// ea: 0x0068F350
void ADSMetaAnimData::DelayCreate(void** animArray, int numAnims)
{
    if (animArray == nullptr || numAnims != 2)
    {
        CG_ASSERT("animArray && numAnims == 2",
                  "c:\\cod\\code\\game\\cg_weapons.cpp", 194);
    }
    bool v4 = *animArray == nullptr;
    mAnimPtr = (nalAnyPoseAnim*)*animArray;
    mRevPtr = (nalAnyPoseAnim*)animArray[1];
    if (v4)
    {
        CG_ASSERT("mAnimPtr", "c:\\cod\\code\\game\\cg_weapons.cpp", 198);
    }
    if (mRevPtr == nullptr)
    {
        CG_ASSERT("mRevPtr", "c:\\cod\\code\\game\\cg_weapons.cpp", 199);
    }
    tlFixedString v5;
    tlFixedString_ctor(&v5, "ADSMetaAnim");
    mName = v5;
}

// ea: 0x0068F490
void ADSMetaAnimPlayer::DeleteMetaAnim()
{
    void* mMetaNalBaseAnimPtr = this->mMetaNalBaseAnimPtr;
    if (mMetaNalBaseAnimPtr != nullptr)
    {
        (*(void(**)(void*, int))*(void**)mMetaNalBaseAnimPtr)(
            mMetaNalBaseAnimPtr, 1);
        this->mMetaNalBaseAnimPtr = nullptr;
    }
    if (mADSMetaAnimDataPtr != nullptr)
    {
        mem_heap_free(mADSMetaAnimDataPtr);
        mADSMetaAnimDataPtr = nullptr;
    }
}

// ea: 0x00699240
void ADSMetaAnimPlayer::CreateMetaAnim(XAnimTree* pAnimTree)
{
    if (mMetaNalBaseAnimPtr == nullptr)
    {
        void* v3 = tlMemAlloc(0x44, 8, 0);
        if (v3 != nullptr)
            mMetaNalBaseAnimPtr = MetaNalBaseAnim_Ctor(v3);
        else
            mMetaNalBaseAnimPtr = nullptr;
    }
    if (mADSMetaAnimDataPtr == nullptr)
    {
        void* v5 = mem_heap_malloc(0x2C);
        if (v5 != nullptr)
        {
            memset(v5, 0, 0x2C);
            *(void**)v5 = (void*)0x00D0F4FC;  // &ADSMetaAnimData::vftable
            ((ADSMetaAnimData*)v5)->mAnimPtr = nullptr;
            ((ADSMetaAnimData*)v5)->mRevPtr = nullptr;
        }
        mADSMetaAnimDataPtr = (ADSMetaAnimData*)v5;
    }
    MetaNalBaseAnim_Create(mMetaNalBaseAnimPtr, mADSMetaAnimDataPtr);
    XAnimEntry* e23 = AnimTreeEntry(pAnimTree, 0x17);
    if (e23->anim == nullptr)
        XAnimEntry_Create(e23);
    XAnimEntry* e24 = AnimTreeEntry(pAnimTree, 0x18);
    if (e24->anim == nullptr)
        XAnimEntry_Create(e24);
    void* animArray[2];
    animArray[0] = AnimTreeEntry(pAnimTree, 0x17)->anim;
    animArray[1] = AnimTreeEntry(pAnimTree, 0x18)->anim;
    MetaNalBaseAnim_DelayCreate(mMetaNalBaseAnimPtr, animArray, 2);
}

// ea: 0x0069EB30
int ADSMetaAnimPlayer::Update(XAnimTree* pAnimTree, weaponInfo_s* weaponInfo)
{
    Client* client = EntityManager::sInst->GetPlayer( currCl)->client;
    int v5 = client->ps.weapAnim & 0xFFFFFDFF;
    if (client->ps.weapAnim < 0 || v5 >= 24)
    {
        CG_ASSERT("weapAnimNum >= 0 && weapAnimNum < MAX_WP_ANIMATIONS",
                  "c:\\cod\\code\\game\\cg_weapons.cpp", 213);
    }
    float fWeaponPosFrac = client->ps.fWeaponPosFrac;
    bool v7 = fWeaponPosFrac != 0.0f && fWeaponPosFrac != 1.0f;
    if ((v5 == 21 || v5 == 22 || v5 == 0 || v5 == 23
         || (v5 == 4 || v5 == 7) && client->ps.weaponstate != 4)
        && v7)
    {
        if (mMetaNalBaseAnimPtr == nullptr)
        {
            CreateMetaAnim(pAnimTree);
            DObj* v8 = (DObj*)dword_F6A2A0[802 * currCl];
            AnimationPlayer_Play(((DObj*)v8)->animPlayers[0],
                                 mMetaNalBaseAnimPtr, true,
                                 kADSAnimFadeInTime, &gMetaAnimPlayMethod,
                                 0.0f, &sWeaponAnimCallback, 1.0f, 0.0f);
            if (((DObj*)v8)->animPlayers[1] != 0)
            {
                XAnimTree* v9 = (XAnimTree*)((DObj*)v8)->tree[1];
                if (v9 != 0)
                {
                    XAnimEntry* e1 = AnimTreeEntry(v9, 1);
                    if (e1->anim == nullptr)
                        XAnimEntry_Create(e1);
                    void* anim = e1->anim;
                    if (anim != nullptr)
                    {
                        AnimationPlayer_Play(
                            ((DObj*)v8)->animPlayers[1], anim, true,
                            kADSAnimFadeInTime, nullptr, 0.0f,
                            &sWeaponAnimCallback,
                            weaponInfo->viewModelAnimRates[1], 0.0f);
                        return mMetaNalBaseAnimPtr != nullptr;
                    }
                }
            }
        }
    }
    else if (mMetaNalBaseAnimPtr != nullptr)
    {
        DeleteMetaAnim();
        if (v5 != 0)
        {
            if (v5 == 23 && fWeaponPosFrac == 1.0f)
                client->ps.weapAnim = (~client->ps.weapAnim & 0x200) | 0x17;
        }
        else if (fWeaponPosFrac == 0.0f)
        {
            client->ps.weapAnim = ~client->ps.weapAnim & 0x200;
            return mMetaNalBaseAnimPtr != nullptr;
        }
    }
    return mMetaNalBaseAnimPtr != nullptr;
}

// ea: 0x006994A0
int CG_StartAnimBlend(int weaponNum, DObj* dobj, int toAnimIndex,
                      unsigned int fromAnimIndex, float blendTime)
{
    if (dobj == nullptr)
        return 0;
    XAnimTree* v7 = (XAnimTree*)dobj->tree[0];
    if (v7 == nullptr)
        return 0;
    void* anim = AnimTreeEntry(v7, fromAnimIndex)->anim;
    void* v9 = AnimTreeEntry(v7, toAnimIndex)->anim;
    if (anim == v9 || anim == nullptr || v9 == nullptr)
        return 0;
    void* v10 = dobj->animPlayers[0];
    float v11 = 0.0f;
    if (v10 != nullptr)
        v11 = AnimationPlayer_GetAnimTime(v10, anim);
    if (blendTime <= 0.0f)
        blendTime = 0.0f;
    CG_StartWeaponAnim(weaponNum, dobj, toAnimIndex, blendTime,
                       ((nalAnyPoseAnim*)v9)->Duration * v11, 1);
    return 1;
}

// ea: 0x00699560
bool CanInterrupt(XAnimTree* pAnimTree, void* client_cgs)
{
    unsigned int v2 = 4;
    while (1)
    {
        void* anims = *(void**)((char*)pAnimTree + 8);
        unsigned int mSize = *(unsigned int*)((char*)anims + 4);
        XAnimEntry* mList = *(XAnimEntry**)((char*)anims + 8);
        unsigned int v6 = v2;
        if (v2 >= mSize)
        {
            CG_ASSERT("index < mSize", "../ae\\inplace/InplaceVector.h", 81);
            if (v2 >= mSize)
                v6 = 0;
        }
        void* curAnim = *(void**)((char*)&sWeaponAnimCallback + 4);
        if (curAnim != nullptr && mList[v6].anim == curAnim)
            break;
        if (++v2 >= 23)
            return true;
    }
    *(int*)((char*)client_cgs + 0xD0) = -1;
    return false;
}

// ============================================================================
// FixupGunModelParts (cg.o cg_misc.cpp)
// ============================================================================

struct XModelPartsEntry {
    char* mStr;           // +0x00
    unsigned int mHash;   // +0x04
    int mFlags;           // +0x08
};

struct XModelParts {
    unsigned char _pad0[0x10];
    int mHierarchySize;       // +0x10
    XModelPartsEntry* mHierarchyList;  // +0x14
    unsigned char _pad1[0x28 - 0x18];
    int mMeshPtrsSize;        // +0x28
    void** mMeshPtrsList;     // +0x2C
};

extern int _stricmp(const char* dst, const char* src);
extern int _strnicmp(const char* dst, const char* src, size_t count);

// ea: 0x00699A90
void FixupGunModelParts(XModelParts* xmp)
{
    int mSize = xmp->mHierarchySize;
    void* bulletMesh = nullptr;
    for (int v2 = 0; v2 < mSize; ++v2)
    {
        unsigned int v4 = (unsigned int)v2 < (unsigned int)mSize ? v2 : 0;
        if (_stricmp(xmp->mHierarchyList[v4].mStr, "tag_b") == 0)
        {
            unsigned int idx = (unsigned int)v2
                               < (unsigned int)xmp->mMeshPtrsSize
                                   ? v2
                                   : 0;
            bulletMesh = xmp->mMeshPtrsList[idx];
        }
    }
    if (bulletMesh != nullptr)
    {
        for (int v7 = 0; v7 < mSize; ++v7)
        {
            unsigned int v9 = (unsigned int)v7 < (unsigned int)mSize ? v7 : 0;
            if (_strnicmp(xmp->mHierarchyList[v9].mStr, "tag_b", 5) == 0)
            {
                unsigned int v10 =
                    (unsigned int)v7 < (unsigned int)xmp->mMeshPtrsSize
                        ? v7
                        : 0;
                if (xmp->mMeshPtrsList[v10] == nullptr)
                    xmp->mMeshPtrsList[v10] = bulletMesh;
            }
        }
    }
}

// ============================================================================
// Tank / driver cameras + CG_Init (cg.o cg_misc.cpp)
// ============================================================================

struct vehicle_info_full_t {
    unsigned char _pad0[0x16C];
    float turretVertSpanUp;      // +0x16C
    float turretVertSpanDown;    // +0x170
    unsigned char _pad1[0x1B0 - 0x174];
    float turretCamOffset;       // +0x1B0
    float cameraFPHeightOffset;  // +0x1B4
    float cameraFPFwdOffset;     // +0x1B8
    float cameraFPHeightLerp;    // +0x1BC
    float cameraChaseOffsetX;    // +0x1C0
    float cameraChaseOffsetY;    // +0x1C4
    float cameraChaseOffsetZ;    // +0x1C8
    float cameraChaseRadiusInner;  // +0x1CC
    float cameraChaseRadiusOuter;  // +0x1D0
    unsigned char _pad3[0x1D8 - 0x1D4];
    float turretPitchUp;         // +0x1D8
    float turretPitchFactor;     // +0x1DC
    float turretPitchBias;       // +0x1E0
    float turretPitchScale;      // +0x1E4
    float camLinkedPitchFactor;  // +0x1E8
    float pitchBasedCamOffsetX;  // +0x1EC
    float pitchBasedCamOffsetZ;  // +0x1F0
};

extern vehicle_info_t* VEH_GetInfo(int idx);
extern bool IsPlayerFullySeatedInVehicle(Entity* player);
extern float VectorNormalize(float* v);
extern void MatrixMultiply(const float (*in1)[3], const float (*in2)[3],
                           float (*out)[3]);
extern float AngleNormalize360(float angle);
extern int PadAliasMgr_GetButtonValue(void* self, int ctrlNum,
                                      int buttonAlias);
extern void* PadAliasMgr_sInst;  // 0x00F4F458
extern int g_vehicle_button_threshold;  // 0x00E01F04
extern vmCvar_t g_vehControlMode;       // 0x00EADD48
float gTankDriverAnglesFrac;     // 0x00D0D1E0
extern unsigned char unk_F6A294[4 * 3208];
extern vmCvar_t cg_thirdPersonRange;    // 0x00F5F1E8
extern vmCvar_t cg_redFlashTime;        // 0x00F5C4A8
float the_rate;                  // 0x00DFA3E8
extern int dword_F64018[4 * 1580];
extern void CG_StartShakeCamera(float p, int duration, const float* src,
                                float radius, int client);
extern void Entity_Notify(void* ent, unsigned int hash);

struct RumbleEffect_local {
    struct RumbleData {
        unsigned char enabled;          // +0x00
        unsigned char _pad[3];
        float delay;                    // +0x04
        float intensity;                // +0x08
        float ramp_up_duration;         // +0x0C
        float steady_duration;          // +0x10
        float ramp_down_duration;       // +0x14
        void* rumble_notes;             // +0x18
        unsigned int m_flags;           // +0x1C
    };  // 0x20
    RumbleData mRumbleDataArray[2];     // +0x00
};
extern void RumbleEffect_Ctor(void* self);
extern void RumbleEffect_SetIntensity(void* self, int rumbleID,
                                      float new_intensity);
extern void RumbleEffect_SetNotes(void* self, int rumbleID, void* notes);
extern const char* gTankRumbleNotes;  // 0x00DF9D80
extern void BrocString_ctor(void* self, const char* s);
extern void BrocString_dtor(void* self);
struct DObjSkelMat;
extern bool G_DObjGetWorldBoneIndexMatrix(Entity* ent, int boneIndex,
                                          DObjSkelMat* tagMat);
extern void Axis4ToAngles(const float (*axis)[4], float* angles);
extern void CG_InitConsoleCommands();
struct glconfig_t;
struct glconfig_t;
extern void CL_GetGlconfig(glconfig_t* glconfig);
extern void CG_Error(const char* msg, ...);
extern void SCR_UpdateScreen();
extern void trap_R_ClearScene();
extern void LoadWorld(const char* name);
extern void CG_RegisterGraphics();
extern void CG_MapInit(int restart);
extern void* SoundMediaMgr_sInst;
extern void* GetTextureData(const char* name, int image_type,
                            const char* fromPak);
extern void Cvar_Register(vmCvar_t* vmCvar, const char* varName,
                          const char* defaultValue, int flags);
extern void Cvar_Set(const char* var_name, const char* value);
extern int trap_R_RegisterShaderNoMip(const char* name, int imagetype);
void* off_DF9208[64];  // cg.o data
extern void* SoundMediaMgr_j_nullsub_91(void* self);
enum netsrc_t {
    NS_CLIENT = 0,
    NS_SERVER = 1,
};
extern void Com_sprintf(char* dest, int size, const char* fmt, ...);
extern void CG_Trace(trace_t* result, const math::Position3* start,
                     const math::Position3* mins, const math::Position3* maxs,
                     const math::Position3* end,
                     const collision_context_t* context);

struct cgs_t {
    unsigned char _pad[0x84];
    int vidWidth;   // +0x84
    int vidHeight;  // +0x88
};
extern struct cgs_t* cgs;  // ?cgs@@3PAUcgs_t@@A

struct cgsGlobal_t {
    char mapname[128];  // +0x00
    unsigned char _pad[0x578 - 0x80];
    struct {
        void* whiteShader;        // +0x578
        void* softLineShader;     // +0x57C
        void* softLineHShader;    // +0x580
        void* friendlyFireShader; // +0x584
    } media;
};
cgsGlobal_t cgsGlobal;         // ?cgsGlobal@@3UcgsGlobal_t@@A (cg.o @ 0x13590F8)
extern void* cg_items;
extern weaponInfo_s cg_weapons[1];
extern vmCvar_t fs_debug_vm;
extern const float* InteractionController_GetHandsOrigin(void* self);
extern const float* InteractionController_GetHandsAngles(void* self);
extern int InteractionController_GetCameraMode(void* self);
extern void InteractionController_EndInteraction(void* self, int wasInteracting);
extern int InteractionController_StartInteraction(void* self, Entity* interactable,
                                                  const char* name, int curPakId);
extern float InteractionController_GetRotation(void* self);
extern int Entity_GetPlayerIndex(const Entity* self);
bool gSceneAnimCamera;  // 0x00F258F6
extern vmCvar_t cg_altTankCam;  // 0x00F5BC30
extern TPakId CurPakId();
extern vehicle_info_t* G_GetVehicleInfo(Entity* veh);
extern void CG_ClampViewAngles(PlayerState* ps, const float* centerAngles,
                               const float* minClamp, const float* maxClamp);

static unsigned int s_tagTurretHash;
static bool s_tagTurretHashInit;
static unsigned int s_tagDriverHash;
static bool s_tagDriverHashInit;
static float s_prevCamHeight;

// ea: 0x006AD8A0
void CG_Init()
{
    memset(&cgsGlobal, 0, sizeof(cgsGlobal));
    memset(cg_items, 0, sizeof(int));
    memset(cg_weapons, 0, sizeof(weaponInfo_s));
    cgsGlobal.media.whiteShader =
        GetTextureData("white", 0, "mp_frontEnd");
    cgsGlobal.media.softLineShader =
        GetTextureData("softline", 0, "mp_frontEnd");
    cgsGlobal.media.softLineHShader =
        GetTextureData("softlineh", 0, "mp_frontEnd");
    cgsGlobal.media.friendlyFireShader =
        GetTextureData("friendlyfire", 0, "mp_frontEnd");
    byte_F64194[0] = 0;
    cgGlobal.teamGame = false;
    for (int i = 170; i != 0; --i)
    {
        void** row = &off_DF9208[4 * i - 2];
                        Cvar_Register((vmCvar_t*)row[0],
                                      (const char*)row[1],
                                      (const char*)row[2], (int)row[3]);
    }
    if (fs_debug_vm.integer == 2)
        Cvar_Set("fs_debug", "0");
    CG_InitConsoleCommands();
    CL_GetGlconfig((glconfig_t*)cgs);
    unk_F6A278[0] = (float)cgs->vidWidth * 0.0015625f;
    unk_F6A27C[0] = (float)cgs->vidHeight * 0.0020833334f;
    currCl = NS_CLIENT;
    const char* ConfigString = CL_GetConfigString(2);
    if (strcmp(ConfigString, "cod-sp") != 0)
        CG_Error("Client/Server game mismatch: %s/%s", "cod-sp",
                 ConfigString);
    const char* v3 = CL_GetConfigString(0);
    const char* v4 = Info_ValueForKey(v3, "mapname");
    Com_sprintf(cgsGlobal.mapname, 128, "maps/%s.bsp", v4);
    SCR_UpdateScreen();
    extern void j_nullsub_33(const char* mapname);
    j_nullsub_33(cgsGlobal.mapname);
    SCR_UpdateScreen();
    memset(&dword_F63C50[0], 0, 0x60);
    currCl = NS_CLIENT;
    trap_R_ClearScene();
    SCR_UpdateScreen();
    LoadWorld(cgsGlobal.mapname);
    SCR_UpdateScreen();
    SCR_UpdateScreen();
    SoundMediaMgr_j_nullsub_91(SoundMediaMgr_sInst);
    SCR_UpdateScreen();
    CG_RegisterGraphics();
    SCR_UpdateScreen();
    SCR_UpdateScreen();
    SCR_UpdateScreen();
    for (int j = 725; j < 980; ++j)
    {
        if (j < 724)
        {
            CG_ASSERT("num >= CS_SERVER_SHADERS && num < "
                      "CS_SERVER_SHADERS + 256",
                      "c:\\cod\\code\\game\\cg_servercmds.cpp", 106);
        }
        const char* v6;
        if (j < 0)
        {
            CG_ASSERT("CG_ConfigString: bad index: %i",
                      "c:\\cod\\code\\game\\cg_main.cpp", 1215);
            v6 = nullptr;
        }
        else
        {
            v6 = CL_GetConfigString(j);
        }
        if (*v6 != 0)
            trap_R_RegisterShaderNoMip(v6, 5);
    }
    SCR_UpdateScreen();
    SCR_UpdateScreen();
    currCl = NS_CLIENT;
    if (dword_F6A290[0] == 2)
    {
        CG_MapInit(0);
        currCl = NS_CLIENT;
    }
}

// ea: 0x006B0410
void Camera::UpdatePostViewModels()
{
    if (mCamMode == 16 || mCamMode == 18)
    {
        void* ic = (void*)InteractionController::Inst(mClient);
        if ((*(unsigned char*)ic & 1) == 0)
        {
            CG_ASSERT("InteractionController::Inst(mClient)->IsFlagged("
                      "InteractionController::kHandsSet)",
                      "c:\\cod\\code\\game\\Camera.cpp", 308);
        }
        dword_F63C70[1580 * mClient] =
            InteractionController_GetHandsOrigin(ic)[0];
        dword_F63C74[1580 * mClient] =
            InteractionController_GetHandsOrigin(ic)[1];
        dword_F63C78[1580 * mClient] =
            InteractionController_GetHandsOrigin(ic)[2];
        angle[1580 * mClient] =
            InteractionController_GetHandsAngles(ic)[0];
        dword_F63CB4[1580 * mClient] =
            InteractionController_GetHandsAngles(ic)[1];
        dword_F63CB8[1580 * mClient] =
            InteractionController_GetHandsAngles(ic)[2];
    }
    UpdateAnimation();
    UpdateTween(mTweenStartPos, mTweenStartAngles);
    AnglesToAxis((const float*)&angle[1580 * mClient],
                 (float(*)[3])&dword_F63C80[1580 * mClient]);
    SaveLastPO();
    SaveLastPO();
}

// ea: 0x0068EDB0
void Camera::UpdateVehicleDriverCamPos(Entity* veh, PlayerState* ps,
                                       float extra_height_offset)
{
    void* Info = VEH_GetInfo(*(short*)((char*)veh->scr_vehicle + 0x178));
    if (!s_tagDriverHashInit)
    {
        s_tagDriverHashInit = true;
        s_tagDriverHash = HashString::CalcHash("tag_driver");
    }
    float tagMtx[16];
    if (G_DObjGetWorldTagMatrix(veh, s_tagDriverHash,
                                (DObjSkelMat*)tagMtx) == 0)
        G_DObjGetWorldTagMatrix(veh, HashString::CalcHash("tag_body"),
                                (DObjSkelMat*)tagMtx);
    float height = ((vehicle_info_full_t*)Info)->cameraFPHeightOffset
                   + extra_height_offset;
    height = (height - s_prevCamHeight)
                 * ((vehicle_info_full_t*)Info)->cameraFPHeightLerp
             + s_prevCamHeight;
    s_prevCamHeight = height;
    // result = tag origin + tag up-axis * height + tag x-axis * fwd offset
    float result[4] = {tagMtx[12] + tagMtx[8] * height
                           + tagMtx[0] * ((vehicle_info_full_t*)Info)
                                 ->cameraFPFwdOffset,
                       tagMtx[13] + tagMtx[9] * height
                           + tagMtx[1] * ((vehicle_info_full_t*)Info)
                                 ->cameraFPFwdOffset,
                       tagMtx[14] + tagMtx[10] * height
                           + tagMtx[2] * ((vehicle_info_full_t*)Info)
                                 ->cameraFPFwdOffset,
                       0.0f};
    dword_F63C70[1580 * mClient] = result[0];
    dword_F63C74[1580 * mClient] = result[1];
    dword_F63C78[1580 * mClient] = result[2];
}

// ea: 0x006A8690
void Camera::UpdateTankCam()
{
    Client* client = EntityManager::sInst->GetPlayer( mClient)->client;
    Entity* mObject = DbHandleToEntity(client->ps.mViewLockedEntity);
    dword_F63C70[1580 * mClient] = client->ps.origin.v.m128_f32[0];
    dword_F63C74[1580 * mClient] = client->ps.origin.v.m128_f32[1];
    dword_F63C78[1580 * mClient] = client->ps.origin.v.m128_f32[2];
    angle[1580 * mClient] = client->ps.viewangles[0];
    dword_F63CB4[1580 * mClient] = client->ps.viewangles[1];
    dword_F63CB8[1580 * mClient] = client->ps.viewangles[2];
    if (!s_tagTurretHashInit)
    {
        s_tagTurretHashInit = true;
        s_tagTurretHash = HashString::CalcHash("tag_turret");
    }
    float tagMtx[16];
    if (G_DObjGetWorldTagMatrix(mObject, s_tagTurretHash,
                                (DObjSkelMat*)tagMtx) != 0)
    {
        vehicle_info_full_t* info =
            (vehicle_info_full_t*)VEH_GetInfo(
                *(short*)((char*)mObject->scr_vehicle + 0x178));
        float forward[4] = {
            mObject->r.currentAngles.v.m128_f32[0],
            mObject->r.currentAngles.v.m128_f32[1]
                + *(float*)((char*)mObject->scr_vehicle + 0x3F4),
            mObject->r.currentAngles.v.m128_f32[2],
            mObject->r.currentAngles.v.m128_f32[3]};
        float axis[3][3];
        Entity* Player =
            EntityManager::sInst->GetPlayer( mClient);
        if (IsPlayerFullySeatedInVehicle(Player))
            AnglesToAxis((const float*)&angle[1580 * mClient],
                         axis);
        else
            AnglesToAxis((const float*)forward, axis);
        VectorNormalize(axis[0]);
        float v11 = 0.0f;
        if (IsPlayerFullySeatedInVehicle(
                EntityManager::sInst->GetPlayer( mClient))
            && client->ps.vehPos == 0)
        {
            v11 = angle[1580 * mClient];
            if (v11 < -40.0f)
                v11 = -40.0f;
            else if (v11 > 40.0f)
                v11 = 40.0f;
        }
        float v15 = info->turretPitchUp + v11;
        float v16;
        if (v15 >= 0.0f)
        {
            v16 = info->turretPitchUp * 2.0f;
            if (v15 <= v16)
                v16 = v15;
        }
        else
        {
            v16 = 0.0f;
        }
        float v17 = v16 / (info->turretPitchUp * 2.0f);
        float v18 = v17 * info->turretPitchFactor
                    - info->turretPitchBias * info->turretPitchFactor;
        float v19 = info->turretPitchScale;
        float turretPos[3] = {tagMtx[9] + v18 * axis[0][0],
                              tagMtx[10] + v18 * axis[0][1],
                              tagMtx[11] + v18 * axis[0][2]};
        float v23 = v17 / v19;
        if (v19 <= v17)
            v23 = 1.0f;
        float v24 = 0.0f - info->cameraChaseRadiusOuter * v23;
        float finalPos[3] = {turretPos[0] + info->turretCamOffset * axis[1][0]
                                 + v24 * axis[0][0],
                             turretPos[1] + info->turretCamOffset * axis[1][1]
                                 + v24 * axis[0][1],
                             turretPos[2] + info->turretCamOffset * axis[1][2]
                                 + v24 * axis[0][2]};
        Entity* v31 = EntityManager::sInst->GetPlayer( mClient);
        float delta[4];
        if (IsPlayerFullySeatedInVehicle(v31))
        {
            if (mVehPrevAnglesTime != 0
                && mVehPrevAnglesTime > cgGlobal.time - 500)
            {
                for (int i = 0; i < 4; ++i)
                    delta[i] = mObject->r.currentAngles.v.m128_f32[i]
                               - mVehPrevAngles.v.m128_f32[i];
            }
            else
            {
                memset(delta, 0, sizeof(delta));
            }
        }
        else
        {
            angle[1580 * mClient] = forward[0];
            dword_F63CB4[1580 * mClient] = forward[1];
            dword_F63CB8[1580 * mClient] = forward[2];
        }
        mVehPrevAngles.v = mObject->r.currentAngles.v;
        mVehPrevAnglesTime = cgGlobal.time;
        if (IsPlayerFullySeatedInVehicle(
                EntityManager::sInst->GetPlayer( mClient)))
        {
            *(float*)((char*)mObject->scr_vehicle + 0x434) =
                AngleNormalize360(*(float*)((char*)mObject->scr_vehicle
                                            + 0x434)
                                  - delta[1]);
        }
        float mins[3] = {-1.0f, -1.0f, -1.0f};
        float maxs[3] = {1.0f, 1.0f, 1.0f};
        trace_t tr;
        collision_context_t ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
        ctx.pass_entity1.mHandle.mVal =
            EntityManager::sInst->GetPlayer( mClient)->mHandle
                .mHandle.mVal;
        ctx.pass_entity2.mHandle.mVal = mObject->mHandle.mHandle.mVal;
        CG_Trace(&tr, (const math::Position3*)turretPos,
                 (const math::Position3*)mins, (const math::Position3*)maxs,
                 (const math::Position3*)finalPos, &ctx);
        if (tr.normal.v.m128_f32[1] < 1.0f)
        {
            finalPos[0] = tr.endpos.v.m128_f32[0];
            finalPos[1] = tr.endpos.v.m128_f32[1];
            finalPos[2] = tr.endpos.v.m128_f32[2];
        }
        dword_F63C70[1580 * mClient] = finalPos[0];
        dword_F63C74[1580 * mClient] = finalPos[1];
        dword_F63C78[1580 * mClient] = finalPos[2];
        UpdateTankShakeRumble(mObject, false);
    }
}

// ea: 0x006A8000
void Camera::UpdateTankCommanderCam()
{
    Client* client = EntityManager::sInst->GetPlayer( mClient)->client;
    Entity* mObject = DbHandleToEntity(client->ps.mViewLockedEntity);
    float vehAngles[4] = {mObject->r.currentAngles.v.m128_f32[0],
                          mObject->r.currentAngles.v.m128_f32[1],
                          mObject->r.currentAngles.v.m128_f32[2],
                          mObject->r.currentAngles.v.m128_f32[3]};
    vehAngles[1] += *(float*)((char*)mObject->scr_vehicle + 0x3F4);
    float delta[4];
    if (mVehPrevAnglesTime != 0 && mVehPrevAnglesTime > cgGlobal.time - 500)
    {
        for (int i = 0; i < 4; ++i)
            delta[i] = mObject->r.currentAngles.v.m128_f32[i]
                       - mVehPrevAngles.v.m128_f32[i];
    }
    else
    {
        memset(delta, 0, sizeof(delta));
    }
    mVehPrevAngles.v = mObject->r.currentAngles.v;
    delta[0] *= gTankDriverAnglesFrac;
    delta[2] *= gTankDriverAnglesFrac;
    mVehPrevAnglesTime = cgGlobal.time;
    mTweenStartAngles.v = _mm_add_ps(mTweenStartAngles.v,
                                     _mm_loadu_ps(delta));
    int port = dword_F6A28C[802 * mClient];
    if (*(bool*)((char*)controller::inst() + 0x1C))  // is_locked
        port = *(int*)((char*)controller::inst() + 0x18);  // locked_port
    bool v16 = PadAliasMgr_GetButtonValue(
                   (char*)PadAliasMgr_sInst + 0x148, port,
                   3 /* kPadAliasButtonAlignTurret */)
               > g_vehicle_button_threshold;
    if (unk_F6A294[3208 * mClient] == 0 || v16)
    {
        float angle = *(float*)((char*)mObject->scr_vehicle + 0x434)
                      - delta[1];
        *(float*)((char*)mObject->scr_vehicle + 0x434) =
            AngleNormalize360(angle);
    }
    dword_F63C70[1580 * mClient] = client->ps.origin.v.m128_f32[0];
    dword_F63C74[1580 * mClient] = client->ps.origin.v.m128_f32[1];
    dword_F63C78[1580 * mClient] = client->ps.origin.v.m128_f32[2];
    angle[1580 * mClient] = client->ps.viewangles[0];
    dword_F63CB4[1580 * mClient] = client->ps.viewangles[1];
    dword_F63CB8[1580 * mClient] = client->ps.viewangles[2];
    if (!s_tagTurretHashInit)
    {
        s_tagTurretHashInit = true;
        s_tagTurretHash = HashString::CalcHash("tag_turret");
    }
    float tagMtx[16];
    if (G_DObjGetWorldTagMatrix(mObject, s_tagTurretHash,
                                (DObjSkelMat*)tagMtx) != 0)
    {
        vehicle_info_full_t* info =
            (vehicle_info_full_t*)VEH_GetInfo(
                *(short*)((char*)mObject->scr_vehicle + 0x178));
        dword_F63C70[1580 * mClient] +=
            info->cameraFPFwdOffset * tagMtx[2];
        dword_F63C74[1580 * mClient] +=
            info->cameraFPFwdOffset * tagMtx[6];
        dword_F63C78[1580 * mClient] +=
            info->cameraFPFwdOffset * tagMtx[10];
        dword_F63C70[1580 * mClient] +=
            info->cameraFPHeightOffset * tagMtx[5];
        dword_F63C74[1580 * mClient] +=
            info->cameraFPHeightOffset * tagMtx[9];
        dword_F63C78[1580 * mClient] +=
            info->cameraFPHeightOffset * tagMtx[13];
        float mins[3] = {-1.0f, -1.0f, -1.0f};
        float maxs[3] = {1.0f, 1.0f, 1.0f};
        float start[3] = {
            tagMtx[8] + info->cameraFPHeightOffset * tagMtx[5],
            tagMtx[9] + info->cameraFPHeightOffset * tagMtx[9],
            tagMtx[10] + info->cameraFPHeightOffset * tagMtx[13]};
        collision_context_t ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
        ctx.pass_entity1.mHandle.mVal =
            EntityManager::sInst->GetPlayer( mClient)->mHandle
                .mHandle.mVal;
        ctx.pass_entity2.mHandle.mVal = mObject->mHandle.mHandle.mVal;
        trace_t tr;
        CG_Trace(&tr, (const math::Position3*)start,
                 (const math::Position3*)mins, (const math::Position3*)maxs,
                 (const math::Position3*)&dword_F63C70[1580 * mClient], &ctx);
        if (tr.normal.v.m128_f32[1] < 1.0f)
        {
            dword_F63C70[1580 * mClient] = tr.endpos.v.m128_f32[0];
            dword_F63C74[1580 * mClient] = tr.endpos.v.m128_f32[1];
            dword_F63C78[1580 * mClient] = tr.endpos.v.m128_f32[2];
        }
        Entity* Player =
            EntityManager::sInst->GetPlayer( mClient);
        if (!IsPlayerFullySeatedInVehicle(Player))
        {
            angle[1580 * mClient] = vehAngles[0];
            dword_F63CB4[1580 * mClient] = vehAngles[1];
            dword_F63CB8[1580 * mClient] = vehAngles[2];
        }
        UpdateTankShakeRumble(mObject, true);
    }
}

// ea: 0x00699020
void Camera::UpdateTankCamAngles(Entity* veh, PlayerState* ps)
{
    float delta[4];
    if (mVehPrevAnglesTime != 0 && mVehPrevAnglesTime > cgGlobal.time - 500)
    {
        for (int i = 0; i < 4; ++i)
            delta[i] = veh->r.currentAngles.v.m128_f32[i]
                       - mVehPrevAngles.v.m128_f32[i];
    }
    else
    {
        memset(delta, 0, sizeof(delta));
    }
    float playerAxis[4] = {
        AngleDelta(veh->r.currentAngles.v.m128_f32[0], 0.0f) * 0.64999998f,
        veh->r.currentAngles.v.m128_f32[1],
        AngleDelta(veh->r.currentAngles.v.m128_f32[2], 0.0f) * 0.64999998f,
        0.0f};
    float viewAxis[3][3];
    AnglesToAxis((const float*)playerAxis, viewAxis);
    int port = dword_F6A28C[802 * currCl];
    float vehOffsetAngles[4] = {ps->viewangles[0] - mPrevAngles.v.m128_f32[0],
                                ps->viewangles[1] - mPrevAngles.v.m128_f32[1],
                                ps->viewangles[2] - mPrevAngles.v.m128_f32[2],
                                0.0f};
    if (*(bool*)((char*)controller::inst() + 0x1C))
        port = *(int*)((char*)controller::inst() + 0x18);
    bool v9 = PadAliasMgr_GetButtonValue(
                  (char*)PadAliasMgr_sInst + 0x148, port,
                  3 /* kPadAliasButtonAlignTurret */)
              != 0;
    if (g_vehControlMode.integer == 0 || v9)
        vehOffsetAngles[1] -= delta[1];
    mTankRelativeAngles.v = _mm_add_ps(
        mTankRelativeAngles.v, _mm_loadu_ps(vehOffsetAngles));
    float vehAxis[3][3];
    AnglesToAxis((const float*)&mTankRelativeAngles, vehAxis);
    float out[3][3];
    MatrixMultiply(vehAxis, viewAxis, out);
    float newViewAngles[3];
    AxisToAngles(out, newViewAngles);
    newViewAngles[0] = AngleNormalize180(newViewAngles[0]);
    newViewAngles[1] = AngleNormalize180(newViewAngles[1]);
    angle[1580 * currCl] = newViewAngles[0];
    angle[1580 * currCl + 1] = newViewAngles[1];
    angle[1580 * currCl + 2] = newViewAngles[2];
    angle[1580 * currCl + 3] = 0.0f;
    SetPlayerAngles(newViewAngles);
}

// ea: 0x006A5A10
ECameraModes Camera::CalcCamMode()
{
    void* v2 = (void*)InteractionController::Inst(mClient);
    int mode = InteractionController_GetCameraMode(v2);
    if (mode != -1)
        return (ECameraModes)InteractionController_GetCameraMode(v2);
    if (*(int*)((char*)EntityManager::sInst->GetPlayer( mClient)
                    ->client
                + 0x7E8)
        != 0)
        return CAM_NORMAL_FIRST;
    if (gSceneAnimCamera)
        return CAM_SCENE_ANIMATED;
    Client* client =
        EntityManager::sInst->GetPlayer( mClient)->client;
    if (client->ps.pm_type == 5)
        return CAM_INTERMISSION;
    if (client->ps.eFlags < 0)
        return CAM_MP_DEATH_CAMERA_NO_KILLER;
    if (*(int*)(dword_F62960[1580 * mClient] + 52) >= 6)
        return CAM_MP_DEATH_CAMERA;
    Entity* Player = GetPlayer(mClient);
    if (!IsPlayerFullySeatedInVehicle(Player))
    {
        void* ic = (void*)InteractionController::Inst(mClient);
        if (*(void**)((char*)ic + 4) != nullptr)
            InteractionController_EndInteraction(ic, 1);
        if ((client->ps.vehType != 2
             && *(bool*)((char*)GetPlayer(mClient)->client + 0xAD8))
            || *(bool*)((char*)GetPlayer(mClient)->client + 0xAE0))
            return CAM_VEHICLE_ANIM;
        int vehPos = client->ps.vehPos;
        if (vehPos >= 8)
            return CAM_VEHICLE_ANIM_FIRST;
        if (client->ps.vehType != 2)
        {
            if (vehPos != 0 || mVehicleCamMode != VEH_MODE_CHASECAM)
                return CAM_VEHICLE_ANIM;
            return CAM_VEHICLE_THIRD;
        }
        if (!*(bool*)((char*)GetPlayer(mClient)->client + 0xAD8))
            return CAM_VEHICLE_TANK;
    }
    Entity* v9 = DbHandleToEntity(client->ps.mViewLockedEntity);
    if ((client->ps.eFlags & 0x6000) != 0 && v9 != nullptr
        && *(int*)((char*)client + 0x49C) != 0)
        return CAM_TURRET;
    if ((client->ps.eFlags & 0x100000) == 0 || v9 == nullptr
        || v9->scr_vehicle == nullptr)
    {
        if (client->ps.pm_type == 1)
            return CAM_TURRET_FIRST;
        return (ECameraModes)(dword_F6355C[1580 * mClient] != 0);
    }
    if (client->ps.vehPos == 0)
    {
        int v16 = dword_F6A28C[802 * mClient];
        if (controller_button_pressed(controller::inst(), v16,
                                      12 /* R3 */))
        {
            if (mVehicleCamMode != VEH_MODE_FIRSTPERSON)
            {
                mVehicleCamMode = VEH_MODE_FIRSTPERSON;
            }
            else
            {
                int mClient2 = mClient;
                mVehicleCamMode = VEH_MODE_CHASECAM;
                if ((*(unsigned char*)((char*)GetPlayer(mClient2)->client
                                       + 0x800)
                     & 8)
                    != 0)
                {
                    mVehicleCamMode =
                        mVehicleCamMode == VEH_MODE_FIRSTPERSON;
                }
                void* ic2 = (void*)InteractionController::Inst(mClient);
                if (*(void**)((char*)ic2 + 4) != nullptr)
                    InteractionController_EndInteraction(ic2, 1);
            }
        }
    }
    if (cg_altTankCam.integer == 0 || client->ps.vehType != 2)
    {
        if (*(void**)((char*)v9->scr_vehicle + 0x518) != nullptr
            && client->ps.vehType == 1)
        {
            if (dword_F6355C[1580 * mClient] == 0)
            {
                int v13 = client->ps.vehPos;
                if (v13 != 0)
                    return v13 != 1 ? CAM_VEHICLE_PASSENGER : CAM_VEHICLE_GUNNER;
                if (mVehicleCamMode == VEH_MODE_FIRSTPERSON)
                {
                    if (mTweenTime >= mTweenDuration
                        && mCamMode == CAM_VEHICLE_FIRST
                        && *(void**)((char*)InteractionController::Inst(mClient)
                                     + 4)
                               == nullptr)
                    {
                        void* VehicleInfo = G_GetVehicleInfo(v9);
                        int v18 = CurPakId();
                        InteractionController_StartInteraction(
                            (void*)InteractionController::Inst(mClient), v9,
                            (const char*)((char*)VehicleInfo + 0x284), v18);
                    }
                    return CAM_VEHICLE_FIRST;
                }
            }
            return CAM_VEHICLE_THIRD;
        }
        if (client->ps.vehPos == 1)
            return CAM_VEHICLE_GUNNER;
        if (client->ps.pm_type == 1)
            return CAM_TURRET_FIRST;
        return (ECameraModes)(dword_F6355C[1580 * mClient] != 0);
    }
    int v12 = client->ps.vehPos;
    if (v12 == 1)
        return CAM_VEHICLE_GUNNER;
    if (v12 == 6)
        return CAM_VEHICLE_GUNNER_CROUCHED;
    if (*(void**)((char*)v9->scr_vehicle + 0x518) == nullptr
        || mVehicleCamMode != VEH_MODE_FIRSTPERSON)
        return CAM_VEHICLE_TANK;
    return CAM_VEHICLE_TANK_COMMANDER;
}

// ea: 0x006A68A0
void Camera::UpdateVehicleDriverCamAngles(Entity* veh, PlayerState* ps)
{
    float vehAxis[3][3];
    float playerAxis[3][3];
    vehAxis[2][1] = veh->r.currentAngles.v.m128_f32[1];
    vehAxis[2][2] =
        AngleDelta(veh->r.currentAngles.v.m128_f32[2], 0.0f) * 0.47999999f;
    vehAxis[2][0] =
        AngleDelta(veh->r.currentAngles.v.m128_f32[0], 0.0f);
    AnglesToAxis((const float*)vehAxis[2], playerAxis);
    float vehOffsetAngles[4] = {0.0f, 0.0f, -90.0f, -90.0f};
    float minClamp[3] = {5.0f, 40.0f, 0.0f};
    if (ps->vehSubType == 2
        && *(void**)((char*)veh->scr_vehicle + 0x518) != nullptr)
    {
        Entity* mObject = DbHandleToEntity(veh->r.mOwner.mHandle.mVal);
        int PlayerIndex = Entity_GetPlayerIndex(mObject);
        float Rotation =
            InteractionController_GetRotation(
                (void*)InteractionController::Inst(PlayerIndex));
        float angle = mPrevAngles.v.m128_f32[1]
                      - veh->r.currentAngles.v.m128_f32[1];
        float newViewAngles3 = Rotation * 0.028571429f;
        float v40 = -AngleNormalize180(angle);
        float v14 = fabsf(v40);
        if ((newViewAngles3 < 0.0f) == (v40 < 0.0f))
        {
            float v15 = v14 * 0.06666667f;
            v40 = v15;
            float v16 = v40;
            float v17;
            if (v15 >= 0.0f)
            {
                v17 = 1.0f;
                if (v40 <= 1.0f)
                    v17 = v40;
            }
            else
            {
                v17 = 0.0f;
            }
            newViewAngles3 = fabsf(newViewAngles3);
            float v18 = newViewAngles3 * 4.0f;
            if ((newViewAngles3 * v17 * 13.0f) + v18 >= 0.0f)
            {
                float v19;
                if (v40 >= 0.0f)
                {
                    v19 = 1.0f;
                    if (v40 <= 1.0f)
                        v19 = v40;
                }
                else
                {
                    v19 = 0.0f;
                }
                if (((newViewAngles3 * v19) * 13.0f) + v18 <= 17.0f)
                {
                    if (v40 >= 0.0f)
                    {
                        if (v40 > 1.0f)
                            v16 = 1.0f;
                    }
                    else
                    {
                        v16 = 0.0f;
                    }
                    minClamp[0] =
                        13.0f - (((newViewAngles3 * v16) * 13.0f) + v18);
                }
                else
                {
                    minClamp[0] = 13.0f - 17.0f;
                }
            }
            else
            {
                minClamp[0] = 13.0f - 0.0f;
            }
        }
        else
        {
            float v20 = v14 * 0.033333335f;
            v40 = v20;
            float v21;
            if (v20 >= 0.0f)
            {
                v21 = v40;
                v40 = 1.0f;
                if (v21 <= 1.0f)
                    v40 = v21;
            }
            else
            {
                v40 = 0.0f;
            }
            minClamp[0] = 13.0f
                          - fabsf(newViewAngles3) * (1.0f - v40) * 3.0f;
        }
        if (minClamp[0] == 0.0f)
            minClamp[0] = -0.1f;
        vehOffsetAngles[3] = -30.0f;
        minClamp[1] = 30.0f;
    }
    CG_ClampViewAngles(ps, vec3_origin, &vehOffsetAngles[2], minClamp);
    float viewAxis[3][3];
    AnglesToAxis((const float*)ps->viewangles, viewAxis);
    float v30[3][3];
    MatrixMultiply(viewAxis, playerAxis, v30);
    float outAngles[3];
    AxisToAngles(v30, outAngles);
    outAngles[0] = AngleNormalize180(outAngles[0]);
    outAngles[1] = AngleNormalize180(outAngles[1]);
    UpdateVehicleDriverSteerLookAhead(veh);
    outAngles[1] = mSteerYawOffset + outAngles[1];
    if (mTweenDuration <= mTweenTime)
    {
        InterpolateAngles((math::Position3*)&angle[1580 * mClient],
                          (const math::Position3*)&mPrevAngles.v.m128_f32[0],
                          (const math::Position3*)outAngles,
                          ServerTime_sInst.mTickDelta * 15.0f);
    }
    else
    {
        angle[1580 * mClient] = outAngles[0];
        angle[1580 * mClient + 1] = outAngles[1];
        angle[1580 * mClient + 2] = outAngles[2];
        angle[1580 * mClient + 3] = 0.0f;
    }
    __m128 v28;
    if (mVehPrevAnglesTime != 0 && mVehPrevAnglesTime > cgGlobal.time - 500)
    {
        v28 = _mm_sub_ps(veh->r.currentAngles.v, mVehPrevAngles.v);
    }
    else
    {
        outAngles[0] = 0.0f;
        outAngles[1] = 0.0f;
        outAngles[2] = 0.0f;
        v28 = _mm_setzero_ps();
    }
    mVehPrevAngles.v = veh->r.currentAngles.v;
    mVehPrevAnglesTime = cgGlobal.time;
    mTweenStartAngles.v = _mm_add_ps(mTweenStartAngles.v, v28);
}

// ea: 0x006AF000
void Camera::UpdateVehicleDriverCamThird()
{
    if (special_tween_bool)
    {
        if (mTweenTime < 0.40000001f)
        {
            UpdateVehicleDriverCam(30.0f);
            return;
        }
        StartTween(0.5f, false);
        special_tween_bool = false;
    }
    Client* client =
        EntityManager::sInst->GetPlayer( mClient)->client;
    Entity* mObject = DbHandleToEntity(client->ps.mViewLockedEntity);
    UpdateVehicleDriverCamAnglesInput(mObject, &client->ps);
    vehicle_info_full_t* v7 =
        (vehicle_info_full_t*)VEH_GetInfo(
            *(short*)((char*)mObject->scr_vehicle + 0x178));
    float chaseOffset[4] = {v7->cameraChaseOffsetX, v7->cameraChaseOffsetY,
                            v7->cameraChaseOffsetZ, 0.0f};
    float start[4];
    start[0] = mObject->r.currentOrigin.v.m128_f32[0] + chaseOffset[0];
    start[1] = mObject->r.currentOrigin.v.m128_f32[1] + chaseOffset[1];
    start[2] = mObject->r.currentOrigin.v.m128_f32[2] + chaseOffset[2];
    float dist2 = (start[0] - mPrevViewPos.v.m128_f32[0])
                      * (start[0] - mPrevViewPos.v.m128_f32[0])
                  + (start[1] - mPrevViewPos.v.m128_f32[1])
                        * (start[1] - mPrevViewPos.v.m128_f32[1])
                  + (start[2] - mPrevViewPos.v.m128_f32[2])
                        * (start[2] - mPrevViewPos.v.m128_f32[2]);
    float dist = sqrtf(dist2);
    float v12 = mObject->speed * 0.0022222223f;
    if (v12 < 0.0f)
        v12 = 0.0f;
    else if (v12 > 1.0f)
        v12 = 1.0f;
    float pitch = 14.0f;  // (v12 * 0.0) + 14.0
    float yaw;
    float roll = 0.0f;
    if (mVehInputState != INPUT_NONE)
    {
        Entity* Player =
            EntityManager::sInst->GetPlayer( mClient);
        float v17, v18;
        if (IsPlayerFullySeatedInVehicle(Player)
            || client->ps.vehPos >= 8)
        {
            v17 = client->ps.viewangles[0];
            v18 = client->ps.viewangles[1];
            roll = client->ps.viewangles[2];
        }
        else
        {
            v17 = 0.0f;
            v18 = 0.0f;
            roll = 0.0f;
        }
        yaw = mObject->r.currentAngles.v.m128_f32[1] + v18;
        pitch = v17 + pitch;
    }
    else
    {
        float dir[3] = {
            (start[0] - mPrevViewPos.v.m128_f32[0]) / dist,
            (start[1] - mPrevViewPos.v.m128_f32[1]) / dist,
            (start[2] - mPrevViewPos.v.m128_f32[2]) / dist};
        float signedAngles[3];
        vectosignedangles(dir, signedAngles);
        yaw = signedAngles[1];
        pitch = client->ps.viewangles[0] + pitch;
        void* mRBVeh = *(void**)((char*)mObject->scr_vehicle + 0x518);
        if (mRBVeh != nullptr && *(float*)((char*)mRBVeh + 0x254) > 0.0f)
        {
            LerpAngle(yaw, mObject->r.currentAngles.v.m128_f32[1],
                      ServerTime_sInst.mTickDelta * 1.4f);
            yaw = dist;
        }
    }
    if (mTweenDuration > mTweenTime)
    {
        pitch = client->ps.viewangles[0] + 14.0f;
        yaw = mObject->r.currentAngles.v.m128_f32[1] + client->ps.viewangles[1];
        dist = 250.0f;
    }
    angle[1580 * mClient] = pitch;
    angle[1580 * mClient + 1] = yaw;
    angle[1580 * mClient + 2] = roll;
    angle[1580 * mClient + 3] = 0.0f;
    float sinY, cosY, sinX, cosX;
    FastSinCos(angle[1580 * mClient + 1] * 0.017453292f, &sinY, &cosY);
    FastSinCos(angle[1580 * mClient] * 0.017453292f, &sinX, &cosX);
    float dir[3] = {cosX * cosY, cosX * sinY, -sinX};
    float finalPos[3] = {start[0] - dir[0] * dist,
                         start[1] - dir[1] * dist,
                         start[2] - dir[2] * dist};
    if (dist > v7->cameraChaseRadiusOuter)
    {
        float adjust = dist - v7->cameraChaseRadiusOuter;
        finalPos[0] += dir[0] * adjust;
        finalPos[1] += dir[1] * adjust;
        finalPos[2] += dir[2] * adjust;
    }
    else if (v7->cameraChaseRadiusInner > dist)
    {
        float adjust = 0.0f - (v7->cameraChaseRadiusInner - dist);
        finalPos[0] += dir[0] * adjust;
        finalPos[1] += dir[1] * adjust;
        finalPos[2] += dir[2] * adjust;
    }
    float mins[3] = {-1.0f, -1.0f, -1.0f};
    float maxs[3] = {1.0f, 1.0f, 1.0f};
    collision_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
    ctx.pass_entity1.mHandle.mVal = client->ps.mViewLockedEntity;
    ctx.pass_entity2.mHandle.mVal = mObject->mHandle.mHandle.mVal;
    trace_t tr;
    CG_Trace(&tr, (const math::Position3*)start, (const math::Position3*)mins,
             (const math::Position3*)maxs, (const math::Position3*)finalPos,
             &ctx);
    if (tr.normal.v.m128_f32[1] < 1.0f)
    {
        finalPos[0] = tr.endpos.v.m128_f32[0];
        finalPos[1] = tr.endpos.v.m128_f32[1];
        finalPos[2] = tr.endpos.v.m128_f32[2];
    }
    // height smoothing (chase cam)
    float height = finalPos[2];
    static float s_prevChaseHeight;
    height = (height - s_prevChaseHeight) * v7->cameraFPHeightLerp
             + s_prevChaseHeight;
    s_prevChaseHeight = height;
    finalPos[2] = height;
    dword_F63C70[1580 * mClient] = finalPos[0];
    dword_F63C74[1580 * mClient] = finalPos[1];
    dword_F63C78[1580 * mClient] = finalPos[2];
    mTweenStartPos.v = _mm_add_ps(
        mTweenStartPos.v,
        _mm_add_ps(mObject->r.currentOrigin.v,
                   _mm_sub_ps(_mm_setzero_ps(), mVehPrevOrigin.v)));
    mVehPrevOrigin.v = mObject->r.currentOrigin.v;
}

// ea: 0x006A7910
void Camera::UpdateMPDeathCameraNoKiller()
{
    Client* client =
        EntityManager::sInst->GetPlayer( mClient)->client;
    float eye[4] = {client->ps.origin.v.m128_f32[0],
                    client->ps.origin.v.m128_f32[1],
                    client->ps.origin.v.m128_f32[2],
                    client->ps.origin.v.m128_f32[3]};
    eye[2] = eye[2] + 10.0f;
    float yaw = client->ps.viewangles[1];
    float roll = client->ps.viewangles[2];
    float sinYaw, cosYaw, sinPitch, cosPitch;
    FastSinCos(yaw * 0.017453292f, &sinYaw, &cosYaw);
    FastSinCos(0.34906584f, &sinPitch, &cosPitch);
    float range = cg_thirdPersonRange.value;
    float target[4] = {eye[0] + (cosPitch * cosYaw) * (512.0f - range),
                       eye[1] + (cosPitch * sinYaw) * (512.0f - range),
                       eye[2] + (0.0f - sinPitch) * (512.0f - range), 0.0f};
    float delta[3] = {target[0] - mPrevViewPos.v.m128_f32[0],
                      target[1] - mPrevViewPos.v.m128_f32[1],
                      target[2] - mPrevViewPos.v.m128_f32[2]};
    float newAngles[3];
    vectoangles(delta, newAngles);
    angle[1580 * mClient] = newAngles[0];
    angle[1580 * mClient + 1] = yaw;
    angle[1580 * mClient + 2] = roll;
    angle[1580 * mClient + 3] = 0.0f;
    float mins[3] = {-1.0f, -1.0f, -1.0f};
    float maxs[3] = {1.0f, 1.0f, 1.0f};
    collision_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
    ctx.pass_entity1.mHandle.mVal = 0;
    ctx.pass_entity2.mHandle.mVal = 0x80206D + 6;
    trace_t tr;
    CG_Trace(&tr, (const math::Position3*)eye, (const math::Position3*)mins,
             (const math::Position3*)maxs, (const math::Position3*)target,
             &ctx);
    float result[4];
    if (tr.normal.v.m128_f32[1] >= 1.0f)
    {
        result[0] = target[0];
        result[1] = target[1];
        result[2] = target[2];
        result[3] = eye[3];
    }
    else
    {
        result[0] = tr.endpos.v.m128_f32[0];
        result[1] = tr.endpos.v.m128_f32[1];
        result[2] = tr.endpos.v.m128_f32[2];
        result[3] = tr.endpos.v.m128_f32[3];
    }
    dword_F63C70[1580 * mClient] = result[0];
    dword_F63C74[1580 * mClient] = result[1];
    dword_F63C78[1580 * mClient] = result[2];
    dword_F63C70[1580 * mClient + 3] = result[3];
    mGlobalEffectNode.mDof.r1 = 100.0f;
    mGlobalEffectNode.mDof.r2 = 500.0f;
    mGlobalEffectNode.mDof.r3 = the_rate;
    mGlobalEffectNode.mDof.on = 1;
    mGlobalEffectNode.mDof.r4 = 100000.0f;
}

// ea: 0x006A7C50
void Camera::UpdateMPDeathCamera()
{
    Client* client =
        EntityManager::sInst->GetPlayer( mClient)->client;
    unsigned int killer = *(unsigned int*)((char*)client + 0x90);
    Entity* mObject = DbHandleToEntity(killer);
    if (mObject != nullptr)
    {
        float eye[4] = {client->ps.origin.v.m128_f32[0],
                        client->ps.origin.v.m128_f32[1],
                        client->ps.origin.v.m128_f32[2],
                        client->ps.origin.v.m128_f32[3]};
        float yaw = client->ps.viewangles[1];
        float roll = client->ps.viewangles[2];
        float enemyPos[4] = {mObject->r.currentOrigin.v.m128_f32[0],
                             mObject->r.currentOrigin.v.m128_f32[1],
                             mObject->r.currentOrigin.v.m128_f32[2],
                             mObject->r.currentOrigin.v.m128_f32[3]};
        float delta[3] = {enemyPos[0] - eye[0], enemyPos[1] - eye[1],
                          enemyPos[2] - eye[2]};
        float newAngles[3];
        vectoangles(delta, newAngles);
        newAngles[0] = 20.0f;
        float sinYaw, cosYaw, sinPitch, cosPitch;
        FastSinCos(yaw * 0.017453292f, &sinYaw, &cosYaw);
        FastSinCos(0.34906584f, &sinPitch, &cosPitch);
        float range = cg_thirdPersonRange.value;
        float target[4] = {eye[0] + (cosPitch * cosYaw) * (512.0f - range),
                           eye[1] + (cosPitch * sinYaw) * (512.0f - range),
                           eye[2] + (0.0f - sinPitch) * (512.0f - range),
                           0.0f};
        float delta2[3] = {enemyPos[0] - mPrevViewPos.v.m128_f32[0],
                           enemyPos[1] - mPrevViewPos.v.m128_f32[1],
                           enemyPos[2] - mPrevViewPos.v.m128_f32[2]};
        float dist = sqrtf(delta2[0] * delta2[0] + delta2[1] * delta2[1]
                           + delta2[2] * delta2[2]);
        vectoangles(delta2, newAngles);
        angle[1580 * mClient] = newAngles[0];
        angle[1580 * mClient + 1] = yaw;
        angle[1580 * mClient + 2] = roll;
        angle[1580 * mClient + 3] = 0.0f;
        float mins[3] = {-1.0f, -1.0f, -1.0f};
        float maxs[3] = {1.0f, 1.0f, 1.0f};
        collision_context_t ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
        ctx.pass_entity1.mHandle.mVal = 0;
        ctx.pass_entity2.mHandle.mVal = 0x80206D + 6;
        trace_t tr;
        CG_Trace(&tr, (const math::Position3*)eye, (const math::Position3*)mins,
                 (const math::Position3*)maxs, (const math::Position3*)target,
                 &ctx);
        float result[4];
        if (tr.normal.v.m128_f32[1] >= 1.0f)
        {
            result[0] = target[0];
            result[1] = target[1];
            result[2] = target[2];
            result[3] = eye[3];
        }
        else
        {
            result[0] = tr.endpos.v.m128_f32[0];
            result[1] = tr.endpos.v.m128_f32[1];
            result[2] = tr.endpos.v.m128_f32[2];
            result[3] = tr.endpos.v.m128_f32[3];
        }
        dword_F63C70[1580 * mClient] = result[0];
        dword_F63C74[1580 * mClient] = result[1];
        dword_F63C78[1580 * mClient] = result[2];
        dword_F63C70[1580 * mClient + 3] = result[3];
        mGlobalEffectNode.mDof.r1 = 50.0f;
        mGlobalEffectNode.mDof.r3 = 1.0f;
        mGlobalEffectNode.mDof.on = 1;
        mGlobalEffectNode.mDof.r2 = dist + 150.0f;
        mGlobalEffectNode.mDof.r4 = 100000.0f;
    }
    else
    {
        UpdateMPDeathCameraNoKiller();
    }
}

// ea: 0x006A8D50
void Camera::UpdateDeathCamera()
{
    Entity* Player = EntityManager::sInst->GetPlayer( mClient);
    if (Player != nullptr)
    {
        Client* client = Player->client;
        if (Player->enemy != nullptr)
        {
            float eye[4] = {client->ps.origin.v.m128_f32[0],
                            client->ps.origin.v.m128_f32[1],
                            client->ps.origin.v.m128_f32[2],
                            client->ps.origin.v.m128_f32[3]};
            float enemyPos[4] = {Player->enemy->r.currentOrigin.v.m128_f32[0],
                                 Player->enemy->r.currentOrigin.v.m128_f32[1],
                                 Player->enemy->r.currentOrigin.v.m128_f32[2],
                                 Player->enemy->r.currentOrigin.v.m128_f32[3]};
            float delta[3] = {enemyPos[0] - eye[0], enemyPos[1] - eye[1],
                              enemyPos[2] - eye[2]};
            float newAngles[3];
            vectoangles(delta, newAngles);
            newAngles[0] = 20.0f;
            float fwd[3];
            AnglesToForward(*(const math::Position3*)newAngles, *(math::Dir3*)fwd);
            float range = cg_thirdPersonRange.value;
            float target[4] = {eye[0] + fwd[0] * range,
                               eye[1] + fwd[1] * range,
                               eye[2] + fwd[2] * range, 0.0f};
            float delta2[3] = {enemyPos[0] - mPrevViewPos.v.m128_f32[0],
                               enemyPos[1] - mPrevViewPos.v.m128_f32[1],
                               enemyPos[2] - mPrevViewPos.v.m128_f32[2]};
            float dist = sqrtf(delta2[0] * delta2[0] + delta2[1] * delta2[1]
                               + delta2[2] * delta2[2]);
            vectoangles(delta2, newAngles);
            newAngles[2] = 90.0f;
            angle[1580 * mClient] = newAngles[0];
            angle[1580 * mClient + 1] = newAngles[1];
            angle[1580 * mClient + 2] = newAngles[2];
            angle[1580 * mClient + 3] = 0.0f;
            float mins[3] = {-1.0f, -1.0f, -1.0f};
            float maxs[3] = {1.0f, 1.0f, 1.0f};
            collision_context_t ctx;
            memset(&ctx, 0, sizeof(ctx));
            ctx.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
            ctx.pass_entity1.mHandle.mVal = 0;
            ctx.pass_entity2.mHandle.mVal = 0x80206D + 6;
            trace_t tr;
            CG_Trace(&tr, (const math::Position3*)eye,
                     (const math::Position3*)mins, (const math::Position3*)maxs,
                     (const math::Position3*)target, &ctx);
            float finalPos[4];
            if (tr.normal.v.m128_f32[1] >= 1.0f)
            {
                finalPos[0] = target[0];
                finalPos[1] = target[1];
                finalPos[2] = target[2];
                finalPos[3] = eye[3];
            }
            else
            {
                finalPos[0] = tr.endpos.v.m128_f32[0];
                finalPos[1] = tr.endpos.v.m128_f32[1];
                finalPos[2] = tr.endpos.v.m128_f32[2] + 3.0f;
                finalPos[3] = tr.endpos.v.m128_f32[3];
            }
            dword_F63C70[1580 * mClient] = finalPos[0];
            dword_F63C74[1580 * mClient] = finalPos[1];
            dword_F63C78[1580 * mClient] = finalPos[2];
            dword_F63C70[1580 * mClient + 3] = finalPos[3];
            float toPrev[3] = {finalPos[0] - mPrevViewPos.v.m128_f32[0],
                               finalPos[1] - mPrevViewPos.v.m128_f32[1],
                               finalPos[2] - mPrevViewPos.v.m128_f32[2]};
            if (toPrev[0] * toPrev[0] + toPrev[1] * toPrev[1]
                        + toPrev[2] * toPrev[2]
                    < 9.0f
                && !mDeathRumble)
            {
                mDeathRumble = true;
                CG_StartShakeCamera(0.80000001f, 800,
                                    &dword_F63C70[1580 * currCl], 800.0f,
                                    currCl);
                dword_F64018[1580 * currCl] =
                    cgGlobal.time + (int)cg_redFlashTime.value;
                if (RumbleManager::Inst(currCl) != nullptr)
                {
                    RumbleEffect_local effect;
                    RumbleEffect_Ctor(&effect);
                    effect.mRumbleDataArray[0].enabled = 1;
                    RumbleEffect_SetIntensity(&effect, 0, 1.0f);
                    effect.mRumbleDataArray[0].steady_duration = 0.2f;
                    effect.mRumbleDataArray[0].delay = 0.0f;
                    effect.mRumbleDataArray[1].enabled = 1;
                    RumbleEffect_SetIntensity(&effect, 1, 1.0f);
                    effect.mRumbleDataArray[1].steady_duration = 0.2f;
                    effect.mRumbleDataArray[1].delay = 0.0f;
                    effect.mRumbleDataArray[1].ramp_up_duration = 0.2f;
                    effect.mRumbleDataArray[1].ramp_down_duration = 0.2f;
                    RumbleManager::Inst(currCl)->Play((RumbleEffect*)&effect, 1.0f);
                }
                void* mWorld =
                    *(void**)((char*)EntityManager::sInst + 0x44);
                unsigned int hash =
                    HashString::CalcHash("playerDeathHitGround");
                if (mWorld != nullptr)
                    Entity_Notify(mWorld, hash);
            }
            dword_F63C70[1580 * currCl] = finalPos[0];
            dword_F63C74[1580 * currCl] = finalPos[1];
            dword_F63C78[1580 * currCl] = finalPos[2];
            dword_F63C70[1580 * currCl + 3] = finalPos[3];
        }
        else
        {
            UpdateMPDeathCameraNoKiller();
        }
    }
}

extern void InterpolatePositionSmooth(float* a1, const float* a2,
                                      const float* a3, float a4);
extern void InterpolateAnglesSmooth(float* a1, float* a2, float* a3,
                                    float a4);
extern float AngleNormalize360(float angle);

// ea: 0x006A5E40
void Camera::UpdateTween(math::Position3& tweenStartPos,
                         math::Position3& tweenStartAngles)
{
    mTweenTime = ServerTime_sInst.mTickDelta + mTweenTime;
    if (IsInvalidFloat(tweenStartPos.v.m128_f32[0])
        || IsInvalidFloat(tweenStartPos.v.m128_f32[1])
        || IsInvalidFloat(tweenStartPos.v.m128_f32[2]))
    {
        CG_ASSERT("!IS_NAN((tweenStartPos)[0]) && !IS_NAN((tweenStartPos)[1]) "
                  "&& !IS_NAN((tweenStartPos)[2])",
                  "c:\\cod\\code\\game\\Camera.cpp", 1075);
    }
    if (IsInvalidFloat(tweenStartAngles.v.m128_f32[0])
        || IsInvalidFloat(tweenStartAngles.v.m128_f32[1])
        || IsInvalidFloat(tweenStartAngles.v.m128_f32[2]))
    {
        CG_ASSERT("!IS_NAN((tweenStartAngles)[0]) && "
                  "!IS_NAN((tweenStartAngles)[1]) && "
                  "!IS_NAN((tweenStartAngles)[2])",
                  "c:\\cod\\code\\game\\Camera.cpp", 1076);
    }
    if (mTweenDuration > mTweenTime)
    {
        float frac = mTweenTime / mTweenDuration;
        Client* client =
            EntityManager::sInst->GetPlayer( mClient)->client;
        unsigned int mVal = client->ps.mViewLockedEntity;
        bool circleTween = (mTweenFlags & 2) != 0;
        if (!circleTween)
        {
            if ((0x100000
                 & EntityManager::sInst->GetPlayer( mClient)
                       ->client->ps.eFlags)
                != 0)
            {
                Entity* Player =
                    EntityManager::sInst->GetPlayer( mClient);
                Entity* v24 = DbHandleToEntity(Player->r.mOwner.mHandle.mVal);
                float px = mTweenParentPos.v.m128_f32[0];
                float py = mTweenParentPos.v.m128_f32[1];
                float pz = mTweenParentPos.v.m128_f32[2];
                if (px * px + py * py + pz * pz > 0.1f)
                {
                    float dx = v24->r.currentOrigin.v.m128_f32[0] - px;
                    float dy = v24->r.currentOrigin.v.m128_f32[1] - py;
                    float dz = v24->r.currentOrigin.v.m128_f32[2] - pz;
                    if (dx * dx + dy * dy + dz * dz < 65536.0f)
                    {
                        tweenStartPos.v = _mm_add_ps(
                            tweenStartPos.v,
                            _mm_setr_ps(dx, dy, dz, 0.0f));
                        float a0 = AngleNormalize180(
                                       v24->r.currentAngles.v.m128_f32[0]
                                       - mTweenParentAngles.v.m128_f32[0])
                                   + tweenStartAngles.v.m128_f32[0];
                        tweenStartAngles.v.m128_f32[0] =
                            AngleNormalize180(a0);
                        float a1 = AngleNormalize180(
                                       v24->r.currentAngles.v.m128_f32[1]
                                       - mTweenParentAngles.v.m128_f32[1])
                                   + tweenStartAngles.v.m128_f32[1];
                        tweenStartAngles.v.m128_f32[1] =
                            AngleNormalize360(a1);
                        float a2 = AngleNormalize180(
                                       v24->r.currentAngles.v.m128_f32[2]
                                       - mTweenParentAngles.v.m128_f32[2])
                                   + tweenStartAngles.v.m128_f32[2];
                        tweenStartAngles.v.m128_f32[2] =
                            AngleNormalize180(a2);
                    }
                }
                mTweenParentPos.v = v24->r.currentOrigin.v;
                mTweenParentAngles.v = v24->r.currentAngles.v;
            }
            if ((mTweenFlags & 1) == 0)
            {
                if ((mAnimFlags & 1) == 0 && client->ps.vehType != 2)
                {
                    // non-tank vehicle: straight forward-based smoothing
                    float fwd[3];
                    AnglesToForward(
                        *(const math::Position3*)&angle[1580 * mClient],
                        *(math::Dir3*)fwd);
                    float start[3] = {
                        dword_F63C70[1580 * mClient] + fwd[0] * 250.0f,
                        dword_F63C74[1580 * mClient] + fwd[1] * 250.0f,
                        dword_F63C78[1580 * mClient] + fwd[2] * 250.0f};
                    InterpolatePositionSmooth(start, tweenStartPos.v.m128_f32,
                                              start, frac);
                    InterpolateAnglesSmooth(&angle[1580 * mClient],
                                            tweenStartAngles.v.m128_f32,
                                            &angle[1580 * mClient], frac);
                    AnglesToForward(
                        *(const math::Position3*)&angle[1580 * mClient],
                        *(math::Dir3*)fwd);
                    dword_F63C70[1580 * mClient] = start[0] - fwd[0] * 250.0f;
                    dword_F63C74[1580 * mClient] = start[1] - fwd[1] * 250.0f;
                    dword_F63C78[1580 * mClient] = start[2] - fwd[2] * 250.0f;
                    if (mCamMode != CAM_VEHICLE_GUNNER_CROUCHED
                        && mCamMode != CAM_VEHICLE_GUNNER)
                    {
                        float mins[3] = {-4.0f, -4.0f, -4.0f};
                        float maxs[3] = {4.0f, 4.0f, 4.0f};
                        collision_context_t ctx;
                        memset(&ctx, 0, sizeof(ctx));
                        ctx.__vftable =
                            (collision_context_t_vtbl*)0x00CD8F6C;
                        ctx.pass_entity1.mHandle.mVal =
                            EntityManager::sInst->GetPlayer(
                                                    mClient)
                                ->mHandle.mHandle.mVal;
                        ctx.pass_entity2.mHandle.mVal = mVal;
                        ctx.contentmask = 17;
                        trace_t tr;
                        float end[3] = {dword_F63C70[1580 * mClient],
                                        dword_F63C74[1580 * mClient],
                                        dword_F63C78[1580 * mClient]};
                        CG_Trace(&tr, (const math::Position3*)start,
                                 (const math::Position3*)mins,
                                 (const math::Position3*)maxs,
                                 (const math::Position3*)end, &ctx);
                        if (tr.normal.v.m128_f32[1] < 1.0f)
                        {
                            dword_F63C70[1580 * mClient] =
                                tr.endpos.v.m128_f32[0];
                            dword_F63C74[1580 * mClient] =
                                tr.endpos.v.m128_f32[1];
                            dword_F63C78[1580 * mClient] =
                                tr.endpos.v.m128_f32[2];
                        }
                    }
                }
                else
                {
                    // tank: collision-smoothed position
                    float mins[3] = {-15.0f, -15.0f, -15.0f};
                    float maxs[3] = {15.0f, 15.0f, 15.0f};
                    collision_context_t ctx;
                    memset(&ctx, 0, sizeof(ctx));
                    ctx.__vftable =
                        (collision_context_t_vtbl*)0x00CD8F6C;
                    ctx.contentmask = 0x802033;
                    float cur[3] = {dword_F63C70[1580 * mClient],
                                    dword_F63C74[1580 * mClient],
                                    dword_F63C78[1580 * mClient]};
                    float start[3];
                    InterpolatePositionSmooth(start, tweenStartPos.v.m128_f32,
                                              cur, frac);
                    float end[3] = {start[0], start[1], start[2] + 1.0f};
                    trace_t tr;
                    CG_Trace(&tr, (const math::Position3*)start,
                             (const math::Position3*)mins,
                             (const math::Position3*)maxs,
                             (const math::Position3*)end, &ctx);
                    if (tr.normal.v.m128_f32[1] == 0.0f)
                    {
                        InterpolatePositionSmooth(
                            cur, tweenStartPos.v.m128_f32, cur, frac);
                    }
                    else
                    {
                        if (tr.surfaceFlags == 0
                            || tr.surfaceFlags
                                   == *(int*)((char*)EntityManager::sInst + 0x44
                                              + 0x234))
                        {
                            CG_Trace(&tr, (const math::Position3*)cur,
                                     (const math::Position3*)mins,
                                     (const math::Position3*)maxs,
                                     (const math::Position3*)start, &ctx);
                        }
                        else
                        {
                            float pos[4] = {start[0], start[1], start[2],
                                            start[2]};
                            while (1)
                            {
                                pos[2] = pos[3] + 32.0f;
                                CG_Trace(&tr, (const math::Position3*)pos,
                                         (const math::Position3*)mins,
                                         (const math::Position3*)maxs,
                                         (const math::Position3*)start, &ctx);
                                if (*(unsigned char*)((char*)&tr.mEntity
                                                      + 1)
                                    == 0)
                                    break;
                                pos[3] = pos[2];
                                if ((pos[2] - start[2]) >= 512.0f)
                                {
                                    CG_Trace(
                                        &tr, (const math::Position3*)cur,
                                        (const math::Position3*)mins,
                                        (const math::Position3*)maxs,
                                        (const math::Position3*)&tweenStartPos,
                                        &ctx);
                                    break;
                                }
                            }
                        }
                        dword_F63C70[1580 * mClient] =
                            tr.endpos.v.m128_f32[0];
                        dword_F63C74[1580 * mClient] =
                            tr.endpos.v.m128_f32[1];
                        dword_F63C78[1580 * mClient] =
                            tr.endpos.v.m128_f32[2];
                        dword_F63C70[1580 * mClient + 3] =
                            tr.endpos.v.m128_f32[0];
                    }
                }
            }
            if (IsInvalidFloat(angle[1580 * mClient])
                || IsInvalidFloat(dword_F63CB4[1580 * mClient])
                || IsInvalidFloat(dword_F63CB8[1580 * mClient]))
            {
                CG_ASSERT("!IS_NAN((cg[mClient].refdefViewAngles)[0]) && "
                          "!IS_NAN((cg[mClient].refdefViewAngles)[1]) && "
                          "!IS_NAN((cg[mClient].refdefViewAngles)[2])",
                          "c:\\cod\\code\\game\\Camera.cpp", 1183);
            }
            InterpolateAnglesSmooth(&angle[1580 * mClient],
                                    tweenStartAngles.v.m128_f32,
                                    &angle[1580 * mClient], frac);
        }
        else
        {
            float fwd[3];
            AnglesToForward(
                *(const math::Position3*)&angle[1580 * mClient],
                *(math::Dir3*)fwd);
            float start[3] = {dword_F63C70[1580 * mClient] + fwd[0] * 250.0f,
                              dword_F63C74[1580 * mClient] + fwd[1] * 250.0f,
                              dword_F63C78[1580 * mClient] + fwd[2] * 250.0f};
            InterpolatePositionSmooth(start, tweenStartPos.v.m128_f32, start,
                                      frac);
            InterpolateAnglesSmooth(&angle[1580 * mClient],
                                    tweenStartAngles.v.m128_f32,
                                    &angle[1580 * mClient], frac);
            AnglesToForward(
                *(const math::Position3*)&angle[1580 * mClient],
                *(math::Dir3*)fwd);
            dword_F63C70[1580 * mClient] = start[0] - fwd[0] * 250.0f;
            dword_F63C74[1580 * mClient] = start[1] - fwd[1] * 250.0f;
            dword_F63C78[1580 * mClient] = start[2] - fwd[2] * 250.0f;
            if (mCamMode != CAM_VEHICLE_GUNNER_CROUCHED
                && mCamMode != CAM_VEHICLE_GUNNER)
            {
                float mins[3] = {-4.0f, -4.0f, -4.0f};
                float maxs[3] = {4.0f, 4.0f, 4.0f};
                collision_context_t ctx;
                memset(&ctx, 0, sizeof(ctx));
                ctx.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
                ctx.pass_entity1.mHandle.mVal =
                    EntityManager::sInst->GetPlayer( mClient)
                        ->mHandle.mHandle.mVal;
                ctx.pass_entity2.mHandle.mVal = mVal;
                ctx.contentmask = 17;
                trace_t tr;
                float end[3] = {dword_F63C70[1580 * mClient],
                                dword_F63C74[1580 * mClient],
                                dword_F63C78[1580 * mClient]};
                CG_Trace(&tr, (const math::Position3*)start,
                         (const math::Position3*)mins,
                         (const math::Position3*)maxs,
                         (const math::Position3*)end, &ctx);
                if (tr.normal.v.m128_f32[1] < 1.0f)
                {
                    dword_F63C70[1580 * mClient] =
                        tr.endpos.v.m128_f32[0];
                    dword_F63C74[1580 * mClient] =
                        tr.endpos.v.m128_f32[1];
                    dword_F63C78[1580 * mClient] =
                        tr.endpos.v.m128_f32[2];
                }
            }
        }
    }
}

// ea: 0x006AE760
float Camera::SetNewMode(ECameraModes newMode)
{
    if (newMode == (ECameraModes)mCamMode)
        return 0.0f;
    float mpTweenTime = 0.69999999f;
    if ((0x100000
         & EntityManager::sInst->GetPlayer( mClient)
               ->client->ps.eFlags)
        != 0)
    {
        Entity* mObject = DbHandleToEntity(
            EntityManager::sInst->GetPlayer( mClient)
                ->r.mOwner.mHandle.mVal);
        mTweenParentPos.v = mObject->r.currentOrigin.v;
        mTweenParentAngles.v = mObject->r.currentAngles.v;
    }
    if (newMode != 12 && newMode != 16 && newMode != 17 && newMode != 18)
        StopAnimating(0.0f);
    if (newMode >= CAM_VEHICLE_FIRST
        && newMode <= CAM_VEHICLE_TANK_COMMANDER)
    {
        Entity* Player =
            EntityManager::sInst->GetPlayer( mClient);
        Entity* v11 = DbHandleToEntity(Player->client->ps.mViewLockedEntity);
        void* mWorld = *(void**)((char*)EntityManager::sInst + 0x44);
        if (mVehicleCamMode != VEH_MODE_FIRSTPERSON)
        {
            if (mVehicleCamMode == VEH_MODE_CHASECAM)
            {
                if (mWorld != nullptr)
                {
                    Entity_Notify(
                        mWorld,
                        *(unsigned int*)((char*)0x00ED2AB0 + 0x18));
                }
                if (v11 != nullptr)
                {
                    Entity_Notify(
                        v11, *(unsigned int*)((char*)0x00ED2AB0 + 0x18));
                }
            }
        }
        else
        {
            if (mWorld != nullptr)
            {
                Entity_Notify(
                    mWorld, *(unsigned int*)((char*)0x00ED2AB0 + 0x14));
            }
            if (v11 != nullptr)
            {
                Entity_Notify(
                    v11, *(unsigned int*)((char*)0x00ED2AB0 + 0x14));
            }
        }
    }
    if ((0x100000
         & EntityManager::sInst->GetPlayer( mClient)
               ->client->ps.eFlags)
        != 0)
    {
        Entity* v17 = DbHandleToEntity(
            EntityManager::sInst->GetPlayer( mClient)
                ->r.mOwner.mHandle.mVal);
        mTweenParentPos.v = v17->r.currentOrigin.v;
        mTweenParentAngles.v = v17->r.currentAngles.v;
    }
    Client* playerClient =
        EntityManager::sInst->GetPlayer( mClient)->client;
    if (playerClient->ps.vehPos == 6)
    {
        if (mVehicleCamMode == VEH_MODE_CHASECAM
            && !*(bool*)((char*)playerClient + 0xAD8)
            && newMode == CAM_TURRET)
            mpTweenTime = 3.3f;
        else if (mVehicleCamMode == VEH_MODE_FIRSTPERSON
                 && newMode != CAM_TURRET)
            mpTweenTime = 1.0f;
    }
    switch (mCamMode)
    {
    case CAM_VEHICLE_FIRST:
    case CAM_VEHICLE_THIRD:
        EndVehicleCam();
        break;
    case CAM_VEHICLE_TANK:
    case CAM_VEHICLE_TANK_COMMANDER:
        if (mRumbleEffect != 0)
        {
            RumbleEffectInstanceHandle handle = {mRumbleEffect};
            RumbleManager_Remove(RumbleManager::Inst(mClient), handle);
            mRumbleEffect = 0;
        }
        break;
    case CAM_VEHICLE_GUNNER:
        if (newMode == CAM_VEHICLE_PASSENGER)
            mpTweenTime = 0.40000001f;
        break;
    case CAM_INTERACTION_FREE:
        mpTweenTime = 0.0f;
        break;
    case CAM_MP_DEATH_CAMERA:
        GetPlayer(mClient);
        {
            float newAngles[3] = {mPrevAngles.v.m128_f32[0],
                                  mPrevAngles.v.m128_f32[1], 0.0f};
            SetPlayerAngles(newAngles);
        }
        break;
    default:
        break;
    }
    float v21 = 2.0f;
    switch (newMode)
    {
    case CAM_VEHICLE_FIRST:
    case CAM_VEHICLE_THIRD:
        {
            Entity* v23 = GetPlayer(mClient);
            float v24 = IsPlayerFullySeatedInVehicle(v23) ? 1.0f : 3.0f;
            mpTweenTime = v24;
        }
        BeginVehicleCam();
        break;
    case CAM_VEHICLE_TANK:
        mpTweenTime = 2.0f;
        // fall through
    case CAM_VEHICLE_TANK_COMMANDER:
        if (mCamMode == CAM_VEHICLE_TANK)
            goto tank_rumble;
        if (mCamMode == CAM_VEHICLE_GUNNER)
            v21 = 1.4f;
    tank_rumble:
        mpTweenTime = v21;
        {
            RumbleEffect_local effect;
            RumbleEffect_Ctor(&effect);
            effect.mRumbleDataArray[0].enabled = 1;
            effect.mRumbleDataArray[1].enabled = 1;
            RumbleEffect_SetIntensity(&effect, 0, 0.0f);
            effect.mRumbleDataArray[0].steady_duration = 100000000.0f;
            effect.mRumbleDataArray[0].delay = 0.0f;
            RumbleEffect_SetIntensity(&effect, 1, 1.0f);
            effect.mRumbleDataArray[1].delay = 0.0f;
            effect.mRumbleDataArray[1].steady_duration = 100000000.0f;
            effect.mRumbleDataArray[1].ramp_up_duration = 0.0f;
            effect.mRumbleDataArray[1].ramp_down_duration = 0.0f;
            char notes[32];
            BrocString_ctor(notes, gTankRumbleNotes);
            RumbleEffect_SetNotes(&effect, 1, notes);
            BrocString_dtor(notes);
            RumbleEffectInstanceHandle h =
                RumbleManager::Inst(mClient)->Play((RumbleEffect*)&effect, 0.0f);
            mRumbleEffect = h.mVal;
            Client* client = GetPlayer(mClient)->client;
            float newAngles[3] = {client->ps.viewangles[0],
                                  client->ps.viewangles[1], 0.0f};
            SetPlayerAngles(newAngles);
            Entity* v30 = DbHandleToEntity(client->ps.mViewLockedEntity);
            mTankRelativeAngles.v.m128_f32[0] =
                mPrevAngles.v.m128_f32[0]
                - v30->r.currentAngles.v.m128_f32[0];
            mTankRelativeAngles.v.m128_f32[2] = 0.0f;
            mTankRelativeAngles.v.m128_f32[1] =
                mPrevAngles.v.m128_f32[1]
                - v30->r.currentAngles.v.m128_f32[1];
        }
        break;
    case CAM_VEHICLE_GUNNER:
        switch (mCamMode)
        {
        case 7:
            mpTweenTime = 0.40000001f;
            break;
        case CAM_VEHICLE_TANK:
            mpTweenTime = 2.0f;
            break;
        case CAM_VEHICLE_TANK_COMMANDER:
            mpTweenTime = 1.2f;
            break;
        default:
            break;
        }
        break;
    case CAM_VEHICLE_ANIM:
        if (mCamMode == CAM_VEHICLE_THIRD
            && GetPlayer(mClient)->client->ps.vehType == 1)
            mpTweenTime = 4.0f;
        break;
    case CAM_VEHICLE_ANIM_FIRST:
        mpTweenTime = 2.0f;
        break;
    case CAM_TURRET_FIRST:
        if (mCamMode != 18)
        {
            Client* v32 = GetPlayer(mClient)->client;
            if (v32 != nullptr && v32->ps.vehType == 5)
            {
                Entity* v33 = DbHandleToEntity(v32->ps.mViewLockedEntity);
                Entity* v34 = v33;
                void* scr_vehicle = v33->scr_vehicle;
                if (scr_vehicle != nullptr)
                {
                    weaponInfoCam* InfoForWeapon =
                        (weaponInfoCam*)BG_GetInfoForWeapon(
                            v33->s.weapon);
                    int barrel;
                    if (InfoForWeapon != nullptr
                        && *(int*)((char*)InfoForWeapon + 0x7A0) != 0)
                        barrel = *(int*)((char*)scr_vehicle + 0x460);
                    else
                        barrel = *(int*)((char*)scr_vehicle + 0x474);
                    if (barrel < 0)
                    {
                        CG_ASSERT("boneIdx >= 0",
                                  "c:\\cod\\code\\game\\Camera.cpp", 866);
                    }
                    float boneMtx[16];
                    G_DObjGetWorldBoneIndexMatrix(
                        v34, barrel, (DObjSkelMat*)boneMtx);
                    float newAngles[3];
                    Axis4ToAngles((const float(*)[4])boneMtx, newAngles);
                    newAngles[0] = AngleNormalize180(newAngles[0]);
                    SetPlayerAngles(newAngles);
                }
            }
        }
        break;
    case CAM_INTERMISSION:
    case CAM_INTERACTION_FREE:
    case CAM_INTERACTION_LOCKED:
    case CAM_MP_DEATH_CAMERA:
        mpTweenTime = 0.0f;
        break;
    case 18:
    case CAM_DEATH_CAMERA:
        break;
    default:
        {
            Client* v38 =
                EntityManager::sInst->GetPlayer( mClient)->client;
            float newAngles[3] = {v38->ps.viewangles[0],
                                  v38->ps.viewangles[1], 0.0f};
            SetPlayerAngles(newAngles);
            if (mCamMode == 17 || mCamMode == 16)
                mpTweenTime = 0.2f;
        }
        break;
    }
    mCamMode = newMode;
    return mpTweenTime;
}

extern const float* DObjGetMat(void* dobj, int boneIndex);

static void MatrixMultiply4x4(const float* a, const float* b, float* out)
{
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            out[r * 4 + c] = a[r * 4 + 0] * b[0 * 4 + c]
                             + a[r * 4 + 1] * b[1 * 4 + c]
                             + a[r * 4 + 2] * b[2 * 4 + c]
                             + a[r * 4 + 3] * b[3 * 4 + c];
        }
    }
}

// ea: 0x0069E630
void Camera::UpdateAnimation()
{
    if ((mAnimFlags & 1) != 0)
    {
        if (EntityManager::sInst->GetPlayer( mClient)
                ->client->ps.weapon
            > 0)
        {
            if (mTagCameraIndex == -1)
            {
                CG_ASSERT("mTagCameraIndex != -1",
                          "c:\\cod\\code\\game\\Camera.cpp", 2074);
            }
            const float* mat;
            if ((mAnimFlags & 4) != 0)
                mat = &mLastTagCamMat.x.v.m128_f32[0];
            else
                mat = DObjGetMat((void*)dword_F6A2A0[802 * mClient],
                                 mTagCameraIndex);
            float tag[16];
            for (int i = 0; i < 16; ++i)
                tag[i] = mat[i];
            float axis[3][3];
            AnglesToAxis((const float*)&angle[1580 * mClient],
                         axis);
            float t[16] = {
                axis[0][0], axis[0][1], axis[0][2], 0.0f,
                axis[1][0], axis[1][1], axis[1][2], 0.0f,
                axis[2][0], axis[2][1], axis[2][2], 0.0f,
                0.0f,       0.0f,       0.0f,       1.0f};
            float out[16];
            MatrixMultiply4x4(tag, t, out);
            out[12] += dword_F63C70[1580 * mClient];
            out[13] += dword_F63C74[1580 * mClient];
            out[14] += dword_F63C78[1580 * mClient];
            float viewAngles[3];
            Axis4ToAngles((const float(*)[4])out, viewAngles);
            angle[1580 * mClient] = viewAngles[0];
            dword_F63CB4[1580 * mClient] = viewAngles[1];
            dword_F63CB8[1580 * mClient] = viewAngles[2];
            dword_F63C70[1580 * mClient] = out[12];
            dword_F63C74[1580 * mClient] = out[13];
            dword_F63C78[1580 * mClient] = out[14];
            AnglesToAxis((const float*)&angle[1580 * mClient],
                         (float(*)[3])&dword_F63C80[1580 * mClient]);
            for (int i = 0; i < 16; ++i)
                mLastTagCamMat.x.v.m128_f32[i] = tag[i];
        }
        mAnimFlags |= 2u;
    }
    else if ((mAnimFlags & 2) != 0)
    {
        mAnimFlags &= ~2u;
    }
}

float gSceneAnimCameraFOV;  // 0x00F258F0
extern void* gSceneAnimCameraPO;   // 0x00F25AB0
extern float dword_F63C60[4 * 1580];
extern float dword_F63C8C[4 * 1580];
extern float dword_F63C90[4 * 1580];
extern float dword_F63C94[4 * 1580];
extern float dword_F63C98[4 * 1580];
extern float dword_F63C9C[4 * 1580];
extern float dword_F63CA0[4 * 1580];
extern void G_SetOrigin(Entity* ent, const float* origin);

// ea: 0x0068E230
void Camera::UpdateSceneAnimCam()
{
    // Flip/permute matrix built from four vectors in the original
    float M[16] = {0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                   0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float C[16];
    memcpy(C, gSceneAnimCameraPO, sizeof(C));
    float out[16];
    MatrixMultiply4x4(M, C, out);
    dword_F63C80[1580 * mClient] = out[0];
    dword_F63C84[1580 * mClient] = out[1];
    dword_F63C88[1580 * mClient] = out[2];
    dword_F63C8C[1580 * mClient] = out[4];
    dword_F63C90[1580 * mClient] = out[5];
    dword_F63C94[1580 * mClient] = out[8];
    dword_F63C98[1580 * mClient] = out[9];
    dword_F63C9C[1580 * mClient] = out[10];
    dword_F63CA0[1580 * mClient] = out[11];
    dword_F63C70[1580 * mClient] = out[12];
    dword_F63C74[1580 * mClient] = out[13];
    dword_F63C78[1580 * mClient] = out[14];
    AxisToAngles((const float(*)[3])&dword_F63C80[1580 * mClient],
                 &angle[1580 * mClient]);
    dword_F63C60[1580 * mClient] = gSceneAnimCameraFOV;
    float origin[3] = {out[12], out[13], out[14]};
    Entity* Player =
        EntityManager::sInst->GetPlayer( mClient);
    G_SetOrigin(Player, origin);
    Player->s.pos.trBase[0] = origin[0];
    Player->s.pos.trBase[1] = origin[1];
    Player->s.pos.trBase[2] = origin[2];
    Player->client->ps.origin.v.m128_f32[0] = origin[0];
    Player->client->ps.origin.v.m128_f32[1] = origin[1];
    Player->client->ps.origin.v.m128_f32[2] = origin[2];
    EntityManager::sInst->GetPlayer( currCl)
        ->client->ps.viewHeightCurrent = 0.0f;
}

extern void vectosignedangles(const float* vec, float* angles);

static unsigned int s_pelvisHash;
static bool s_pelvisHashInit;
static unsigned int s_bodyHash;
static bool s_bodyHashInit;
static unsigned int s_animHeadHash;
static bool s_animHeadHashInit;
static unsigned int s_animSpineHash;
static bool s_animSpineHashInit;

// ea: 0x006A6D00
void Camera::UpdateVehicleAnimCam()
{
    bool v79 = mTweenTime != 0.0f || mTweenDuration <= 0.0f;
    Client* client =
        EntityManager::sInst->GetPlayer( mClient)->client;
    Entity* mObject = DbHandleToEntity(client->ps.mViewLockedEntity);
    if (mObject != nullptr)
    {
        vehicle_info_full_t* Info =
            (vehicle_info_full_t*)VEH_GetInfo(
                *(short*)((char*)mObject->scr_vehicle + 0x178));
        if (mVehicleCamMode != VEH_MODE_FIRSTPERSON
            && (*(short*)((char*)Info + 0x20) == 2
                || !*(bool*)((char*)client + 0xAD8))
            && !*(bool*)((char*)client + 0xAE0))
        {
            // third-person chase camera
            if (mVehicleCamMode == VEH_MODE_CHASECAM
                && client->ps.vehPos == 1
                && !*(bool*)((char*)GetPlayer(mClient)->client + 0xAD8))
            {
                Entity* Player = GetPlayer(mClient);
                dword_F63C70[1580 * mClient] =
                    Player->r.currentOrigin.v.m128_f32[0];
                dword_F63C74[1580 * mClient] =
                    Player->r.currentOrigin.v.m128_f32[1];
                dword_F63C78[1580 * mClient] =
                    Player->r.currentOrigin.v.m128_f32[2]
                    + *(float*)((char*)Player->client + 224);
                angle[1580 * mClient] = mPrevAngles.v.m128_f32[0];
                dword_F63CB4[1580 * mClient] =
                    mPrevAngles.v.m128_f32[1];
                dword_F63CB8[1580 * mClient] =
                    mPrevAngles.v.m128_f32[2];
                return;
            }
            StartTween(0.25f, false);
            float radius = Info->cameraChaseRadiusOuter;
            if (!s_pelvisHashInit)
            {
                s_pelvisHashInit = true;
                s_pelvisHash = HashString::CalcHash("bip01 pelvis");
            }
            float tagMtx[16];
            G_DObjGetWorldTagMatrix(
                EntityManager::sInst->GetPlayer( mClient),
                s_pelvisHash, (DObjSkelMat*)tagMtx);
            if (client->ps.vehPos == 1 || client->ps.vehPos == 2)
                radius = Info->cameraChaseRadiusInner;
            float delta[3] = {
                (tagMtx[9] - mObject->r.currentOrigin.v.m128_f32[0]) * 2.8f,
                (tagMtx[10] - mObject->r.currentOrigin.v.m128_f32[1]) * 2.8f,
                ((tagMtx[11] + 24.0f) - mObject->r.currentOrigin.v.m128_f32[2])
                        * 0.25f
                    * 2.8f};
            float startOffset[3];
            if (v79)
            {
                float len = -sqrtf(delta[0] * delta[0] + delta[1] * delta[1]
                                   + delta[2] * delta[2]);
                startOffset[0] = delta[0] / len;
                startOffset[1] = delta[1] / len;
                startOffset[2] = delta[2] / len;
                if (client->ps.vehPos >= 8)
                {
                    if (!s_bodyHashInit)
                    {
                        s_bodyHashInit = true;
                        s_bodyHash = HashString::CalcHash("tag_body");
                    }
                    float bodyMtx[16];
                    G_DObjGetWorldTagMatrix(mObject, s_bodyHash,
                                            (DObjSkelMat*)bodyMtx);
                    float combined[3] = {
                        startOffset[0] + bodyMtx[12] * 2.0f,
                        startOffset[1] + bodyMtx[13] * 2.0f,
                        startOffset[2] + bodyMtx[14] * 2.0f};
                    float len2 = sqrtf(combined[0] * combined[0]
                                       + combined[1] * combined[1]
                                       + combined[2] * combined[2]);
                    startOffset[0] = combined[0] / len2;
                    startOffset[1] = combined[1] / len2;
                    startOffset[2] = combined[2] / len2;
                }
            }
            else
            {
                startOffset[0] = 0.0f;
                startOffset[1] = 0.0f;
                startOffset[2] = 0.0f;
            }
            float outer = Info->cameraChaseRadiusOuter * 2.8f;
            delta[0] -= startOffset[0] * outer;
            delta[1] -= startOffset[1] * outer;
            delta[2] -= startOffset[2] * outer;
            float f = (client->ps.vehPos < 8)
                          ? Info->cameraChaseRadiusOuter * 0.80000001f
                          : Info->cameraChaseRadiusOuter * 0.30000001f;
            delta[2] += f;
            if (v79)
                delta[2] += 80.0f;
            float len = sqrtf(delta[0] * delta[0] + delta[1] * delta[1]
                              + delta[2] * delta[2]);
            float dir[3] = {delta[0] / len, delta[1] / len, delta[2] / len};
            float chasePos[3] = {tagMtx[12] + dir[0] * radius,
                                 tagMtx[13] + dir[1] * radius,
                                 tagMtx[14] + dir[2] * radius};
            float d2[3] = {tagMtx[12] - chasePos[0],
                           tagMtx[13] - chasePos[1],
                           tagMtx[14] - chasePos[2]};
            float len2 = sqrtf(d2[0] * d2[0] + d2[1] * d2[1]
                               + d2[2] * d2[2]);
            float dir2[3] = {d2[0] / len2, d2[1] / len2, d2[2] / len2};
            float signedAngles[3];
            vectosignedangles(dir2, signedAngles);
            float fwd[3];
            AnglesToForward(*(const math::Position3*)signedAngles,
                            *(math::Dir3*)fwd);
            angle[1580 * mClient] = signedAngles[0];
            angle[1580 * mClient + 1] = signedAngles[1];
            angle[1580 * mClient + 2] = signedAngles[2];
            angle[1580 * mClient + 3] = 0.0f;
            float rotScale[3] = {chasePos[0], chasePos[1], chasePos[2]};
            if (radius <= Info->cameraChaseRadiusOuter)
            {
                if (Info->cameraChaseRadiusInner <= radius)
                    goto do_trace;
                radius = 0.0f - (Info->cameraChaseRadiusInner - radius);
            }
            else
            {
                radius = radius - Info->cameraChaseRadiusOuter;
            }
            rotScale[0] = chasePos[0] + dir2[0] * radius;
            rotScale[1] = chasePos[1] + dir2[1] * radius;
            rotScale[2] = chasePos[2] + dir2[2] * radius;
        do_trace:
            {
                float mins[3] = {-1.0f, -1.0f, -1.0f};
                float maxs[3] = {1.0f, 1.0f, 1.0f};
                collision_context_t ctx;
                memset(&ctx, 0, sizeof(ctx));
                ctx.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
                ctx.pass_entity1.mHandle.mVal = client->ps.mViewLockedEntity;
                ctx.pass_entity2.mHandle.mVal = mObject->mHandle.mHandle.mVal;
                trace_t tr;
                CG_Trace(&tr, (const math::Position3*)chasePos,
                         (const math::Position3*)mins,
                         (const math::Position3*)maxs,
                         (const math::Position3*)rotScale, &ctx);
                if (tr.normal.v.m128_f32[1] < 1.0f)
                {
                    rotScale[0] = tr.endpos.v.m128_f32[0];
                    rotScale[1] = tr.endpos.v.m128_f32[1];
                    rotScale[2] = tr.endpos.v.m128_f32[2];
                }
                dword_F63C70[1580 * mClient] = rotScale[0];
                dword_F63C74[1580 * mClient] = rotScale[1];
                dword_F63C78[1580 * mClient] = rotScale[2];
            }
            return;
        }
        // first-person anim camera
        float tween = ServerTime_sInst.mTickDelta * 4.0f;
        if (mTweenDuration < 1.0f)
            StartTween(tween, false);
        if (!s_animHeadHashInit)
        {
            s_animHeadHashInit = true;
            s_animHeadHash = HashString::CalcHash("bip01 head");
        }
        if (!s_animSpineHashInit)
        {
            s_animSpineHashInit = true;
            s_animSpineHash = HashString::CalcHash("bip01 spine1");
        }
        float headMtx[16];
        G_DObjGetWorldTagMatrix(
            EntityManager::sInst->GetPlayer( mClient),
            s_animHeadHash, (DObjSkelMat*)headMtx);
        float spineMtx[16];
        G_DObjGetWorldTagMatrix(
            EntityManager::sInst->GetPlayer( mClient),
            s_animSpineHash, (DObjSkelMat*)spineMtx);
        dword_F63C70[1580 * mClient] = headMtx[12];
        dword_F63C74[1580 * mClient] = headMtx[13];
        dword_F63C78[1580 * mClient] = headMtx[14];
        float axis[3][3] = {{spineMtx[4], spineMtx[5], spineMtx[6]},
                            {spineMtx[0], spineMtx[1], spineMtx[2]},
                            {spineMtx[8], spineMtx[9], spineMtx[10]}};
        float angles[3];
        AxisToAngles(axis, angles);
        angle[1580 * mClient] = angles[0];
        dword_F63CB4[1580 * mClient] = angles[1];
        dword_F63CB8[1580 * mClient] = angles[2];
        float rotScale[3] = {0.3f, 1.0f, 0.1f};
        for (int i = 0; i < 3; ++i)
        {
            dword_F63C70[1580 * mClient + i] -= spineMtx[4 * i] * 10.0f;
            float baseAngle = *(float*)((char*)mObject + 0x160 + 4 * i);
            if (rotScale[i] >= 1.0f)
            {
                angle[1580 * mClient + i] = AngleNormalize180(
                    AngleNormalize180(angle[1580 * mClient + i]));
            }
            else
            {
                float diff =
                    AngleNormalize180(angle[1580 * mClient + i]
                                      - AngleNormalize180(baseAngle));
                angle[1580 * mClient + i] = AngleNormalize180(
                    diff * rotScale[i] + baseAngle);
            }
        }
    }
}

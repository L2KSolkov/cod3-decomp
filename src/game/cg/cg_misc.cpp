// ============================================================================
// cg_misc.cpp - misc client game helpers: bars/rects, HUD state, camera (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"

#include <math.h>
#include <string.h>

extern int currCl;
extern float unk_F6A278[4 * 802];
extern float unk_F6A27C[4 * 802];
extern void* cgsGlobal_media_whiteShader;
extern void* g_femanager;

extern void trap_R_SetColor(const float* rgba);
extern void trap_R_DrawStretchPic(float x, float y, float w, float h, float s1,
                                  float t1, float s2, float t2, void* tex,
                                  float z);
extern void CL_AddDebugLine(float* start, const float* end,
                            const float* color, int depthTest, int duration,
                            int fromServer, int fadeOut);
extern void* Entity_GetRefEntity(Entity* ent);
extern void RE_AddRefEntityToScene(void* ent, int iCellNum);
extern void ByteToDir(int b, float* dir);
extern void PerpendicularVector(float* dst, const float* src);
extern void CrossProduct(const float* v1, const float* v2, float* cross);

extern void* controller_inst();
extern int controller_button_value(void* self, int i_controller_num,
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

// ea: 0x00687BC0
void CG_ScoresDown_f()
{
    void* v0 = controller_inst();
    if (controller_button_value(v0, 0, 11 /* R3 */) <= 0)
    {
        if (dword_F641D0[1580 * currCl] == 0
            && byte_F64194[6320 * currCl] == 0)
        {
            dword_F641D0[1580 * currCl] = 1;
            void* IGMS = FEManager_GetIGMS(g_femanager, currCl);
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
            refEntity_t* ref = (refEntity_t*)Entity_GetRefEntity(entity);
            ref->lightingOrigin[0] = 0.0f;
            ref->lightingOrigin[1] = 0.0f;
            ref->lightingOrigin[2] = 0.0f;
        }
    }
    else
    {
        refEntity_t* ref = (refEntity_t*)Entity_GetRefEntity(entity);
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
    refEntity_t* RefEntity = (refEntity_t*)Entity_GetRefEntity(entity);
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
extern const char defaultFileName[];

struct game_hudelem_s {
    struct {
        int type;  // +0x00
    } elem;        // +0x00
    unsigned char _pad[0x7C - 0x04];
};
extern game_hudelem_s g_hudelems[16];  // 0x00EA5580

struct _objectiveInfo_t {
    _objectiveInfo_t* pChild;  // +0x00
};

struct localEntity_t {
    void*        next;  // +0x00
    void*        prev;  // +0x04
    unsigned char _pad[0x40 - 0x08];
    refEntity_t  refEntity;  // +0x40
};

extern void* EntityManager_sInst;
extern Entity* EntityManager_GetPlayer(void* mgr, int idx);
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
    Client* client = EntityManager_GetPlayer(EntityManager_sInst, currCl)->client;
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
    Client* client = EntityManager_GetPlayer(EntityManager_sInst, currCl)->client;
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
    unsigned char _pad[0x20 - 0x04];
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
    CAM_TURRET = 4,
    CAM_VEHICLE_TANK = 5,
    CAM_VEHICLE_PASSENGER = 6,
    CAM_VEHICLE_GUNNER = 7,
    CAM_VEHICLE_DRIVER = 8,
    CAM_VEHICLE_TANK_GUNNER = 9,
    CAM_VEHICLE_ANIM = 10,
    CAM_VEHICLE_ANIM_FIRST = 11,
    CAM_MP_DEATH_CAMERA = 12,
    CAM_LINKED = 13,
    CAM_DEATH_CAMERA = 14,
    CAM_SCENE_ANIMATED = 15,
    CAM_INTERACTION_FREE = 16,
    CAM_INTERACTION_LOCKED = 17,
    CAM_TURRET_TANK = 18,
    CAM_MP_TEAM = 19,
    CAM_MP_DEATH_CAMERA_NO_KILLER = 20,
    CAM_MP_DEATH_CAMERA_KILLER = 21,
    CAM_VEHICLE_TANK_COMMANDER = 22,
};

struct RumbleEffectInstanceHandle {
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

extern void* RumbleManager_Inst(int instance);
extern void RumbleManager_Remove(void* self, RumbleEffectInstanceHandle handle);
extern float CG_GetViewFov();
extern unsigned int HashString_CalcHash(const char* str);
extern int DObjGetBoneIndex(const DObj* obj, unsigned int boneNameHash);
extern void FastSinCos(float radians, float* psin, float* pcos);
extern int _fpclass(double x);
extern void* dword_F6A2A0[4 * 802];
extern float angle[4 * 395];
extern float dword_F63C70[4 * 1580];
extern float dword_F63C74[4 * 1580];
extern float dword_F63C78[4 * 1580];
extern float dword_F63CB4[4 * 1580];
extern float dword_F63CB8[4 * 1580];
extern float dword_F63C80[4 * 1580];
extern float dword_F63C84[4 * 1580];
extern float dword_F63C88[4 * 1580];
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
    unsigned char _pad6[0x190 - 0x148];
    int mCamMode;                              // +0x190
    int mVehicleCamMode;                       // +0x194
    unsigned char _pad7[0x1B0 - 0x198];
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
        mPrevAngles.v.m128_f32[0] = angle[6320 * mClient];
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
        RumbleManager_Remove(RumbleManager_Inst(mClient), v4);
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
                s_tagCameraHash = HashString_CalcHash("tag_camera");
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
    Client* client = EntityManager_GetPlayer(EntityManager_sInst, mClient)->client;
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
    Client* client = EntityManager_GetPlayer(EntityManager_sInst, mClient)->client;
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
    Client* client = EntityManager_GetPlayer(EntityManager_sInst, mClient)->client;
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

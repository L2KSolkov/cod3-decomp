// ============================================================================
// cg_misc.cpp - misc client game helpers: bars/rects, HUD state, camera (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"

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

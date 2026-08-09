// ============================================================================
// cg_draw.cpp - 2D draw helpers (cg.o cg_draw.cpp)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"

#include <string.h>

extern int currCl;
extern int cgGlobal_time;
extern float unk_F6A278[4 * 802];
extern float unk_F6A27C[4 * 802];
extern void* cgsGlobal_media_whiteShader;
extern void* EntityManager_sInst;
extern Entity* EntityManager_GetPlayer(void* mgr, int idx);
extern char* va(const char* fmt, ...);
extern int RE_Text_Width(const char* text, int font, float scale,
                         float charWidth, int limit);
extern void trap_R_Text_Paint(float x, float y, int font, float scale,
                              const float* color, const char* text,
                              float charWidth, int limit, int style);
extern void trap_R_DrawStretchPic(float x, float y, float w, float h, float s1,
                                  float t1, float s2, float t2, void* tex,
                                  float z);
struct sentient_s {
    int eTeam;  // +0x00
};

// ea: 0x00687CB0
float CG_DrawTimer(float y)
{
    float vColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    char* v1 = va("%i:%i%i", cgGlobal_time / 1000 / 60,
                  cgGlobal_time / 1000 % 60 / 10,
                  cgGlobal_time / 1000 % 60 % 10);
    float v2 = RE_Text_Width(v1, 0, 0.66666669f, 0.0f, 0);
    trap_R_Text_Paint(620.0f - v2, y + 18.0f, 0, 0.66666669f, vColor, v1,
                      0.0f, 0, 3);
    return y + 20.0f;
}

// ea: 0x00688600
Entity* CG_DrawFriendlyFire()
{
    return EntityManager_GetPlayer(EntityManager_sInst, currCl);
}

// ea: 0x00688680
void CG_DrawVersion()
{
}

// ea: 0x00688690
void CG_DrawDebugOverlays()
{
}

// ea: 0x006886A0
void CG_DrawFriendOverlay()
{
}

// ea: 0x00688E20
void CG_DrawSides(float x, float y, float w, float h, float size)
{
    float xa = unk_F6A278[802 * currCl] * x;
    float ha = h * unk_F6A27C[802 * currCl];
    float sizea = size * unk_F6A278[802 * currCl];
    float v5 = unk_F6A27C[802 * currCl] * y;
    float wa = unk_F6A278[802 * currCl] * w;
    trap_R_DrawStretchPic(xa, v5, sizea, ha, 0.0f, 0.0f, 0.0f, 0.0f,
                          cgsGlobal_media_whiteShader, 0.0f);
    trap_R_DrawStretchPic((xa + wa) - sizea, v5, sizea, ha, 0.0f, 0.0f, 0.0f,
                          0.0f, cgsGlobal_media_whiteShader, 0.0f);
}

// ea: 0x00688EE0
void CG_DrawTopBottom(float x, float y, float w, float h, float size)
{
    float wa = w * unk_F6A278[802 * currCl];
    float v5 = unk_F6A278[802 * currCl] * x;
    float sizea = size * unk_F6A27C[802 * currCl];
    float ya = unk_F6A27C[802 * currCl] * y;
    float ha = unk_F6A27C[802 * currCl] * h;
    trap_R_DrawStretchPic(v5, ya, wa, sizea, 0.0f, 0.0f, 0.0f, 0.0f,
                          cgsGlobal_media_whiteShader, 0.0f);
    trap_R_DrawStretchPic(v5, (ya + ha) - sizea, wa, sizea, 0.0f, 0.0f, 0.0f,
                          0.0f, cgsGlobal_media_whiteShader, 0.0f);
}

// ea: 0x00688FA0
void CG_DrawPic(float x, float y, float width, float height, void* tex)
{
    trap_R_DrawStretchPic(unk_F6A278[802 * currCl] * x,
                          unk_F6A27C[802 * currCl] * y,
                          unk_F6A278[802 * currCl] * width,
                          unk_F6A27C[802 * currCl] * height, 0.0f, 0.0f, 1.0f,
                          1.0f, tex, 0.0f);
}

// ea: 0x00689020
void CG_DrawCroppedPic(float x, float y, float width, float height, float s1,
                       float t1, float s2, float t2, void* tex)
{
    trap_R_DrawStretchPic(unk_F6A278[802 * currCl] * x,
                          unk_F6A27C[802 * currCl] * y,
                          unk_F6A278[802 * currCl] * width,
                          unk_F6A27C[802 * currCl] * height, s1, t1, s2, t2,
                          tex, 0.0f);
}

// ea: 0x006890A0
void CG_DrawStringExt(float x, float y, const char* string,
                      const float* setColor, int forceColor, int shadow,
                      float charWidth, float charHeight, int maxChars,
                      int adjust)
{
    float vColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float v10 = (charHeight * 0.80000001f) + y;
    float v11 = charHeight * 0.041666668f;
    float ya = v10;
    float fFontScale = charHeight * 0.041666668f;
    if (adjust == 0)
    {
        float v12 = 1.0f / unk_F6A278[802 * currCl];
        float v13 = 1.0f / unk_F6A27C[802 * currCl];
        x = v12 * x;
        ya = v13 * v10;
        charWidth = v12 * charWidth;
        fFontScale = v13 * v11;
    }
    const float* v14 = setColor != nullptr ? setColor : vColor;
    trap_R_Text_Paint(x, ya, 5, fFontScale, v14, string, charWidth, maxChars,
                      shadow != 0 ? 3 : 0);
}

// ea: 0x00689180
void CG_DrawBigString(float x, float y, const char* s, float alpha)
{
    float vColor[4] = {1.0f, 1.0f, 1.0f, alpha};
    trap_R_Text_Paint(x, (y + 16.0f) - 2.0f, 4, 0.66666669f, vColor, s,
                      16.0f, 0, 3);
}

// ea: 0x006891F0
void CG_DrawBigStringColor(float x, float y, const char* s, const float* color)
{
    trap_R_Text_Paint(x, (y + 16.0f) - 2.0f, 4, 0.66666669f, color, s, 16.0f,
                      0, 3);
}

// ea: 0x00689240
void CG_DrawSmallString(float x, float y, const char* s, float alpha)
{
    float vColor[4] = {1.0f, 1.0f, 1.0f, alpha};
    trap_R_Text_Paint(x, (y + 12.0f) - 2.0f, 5, 0.5f, vColor, s, 8.0f, 0, 0);
}

// ea: 0x006892B0
void CG_DrawSmallStringColor(float x, float y, const char* s,
                             const float* color)
{
    trap_R_Text_Paint(x, (y + 12.0f) - 2.0f, 5, 0.5f, color, s, 8.0f, 0, 0);
}

// ea: 0x00689300
int CG_DrawStrlen(const char* str)
{
    const char* v1 = str;
    int result = 0;
    while (*v1 != 0)
    {
        if (*v1 == 94 && v1[1] != 0 && v1[1] != 94 && v1[1] >= 48
            && v1[1] <= 57)
        {
            v1 += 2;
        }
        else
        {
            ++result;
            ++v1;
        }
    }
    return result;
}

// ea: 0x0068B9A0
void CG_DrawScoreboard_GetTeamColor(int iTeam, float* vColor)
{
    if (iTeam == 1 || iTeam == 2)
    {
        Entity* p = EntityManager_GetPlayer(EntityManager_sInst, currCl);
        if (p->sentient == nullptr || p->sentient->eTeam == iTeam)
        {
            vColor[0] = 0.25f;
            vColor[1] = 1.0f;
            vColor[2] = 0.25f;
        }
        else
        {
            vColor[0] = 1.0f;
            vColor[1] = 0.25f;
            vColor[2] = 0.25f;
        }
    }
    else
    {
        vColor[0] = 1.0f;
        vColor[1] = 1.0f;
        vColor[2] = 1.0f;
    }
}

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

extern int cg_widescreen;
extern int cg_hudAlpha;
extern int cg_hudCompassSize;
extern int cg_drawPosition;
extern int cg_drawTimer;
extern int cg_minicon;
extern int cg_developer;
extern int cg_subtitles;
extern int cg_drawpaused;
extern int cg_drawGun;
extern int cg_crosshairAlpha;
extern int cg_crosshairDynamic;
extern int dword_F641D0[4 * 1580];
extern int dword_F641D4[4 * 1580];
extern float color[4];
extern int dword_F6400C[4 * 1580];
extern int dword_F62960[4 * 1580];
extern int dword_F6295C[4 * 1580];
extern int dword_F64154[4 * 1580];
extern int dword_F64158[4 * 1580];
extern int dword_F6415C[4 * 1580];
extern int dword_F64160[4 * 1580];
extern int gBlackStartTime[4];
extern int lastTime_0[4];
extern float unk_F6A284[4 * 802];
extern void Con_DrawNotify(int iXPos, int iYPos, float fAlpha, int eMode);
extern void Con_DrawBoldMessages(int iXPos, int iYPos, float fAlpha, int eMode);
extern void j_nullsub_72(int iXPos, int iYPos, float fAlpha);
extern void j_nullsub_121(int iXPos, int iYPos, float fAlpha, int eMode);
extern void CG_FillRect(float x, float y, float width, float height,
                        float* color, float z);
extern void SpinnerDrawFrame(bool bEndFrame);
extern int Sys_Milliseconds();
extern void StatMon_GetStatsArray(void** stats, int* count);
extern int GamePause_IsGamePaused(int client);
extern const char* CG_SafeTranslateString_Internal(const char* pszReference,
                                                   const char* pszSystem);
extern int trap_R_Text_Height(int font, float scale);
extern void SCR_UpdateScreen();
extern int lastDraw;
extern int callCount;
extern void* cgsGlobal_media_tracerShader;
extern void FastSinCos(float radians, float* psin, float* pcos);
extern void re_DrawQuadPic(const float* verts, const float* texCoords,
                           void* tex);
extern float* vST;
extern void* Cvar_Get(const char* var_name, const char* var_value, int flags);
extern void Cmd_Where_f(Entity* ent);
extern int dword_F641D0[4 * 1580];
extern int dword_F641D4[4 * 1580];

// ea: 0x0069C3C0
void CG_DrawRotatedPic(float x, float y, float width, float height,
                       float angle, void* tex)
{
    float fSin, fCos;
    FastSinCos((angle * 3.1415927f) * 0.0055555557f, &fSin, &fCos);
    float v6 = (unk_F6A27C[802 * currCl] * height) * 0.5f;
    float v7 = (width * unk_F6A278[802 * currCl]) * 0.5f;
    float v8 = (unk_F6A27C[802 * currCl] * y) + v6;
    float v9 = (x * unk_F6A278[802 * currCl]) + v7;
    float v14 = v6 * fCos;
    float v10 = v7 * fCos;
    float v11 = v9 - v10;
    float vVerts[8];
    vVerts[0] = v11 - ((v7 * fSin) * -1.0f);
    float negSin = (v6 * fSin) * -1.0f;
    vVerts[1] = (negSin + (v14 * -1.0f)) + v8;
    float v12 = v10 + v9;
    vVerts[2] = v12 - ((v7 * fSin) * -1.0f);
    vVerts[3] = ((v6 * fSin) + (v14 * -1.0f)) + v8;
    vVerts[4] = v12 - (v7 * fSin);
    vVerts[5] = (v14 + (v6 * fSin)) + v8;
    vVerts[6] = v11 - (v7 * fSin);
    vVerts[7] = (v14 + negSin) + v8;
    re_DrawQuadPic(vVerts, vST, tex);
}

// ea: 0x0069C540
void CG_DrawRotatedQuadPic(float x, float y, const float (*verts)[2],
                           const float (*texCoords)[2], float angle, void* tex)
{
    float s, c;
    FastSinCos((angle * 3.1415927f) * 0.0055555557f, &s, &c);
    float v6 = unk_F6A27C[802 * currCl] * y;
    float v7 = unk_F6A27C[802 * currCl] * s;
    float v8 = c * unk_F6A278[802 * currCl];
    float v9 = s * unk_F6A278[802 * currCl];
    float v10 = unk_F6A27C[802 * currCl] * c;
    float v11 = x * unk_F6A278[802 * currCl];
    float xy[8];
    xy[0] = ((verts[0][0] * v8) + v11) - (verts[0][1] * v9);
    xy[1] = ((verts[0][1] * v10) + (verts[0][0] * v7)) + v6;
    xy[2] = ((v8 * verts[1][0]) + v11) - (verts[1][1] * v9);
    xy[3] = ((verts[1][1] * v10) + (v7 * verts[1][0])) + v6;
    xy[4] = ((v8 * verts[2][0]) + v11) - (verts[2][1] * v9);
    xy[5] = ((verts[2][1] * v10) + (v7 * verts[2][0])) + v6;
    xy[6] = ((v8 * verts[3][0]) + v11) - (verts[3][1] * v9);
    xy[7] = ((verts[3][1] * v10) + (v7 * verts[3][0])) + v6;
    re_DrawQuadPic(xy, (const float*)texCoords, tex);
}

// ea: 0x006948B0
void CG_DrawUpperRight()
{
    if (*(int*)Cvar_Get("capture_movie", "", 0) == 0)
    {
        float y = 50.0f;
        if (cg_drawPosition != 0)
        {
            Entity* Player =
                EntityManager_GetPlayer(EntityManager_sInst, currCl);
            if (Player != nullptr)
                Cmd_Where_f(Player);
            char text[128];
            int i = 0;
            char v2;
            do
            {
                v2 = ((char*)&cg_drawPosition)[i];
                text[i++] = v2;
            } while (v2 != 0);
            float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            trap_R_Text_Paint(50.0f, 64.0f, 4, 0.66666669f, color, text,
                              16.0f, 0, 3);
        }
        if (cg_drawTimer != 0)
            y = CG_DrawTimer(50.0f);
        Entity* p = EntityManager_GetPlayer(EntityManager_sInst, currCl);
        if (p->takedamage == 0)
        {
            float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            trap_R_Text_Paint(500.0f, (y + 16.0f) - 2.0f, 4, 0.66666669f,
                              color, "No Damage", 16.0f, 0, 3);
        }
    }
}

// ea: 0x00695300
void CG_DrawFlashFade()
{
    int time = cgGlobal_time;
    if (lastTime_0[currCl] > cgGlobal_time)
        lastTime_0[currCl] = 0;
    int v2 = 1580 * currCl;
    int v3 = dword_F64160[1580 * currCl];
    if (v3 + dword_F6415C[1580 * currCl] < time)
    {
        dword_F64158[1580 * currCl] = dword_F64154[1580 * currCl];
    }
    else
    {
        float v4 = *(float*)&dword_F64158[1580 * currCl];
        if (v4 != *(float*)&dword_F64154[1580 * currCl])
        {
            int v5 = time - lastTime_0[currCl];
            lastTime_0[currCl] = time;
            if (v5 < 500 && v5 > 0)
            {
                float v8;
                bool v7;
                if (v4 <= *(float*)&dword_F64154[1580 * currCl])
                {
                    v8 = (v5 / (float)v3) + *(float*)&dword_F64158[1580 * currCl];
                    *(float*)&dword_F64158[1580 * currCl] = v8;
                    v7 = v8 <= *(float*)&dword_F64154[1580 * currCl];
                }
                else
                {
                    v8 = *(float*)&dword_F64158[1580 * currCl] - (v5 / (float)v3);
                    *(float*)&dword_F64158[1580 * currCl] = v8;
                    v7 = *(float*)&dword_F64154[1580 * currCl] <= v8;
                }
                if (!v7)
                    dword_F64158[1580 * currCl] = dword_F64154[1580 * currCl];
            }
        }
    }
    if (*(float*)&dword_F64158[1580 * currCl] <= 0.0f)
    {
        gBlackStartTime[currCl] = lastTime_0[currCl];
    }
    else
    {
        float col[4] = {0.0f, 0.0f, 0.0f, *(float*)&dword_F64158[v2]};
        int v9 = (int)unk_F6A284[802 * currCl];
        switch (v9)
        {
        case 3:
            CG_FillRect(0.0f, 0.0f, 640.0f, 240.0f, col, 0.0f);
            break;
        case 4:
            CG_FillRect(0.0f, 240.0f, 640.0f, 480.0f, col, 0.0f);
            break;
        case 5:
            CG_FillRect(0.0f, 0.0f, 320.0f, 240.0f, col, 0.0f);
            break;
        case 6:
            CG_FillRect(320.0f, 0.0f, 640.0f, 240.0f, col, 0.0f);
            break;
        case 7:
            CG_FillRect(0.0f, 240.0f, 320.0f, 480.0f, col, 0.0f);
            break;
        case 8:
            CG_FillRect(320.0f, 240.0f, 640.0f, 480.0f, col, 0.0f);
            break;
        default:
            CG_FillRect(0.0f, 0.0f, 640.0f, 480.0f, col, 0.0f);
            break;
        }
        if (*(float*)&dword_F64158[1580 * currCl] < 1.0f)
        {
            gBlackStartTime[currCl] = lastTime_0[currCl];
        }
        else
        {
            SpinnerDrawFrame(false);
            if (lastTime_0[currCl] - gBlackStartTime[currCl] > 30000)
            {
                int v11 = cgGlobal_time;
                dword_F64154[1580 * currCl] = 0;
                dword_F6415C[1580 * currCl] = v11;
                dword_F64160[1580 * currCl] = 0;
                if (dword_F6415C[1580 * currCl] <= v11)
                    dword_F64158[1580 * currCl] = dword_F64154[1580 * currCl];
            }
        }
    }
}

// ea: 0x006956A0
void CG_DrawGameMessages()
{
    float v0 = 50.0f;
    if (cg_widescreen == 0)
        v0 = 37.5f;
    if (dword_F641D0[1580 * currCl] == 0)
    {
        Con_DrawNotify((int)(v0 + 0.5f),
                       (int)(((342.0f - ((*(float*)&cg_hudCompassSize - 1.0f)
                                         * 115.0f))
                              - 20.0f)
                             + 0.5f),
                       *(float*)&cg_hudAlpha, 0 /* MWM_BOTTOMUP */);
        return;
    }
    int v1 = dword_F641D4[1580 * currCl];
    if (v1 != 0 && cgGlobal_time - v1 < 100)
    {
        float v3 = (100 - (cgGlobal_time - v1)) >= 100
                       ? 1.0f
                       : (100 - (cgGlobal_time - v1)) * 0.0099999998f;
        *(int*)&color[3] = *(int*)&v3;
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        Con_DrawNotify((int)(v0 + 0.5f),
                       (int)(((342.0f - ((*(float*)&cg_hudCompassSize - 1.0f)
                                         * 115.0f))
                              - 20.0f)
                             + 0.5f),
                       v3 * *(float*)&cg_hudAlpha, 0);
    }
}

// ea: 0x006957A0
void CG_DrawBoldGameMessages()
{
    if (dword_F641D0[1580 * currCl] == 0)
    {
        Con_DrawBoldMessages(320, 180, *(float*)&cg_hudAlpha, 1);
        return;
    }
    int v0 = dword_F641D4[1580 * currCl];
    if (v0 != 0 && cgGlobal_time - v0 < 100)
    {
        float v2 = (100 - (cgGlobal_time - v0)) >= 100
                       ? 1.0f
                       : (100 - (cgGlobal_time - v0)) * 0.0099999998f;
        *(int*)&color[3] = *(int*)&v2;
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        Con_DrawBoldMessages(320, 180, v2 * *(float*)&cg_hudAlpha, 1);
    }
}

// ea: 0x00695850
void CG_DrawMiniConsole()
{
    if (cg_minicon >= 0 && (cg_developer != 0 || cg_minicon != 0))
        j_nullsub_72(2, 4, *(float*)&cg_hudAlpha);
}

// ea: 0x00695880
void CG_DrawSubtitles()
{
    if (cg_subtitles != 0)
        j_nullsub_121(123, 399, *(float*)&cg_hudAlpha, 0);
}

// ea: 0x006958B0
int CG_DrawPerformanceWarnings()
{
    int v0 = Sys_Milliseconds();
    void* stats;
    int statCount;
    StatMon_GetStatsArray(&stats, &statCount);
    float x = 2.0f;
    float y = 200.0f;
    for (int v2 = 0; v2 < statCount; ++v2)
    {
        int* p = (int*)stats + 3 * v2;
        if (p[2] >= v0)
        {
            trap_R_DrawStretchPic(unk_F6A278[802 * currCl] * x,
                                  unk_F6A27C[802 * currCl] * y,
                                  unk_F6A278[802 * currCl] * 32.0f,
                                  unk_F6A27C[802 * currCl] * 32.0f, 0.0f, 0.0f,
                                  1.0f, 1.0f, (void*)p[1], 0.0f);
        }
        x += 34.0f;
        if ((x + 32.0f) > 68.0f)
        {
            x = 2.0f;
            y += 34.0f;
        }
    }
    return statCount;
}

// ea: 0x00695F90
void CG_DrawGameScreenFade()
{
    if (*(float*)&dword_F6400C[1580 * currCl] > 0.0f
        && dword_F62960[1580 * currCl] != 0)
    {
        float col[4] = {0.0f, 0.0f, 0.0f,
                        *(float*)&dword_F6400C[1580 * currCl]};
        CG_FillRect(0.0f, 0.0f, 640.0f, 480.0f, col, 0.0f);
    }
}

// ea: 0x00696000
void CG_DrawPaused()
{
    if (GamePause_IsGamePaused(currCl) && cg_drawpaused != 0)
    {
        const char* v0 = CG_SafeTranslateString_Internal("CGAME_PAUSED",
                                                         "cgame");
        float vColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        float fX = (640.0f - RE_Text_Width(v0, 0, 0.5f, 0.0f, 0)) * 0.5f;
        float fY = (480.0f - trap_R_Text_Height(0, 0.5f)) * 0.5f;
        trap_R_Text_Paint(fX, fY, 0, 0.5f, vColor, v0, 0.0f, 0, 6);
    }
}

// ea: 0x00696760
void CG_DrawViewportFrames(int numViewports)
{
    float col_black[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    switch (numViewports)
    {
    case 2:
        CG_FillRect(0.0f, 239.0f, 640.0f, 2.0f, col_black, 0.0f);
        break;
    case 3:
        CG_FillRect(0.0f, 239.0f, 640.0f, 2.0f, col_black, 0.0f);
        CG_FillRect(319.0f, 239.0f, 2.0f, 240.0f, col_black, 0.0f);
        break;
    case 4:
        CG_FillRect(0.0f, 239.0f, 640.0f, 2.0f, col_black, 0.0f);
        CG_FillRect(319.0f, 0.0f, 2.0f, 480.0f, col_black, 0.0f);
        break;
    default:
        break;
    }
}

// ea: 0x00697990
void CG_DrawInformation()
{
    if (dword_F6295C[1580 * currCl] == 0 && callCount == 0)
    {
        int v0 = Sys_Milliseconds();
        if (lastDraw > v0 || lastDraw <= v0 - 100)
        {
            lastDraw = v0;
            ++callCount;
            SCR_UpdateScreen();
            --callCount;
        }
    }
}

extern int dword_F63584[4 * 1580];
extern int dword_F64180[4 * 1580];
extern int dword_F6A2AC[4 * 3208];
extern int dword_F6A28C[4 * 802];
extern int dword_F6355C[4 * 1580];
extern float dword_F63C50[4 * 1580];
extern float dword_F63C54[4 * 1580];
extern float dword_F63C58[4 * 1580];
extern float dword_F63C5C[4 * 1580];
extern int cg_shellshockblur;
extern int gSaveGameData_mCrosshair;
extern int cg_drawpaused;
extern int cg_drawGun;
extern void* cg_weapons;
extern re_export_view re;
extern void* BG_GetInfoForWeapon(int weapon);
extern PlayerState* GetPlayerState(int idx);
extern float* CG_FadeColor(int startMsec, int totalMsec, int fadeMsec);
extern void CG_FillRect(float x, float y, float width, float height,
                        float* color, float z);
extern void CG_AdjustFrom640(float* x, float* y, float* w, float* h);
extern void trap_R_SetColor(const float* rgba);
extern void CG_GetCenterOfScreen(float* x, float* y);
class EntityHandleDbLocal2;
class EntityHandleDbLocal2 {
public:
    struct DbElement {
        Entity* mObject;
        int mKey;
    };
    DbElement mElements[0x540];
};
extern EntityHandleDbLocal2 EntityHandleDb_sInst2;
static Entity* EntityHandleDb_Get2(unsigned int handleVal)
{
    unsigned int v = handleVal & 0xFFF;
    if (v < 0x540
        && handleVal >> 12
               == EntityHandleDb_sInst2.mElements[v].mKey)
        return EntityHandleDb_sInst2.mElements[v].mObject;
    return nullptr;
}

// ea: 0x0068BA90
void CG_DrawObjectives()
{
}

// ea: 0x0068BAA0
int CG_DrawScoreboard()
{
    if ((GamePause_IsGamePaused(currCl) && cg_drawpaused != 0)
        || dword_F63584[1580 * currCl] >= 6
        || dword_F641D0[1580 * currCl] == 0)
    {
        return 0;
    }
    float* v1 = CG_FadeColor(dword_F641D4[1580 * currCl], 100, 100);
    if (v1 != nullptr)
        v1[3] = 1.0f - v1[3];
    return 1;
}

// ea: 0x006983C0
int CG_DrawShellShockSavedScreenBlend(const void* parms, int start,
                                      int duration)
{
    if (cg_shellshockblur == 0)
        return 1;
    if (start != 0 && duration > 0 && duration + start - cgGlobal_time > 0)
    {
        re.SaveScreen();
        dword_F64180[1580 * currCl] = 1;
        return 1;
    }
    dword_F64180[1580 * currCl] = 0;
    return 0;
}

// ea: 0x006A0030
void CG_DrawTurretCrossHair()
{
    int hcolor[3] = {1065353216, 1065353216, 1065353216};
    float value = 0.0f;
    char v0 = *(char*)&dword_F6A2AC[3208 * currCl];
    if (v0 != 0
        && (!GamePause_IsGamePaused(currCl) || cg_drawpaused == 0)
        && dword_F6355C[1580 * currCl] == 0
        && dword_F6A28C[802 * currCl] == 0
        && gSaveGameData_mCrosshair)
    {
        PlayerState* ps = GetPlayerState(currCl);
        Entity* v2 = EntityHandleDb_Get2(ps->mViewLockedEntity);
        if (v2 != nullptr && v2->s.eType == 10)
        {
            int weapon = v2->s.weapon;
            if (weapon != 0)
            {
                weaponFileInfoFull* InfoForWeapon =
                    (weaponFileInfoFull*)BG_GetInfoForWeapon(weapon);
                weaponInfo_s* v5 = &((weaponInfo_s*)cg_weapons)[weapon];
                char v6 = ((char*)InfoForWeapon)[0x280];
                if (v6 != 0)
                {
                    if (v6 == 84)
                    {
                        float col[4] = {0.0f, 0.0f, 0.0f, 0.6f};
                        CG_FillRect(310.0f, 240.0f, 20.0f, 2.0f, col, 0.0f);
                        CG_FillRect(319.0f, 242.0f, 2.0f, 8.0f, col, 0.0f);
                    }
                    else
                    {
                        value = *(float*)&cg_crosshairAlpha;
                        if (value >= 0.0099999998f)
                        {
                            trap_R_SetColor((const float*)hcolor);
                            float x = 0.0f, y = 0.0f;
                            float w = (float)InfoForWeapon->iReticleCenterSize;
                            float h = w;
                            CG_AdjustFrom640(&x, &y, &w, &h);
                            trap_R_DrawStretchPic(
                                (((dword_F63C58[1580 * currCl] - w) * 0.5f)
                                 + dword_F63C50[1580 * currCl])
                                    + x,
                                (((dword_F63C5C[1580 * currCl] - h) * 0.5f)
                                 + dword_F63C54[1580 * currCl])
                                    + y,
                                w, h, 0.0f, 0.0f, 1.0f, 1.0f,
                                v5->hReticleCenter, 0.0f);
                        }
                    }
                }
            }
        }
    }
}

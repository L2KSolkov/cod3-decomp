// ============================================================================
// cg_draw.cpp - 2D draw helpers (cg.o cg_draw.cpp)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// Minimal view of GamePause (full class in game/sv/sv_stubs.h).
struct GamePause { static bool IsGamePaused(int client); };


// Minimal view of SoundDevice (full class in game/sv/sv_stubs.h).
class SoundDevice { public: static SoundDevice* sInst; };  // ?sInst@SoundDevice@@2PAV1@A


extern const char* CL_GetConfigString(int index);  // ?CL_GetConfigString@@YAPBDH@Z (cl.o)


// Minimal view of EntityManager (full class in game/sv/sv_stubs.h).
class EntityManager {
public:
    static EntityManager* sInst;  // ?sInst@EntityManager@@2PAV1@A (game.o)
    Entity* GetPlayer(int idx);   // ?GetPlayer@EntityManager@@QAEPAVEntity@@H@Z
    Entity* mPlayers[16];         // +0x04
};


extern int currCl;
int cgGlobal_time = 0;  // ?cgGlobal_time@@3HA (cg.o)
extern float unk_F6A278[4 * 802];
extern float unk_F6A27C[4 * 802];
extern void* cgsGlobal_media_whiteShader;
extern char* va(const char* fmt, ...);
extern int RE_Text_Width(const char* text, int font, float scale,
                         float charWidth, int limit);
extern void trap_R_Text_Paint(float x, float y, int font, float scale,
                              const float* color, const char* text,
                              float charWidth, int limit, int style);
extern void trap_R_DrawStretchPic(float x, float y, float w, float h, float s1,
                                  float t1, float s2, float t2, void* tex,
                                  float z);
extern void trap_R_RenderScene(const void* fd);
struct parseInfo_t {
    char token[128];         // +0x00
    int lines;               // +0x80
    int ungetToken;          // +0x84
    int spaceDelimited;      // +0x88
    int csv;                 // +0x8C
    int negativeNumbers;     // +0x90
    int backup_lines;        // +0x94
    const char* backup_text; // +0x98
};
extern parseInfo_t* Com_ParseOnLine(const char** data);
extern float CG_GetViewFov();
extern void CG_Error(const char* msg, ...);
extern int CG_DrawSingleHudElem(void* elem);
extern int compare_hudelems(const void* pe0, const void* pe1);
int dword_F63CA4[4 * 1580];  // cg.o BSS
int dword_F63CA8[4 * 1580];  // cg.o BSS
float dword_F63C60[4 * 1580];  // cg.o BSS
float dword_F63C58[4 * 1580];  // cg.o BSS
float dword_F63C5C[4 * 1580];  // cg.o BSS
float dword_F63C8C[4 * 1580];  // cg.o BSS
float dword_F63C90[4 * 1580];  // cg.o BSS
float dword_F63C94[4 * 1580];  // cg.o BSS
float dword_F63CB4[4 * 1580];  // cg.o BSS
int dword_F63BAC[4 * 1580];    // cg.o BSS
int dword_F63BB4[4 * 1580];    // cg.o BSS
int dword_F64018[4 * 1580];    // cg.o BSS
float dword_F63C64[4 * 1580];  // cg.o BSS
float dword_F63C70[4 * 1580];  // cg.o BSS
float dword_F63C74[4 * 1580];
float dword_F63C78[4 * 1580];
float dword_F63C50[4 * 1580];  // cg.o BSS
// --- cg.o BSS sweep (anonymous .bss region 0xF62940-0xF64200) ---
float dword_F63C54[4 * 1580];  // cg.o BSS
float dword_F63CB8[4 * 1580];  // cg.o BSS
float dword_F63CF0[4 * 1580];  // cg.o BSS
float dword_F63B70[4 * 1580];  // cg.o BSS
float dword_F63BB0[4 * 1580];  // cg.o BSS
float dword_F63CC0[4 * 1580];
float dword_F63CC4[4 * 1580];
float dword_F63CC8[4 * 1580];
float dword_F63CD0[4 * 1580];
float dword_F63CD4[4 * 1580];
float dword_F63CE4[4 * 1580];
float dword_F63CE8[4 * 1580];
float dword_F63F34[4 * 1580];
float dword_F63F38[4 * 1580];
float dword_F63F3C[4 * 1580];
float dword_F63F40[4 * 1580];
float dword_F63F44[4 * 1580];
float dword_F63F48[4 * 1580];
float dword_F63F4C[4 * 1580];
float dword_F63F50[4 * 1580];
float dword_F63F54[4 * 1580];
float dword_F63F58[4 * 1580];
float dword_F63550[4 * 1580];
float dword_F64068[4 * 1580];
float dword_F6406C[4 * 1580];
float dword_F64070[4 * 1580];
float dword_F64074[4 * 1580];
float dword_F64078[4 * 1580];
float dword_F6407C[4 * 1580];
float dword_F641D8[4 * 1580];
float dword_F641DC[4 * 1580];
float unk_F63BC4[4 * 1580 * 4];
float unk_F63BF4[4 * 6320];
float unk_F63C24[4 * 6320];
float unk_F64080[4 * 6320];
float unk_F640A8[4 * 6320];
float unk_F6A278[4 * 802];
float unk_F6A27C[4 * 802];
float unk_F6A284[4 * 802];
float unk_F6A288[4 * 802];
int dword_F62944[4 * 1580];
int dword_F62948[4 * 1580];
int dword_F6294C[4 * 1580];
int dword_F62954[4 * 1580];
int dword_F62958[4 * 1580];
int dword_F63554[4 * 1580];
int dword_F63584[4 * 1580];
int dword_F63B34[4 * 1580];
int dword_F63BA4[4 * 1580];
int dword_F63BA8[4 * 1580];
int dword_F63BB8[4 * 1580];
int dword_F63BBC[4 * 1580];
int dword_F63BC0[4 * 1580];
int dword_F63BE8[4 * 1580];
int dword_F63BEC[4 * 1580];
int dword_F63BF0[4 * 1580];
int dword_F63C18[4 * 1580];
int dword_F63C1C[4 * 1580];
int dword_F63C20[4 * 1580];
int dword_F63CF4[4 * 1580];
int dword_F63D1C[4 * 1580];
int dword_F63F9C[4 * 1580 * 3];
int dword_F63FA0[4 * 1580 * 3];
int dword_F63FA8[4 * 1580 * 3];
int dword_F63FFC[4 * 1580];
int dword_F6400C[4 * 1580];
int dword_F6401C[4 * 1580];
int dword_F64020[4 * 1580];
int dword_F64024[4 * 1580];
int dword_F64028[4 * 1580];
int dword_F6402C[4 * 1580];
int dword_F64030[4 * 1580];
int dword_F64034[4 * 1580];
int dword_F64038[4 * 1580];
int dword_F64050[4 * 1580];
int dword_F64054[4 * 1580];
int dword_F64058[4 * 1580];
int dword_F6405C[4 * 1580];
int dword_F64060[4 * 1580];
int dword_F640A4[4 * 1580];
int dword_F640C4[4 * 1580];
int dword_F640E8[4 * 1580];
int dword_F6410C[4 * 1580];
int dword_F64130[4 * 1580];
int dword_F64138[4 * 1580];
int dword_F6413C[4 * 1580];
int dword_F64140[4 * 1580];
int dword_F64144[4 * 1580];
int dword_F64148[4 * 1580];
int dword_F6414C[4 * 1580];
int dword_F64150[4 * 1580];
int dword_F64154[4 * 1580];
int dword_F64158[4 * 1580];
int dword_F6415C[4 * 1580];
int dword_F64160[4 * 1580];
int dword_F64164[4 * 1580];
int dword_F64168[4 * 1580];
int dword_F6416C[4 * 1580];
int dword_F64174[4 * 1580];
int dword_F64178[4 * 1580];
int dword_F6417C[4 * 1580];
int dword_F64180[4 * 1580];
int dword_F64184[4 * 1580];
int dword_F64188[4 * 1580];
int dword_F6418C[4 * 1580];
int dword_F64190[4 * 1580];
int dword_F641B0[4 * 1580];
int dword_F641B8[4 * 1580];
int dword_F641BC[4 * 1580];
int dword_F641C8[4 * 1580];
int dword_F641CC[4 * 1580];
int dword_F641D0[4 * 1580];
int dword_F641D4[4 * 1580];
int dword_F641E4[4 * 1580];
int dword_F641E8[4 * 1580];
int dword_F641EC[4 * 1580];
int dword_F6A2A0[4 * 802];
int dword_F6A2A4[4 * 802];
int dword_F6A2A8[4 * 802];
int dword_F6A2AC[4 * 3208];
unsigned char byte_F64194[4 * 6320];
unsigned char byte_F641C0[4 * 6320];
unsigned char unk_F6A294[4 * 3208];
struct game_hudelem_s {
    struct {
        int type;  // +0x00
    } elem;        // +0x00
    unsigned char _pad[0x7C - 0x04];
};
extern game_hudelem_s g_hudelems[16];
struct sentient_s {
    int eTeam;  // +0x00
};

struct vmCvar_t {
    int   integer;  // +0x00
    float value;    // +0x04
};
extern vmCvar_t cg_skybox;

struct cgGlobal_t {
    int frametime;    // +0x00
    int time;         // +0x04
    int oldTime;      // +0x08
    int cubemapShot;  // +0x0C
    int cubemapSize;  // +0x10
};
extern cgGlobal_t cgGlobal;

float gTracerDistScale;   // 0x00DF9DA8
float tr_viewParms_zFar;  // 0x00F74F60
struct nglMesh;
extern nglMesh* auxCreateScratchMesh(int flags, int num);
extern void* nglListAlloc(unsigned int bytes, unsigned int alignment);
extern void* cdScratchMaterial_Ctor(void* self, void* tex,
                                    unsigned int blendMode, int mapflags,
                                    bool heatHaze);
struct gpuVertexFormat;
struct nglMeshSection;
struct nglMaterial;
extern nglMeshSection* nglCreateScratchSection(int prim, int nIndices,
                                               int nVertices,
                                               gpuVertexFormat* vertexFormat);
extern void nglAddMeshSection(nglMesh* mesh, nglMeshSection* section,
                              nglMaterial* material, int flags);
extern void* nglLockSectionIndices(nglMeshSection* section);
extern void* nglLockSectionVertices(nglMeshSection* section);
struct nglMeshParams;
struct nglShaderParamSet;
struct nglMeshNode;
extern nglMeshNode* nglListAddMesh(nglMesh* mesh,
                                   const math::Mat43& localToWorld,
                                   nglMeshParams* meshParams,
                                   nglShaderParamSet* shaderParams,
                                   void (*fn)(nglMeshNode*));
extern void* cdscratch_vertex_format;
extern void* cgsGlobal_media_tracerShader;
int dword_F6355C[4 * 1580];  // cg.o BSS
extern int dword_F62948[4 * 1580];
extern int dword_F640A4[4 * 1580];
extern int dword_F64158[4 * 1580];
extern int dword_F64164[4 * 1580];
extern int dword_F64168[4 * 1580];
extern int dword_F6416C[4 * 1580];
extern int dword_F64174[4 * 1580];
extern int dword_F6418C[4 * 1580];
extern int dword_F64190[4 * 1580];
extern int dword_F63554[4 * 1580];
extern float dword_F63CF0[4 * 1580];
extern int dword_F63CA4[4 * 1580];
extern int dword_F63CA8[4 * 1580];
extern float dword_F63C60[4 * 1580];
extern float dword_F63C64[4 * 1580];
extern float* dword_F63B8C[4 * 1580];
int dword_F6403C[4 * 1580];  // cg.o BSS
int dword_F64040[4 * 1580];  // cg.o BSS
int dword_F64044[4 * 1580];  // cg.o BSS
int dword_F64048[4 * 1580];  // cg.o BSS
int dword_F6404C[4 * 1580];  // cg.o BSS
extern vmCvar_t cg_crosshairAlpha;
extern vmCvar_t cg_crosshairDynamic;
extern vmCvar_t cg_drawGun;
extern vmCvar_t cg_draw2D;
extern vmCvar_t cg_drawStatus;
extern vmCvar_t cg_norender;
extern vmCvar_t cg_thirdPerson;
extern bool gStillDrawMenus;
extern int gRenderCG_2D;
extern int gRenderViewWeapon;
extern bool gFirstCamera;
extern int g_DOBJF_NOT_RENDERED_LAST_FRAME;
float move_back_distance;
extern int curListener;
struct SaveGameData;
extern SaveGameData* gSaveGameData;
struct FEManager; extern FEManager g_femanager;
extern void* nglBuildScene_RenderTarget;
void* gCurrentCamera;  // ?gCurrentCamera (cg.o Camera* artifact)
extern void* gCamera;
extern void Camera_Update(void* self);
extern void Camera_UpdatePostViewModels(void* self);
extern int LocalClient_FirstLocalClientIndex();
extern int G_GetServerSnapTime();
extern int CG_UpdateCvars();
extern int CG_ProcessSnapshots();
extern void CG_PredictPlayerState_Internal();
struct shellshock_parms_t;
extern void CG_UpdateShellShock(const shellshock_parms_t* parms, int start,
                                int duration);
extern void CG_CalcCubemapViewValues();
extern void CG_CalcVrect(const void* window);
extern int CG_CalcFov();
extern void CG_ShakeCamera(int client);
extern void CG_PerturbCamera();
extern void CG_AddViewWeapon(PlayerState* ps);
extern void CL_SetUserCmdValue(int userCmdValue, int holdableValue,
                               float sensitivityScale);
extern void CL_SetUserCmdAimValues(float gunPitch, float gunYaw,
                                   float gunXOfs, float gunYOfs,
                                   float gunZOfs);
extern void R_ToggleSmpFrame();
extern void CG_DrawViewportFrames(int numViewports);
extern void SoundDevice_SetListenerVectors(void* self, int listener,
                                           const float* position,
                                           const float* front,
                                           const float* up);
extern int SoundDevice_GetNumberOfListeners(void* self);
extern void subtitle_manager_render();
extern bool FEManager_InGameMenusActive(void* self, int client);
extern void FEManager_DrawIGO(void* self, int client);
struct cvar_t;
extern cvar_t* Cvar_Get(const char* name, const char* value, int flags);
extern void CG_DrawFlashDamage();
extern void CG_DrawDamageDirectionIndicators();
extern void CG_DrawPlayerLowHealthOverlay();
extern void CG_DrawCenterString();
extern void CG_ScreenFade();
extern Entity* GetPlayer(int idx);
extern bool IsPlayerFullySeatedInVehicle(Entity* player);
extern bool BG_AllowPlayerWeaponAtVehiclePos(int vehType, int vehPos);
extern int BG_GetWeaponForInfo(weaponFileInfo_t* pWeapInfo);
extern void CG_DrawWeapReticle();
extern void CG_CalcCrosshairColor(float alpha, int* color);
extern void CG_CalcCrosshairPosition(float* pfX, float* pfY);
extern void CG_DrawReticleHitIndicator(void* weapDef, int weapIndex,
                                       int* baseColor, float centerX,
                                       float centerY, float transScale);
extern void CG_TransitionToAds(void* weapDef, float posLerp,
                               float* transScale, float* transShift);
extern void CG_DrawAdsAimIndicator(void* weapDef, int weapIndex, int* color,
                                   float centerX, float centerY,
                                   float transScale);
extern void CG_DrawReticleName(int* color);
extern void CG_DrawReticleCenter(void* weapDef, int weapIndex, int* color,
                                 float centerX, float centerY,
                                 float transScale);
extern void CG_DrawReticleSides(void* weapDef, int weapIndex, int* baseColor,
                                float centerX, float centerY,
                                float transScale);
extern bool CG_AllowedToDrawCrosshair();
extern int CG_ForceDebugCrosshair();
enum nglSceneParamType;
struct nglScene;
extern nglScene* nglListBeginScene(nglSceneParamType paramSource);
extern void nglSetClearFlags(unsigned int flags);
extern void nglSetZTestEnable(bool enable);
extern void nglSetZWriteEnable(bool enable);
extern nglScene* nglListEndScene();
extern void nglSetView(float x1, float y1, float x2, float y2);
extern void nglSetScissor(float x1, float y1, float x2, float y2);
extern void CG_DrawCrosshair(float transScale);
extern void CG_Draw2D(float a2);
extern void CG_DrawActive(float a1);
extern char cgsGlobal_shellshockParms[0x7C];
extern void View_SetViewportClipping(int clientIndex);
extern int View_lNumViewports;
int cg_aWeaponSelect[4];  // ?cg_aWeaponSelect@@3PAHA (cg.o)
int cg_clientFrame[4 * 1580];  // ?cg_clientFrame (cg.o)
extern void FastSinCos(float radians, float* psin, float* pcos);
extern void AnglesToAxis(const float* angles, float (*axis)[3]);
float g_TestForward[4];
extern void InspectorManager_Render(void* self);
class InspectorManager;
extern InspectorManager g_inspectorManager;
int dword_F62964[4 * 1580];  // cg.o BSS
extern void CheckAndRunOverHeatBlur();
extern void trap_R_ClearScene();
extern float angle[4 * 395];
extern float dword_F63C80[4 * 1580];
extern float dword_F63C84[4 * 1580];
extern float dword_F63C88[4 * 1580];
float dword_F63C98[4 * 1580];  // cg.o BSS
float dword_F63C9C[4 * 1580];  // cg.o BSS
float dword_F63CA0[4 * 1580];  // cg.o BSS


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
    return EntityManager::sInst->GetPlayer( currCl);
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
        Entity* p = EntityManager::sInst->GetPlayer( currCl);
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

extern vmCvar_t cg_widescreen;
extern vmCvar_t cg_hudAlpha;
extern vmCvar_t cg_hudCompassSize;
extern vmCvar_t cg_drawPosition;
extern vmCvar_t cg_drawTimer;
extern vmCvar_t cg_minicon;
extern vmCvar_t cg_developer;
extern vmCvar_t cg_subtitles;
extern vmCvar_t cg_drawpaused;
extern vmCvar_t cg_drawGun;
extern vmCvar_t cg_crosshairAlpha;
extern vmCvar_t cg_crosshairDynamic;
extern int dword_F641D0[4 * 1580];
extern int dword_F641D4[4 * 1580];
float color[4];
extern int dword_F6400C[4 * 1580];
extern int dword_F62960[4 * 1580];
int dword_F6295C[4 * 1580];  // cg.o BSS
extern int dword_F64154[4 * 1580];
extern int dword_F64158[4 * 1580];
extern int dword_F6415C[4 * 1580];
extern int dword_F64160[4 * 1580];
int gBlackStartTime[4];  // ?gBlackStartTime@@3PAHA (cg.o)
extern int lastTime_0[4];
extern float unk_F6A284[4 * 802];
enum msgwnd_mode_t;
extern void Con_DrawNotify(int iXPos, int iYPos, float fAlpha,
                           msgwnd_mode_t eMode);
extern void Con_DrawBoldMessages(int iXPos, int iYPos, float fAlpha,
                                 msgwnd_mode_t eMode);
extern void j_nullsub_72(int iXPos, int iYPos, float fAlpha);
extern void j_nullsub_121(int iXPos, int iYPos, float fAlpha, int eMode);
extern void CG_FillRect(float x, float y, float width, float height,
                        const float* color, float z);
extern void SpinnerDrawFrame(bool bEndFrame);
extern int Sys_Milliseconds();
struct statmonitor_s;
extern void StatMon_GetStatsArray(const statmonitor_s** stats, int* count);
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
float* vST;
extern cvar_t* Cvar_Get(const char* var_name, const char* var_value,
                        int flags);
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
        if (cg_drawPosition.integer != 0)
        {
            Entity* Player =
                EntityManager::sInst->GetPlayer( currCl);
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
        if (cg_drawTimer.integer != 0)
            y = CG_DrawTimer(50.0f);
        Entity* p = EntityManager::sInst->GetPlayer( currCl);
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
    if (cg_widescreen.integer == 0)
        v0 = 37.5f;
    if (dword_F641D0[1580 * currCl] == 0)
    {
        Con_DrawNotify((int)(v0 + 0.5f),
                       (int)(((342.0f - ((*(float*)&cg_hudCompassSize - 1.0f)
                                         * 115.0f))
                              - 20.0f)
                             + 0.5f),
                       *(float*)&cg_hudAlpha,
                       (msgwnd_mode_t)0 /* MWM_BOTTOMUP */);
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
                       v3 * *(float*)&cg_hudAlpha, (msgwnd_mode_t)0);
    }
}

// ea: 0x006957A0
void CG_DrawBoldGameMessages()
{
    if (dword_F641D0[1580 * currCl] == 0)
    {
        Con_DrawBoldMessages(320, 180, *(float*)&cg_hudAlpha,
                             (msgwnd_mode_t)1);
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
        Con_DrawBoldMessages(320, 180, v2 * *(float*)&cg_hudAlpha,
                             (msgwnd_mode_t)1);
    }
}

// ea: 0x00695850
void CG_DrawMiniConsole()
{
    if (cg_minicon.integer >= 0 && (cg_developer.integer != 0 || cg_minicon.integer != 0))
        j_nullsub_72(2, 4, *(float*)&cg_hudAlpha);
}

// ea: 0x00695880
void CG_DrawSubtitles()
{
    if (cg_subtitles.integer != 0)
        j_nullsub_121(123, 399, *(float*)&cg_hudAlpha, 0);
}

// ea: 0x006958B0
int CG_DrawPerformanceWarnings()
{
    int v0 = Sys_Milliseconds();
    const statmonitor_s* stats;
    int statCount;
    StatMon_GetStatsArray(&stats, &statCount);
    float x = 2.0f;
    float y = 200.0f;
    for (int v2 = 0; v2 < statCount; ++v2)
    {
        const int* p = (const int*)stats + 3 * v2;
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
    if (GamePause::IsGamePaused(currCl) && cg_drawpaused.integer != 0)
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
extern vmCvar_t cg_shellshockblur;
int gSaveGameData_mCrosshair;  // ?gSaveGameData_mCrosshair@@3HA (g.o)
extern vmCvar_t cg_drawpaused;
extern vmCvar_t cg_drawGun;
extern void* cg_weapons;
extern re_export_view re;
extern weaponFileInfo_t* BG_GetInfoForWeapon(int weapon);
extern PlayerState* GetPlayerState(int idx);
extern float* CG_FadeColor(int startMsec, int totalMsec, int fadeMsec);
extern void CG_FillRect(float x, float y, float width, float height,
                        const float* color, float z);
extern void CG_AdjustFrom640(float* x, float* y, float* w, float* h);
extern void trap_R_SetColor(const float* rgba);
extern void CG_GetCenterOfScreen(float* x, float* y);
static Entity* EntityHandleDb_Get2(unsigned int handleVal)
{
    unsigned int v = handleVal & 0xFFF;
    if (v < 0x540
        && handleVal >> 12
               == EntityHandleDb::sInst.mElements[v].mKey)
        return EntityHandleDb::sInst.mElements[v].mObject;
    return nullptr;
}

// ea: 0x0068BA90
void CG_DrawObjectives()
{
}

// ea: 0x0068BAA0
int CG_DrawScoreboard()
{
    if ((GamePause::IsGamePaused(currCl) && cg_drawpaused.integer != 0)
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
    if (cg_shellshockblur.integer == 0)
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
        && (!GamePause::IsGamePaused(currCl) || cg_drawpaused.integer == 0)
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
// ea: 0x0068BA80
float CG_DrawObjective(const void* pObjective, float a2, float* a3, float& a4,
                       float& a5, float& a6, float& a7, float& a8, float& a9,
                       bool a10)
{
    return 0.0f;
}

static int s_foginited;

// ea: 0x0068DBB0
void CG_DrawSkyBoxPortal()
{
    const char* ConfigString = CL_GetConfigString(10);
    const char* x = ConfigString;
    if (ConfigString != nullptr)
    {
        if (strlen(ConfigString) != 0)
        {
            int v2 = 1580 * currCl;
            char v32[108];
            memcpy(v32, &dword_F63C50[1580 * currCl], 0x60);
            if (cg_skybox.integer == 0)
            {
                int v30 = dword_F63CA8[1580 * currCl] & 0xFFFFFFE7 | 8;
            done:
                int time = cgGlobal_time;
                dword_F63CA8[v2] = v30;
                dword_F63CA4[v2] = time;
                trap_R_RenderScene(&dword_F63C50[v2]);
                memcpy(&dword_F63C50[1580 * currCl], v32, 96);
                return;
            }
            CG_ASSERT("Dead code reached", "c:\\cod\\code\\game\\cg_view.cpp",
                      1542);
            const char* v3 = Com_ParseOnLine(&x)->token;
            const char* v4 = v3;
            if (v3 == nullptr || *v3 == 0)
                CG_Error("CG_DrawSkyBoxPortal: error parsing skybox "
                         "configstring\n");
            dword_F63C70[1580 * currCl] = (float)atof(v4);
            const char* v6 = Com_ParseOnLine(&x)->token;
            const char* v7 = v6;
            if (v6 == nullptr || *v6 == 0)
                CG_Error("CG_DrawSkyBoxPortal: error parsing skybox "
                         "configstring\n");
            dword_F63C74[1580 * currCl] = (float)atof(v7);
            const char* v9 = Com_ParseOnLine(&x)->token;
            const char* v10 = v9;
            if (v9 == nullptr || *v9 == 0)
                CG_Error("CG_DrawSkyBoxPortal: error parsing skybox "
                         "configstring\n");
            dword_F63C78[1580 * currCl] = (float)atof(v10);
            const char* v12 = Com_ParseOnLine(&x)->token;
            const char* v13 = v12;
            if (v12 == nullptr || *v12 == 0)
                CG_Error("CG_DrawSkyBoxPortal: error parsing skybox "
                         "configstring\n");
            atoi(v13);
            const char* v14 = Com_ParseOnLine(&x)->token;
            if (v14 == nullptr || *v14 == 0)
            {
                CG_Error("CG_DrawSkyBoxPortal: error parsing skybox "
                         "configstring.  No fog state\n");
                goto label_44;
            }
            if (atoi(v14) != 0)
            {
                const char* v15 = Com_ParseOnLine(&x)->token;
                const char* v16 = v15;
                if (v15 == nullptr || *v15 == 0)
                    CG_Error("CG_DrawSkyBoxPortal: error parsing skybox "
                             "configstring.  No fog[0]\n");
                *(float*)&v32[100] = (float)atof(v16);
                const char* v17 = Com_ParseOnLine(&x)->token;
                const char* v18 = v17;
                if (v17 == nullptr || *v17 == 0)
                    CG_Error("CG_DrawSkyBoxPortal: error parsing skybox "
                             "configstring.  No fog[1]\n");
                *(float*)&v32[104] = (float)atof(v18);
                const char* v19 = Com_ParseOnLine(&x)->token;
                const char* v20 = v19;
                if (v19 == nullptr || *v19 == 0)
                    CG_Error("CG_DrawSkyBoxPortal: error parsing skybox "
                             "configstring.  No fog[2]\n");
                float v33 = (float)atof(v20);
                const char* v21 = Com_ParseOnLine(&x)->token;
                int v22 = (v21 != nullptr && *v21 != 0) ? atoi(v21) : 0;
                const char* v23 = Com_ParseOnLine(&x)->token;
                int v24 = (v23 != nullptr && *v23 != 0) ? atoi(v23) : 0;
                re.SetFog(2, v22, v24, *(float*)&v32[100], *(float*)&v32[104],
                          v33, 1.1f);
            }
            else
            {
                if (s_foginited != 0)
                {
                label_44:
                    float fogColor3 = CG_GetViewFov();
                    v2 = 1580 * currCl;
                    float v35 = dword_F63C58[1580 * currCl]
                                / tanf(fogColor3 * 0.0087266462f);
                    float v36 = dword_F63C5C[1580 * currCl];
                    float fogColor2 = fabsf(v35);
                    float fov_x = fabsf(v36);
                    float v26;
                    if (0.0f == fov_x + fogColor2)
                    {
                        v26 = 0.0f;
                    }
                    else
                    {
                        float invLen = 1.0f
                                       / sqrtf(v36 * v36 + v35 * v35);
                        if (fov_x <= fogColor2)
                        {
                            float v29 = invLen * fov_x;
                            fov_x = v29;
                            if (v29 >= 0.5f)
                            {
                                fov_x = sqrtf(fabsf((1.0f - fov_x) * 0.5f));
                                float x6 = (fov_x * fov_x) * (fov_x * fov_x)
                                           * (fov_x * fov_x);
                                float x4 = (fov_x * fov_x) * (fov_x * fov_x);
                                float x3 = (fov_x * fov_x) * fov_x;
                                v26 = x6 * -0.1079625f - x4 * 0.15000001f
                                      - x3 * 0.33333331f - fov_x * 2.0f
                                      + 1.570796f;
                            }
                            else
                            {
                                v26 = (((((((v29 * v29) * v29) * (v29 * v29))
                                          * (v29 * v29))
                                         * 0.053981241f)
                                        + ((((v29 * v29) * v29) * (v29 * v29))
                                           * 0.075000003f))
                                       + (((v29 * v29) * v29) * 0.1666667f))
                                      + v29;
                            }
                        }
                        else
                        {
                            fov_x = invLen * fogColor2;
                            float v28;
                            if ((invLen * fogColor2) >= 0.5f)
                            {
                                fov_x = sqrtf(fabsf((1.0f - fov_x) * 0.5f));
                                float x6 = (fov_x * fov_x) * (fov_x * fov_x)
                                           * (fov_x * fov_x);
                                float x4 = (fov_x * fov_x) * (fov_x * fov_x);
                                float x3 = (fov_x * fov_x) * fov_x;
                                v28 = x6 * -0.1079625f - x4 * 0.15000001f
                                      - x3 * 0.33333331f - fov_x * 2.0f
                                      + 1.570796f;
                            }
                            else
                            {
                                float v27 = invLen * fogColor2;
                                v28 = (((((((v27 * v27) * v27) * (v27 * v27))
                                          * (v27 * v27))
                                         * 0.053981241f)
                                        + ((((v27 * v27) * v27) * (v27 * v27))
                                           * 0.075000003f))
                                       + (((v27 * v27) * v27) * 0.1666667f))
                                      + v27;
                            }
                            v26 = 1.5707964f - v28;
                        }
                        if (v35 < 0.0f)
                            v26 = 3.1415927f - v26;
                        if (v36 < 0.0f)
                            v26 = 0.0f - v26;
                    }
                    dword_F63C60[1580 * currCl] = *(int*)&fogColor3;
                    *(float*)&dword_F63C64[v2] = v26 * 114.59155f;
                    int v30 = dword_F63CA8[v2] | 0x18;
                    goto done;
                }
                re.SetFog(2, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f);
            }
            s_foginited = 1;
            goto label_44;
        }
    }
}

// ea: 0x0069D150
void CG_DrawHudElems()
{
    void* elems[16];
    int v0 = 0;
    for (int i = 0; i < 16; ++i)
    {
        if (g_hudelems[i].elem.type != 0 /* HE_TYPE_FREE */)
            elems[v0++] = &g_hudelems[i];
    }
    qsort(elems, v0, 4, compare_hudelems);
    if (v0 != 0)
    {
        for (int i = 0; i < v0; ++i)
            CG_DrawSingleHudElem(elems[i]);
    }
}

// ea: 0x0069F4B0
void CG_DrawTracer(const math::Position3& _start,
                   const math::Position3& _finish, float width)
{
    float start[4] = {_start.v.m128_f32[0], _start.v.m128_f32[1],
                      _start.v.m128_f32[2], _start.v.m128_f32[3]};
    float finish[4] = {_finish.v.m128_f32[0], _finish.v.m128_f32[1],
                       _finish.v.m128_f32[2], _finish.v.m128_f32[3]};
    float cam[4] = {dword_F63C70[1580 * currCl],
                    dword_F63C74[1580 * currCl],
                    dword_F63C78[1580 * currCl], 0.0f};
    float mid[3] = {(start[0] + finish[0]) * 0.5f - cam[0],
                    (start[1] + finish[1]) * 0.5f - cam[1],
                    (start[2] + finish[2]) * 0.5f - cam[2]};
    float dir[3] = {finish[0] - start[0], finish[1] - start[1],
                    finish[2] - start[2]};
    float cross[3] = {mid[1] * dir[2] - mid[2] * dir[1],
                      mid[2] * dir[0] - mid[0] * dir[2],
                      mid[0] * dir[1] - mid[1] * dir[0]};
    float crossLen = sqrtf(cross[0] * cross[0] + cross[1] * cross[1]
                           + cross[2] * cross[2]);
    float midLen = sqrtf(mid[0] * mid[0] + mid[1] * mid[1]
                         + mid[2] * mid[2]);
    float v17 = (gTracerDistScale / tr_viewParms_zFar) * midLen;
    if (v17 <= 0.0f)
        v17 = 0.0f;
    float widtha = (v17 + 1.0f) * width;
    nglMesh* mesh = auxCreateScratchMesh(0x40000, 1);
    void* matMem = nglListAlloc(0x20, 0x10);
    void* material = nullptr;
    if (matMem != nullptr)
    {
        material = cdScratchMaterial_Ctor(matMem, cgsGlobal_media_tracerShader,
                                          0x64078600u, 2, false);
    }
    nglMeshSection* section =
        nglCreateScratchSection(6, 8, 8,
                                (gpuVertexFormat*)cdscratch_vertex_format);
    nglAddMeshSection(mesh, section, (nglMaterial*)material, 1);
    unsigned short* indices =
        (unsigned short*)nglLockSectionIndices(section);
    float* verts = (float*)nglLockSectionVertices(section);
    float norm[3] = {cross[0] / crossLen, cross[1] / crossLen,
                     cross[2] / crossLen};
    float half = widtha * 1.1f;
    float off05[3] = {dir[0] * 0.05f, dir[1] * 0.05f, dir[2] * 0.05f};
    float off90[3] = {dir[0] * 0.9f, dir[1] * 0.9f, dir[2] * 0.9f};
    float v[8][3] = {
        {start[0] + norm[0] * half, start[1] + norm[1] * half,
         start[2] + norm[2] * half},
        {start[0] - norm[0] * half, start[1] - norm[1] * half,
         start[2] - norm[2] * half},
        {start[0] + off05[0] + norm[0] * half,
         start[1] + off05[1] + norm[1] * half,
         start[2] + off05[2] + norm[2] * half},
        {start[0] + off05[0] - norm[0] * half,
         start[1] + off05[1] - norm[1] * half,
         start[2] + off05[2] - norm[2] * half},
        {start[0] + off90[0] + norm[0] * half,
         start[1] + off90[1] + norm[1] * half,
         start[2] + off90[2] + norm[2] * half},
        {start[0] + off90[0] - norm[0] * half,
         start[1] + off90[1] - norm[1] * half,
         start[2] + off90[2] - norm[2] * half},
        {finish[0] + norm[0] * half, finish[1] + norm[1] * half,
         finish[2] + norm[2] * half},
        {finish[0] - norm[0] * half, finish[1] - norm[1] * half,
         finish[2] - norm[2] * half}};
    float uv[8][2] = {{0.0f, 0.0f}, {0.0f, 1.0f}, {0.5f, 0.0f},
                      {0.5f, 1.0f}, {0.5f, 0.0f}, {0.5f, 1.0f},
                      {1.0f, 0.0f}, {1.0f, 1.0f}};
    for (int i = 0; i < 8; ++i)
    {
        verts[i * 6 + 0] = v[i][0];
        verts[i * 6 + 1] = v[i][1];
        verts[i * 6 + 2] = v[i][2];
        verts[i * 6 + 3] = uv[i][0];
        verts[i * 6 + 4] = uv[i][1];
        verts[i * 6 + 5] = -1.0f;
        indices[i] = (unsigned short)i;
    }
    math::Mat43 identity;
    identity.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    identity.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    identity.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    identity.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    nglListAddMesh(mesh, identity, nullptr, nullptr, nullptr);
}

// ea: 0x006A07D0
void CG_DrawCrosshair(float transScaleArg)
{
    int color[4] = {1065353216, 1065353216, 1065353216, 0};
    float fWeaponPosFrac =
        EntityManager::sInst->GetPlayer( currCl)
            ->client->ps.fWeaponPosFrac;
    int v2 = dword_F6355C[1580 * currCl];
    float fPosLerp = fWeaponPosFrac;
    float fTransScale = 1.0f;
    float fTransShift = 0.0f;
    if (v2 == 0)
    {
        float value = *(float*)&cg_crosshairAlpha;
        color[3] = (float)cg_crosshairAlpha.integer;
        if (!GamePause::IsGamePaused(currCl))
        {
            Client* client =
                EntityManager::sInst->GetPlayer( currCl)->client;
            if ((client->ps.eFlags & 0x6000) != 0)
            {
                if (client->ps.mViewLockedEntity != 0)
                    CG_DrawTurretCrossHair();
            }
            else if ((0x100000
                      & EntityManager::sInst->GetPlayer( currCl)
                            ->client->ps.eFlags)
                         == 0
                     || (client->ps.eFlags & 0x400000) != 0)
            {
                goto label_21;
            }
            else
            {
                Entity* Player = GetPlayer(currCl);
                if (!IsPlayerFullySeatedInVehicle(Player))
                    return;
                if (BG_AllowPlayerWeaponAtVehiclePos(client->ps.vehType,
                                                     client->ps.vehPos))
                {
                label_21:
                    int weapnum =
                        BG_GetWeaponForInfo(
                            (weaponFileInfo_t*)dword_F63B8C[1580 * currCl]);
                    if (!CG_ForceDebugCrosshair())
                    {
                        CG_DrawWeapReticle();
                        float centerY = value;
                        CG_CalcCrosshairColor(value, color);
                        if (value >= 0.0099999998f
                            && CG_AllowedToDrawCrosshair())
                        {
                            float centerX;
                            CG_CalcCrosshairPosition(&centerX, &centerY);
                            float v6 = centerY;
                            CG_DrawReticleHitIndicator(
                                dword_F63B8C[1580 * currCl], weapnum, color,
                                centerX, centerY, 1.0f);
                            float v7 = fPosLerp;
                            if (fPosLerp != 1.0f || cg_drawGun.integer == 0)
                            {
                                if (fPosLerp != 0.0f)
                                {
                                    CG_TransitionToAds(
                                        dword_F63B8C[1580 * currCl], fPosLerp,
                                        &fTransScale, &fTransShift);
                                    CG_DrawAdsAimIndicator(
                                        dword_F63B8C[1580 * currCl], weapnum,
                                        color, centerX, centerY, fTransScale);
                                    v7 = fPosLerp;
                                }
                                if (v7 == 1.0f)
                                {
                                    trap_R_SetColor(nullptr);
                                }
                                else
                                {
                                    if (cg_crosshairDynamic.integer == 0)
                                    {
                                        centerX = 0.0f;
                                        centerY = fTransShift;
                                        v6 = fTransShift;
                                    }
                                    int port = dword_F6A28C[802 * currCl];
                                    if (port == 0
                                        && *(unsigned char*)((char*)gSaveGameData
                                                             + 0x3A))
                                    {
                                        CG_DrawReticleName(color);
                                        v6 = centerY;
                                    }
                                    float v9 = centerX;
                                    int v10 = weapnum;
                                    CG_DrawReticleCenter(
                                        dword_F63B8C[1580 * currCl], weapnum,
                                        color, centerX, v6,
                                        transScaleArg);
                                    CG_DrawReticleSides(
                                        dword_F63B8C[1580 * currCl], v10,
                                        color, v9, v6, fTransScale);
                                }
                            }
                        }
                    }
                }
                else
                {
                    color[0] = 0;
                    color[1] = 0;
                    color[2] = 0;
                    color[3] = 1036831949;  // 0.6f
                    if (GetPlayerState(currCl)->vehType == 1
                        || (GetPlayerState(currCl)->vehType == 2
                            && GetPlayerState(currCl)->vehPos == 1))
                    {
                        if (GetPlayerState(currCl)->vehPos != 0)
                        {
                            float x = -10.0f;
                            float y = 0.0f;
                            CG_GetCenterOfScreen(&x, &y);
                            CG_FillRect(x, y, 20.0f, 2.0f, (float*)color, 0.0f);
                            x = -1.0f;
                            y = 2.0f;
                            CG_GetCenterOfScreen(&x, &y);
                            CG_FillRect(x, y, 2.0f, 8.0f, (float*)color, 0.0f);
                        }
                    }
                    else
                    {
                        void* IGO = *(void**)((char*)&g_femanager + 0x14);
                        if (IGO == nullptr)
                            goto label_18;
                        void* widget =
                            *(void**)((char*)IGO + 0x38 + 4 * currCl);
                        if (widget != nullptr
                            && (*(int(**)(void*))*(void**)widget)(widget))
                        {
                            CG_CalcCrosshairColor(0.5f, color);
                            return;
                        }
                        if (IGO == nullptr)
                            goto label_18;
                        widget = *(void**)((char*)IGO + 0x38 + 4 * currCl);
                        if (widget == nullptr
                            || !(*(int(**)(void*))*(void**)widget)(widget))
                        {
                        label_18:
                            CG_FillRect(300.0f, 240.0f, 40.0f, 2.0f,
                                        (float*)color, 0.0f);
                            CG_FillRect(319.0f, 242.0f, 2.0f, 16.0f,
                                        (float*)color, 0.0f);
                        }
                    }
                }
            }
        }
    }
}

// ea: 0x006A0F50
void CG_Draw2D(float a2)
{
    if (dword_F62948[1580 * currCl] != 0
        || cgGlobal.cubemapShot != 0 /* CUBEMAPSHOT_NONE */)
        return;
    nglListBeginScene((nglSceneParamType)0 /* NGLSCENE_PARENT */);
    nglSetClearFlags(0);
    nglSetZTestEnable(false);
    nglSetZWriteEnable(false);
    if (dword_F640A4[1580 * currCl] != 0 || cg_draw2D.integer == 0)
        goto label_46;
    int v2 = currCl;
    int port = dword_F6A28C[802 * currCl];
    if (port == 0 && *(unsigned char*)((char*)gSaveGameData + 0x3A)
        && !*(bool*)((char*)&g_femanager + 0x3C))
    {
        FEManager_InGameMenusActive(&g_femanager, currCl);
        v2 = currCl;
    }
    if (!FEManager_InGameMenusActive(&g_femanager, v2))
    {
        CG_ScreenFade();
        CG_DrawFlashDamage();
        CG_DrawDamageDirectionIndicators();
    }
    int v3 = *(int*)(dword_F62960[1580 * currCl] + 52);
    if (v3 != 5)
    {
        if (v3 == 4)
        {
            if (cg_drawStatus.integer != 0)
                CG_DrawHudElems();
        }
        else
        {
            void* v6 = Cvar_Get("introScreen", "0", 0);
            int v7 = *(int*)(dword_F62960[1580 * currCl] + 52);
            if (*(int*)v6 != 0)
            {
                if (v7 < 6)
                {
                    CG_DrawCrosshair(a2);
                    if (dword_F6A28C[802 * currCl] == 0
                        && *(unsigned char*)((char*)gSaveGameData + 0x3A)
                        && !*(bool*)((char*)&g_femanager + 0x3C)
                        && !FEManager_InGameMenusActive(&g_femanager, currCl))
                    {
                        CG_DrawFriendlyFire();
                    }
                }
                if (*(int*)(dword_F62960[1580 * currCl] + 52) < 6
                    && cg_drawStatus.integer != 0)
                    CG_DrawHudElems();
            }
            else
            {
                if (v7 < 6 && cg_drawStatus.integer != 0)
                {
                    CG_DrawPlayerLowHealthOverlay();
                    CG_DrawHudElems();
                }
                if (*(int*)(dword_F62960[1580 * currCl] + 52) < 6)
                {
                    CG_DrawCrosshair(a2);
                    if (dword_F6A28C[802 * currCl] == 0
                        && *(unsigned char*)((char*)gSaveGameData + 0x3A)
                        && !*(bool*)((char*)&g_femanager + 0x3C)
                        && !FEManager_InGameMenusActive(&g_femanager, currCl))
                    {
                        CG_DrawFriendlyFire();
                    }
                }
            }
            if (!FEManager_InGameMenusActive(&g_femanager, currCl))
                CheckAndRunOverHeatBlur();
        }
        CG_DrawPerformanceWarnings();
        CG_DrawUpperRight();
        CG_DrawCenterString();
        CG_DrawGameMessages();
        CG_DrawBoldGameMessages();
        CG_DrawMiniConsole();
        subtitle_manager_render();
        nglListEndScene();
        int v9 = dword_F64158[1580 * currCl];
        dword_F64158[1580 * currCl] = 1065353216;
        if (*(float*)&v9 < 1.0f)
            gStillDrawMenus = true;
        FEManager_DrawIGO(&g_femanager, currCl);
        if (*(float*)&v9 < 1.0f)
            gStillDrawMenus = false;
        dword_F64158[1580 * currCl] = v9;
        nglListBeginScene((nglSceneParamType)0);
        nglSetClearFlags(0);
        nglSetZTestEnable(false);
        nglSetZWriteEnable(false);
        goto label_46;
    }
    CG_DrawFlashFade();
    nglListEndScene();
    int v5 = dword_F64158[1580 * currCl];
    dword_F64158[1580 * currCl] = 1065353216;
    if (*(float*)&v5 < 1.0f)
        gStillDrawMenus = true;
    FEManager_DrawIGO(&g_femanager, currCl);
    if (*(float*)&v5 < 1.0f)
        gStillDrawMenus = false;
    dword_F64158[1580 * currCl] = v5;
    return;
label_46:
    CG_DrawFlashFade();
    nglListEndScene();
}

// ea: 0x006A12F0
void CG_DrawActive(float a1)
{
    if (dword_F62960[1580 * currCl] != 0)
    {
        int v1 = 1580 * currCl;
        int v2 = dword_F63CA8[1580 * currCl] | 0x10;
        bool v3 = cg_skybox.integer == 0;
        dword_F63CA8[1580 * currCl] = v2;
        if (v3)
            dword_F63CA8[v1] = v2 & 0xFFFFFFEF;
        trap_R_RenderScene(&dword_F63C50[v1]);
        CG_DrawShellShockSavedScreenBlend(
            (void*)dword_F64164[1580 * currCl],
            dword_F64168[1580 * currCl], dword_F6416C[1580 * currCl]);
        if (gRenderCG_2D != 0)
            CG_Draw2D(a1);
        InspectorManager_Render(&g_inspectorManager);
    }
    else
    {
        CG_DrawInformation();
    }
}

// ea: 0x006B0600
void CG_DrawActiveFrame(int serverTime, int demoPlayback, int cubemapShot,
                        int cubemapSize, int animFrametime)
{
    if (currCl == LocalClient_FirstLocalClientIndex())
    {
        int v6 = serverTime - cgGlobal.time;
        cgGlobal.oldTime = cgGlobal.time;
        cgGlobal.time = serverTime;
        cgGlobal.frametime = v6;
        if (v6 < 0)
        {
            cgGlobal.frametime = 0;
            cgGlobal.oldTime = serverTime;
        }
    }
    float v7 = (float)cubemapShot;
    dword_F63554[1580 * currCl] = animFrametime;
    cgGlobal.cubemapShot = cubemapShot;
    cgGlobal.cubemapSize = cubemapSize;
    CG_UpdateCvars();
    CG_ProcessSnapshots();
    int v9 = currCl;
    if (dword_F62960[1580 * currCl] == 0)
    {
        CG_ASSERT("cg[currCl].snap", "c:\\cod\\code\\game\\cg_view.cpp",
                  1740);
        v9 = currCl;
    }
    if (dword_F62964[1580 * v9] == 0)
    {
        CG_ASSERT("cg[currCl].nextSnap", "c:\\cod\\code\\game\\cg_view.cpp",
                  1741);
    }
    if (*(int*)(dword_F62964[1580 * currCl] + 4) != G_GetServerSnapTime())
    {
        CG_ASSERT("cg[currCl].nextSnap->serverTime == G_GetServerSnapTime()",
                  "c:\\cod\\code\\game\\cg_view.cpp", 1742);
    }
    int ServerSnapTime = G_GetServerSnapTime();
    int v11 = currCl;
    if (*(int*)(dword_F62964[1580 * currCl] + 4) != ServerSnapTime)
    {
        CG_ASSERT("cg[currCl].nextSnap->serverTime == G_GetServerSnapTime()",
                  "c:\\cod\\code\\game\\cg_view.cpp", 1747);
        v11 = currCl;
    }
    if (cg_norender.integer == 0)
    {
        ++cg_clientFrame[1580 * v11];
        CG_PredictPlayerState_Internal();
        int v12 = 1580 * currCl;
        int v13;
        if (*(int*)(dword_F62960[1580 * currCl] + 1352) != 0)
        {
            dword_F64164[1580 * currCl] =
                (int)&cgsGlobal_shellshockParms
                    [*(int*)(dword_F62960[1580 * currCl] + 1352)];
            dword_F64168[v12] = *(int*)(dword_F62960[v12] + 1356);
            v13 = *(int*)(dword_F62960[v12] + 1360);
        }
        else
        {
            dword_F64164[1580 * currCl] =
                (int)cgsGlobal_shellshockParms;
            dword_F64168[v12] = dword_F6418C[v12];
            v13 = dword_F64190[v12];
        }
        dword_F6416C[v12] = v13;
        CG_UpdateShellShock((shellshock_parms_t*)dword_F64164[v12],
                            dword_F64168[v12], v13);
        Entity* Player =
            EntityManager::sInst->GetPlayer( currCl);
        Client* client = Player->client;
        if (IsPlayerFullySeatedInVehicle(Player)
            || *(bool*)((char*)client + 0xAD8)
            || *(bool*)((char*)client + 0xAE0))
        {
            dword_F6355C[1580 * currCl] = cg_thirdPerson.integer;
        }
        else
        {
            dword_F6355C[1580 * currCl] = 1;
        }
        nglSetView(-1.0f, -1.0f, 1.0f, 1.0f);
        nglSetScissor(-1.0f, -1.0f, 1.0f, 1.0f);
        nglSetClearFlags(0);
        nglListBeginScene((nglSceneParamType)0);
        nglSetClearFlags(0);
        View_SetViewportClipping(currCl);
        if (cubemapShot != 0)
        {
            void* RenderTarget = nglBuildScene_RenderTarget;
            float v17 = (float)cubemapSize * 2.0f;
            float v18 = v17 / *(float*)((char*)RenderTarget + 4);
            float y2 = (v17 / *(float*)((char*)RenderTarget + 8)) - 1.0f;
            v7 = v18 - 1.0f;
            nglSetView(-1.0f, -1.0f, v18 - 1.0f, y2);
            nglSetScissor(-1.0f, -1.0f, v18 - 1.0f, y2);
        }
        trap_R_ClearScene();
        const void* v19 = nullptr;
        if (cgGlobal.cubemapShot != 0)
        {
            CG_CalcCubemapViewValues();
        }
        else
        {
            Camera_Update((char*)gCamera + 0x1F0 * currCl);
            CG_CalcVrect(v19);
            CG_CalcFov();
        }
        if (cgGlobal.cubemapShot == 0)
        {
            CG_ShakeCamera(currCl);
            AnglesToAxis((const float*)&angle[1580 * currCl],
                         (float(*)[3])&dword_F63C80[1580 * currCl]);
            CG_PerturbCamera();
        }
        CG_DrawSkyBoxPortal();
        if (*(int*)(dword_F62964[1580 * currCl] + 4) != G_GetServerSnapTime())
        {
            CG_ASSERT("cg[currCl].nextSnap->serverTime == G_GetServerSnapTime()",
                      "c:\\cod\\code\\game\\cg_view.cpp", 1857);
        }
        if (gRenderViewWeapon != 0)
        {
            Entity* v20 =
                EntityManager::sInst->GetPlayer( currCl);
            CG_AddViewWeapon(&v20->client->ps);
        }
        gCurrentCamera = (char*)gCamera + 0x1F0 * currCl;
        if (cgGlobal.cubemapShot == 0)
        {
            Camera_UpdatePostViewModels(gCurrentCamera);
            *(int*)((char*)gCurrentCamera + 0x24) = currCl;
        }
        int v21 = currCl;
        int v22 = 1580 * currCl;
        dword_F63CA4[v22] = cgGlobal.time;
        float v23 = *(float*)&dword_F64174[v22];
        float v24 = *(float*)&dword_F63CF0[v22];
        float sens = v24;
        if (v23 != 0.0f)
            sens = v23 * v24;
        CL_SetUserCmdValue(cg_aWeaponSelect[v21], 0, sens);
        CL_SetUserCmdAimValues(*(float*)&dword_F6403C[1580 * currCl],
                               *(float*)&dword_F64040[1580 * currCl],
                               *(float*)&dword_F64044[1580 * currCl],
                               *(float*)&dword_F64048[1580 * currCl],
                               *(float*)&dword_F6404C[1580 * currCl]);
        if (*(int*)(dword_F62964[1580 * currCl] + 4) != G_GetServerSnapTime())
        {
            CG_ASSERT("cg[currCl].nextSnap->serverTime == G_GetServerSnapTime()",
                      "c:\\cod\\code\\game\\cg_view.cpp", 1890);
        }
        CG_DrawActive(v7);
        nglListEndScene();
        R_ToggleSmpFrame();
        gFirstCamera = false;
        g_DOBJF_NOT_RENDERED_LAST_FRAME *= 2;
        nglListBeginScene((nglSceneParamType)0);
        nglSetClearFlags(0);
        CG_DrawViewportFrames(View_lNumViewports);
        nglListEndScene();
        int v25 = 1580 * currCl;
        float sinYaw, cosYaw;
        FastSinCos(angle[1580 * currCl + 1] * 0.017453292f, &sinYaw,
                   &cosYaw);
        float sinPitch, cosPitch;
        FastSinCos(0.0f, &sinPitch, &cosPitch);
        float pos[3] = {
            ((cosPitch * cosYaw) * move_back_distance)
                + dword_F63C70[v25],
            (0.0f - ((cosPitch * sinYaw) * move_back_distance))
                - dword_F63C74[v25],
            ((0.0f - sinPitch) * move_back_distance)
                + dword_F63C78[v25]};
        float listenerPos[3] = {pos[0], pos[1], pos[2]};
        float front[3] = {-dword_F63C84[v25], dword_F63C88[v25],
                          dword_F63C80[v25]};
        float up[3] = {-dword_F63C9C[v25], dword_F63CA0[v25],
                       dword_F63C98[v25]};
        float testFwd[3] = {dword_F63C80[v25], dword_F63C84[v25],
                            dword_F63C88[v25]};
        float len = sqrtf(testFwd[0] * testFwd[0] + testFwd[1] * testFwd[1]
                          + testFwd[2] * testFwd[2]);
        testFwd[0] /= len;
        testFwd[1] /= len;
        testFwd[2] /= len;
        g_TestForward[0] = dword_F63C70[1580 * currCl] + testFwd[0] * 10.0f;
        g_TestForward[1] = dword_F63C74[1580 * currCl] + testFwd[1] * 10.0f;
        g_TestForward[2] = dword_F63C78[1580 * currCl] + testFwd[2] * 10.0f;
        SoundDevice_SetListenerVectors(SoundDevice::sInst, curListener,
                                       listenerPos, front, up);
        ++curListener;
        curListener %= SoundDevice_GetNumberOfListeners(SoundDevice::sInst);
    }
}

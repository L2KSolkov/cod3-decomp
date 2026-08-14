// ============================================================================
// igo_widgets.cpp - IGO widget classes (shell.o IGOWidget.cpp family)
// IGOHealthWidget / IGOTankHealthWidget / IGOVoteWidget / IGORowboatWidget
// ============================================================================

#include "game/shell/shell_types.h"
#include "game/client_types.h"
#include "game/player_types.h"
#include "game/actor_types.h"
#include "core/tlFixedString.h"

extern void* mem_heap_malloc(unsigned int size);  // core.o
extern int currCl;                                // ?currCl@@3HA @ 0xF1579C
extern DbLinkedHandle<EntityHandleDb, Entity> GetPlayersTank();
extern FEManager g_femanager;
extern int dword_F62960[];                        // @ 0xF62960 (cg client base)

// Binary cgGlobal_t starts with frametime at +0x00 (cg.o @ 0xF5FE30).
struct cgGlobal_t {
    int  frametime;      // +0x00
    int  time;           // +0x04
    int  oldTime;        // +0x08
    int  cubemapShot;    // +0x0C
    int  cubemapSize;    // +0x10
    bool teamGame;       // +0x14
    bool showScore;      // +0x15
    float gameTime;      // +0x18
    float gameTimeStartTime;  // +0x1C
    int  teamScores[5];  // +0x20
};
extern cgGlobal_t cgGlobal;

extern PlayerState& GetPlayerState(int idx);       // ?GetPlayerState@@YAAAVPlayerState@@H@Z
extern vmCvar_t g_stanceFadeTime;   // ?g_stanceFadeTime@@3UvmCvar_t@@A @ 0xEAC288
extern vmCvar_t g_stanceSolidTime;  // ?g_stanceSolidTime@@3UvmCvar_t@@A @ 0xEAE1C8
extern int unk_F6A284[];            // @ 0xF6A284 (per-client viewport block)
extern Entity* GetPlayer(int idx);  // ?GetPlayer@@YAPAVEntity@@H@Z

// Global-scope twin of BrocAPI (broc_types.h's lives in namespace Broc);
// only the HQ callbacks used here are declared. Offsets verified against IDA.
struct BrocAPI {
    uint8_t _pad[0xB74];
    bool (*mIsTurretReady)(unsigned int);  // +0xB74
    uint8_t _pad2[0xBE8 - 0xB78];
    struct {
        uint8_t _pad0[0x17C];
        int (*mCallbackGetFlagBeingContested)(const Broc::entity);  // +0x17C
        void (*mCallbackPickupKit)(const Broc::entity,
                                   const unsigned int);  // +0x180
        int (*mCallbackGetTeamCapturingHQPercent)(
            const Broc::entity);  // +0x184
        int (*mCallbackGetTeamDestroyingHQPercent)();    // +0x188
        int (*mCallbackGetHQCaptureStatus)();            // +0x18C
        int (*mCallbackGetFlagCount)();                  // +0x190
        int (*mCallbackGetTeamControllingFlag)(unsigned int);  // +0x194
        int (*mCallbackGetFlagBeingCaptured)();          // +0x198
        int (*mCallbackGetTeamCapturingFlag)();          // +0x19C
        int (*mCallbackGetCapturingFlagPercent)();       // +0x1A0
        uint8_t _pad1A4[4];                              // +0x1A4
        int (*mCallbackGetFlagBreatherTime)();           // +0x1A8
    } mBrocExports;  // +0xBE8
};
extern BrocAPI* gpBrocAPI;          // ?gpBrocAPI@@3PAUBrocAPI@@A (g_scr.cpp)

// Minimal weapon-system views (full weaponFileInfo_t lives in game/logic/g_local.h).
struct weaponFileInfo_t {
    uint8_t _pad[0x594];
    char*   szRadiantName;   // +0x594
    uint8_t _pad2[0x5A0 - 0x598];
    char*   szHudIcon;       // +0x5A0
    uint8_t _pad3[0x5C4 - 0x5A4];
    int     iClipSize;       // +0x5C4
    uint8_t _pad4[0x5F8 - 0x5C8];
    int     iFireTime;       // +0x5F8
    uint8_t _pad5[0x720 - 0x5FC];
    int     bWideListIcon;   // +0x720
    uint8_t _pad6[0x738 - 0x724];
    int     bDoNotDrop;      // +0x738
};
// slot is +0xB4; add an accessor via byte offset cast since it precedes
// szRadiantName in the struct.
static inline int WeaponSlot(weaponFileInfo_t* w)
{
    return *(int*)((char*)w + 0xB4);
}
extern weaponFileInfo_t* BG_GetInfoForWeapon(int iWeapon);  // game.o
extern int BG_ClipForWeapon(int iWeapon);       // game.o
extern int BG_AmmoForWeapon(int iWeapon);       // game.o
extern bool BG_WeaponIsClipOnly(int iWeapon);   // game.o
extern bool BG_AllowPlayerWeaponAtVehiclePos(int vehType, int vehPos);  // game.o
extern int CG_GetGrenadeCount();                // cg.o
extern int CG_GetSpecialGrenadeCount();         // cg.o
extern vmCvar_t g_grenadeFadeTime;   // ?g_grenadeFadeTime@@3UvmCvar_t@@A
extern vmCvar_t g_grenadeSolidTime;  // ?g_grenadeSolidTime@@3UvmCvar_t@@A
extern int dword_F6419C[];  // @ 0xF6419C (special weapon type)
extern int dword_F641A0[];  // @ 0xF641A0 (special weapon end time)
extern int dword_F641A4[];  // @ 0xF641A4 (special weapon duration)
extern float percentToTrimBottom;  // @ 0xDF4460
extern float percentToTrimTop;     // @ 0xDF445C
extern int dword_F6355C[];   // @ 0xF6355C
extern int dword_F63F5C[];   // @ 0xF63F5C (hint icon)
extern int dword_F63F60[];   // @ 0xF63F60 (hint start time)
extern int dword_F63F64[];   // @ 0xF63F64 (hint fade time)
extern int dword_F63F68[];   // @ 0xF63F68
extern int dword_F63F6C[];   // @ 0xF63F6C
extern vmCvar_t cg_cursorHints;  // ?cg_cursorHints@@3UvmCvar_t@@A @ 0xF61378
extern vmCvar_t cg_hintFadeTime; // ?cg_hintFadeTime@@3UvmCvar_t@@A @ 0xF611C8
extern vmCvar_t mp_headIconReviveMaxAlphaDist;  // @ 0xEAC318
extern vmCvar_t mp_headIconReviveMinAlphaDist;  // @ 0xEB0DF8
extern vmCvar_t mp_headIconDistAbovePlayer;   // @ 0xEA7518
extern vmCvar_t mp_headIconDistAboveVehicle;  // @ 0xEAC3A8
extern vmCvar_t mp_headIconHeight;            // @ 0xEA6388
extern vmCvar_t mp_headIconMinScreenSize;     // @ 0xEA65D8
extern vmCvar_t mp_itemIconHeight;      // @ 0xEA6D38
extern vmCvar_t mp_itemIconMaxAlphaDist;  // @ 0xEA5E68
extern vmCvar_t mp_itemIconMinAlphaDist;  // @ 0xEB0F18
extern vmCvar_t mp_itemIconMinScreenSize; // @ 0xEA51D0
extern vmCvar_t mp_itemIconDistAboveItem; // @ 0xEA6540
extern int mpviewport;       // @ 0xF3A574
extern const char* CG_ConfigString(int index);  // cg.o
extern int BG_GetNumWeapons();  // game.o
extern bool IsVehicleSpotted(Entity* vehicle);  // g.o
extern float VectorDistance(const float* v1, const float* v2);  // core.o
struct nglScene;
extern math::Position3* nglProjectPoint(math::Position3* result,
                                        const math::Position3* In,
                                        nglScene* Scene);  // ngl/ngl_scene.h
extern nglScene* nglBuildScene;  // render
extern unsigned int AeHash(const char* str);  // core/ae_hash.cpp

// DObjSkelMat minimal view (full in core/core_types.h).
struct DObjSkelMat {
    uint8_t _pad[0x30];
    float origin[4];  // +0x30
};
extern int G_DObjGetWorldTagMatrix(Entity* ent, unsigned int tag_name_hash,
                                   DObjSkelMat* tagMat);  // g.o
extern nglTexture* cdGetTexture(TPakId pakId,
                                const tlFixedString& name);  // core.o
namespace LocalClient {
int ClientToPort(int client);  // ?ClientToPort@LocalClient@@YAHH@Z
}

namespace View {
float GetCurrentXPos(float pos, int window);  // cg.o
float GetCurrentYPos(float pos, int window);  // cg.o
}


// mp.o dropped-item helpers (local views; manglings tolerated at link)
enum EDroppedItemTypes : int {
    kItemTypeMines = 0,
    kItemTypeWeapons = 1,
    kItemTypeSupport = 2,
    kItemTypeKits = 3,
};
struct MpPlayerItems {
    struct sDroppedItem {
        DbLinkedHandle<EntityHandleDb, Entity> handle;  // +0x00
        unsigned int time;                              // +0x04
    };
    ae_vector<sDroppedItem> mDroppedWeapons;  // +0x00
    ae_vector<sDroppedItem> mDroppedSupport;  // +0x0C
    ae_vector<sDroppedItem> mDroppedMines;    // +0x18
    ae_vector<sDroppedItem> mDroppedKits;     // +0x24
    Entity* FindItem(EDroppedItemTypes item, short id);
};

// mp.o player view with mItems/mClientIndex fields (full layout in mp.o).
struct MpPlayerView2 {
    uint8_t  mId;          // +0x00
    uint8_t  _pad1[3];     // +0x01
    void*    mConnection;  // +0x04
    int      mClientIndex; // +0x08
    MpPlayerItems mItems;  // +0x0C
    uint8_t  _pad2[0x68 - 0x3C];
    char     mName[32];    // +0x68
    uint8_t  _pad3[0x25C - 0x88];
    int16_t  mTeam;        // +0x25C
    bool IsValid() const;  // mp.o
};
struct MpPlayerManagerView {
    MPPlayer* GetPlayer(unsigned char id);  // mp.o
    MPPlayer* GetLocalPlayer(int nLocalPlayer);  // mp.o
};

struct KeyInfo {
    static int GetKey(const char* boundCmdName, int clnt);  // ?GetKey@KeyInfo@@SAHPBDH@Z
};
struct weaponInfo_s {
    uint8_t _pad[0x84];
    const char* pszTranslatedDisplayName;  // +0x84
    uint8_t _pad2[0x4];
    const char* pszTranslatedModename;     // +0x8C
};
extern weaponInfo_s cg_weapons[];  // ?cg_weapons@@3PAUweaponInfo_s@@A @ 0xF6AE60

// Minimal scr_vehicle_t view (full in game/logic/g_local.h).
struct scr_vehicle_t {
    uint8_t _pad[0x180];
    int fireTime;  // +0x180
};


struct level_locals_t {
    int time;   // +0x00
};
extern level_locals_t level;        // ?level@@3Ulevel_locals_t@@A @ 0xEC9650

// mp.o extern (same minimal view as loading_menu.cpp)
struct sServerCreateParams {
    unsigned char mGameType;  // +0x59
};
class MPUIInterface {
public:
    static sServerCreateParams mServerParams;  // mp.o
};

// game.o / core.o externs (link /FORCE-tolerated until those objects land)
class InteractionController {
public:
    static InteractionController* Inst(int instance);  // ?Inst@InteractionController@@SAPAV1@H@Z
    void SetRenderText(const char* text, int x, int y, float scale,
                       float alpha,
                       int index);  // ?SetRenderText@InteractionController@@QAEXPBDHHMMH@Z
};

namespace View {
float GetXScalingForHUD(int window);       // cg.o
float GetYScalingForHUD(int window);       // cg.o
float GetPreviousHUDXPos(float pos, int window, char justification,
                         float width);    // cg.o
float GetPreviousHUDYPos(float pos, int window, char justification,
                         float height);   // cg.o
float GetCurrentHUDXPos(float pos, int window, char justification,
                        float width);     // cg.o
float GetCurrentHUDYPos(float pos, int window, char justification,
                        float height);    // cg.o
}

// Minimal IGOCompassWidget view for split-screen width lookup
// (full layout in the IGOFrontEnd batch).
struct IGOCompassWidget {
    uint8_t _pad[0x160];
    PanelQuad* compass;   // +0x160
};

// IGORowboatWidget fade timing data (shell.o data, copied from IDA)
const float sUpArrowFadeInTime = 0.5f;      // 0xDF410C
const float sHalfCircleFadeInTime = 0.5f;   // 0xDF4110
const float sAllFadeOutTime = 0.2f;         // 0xDF4114 (0x3E4CCCCD)
const float sMoveX = 17.0f;                 // 0xDF4874

// IGOTimerWidget statics (shell.o data @ 0xF30D58 / 0xF30D54)
float IGOTimerWidget::m_TimeLimit = 0.0f;
float IGOTimerWidget::m_StartTime = 0.0f;

// Minimal STBManager view (same pattern as loading_menu.cpp)
class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    const char* GetSTBString(const char* pszReference);  // core.o
    const char* GetSTBString(unsigned int hash);  // core.o
};

// ============================================================================
// IGOHealthWidget
// ============================================================================

// ea: 0x00565EA0
IGOHealthWidget::IGOHealthWidget(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    bar = nullptr;
    frame = nullptr;
    cross = nullptr;
    flash = nullptr;
    health = 0.0f;
    last_health = 0.0f;
    draw_flash = false;
}

// ea: 0x00598330
void IGOHealthWidget::Init(PanelFile* panel)
{
    bar = panel->GetPointer("hudhealthbar");
    frame = panel->GetPointer("hudhealthback");
    cross = panel->GetPointer("hudhealthcross");
    flash = (PanelQuad*)mem_heap_malloc(0x48u);
    if (flash != nullptr)
        flash = new (flash) PanelQuad();
    flash->CopyFrom(bar);
    flash->SetZvalueAbs(flash->GetZvalue() + 1.0f);
    if (mClient > 0)
    {
        bar = PanelQuad::Clone(bar);
        frame = PanelQuad::Clone(frame);
        cross = PanelQuad::Clone(cross);
    }
}

// ea: 0x005826B0
void IGOHealthWidget::Update(float time_inc)
{
    (void)time_inc;
    if (!is_shown)
        return;
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player == nullptr || Player->client == nullptr)
        return;
    Client* client = Player->client;
    if (client == nullptr)
        return;
    int v5 = client->ps.stats[0];
    int v6;
    float v7;
    if (v5 != 0 && (v6 = client->ps.stats[2]) != 0
        && (v7 = (float)v5 / (float)v6, health = v7, v7 >= 0.0f))
    {
        if (v7 > 1.0f)
            health = 1.0f;
    }
    else
    {
        health = 0.0f;
    }
    if (health > 0.0f)
    {
        float r = 0.7f;
        float g = 0.4f;
        if (health <= 0.5f)
            g = (health + 0.2f) * 0.4f + 0.3f;
        else
            r = (1.0f - health) * 1.4f;
        color32 col;
        col.c.b = 0;
        col.c.g = (uint8_t)(g * 255.0f);
        col.c.r = (uint8_t)(r * 255.0f);
        col.c.a = 255;
        bar->SetColor(col);
        bar->Mask(health, RIGHT_MASK, 1.0f);
    }
    float last_health = this->last_health;
    if (last_health <= health)
    {
        this->last_health = health;
    }
    else
    {
        float v10 = last_health - cgGlobal.frametime * 0.0012000001f;
        this->last_health = v10;
        if (health >= v10)
            this->last_health = health;
    }
    if (this->last_health <= health)
    {
        draw_flash = false;
    }
    else
    {
        flash->SetColor(color32(-65536));
        flash->Mask(this->last_health, RIGHT_MASK, 1.0f);
        draw_flash = true;
    }
}

// ea: 0x00565EE0
void IGOHealthWidget::Draw()
{
    if (!is_shown)
        return;
    Client* client = EntityManager::sInst->GetPlayer(currCl)->client;
    if (client != nullptr && (client->ps.eFlags & 0x100000) == 0)
    {
        if (health > 0.0f)
            bar->Draw();
        frame->Draw();
        cross->Draw();
        if (draw_flash)
            flash->Draw();
    }
}

// ea: 0x00582890
void IGOHealthWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    bar->FattenMeForWidescreen(widescreen, about_x);
    frame->FattenMeForWidescreen(widescreen, about_x);
    cross->FattenMeForWidescreen(widescreen, about_x);
    flash->FattenMeForWidescreen(widescreen, about_x);
    bar->SetXYInitialToCurrentPos();
    flash->SetXYInitialToCurrentPos();
}

// ============================================================================
// IGOTankHealthWidget
// ============================================================================

// ea: 0x00567370
IGOTankHealthWidget::IGOTankHealthWidget(int client)
{
    maxHealth = -1.0f;
    force_appear = false;
    mClient = client;
    frame = nullptr;
    bar = nullptr;
    armor = nullptr;
    is_shown = false;
    healthMaxWidth = 0.0f;
}

// ea: 0x00598AB0
void IGOTankHealthWidget::Init(PanelFile* panel)
{
    frame = panel->GetPointer("tankbar");
    bar = panel->GetPointer("tankhealthbar");
    armor = panel->GetPointer("armor");
    healthMaxWidth = bar->GetWidth();
}

// ea: 0x00588A30
void IGOTankHealthWidget::Update(float time_inc)
{
    (void)time_inc;
    if (!is_shown || bar == nullptr)
        return;
    float percent = 0.0f;
    unsigned int mVal = GetPlayersTank().mHandle.mVal;
    unsigned int v4 = mVal & 0xFFF;
    if (v4 < 0x540
        && mVal >> 12 == (unsigned int)EntityHandleDb::sInst.mElements[v4].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
        if (mObject != nullptr && mObject->scr_vehicle != nullptr)
        {
            if (maxHealth == -1.0f)
                maxHealth = (float)mObject->maxHealth;
            percent = (float)mObject->health / maxHealth;
        }
    }
    bar->Mask(percent, RIGHT_MASK, 1.0f);
}

// ea: 0x005673B0
void IGOTankHealthWidget::Draw()
{
    if (is_shown
        && bar != nullptr
        && EntityManager::sInst->GetPlayer(currCl)->client->pers.playerState
               == 3
        && (EntityManager::sInst->GetPlayer(currCl)->client->ps.eFlags
            & 0x100000) != 0)
    {
        bar->Draw();
        frame->Draw();
        armor->Draw();
    }
}

// ea: 0x00583170
void IGOTankHealthWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    bar->FattenMeForWidescreen(widescreen, about_x);
    frame->FattenMeForWidescreen(widescreen, about_x);
    armor->FattenMeForWidescreen(widescreen, about_x);
    bar->SetXYInitialToCurrentPos();
}

// ============================================================================
// IGOVoteWidget
// ============================================================================

// ea: 0x005677A0
IGOVoteWidget::IGOVoteWidget(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    vote = nullptr;
}

// ea: 0x00598D70
void IGOVoteWidget::Init(PanelFile* panel)
{
    PanelQuad* Pointer = panel->GetPointer("vote");
    vote = Pointer;
    if (Pointer != nullptr)
    {
        float x = Pointer->GetCenterX();
        if (x < 320.0f)
            vote->SetCenterPos(640.0f - x, vote->GetCenterY());
    }
}

// ea: 0x005677D0
void IGOVoteWidget::Update(float time_inc)
{
    (void)time_inc;
    if (is_shown)
        (void)EntityManager::sInst->GetPlayer(currCl);
}

// ea: 0x005677F0
void IGOVoteWidget::Draw()
{
    if (is_shown && MultiplayerMgr::sInst->IsVoteOngoing())
        vote->Draw();
}

// ea: 0x005832C0
void IGOVoteWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    vote->FattenMeForWidescreen(widescreen, about_x);
}

// ============================================================================
// IGORowboatWidget
// ============================================================================

// ea: 0x00568FA0
IGORowboatWidget::IGORowboatWidget()
{
    force_appear = false;
    mClient = 0;
    mUpArrow = nullptr;
    mHalfCircle = nullptr;
    mTimer = 0.0f;
    mPhase = PHASE_UP_ARROW;
    is_shown = false;
}

// ea: 0x0059A980
void IGORowboatWidget::Init(PanelFile* panel)
{
    mUpArrow = panel->GetPointer("BA_arrow_up_a");
    mHalfCircle = panel->GetPointer("BA_arrow_half_b");
    float arrowX, arrowY;
    float circleX, circleY;
    mUpArrow->GetCenterPos(arrowX, arrowY);
    mHalfCircle->GetCenterPos(circleX, circleY);
    mUpArrow->SetCenterPos(arrowX - sMoveX, arrowY);
    mHalfCircle->SetCenterPos(sMoveX + circleX, circleY);
    mUpArrow->SetAlpha(0.0f);
    mHalfCircle->SetAlpha(0.0f);
}

// ea: 0x00568FD0
void IGORowboatWidget::Update(float time_inc)
{
    if (mUpArrow == nullptr || !is_shown)
        return;
    mTimer = time_inc + mTimer;
    float v5 = mTimer;
    if (mPhase == PHASE_UP_ARROW)
    {
        float fade = v5 / sUpArrowFadeInTime;
        if (fade >= 1.0f)
            fade = 1.0f;
        mUpArrow->SetAlpha(fade);
        mHalfCircle->SetAlpha(0.0f);
        InteractionController::Inst(currCl)->SetRenderText(
            "Press Right Stick Up and Hold", -1, 320, 0.7f, fade, 0);
    }
    else if (mPhase == PHASE_HALF_CIRCLE)
    {
        float fade = v5 / sHalfCircleFadeInTime;
        if (fade >= 1.0f)
            fade = 1.0f;
        mHalfCircle->SetAlpha(fade);
        mUpArrow->SetAlpha(1.0f);
        if (fade > 0.3f)
        {
            InteractionController::Inst(currCl)->SetRenderText(
                "Now Move Stick Down in Half Circle", -1, 320, 0.7f, 1.0f, 0);
        }
    }
    else if (mPhase == PHASE_FADE_OUT)
    {
        float fade = 0.0f;
        if ((1.0f - (v5 / sAllFadeOutTime)) >= 0.0f)
            fade = 1.0f - (v5 / sAllFadeOutTime);
        mUpArrow->SetAlpha(fade);
        mHalfCircle->SetAlpha(fade);
        InteractionController::Inst(currCl)->SetRenderText(
            "Now Move Stick Down in Half Circle", -1, 320, 0.7f, fade, 0);
        if (mTimer > sAllFadeOutTime)
        {
            mPhase = PHASE_OFF;
            mTimer = 0.0f;
        }
    }
}

// ea: 0x00569180
void IGORowboatWidget::Draw()
{
    if (is_shown)
    {
        if (mUpArrow != nullptr)
        {
            mUpArrow->Draw();
            mHalfCircle->Draw();
        }
    }
}

// ea: 0x00583D60
void IGORowboatWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    mUpArrow->FattenMeForWidescreen(widescreen, about_x);
    mHalfCircle->FattenMeForWidescreen(widescreen, about_x);
}

// ============================================================================
// IGOStanceWidget
// ============================================================================

// ea: 0x00565CC0
IGOStanceWidget::IGOStanceWidget(int client)
{
    force_appear = false;
    is_shown = true;
    mClient = client;
    icons[0][0] = nullptr;
    icons[0][1] = nullptr;
    icons[1][0] = nullptr;
    icons[1][1] = nullptr;
    icons[2][0] = nullptr;
    icons[2][1] = nullptr;
    flash = nullptr;
    cur_stance = 0;
    last_change_time = -1;
    last_stance = -1;
    draw_flash = false;
}

// ea: 0x00598250
void IGOStanceWidget::Init(PanelFile* panel)
{
    icons[0][0] = panel->GetPointer("stance_stand");
    icons[0][1] = panel->GetPointer("stance_stand_man");
    icons[1][0] = panel->GetPointer("stance_crouch");
    icons[1][1] = panel->GetPointer("stance_crouch_man");
    icons[2][0] = panel->GetPointer("stance_prone");
    icons[2][1] = panel->GetPointer("stance_prone_man");
    flash = panel->GetPointer("stance_flash");
    if (mClient > 0)
    {
        icons[0][0] = PanelQuad::Clone(icons[0][0]);
        icons[0][1] = PanelQuad::Clone(icons[0][1]);
        icons[1][0] = PanelQuad::Clone(icons[1][0]);
        icons[1][1] = PanelQuad::Clone(icons[1][1]);
        icons[2][0] = PanelQuad::Clone(icons[2][0]);
        icons[2][1] = PanelQuad::Clone(icons[2][1]);
        flash = PanelQuad::Clone(flash);
    }
}

// ea: 0x00565D00
void IGOStanceWidget::Update(float time_inc)
{
    (void)time_inc;
    if (!is_shown
        || EntityManager::sInst->GetPlayer(currCl) == nullptr
        || EntityManager::sInst->GetPlayer(currCl)->client == nullptr)
    {
        return;
    }
    if (force_appear || last_change_time > cgGlobal.time
        || last_stance
               != (GetPlayerState(currCl).pm_flags & 0x10003))
    {
        last_change_time = cgGlobal.time;
    }
    int v3 = GetPlayerState(currCl).pm_flags & 0x10003;
    last_stance = v3;
    if ((v3 & 1) != 0)
        cur_stance = 2;
    else
        cur_stance = (v3 & 2) != 0;
    int last_change_time2 = last_change_time;
    int time = cgGlobal.time;
    if (last_change_time2 + 1000 > cgGlobal.time)
    {
        icons[cur_stance][1]->SetAlpha(
            ((float)(last_change_time2 - cgGlobal.time + 1000) * 0.001f)
            * 0.8f);
        draw_flash = true;
        time = cgGlobal.time;
    }
    float value = g_stanceFadeTime.value;
    int v7 = (int)((g_stanceFadeTime.value + g_stanceSolidTime.value)
                   * 1000.0f)
             + last_change_time;
    float stance_alpha = 0.0f;
    if (v7 <= time)
    {
        draw_flash = false;
    }
    else
    {
        float v8 = (float)(v7 - time) * 0.001f;
        draw_flash = true;
        if (v8 <= value)
            stance_alpha = v8 / value;
        else
            stance_alpha = 1.0f;
    }
    icons[cur_stance][0]->SetAlpha(stance_alpha);
}

// ea: 0x00582550
void IGOStanceWidget::Draw()
{
    if (!is_shown)
        return;
    Client* client = EntityManager::sInst->GetPlayer(currCl)->client;
    if (client != nullptr && (client->ps.eFlags & 0x100000) == 0
        && client->pers.playerState == 3)
    {
        if (EntityManager::sInst->GetPlayer(currCl)->client->ps.fatigueScale
            > 0.0f)
        {
            icons[cur_stance][0]->SetAlpha(1.0f);
            icons[cur_stance][0]->Mask(GetPlayerState(currCl).fatigueScale,
                                       TOP_MASK, 1.0f);
            icons[cur_stance][0]->Draw();
        }
        icons[cur_stance][0]->Mask(1.0f, TOP_MASK, 1.0f);
        icons[cur_stance][0]->SetAlpha(0.5f);
        icons[cur_stance][0]->Draw();
    }
}

// ea: 0x00582650
void IGOStanceWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    for (int i = 0; i < 3; ++i)
    {
        icons[i][0]->FattenMeForWidescreen(widescreen, about_x);
        icons[i][1]->FattenMeForWidescreen(widescreen, about_x);
    }
    flash->FattenMeForWidescreen(widescreen, about_x);
}

// ea: 0x005775D0
void IGOStanceWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    PanelQuad* compass =
        ((IGOCompassWidget*)g_femanager.IGO->compassWidget[mClient])->compass;
    float width;
    if (compass != nullptr)
        width = compass->GetInitialWidth() * 0.25f;
    else
        width = 0.0f;
    for (int i = 0; i < 3; ++i)
    {
        icons[i][0]->FormatHUDForSplitScreen(viewport, old_viewport, 9,
                                             width, 0.0f);
        icons[i][1]->FormatHUDForSplitScreen(viewport, old_viewport, 9,
                                             width, 0.0f);
    }
}

// ============================================================================
// IGORankWidget
// ============================================================================

// ea: 0x00567630
IGORankWidget::IGORankWidget(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    rank = 0;
    timeForNormalSize = 0;
    friendlyRanks[0] = nullptr;
    friendlyRanks[1] = nullptr;
    friendlyRanks[2] = nullptr;
}

// ea: 0x00598D00
void IGORankWidget::Init(PanelFile* panel)
{
    friendlyRanks[0] = panel->GetPointer("rank1gold");
    friendlyRanks[1] = panel->GetPointer("rank2gold");
    friendlyRanks[2] = panel->GetPointer("rank3gold");
    if (mClient > 0)
    {
        friendlyRanks[0] = PanelQuad::Clone(friendlyRanks[0]);
        friendlyRanks[1] = PanelQuad::Clone(friendlyRanks[1]);
        friendlyRanks[2] = PanelQuad::Clone(friendlyRanks[2]);
    }
}

// ea: 0x00567660
void IGORankWidget::Update(float time_inc)
{
    (void)time_inc;
    if (!is_shown)
        return;
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player->sentient != nullptr)
    {
        int rank = Player->client->pers.rank;
        if (rank != this->rank)
        {
            this->rank = rank;
            timeForNormalSize = cgGlobal.time + 3000;
        }
        if (timeForNormalSize - cgGlobal.time > 3000)
            timeForNormalSize = 0;
        float scale =
            (float)(timeForNormalSize - cgGlobal.time) * 0.00033333333f
            + 1.0f;
        if (scale < 1.0f)
            scale = 1.0f;
        int window = unk_F6A284[802 * currCl];
        float x_scale = View::GetXScalingForHUD(window) * scale;
        float y_scale = View::GetYScalingForHUD(window) * scale;
        friendlyRanks[this->rank]->ScaleAbsoluteCenter(x_scale, y_scale);
    }
}

// ea: 0x00567760
void IGORankWidget::Draw()
{
    if (!is_shown)
        return;
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player != nullptr && Player->client->pers.playerState == 3)
        friendlyRanks[rank & 3]->Draw();
}

// ea: 0x00583280
void IGORankWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    friendlyRanks[0]->FattenMeForWidescreen(widescreen, about_x);
    friendlyRanks[1]->FattenMeForWidescreen(widescreen, about_x);
    friendlyRanks[2]->FattenMeForWidescreen(widescreen, about_x);
}

// ea: 0x00577AB0
void IGORankWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    PanelQuad* compass =
        ((IGOCompassWidget*)g_femanager.IGO->compassWidget[mClient])->compass;
    float width;
    if (compass != nullptr)
        width = compass->GetInitialWidth() * 0.25f;
    else
        width = 0.0f;
    friendlyRanks[0]->FormatHUDForSplitScreen(viewport, old_viewport, 9,
                                              width, 0.0f);
    friendlyRanks[1]->FormatHUDForSplitScreen(viewport, old_viewport, 9,
                                              width, 0.0f);
    friendlyRanks[2]->FormatHUDForSplitScreen(viewport, old_viewport, 9,
                                              width, 0.0f);
}

// ============================================================================
// IGOWeaponNameWidget
// ============================================================================

// ea: 0x00590B00
IGOWeaponNameWidget::IGOWeaponNameWidget(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    name = (FEText*)mem_heap_malloc(0x70u);
    if (name != nullptr)
    {
        name = new (name) FEText(FONT_GARAMOND, defaultFileName, 537.0f,
                                 362.0f, 0, PANEL_LAYER_IGO, 0.54f, 32, 64,
                                 color32(-2961486));
    }
    else
    {
        name = nullptr;
    }
    dont_draw = false;
    last_weapon_index = -1;
    background = (PanelQuad*)mem_heap_malloc(0x48u);
    if (background != nullptr)
        background = new (background) PanelQuad("weapon_name_background");
    else
        background = nullptr;
}

// ea: 0x00566F90
void IGOWeaponNameWidget::Init(PanelFile* panel)
{
    (void)panel;
    float xy[12];
    unsigned char col[16];
    memset(xy, 0, sizeof(xy));
    xy[3] = 1.0f;
    memset(&xy[4], 0, 12);
    xy[7] = 1.0f;
    xy[8] = 0.0f;
    xy[9] = 1.0f;
    xy[10] = 1.0f;
    xy[11] = 0.0f;
    memset(col, 255, sizeof(col));
    background->Init((Broc::vector*)xy, (color32*)col, (panel_layer)8,
                     10.0f, "weaponnameback");
}

// ea: 0x005672E0
void IGOWeaponNameWidget::Draw()
{
    if (is_shown && !dont_draw)
    {
        name->Draw();
        background->Draw();
    }
}

// ea: 0x00582E90
void IGOWeaponNameWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    background->FattenMeForWidescreen(widescreen, about_x);
    name->UpdateForWidescreen(widescreen, (int)about_x);
}

// ea: 0x00577980
void IGOWeaponNameWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    background->FormatHUDForSplitScreen(viewport, old_viewport, 2, 0.0f,
                                        0.0f);
    name->UpdateForHUDSplitScreen(viewport, old_viewport, 2, 0.0f, 0.0f);
}

// ============================================================================
// IGOAmmoWidget
// ============================================================================

// ea: 0x00566530
IGOAmmoWidget::IGOAmmoWidget(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    frame = nullptr;
    clipAmmo = nullptr;
    totalAmmo = nullptr;
    clip_val = 0;
    ammo_val = 0;
    dont_draw = false;
    draw_time = 0.0f;
}

// ea: 0x00598660
void IGOAmmoWidget::Init(PanelFile* panel)
{
    frame = panel->GetPointer("hudammo");
    clipAmmo = panel->GetTextPointer("clipammo");
    clipAmmo->SetNoFlash(color32(-4671333));
    clipAmmo->SetScale(0.6f);
    totalAmmo = panel->GetTextPointer("totalammo");
    totalAmmo->SetNoFlash(color32(-4671333));
    totalAmmo->SetScale(0.6f);
    if (mClient > 0)
    {
        frame = PanelQuad::Clone(frame);
        clipAmmo = clipAmmo->Clone();
        totalAmmo = totalAmmo->Clone();
    }
}

// ea: 0x00566560
void IGOAmmoWidget::Draw()
{
    if (is_shown && !dont_draw)
    {
        frame->Draw();
        clipAmmo->Draw();
        totalAmmo->Draw();
    }
}

// ea: 0x00582D30
void IGOAmmoWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    frame->FattenMeForWidescreen(widescreen, about_x);
    clipAmmo->UpdateForWidescreen(widescreen, (int)about_x);
    totalAmmo->UpdateForWidescreen(widescreen, (int)about_x);
}

// ea: 0x00577840
void IGOAmmoWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    clipAmmo->UpdateForHUDSplitScreen(viewport, old_viewport, 10, -5.0f,
                                      -2.0f);
    totalAmmo->UpdateForHUDSplitScreen(viewport, old_viewport, 10, 5.0f,
                                       -2.0f);
    frame->FormatHUDForSplitScreen(viewport, old_viewport, 10, 0.0f, 0.0f);
}

// ============================================================================
// IGOActionHintWidget
// ============================================================================

// ea: 0x005779C0
IGOActionHintWidget::IGOActionHintWidget(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    text = (FEText*)mem_heap_malloc(0x70u);
    if (text != nullptr)
    {
        text = new (text) FEText(FONT_BUTTON, defaultFileName, 325.0f,
                                 267.0f, 0, PANEL_LAYER_IGO, 0.45f, 0, 64,
                                 color32(-2961486));
    }
    else
    {
        text = nullptr;
    }
    dont_draw = false;
    lastAlpha = 0.0f;
    isFadingDown = false;
    startHintTime = 0;
}

// ea: 0x00567310
void IGOActionHintWidget::Init(PanelFile* panel)
{
    (void)panel;
}

// ea: 0x00582EC0
void IGOActionHintWidget::Update(float time_inc)
{
    (void)time_inc;
    if (!IsShown()
        || EntityManager::sInst->GetPlayer(mClient) == nullptr
        || EntityManager::sInst->GetPlayer(mClient)->client == nullptr)
    {
        return;
    }
    int mProneBlockedTime =
        EntityManager::sInst->GetPlayer(mClient)->client->mProneBlockedTime;
    int hintType = 0;
    if (EntityManager::sInst->GetPlayer(mClient)->client
            ->mMedicNobodyToReviveTime
        > mProneBlockedTime)
    {
        mProneBlockedTime =
            EntityManager::sInst->GetPlayer(currCl)->client
                ->mMedicNobodyToReviveTime;
        hintType = 1;
    }
    if (EntityManager::sInst->GetPlayer(mClient)->client
            ->mTankExitBlockedByMantleTime
        > mProneBlockedTime)
    {
        mProneBlockedTime =
            EntityManager::sInst->GetPlayer(mClient)->client
                ->mTankExitBlockedByMantleTime;
        hintType = 2;
    }
    int* hintTimer = &g_femanager.IGO->actionHintTimer[mClient];
    if (*hintTimer > mProneBlockedTime)
    {
        if (*hintTimer <= level.time)
        {
            mProneBlockedTime = *hintTimer;
            hintType = 3;
        }
        else
        {
            *hintTimer = 0;
        }
    }
    int time_left = mProneBlockedTime - level.time + 3000;
    if (mProneBlockedTime >= 0 && time_left >= 0)
    {
        if (dont_draw)
            startHintTime = level.time;
        int v8 = level.time - startHintTime;
        float psin;
        float fc;
        FastSinCos(((((v8 % 1000) * 0.001f) * 2.0f) - 1.0f) * 3.1415927f,
                   &psin, &fc);
        float v10 = (fc + 1.0f) * 0.5f;
        if (v8 >= 500)
            v10 = (v10 + 1.0f) * 0.5f;
        float v9 = 1.0f;
        if (time_left < 1000)
            v9 = time_left * 0.001f;
        dont_draw = false;
        text->SetAlpha(v9 * v10);
        switch (hintType)
        {
        case 0:
            text->SetText("MPGAME_PRONE_BLOCKED");
            break;
        case 1:
            text->SetText("MPGAME_MEDIC_NOBODY_TO_REVIVE");
            break;
        case 2:
            text->SetText("MPGAME_TANK_EXIT_BLOCKED_BY_MANTLE");
            break;
        case 3:
        {
            const char* STBString = STBManager::sInst->GetSTBString(
                g_femanager.IGO->actionHintText[currCl]);
            if (STBString != nullptr)
                text->SetText(STBString);
            else
                text->SetText("UNKNOWN_STRING");
            break;
        }
        }
    }
    else
    {
        dont_draw = true;
    }
}

// ea: 0x00567320
void IGOActionHintWidget::Draw()
{
    if (IsShown() && !dont_draw)
        text->Draw();
}

// ea: 0x00567340
void IGOActionHintWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    text->UpdateForWidescreen(widescreen, (int)about_x);
}

// ea: 0x00567360
void IGOActionHintWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    text->UpdateForSplitScreen(viewport, old_viewport);
}

// ============================================================================
// IGOHQProgressBarWidget
// ============================================================================

// ea: 0x00565F50
IGOHQProgressBarWidget::IGOHQProgressBarWidget(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    loading_bar_bkg_01 = nullptr;
    loading_bar_bkg_02 = nullptr;
    loading_bar_bkg_03 = nullptr;
    loading_bar_white = nullptr;
    loading_bar_red = nullptr;
    m_pRadioIcon = nullptr;
    m_Draw = false;
}

// ea: 0x00565F90
IGOHQProgressBarWidget::~IGOHQProgressBarWidget()
{
    if (loading_bar_bkg_01 != nullptr)
        delete loading_bar_bkg_01;
    if (loading_bar_bkg_02 != nullptr)
        delete loading_bar_bkg_02;
    if (loading_bar_bkg_03 != nullptr)
        delete loading_bar_bkg_03;
    if (loading_bar_white != nullptr)
        delete loading_bar_white;
    if (loading_bar_red != nullptr)
        delete loading_bar_red;
    if (m_pRadioIcon != nullptr)
        delete m_pRadioIcon;
}

// ea: 0x00598410
void IGOHQProgressBarWidget::Init(PanelFile* panel)
{
    loading_bar_bkg_01 = panel->GetPointer("loading_bar_bkg_01");
    loading_bar_bkg_02 = panel->GetPointer("loading_bar_bkg_02");
    loading_bar_bkg_03 = panel->GetPointer("loading_bar_bkg_03");
    loading_bar_white = panel->GetPointer("loading_bar_use");
    loading_bar_red = panel->GetPointer("loading_bar_red_use");
    m_pRadioIcon = panel->GetPointer("radio_icon");
    if (mClient > 0)
    {
        loading_bar_bkg_01 = PanelQuad::Clone(loading_bar_bkg_01);
        loading_bar_bkg_02 = PanelQuad::Clone(loading_bar_bkg_02);
        loading_bar_bkg_03 = PanelQuad::Clone(loading_bar_bkg_03);
        loading_bar_white = PanelQuad::Clone(loading_bar_white);
        loading_bar_red = PanelQuad::Clone(loading_bar_red);
        m_pRadioIcon = PanelQuad::Clone(m_pRadioIcon);
    }
}

// ea: 0x005828F0
void IGOHQProgressBarWidget::Update(float time_inc)
{
    (void)time_inc;
    m_pRadioIcon->SetShown(false);
    if (is_shown
        && dword_F62960[1580 * currCl] != 0
        && EntityManager::sInst->GetPlayer(currCl) != nullptr
        && EntityManager::sInst->GetPlayer(currCl)->sentient != nullptr
        && MPUIInterface::mServerParams.mGameType == 3
        && gpBrocAPI->mBrocExports.mCallbackGetTeamCapturingHQPercent
               != nullptr
        && gpBrocAPI->mBrocExports.mCallbackGetHQCaptureStatus != nullptr
        && gpBrocAPI->mBrocExports.mCallbackGetTeamDestroyingHQPercent
               != nullptr)
    {
        Entity* Player = GetPlayer(currCl);
        float capturePct =
            (float)gpBrocAPI->mBrocExports
                .mCallbackGetTeamCapturingHQPercent(
                    *(Broc::entity*)(dword_F62960[1580 * currCl] + 176))
            * 0.0001f;
        int v5 = gpBrocAPI->mBrocExports.mCallbackGetHQCaptureStatus();
        float v6 =
            (float)gpBrocAPI->mBrocExports
                .mCallbackGetTeamDestroyingHQPercent()
            * 0.0001f;
        if (Player->key > level.time)
            Player->key = 0;
        int key = Player->key;
        if (key != 0 && key > level.time - 1000)
            m_pRadioIcon->SetShown(true);
        if (v5 != 0)
        {
            if (capturePct > 0.01f)
            {
                loading_bar_white->SetShown(true);
                loading_bar_red->SetShown(false);
                loading_bar_white->Mask(1.0f - capturePct, RIGHT_MASK, 1.0f);
                m_Draw = true;
                return;
            }
            if (v6 > 0.01f
                && ((Player->sentient->eTeam == TEAM_ALLIES && v5 == 1)
                    || (Player->sentient->eTeam == TEAM_AXIS && v5 == -1)))
            {
                loading_bar_white->SetShown(false);
                loading_bar_red->SetShown(true);
                loading_bar_red->Mask(1.0f - v6, RIGHT_MASK, 1.0f);
                m_Draw = true;
                return;
            }
        }
        else if (capturePct > 0.01f)
        {
            loading_bar_white->SetShown(true);
            loading_bar_red->SetShown(false);
            loading_bar_white->Mask(capturePct, RIGHT_MASK, 1.0f);
            m_Draw = true;
            return;
        }
    }
    m_Draw = false;
}

// ea: 0x00566020
void IGOHQProgressBarWidget::Draw()
{
    m_pRadioIcon->Draw();
    if (is_shown && m_Draw)
    {
        loading_bar_white->Draw();
        loading_bar_red->Draw();
        loading_bar_bkg_01->Draw();
        loading_bar_bkg_02->Draw();
        loading_bar_bkg_03->Draw();
    }
}

// ea: 0x00582B70
void IGOHQProgressBarWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    m_pRadioIcon->FattenMeForWidescreen(widescreen, about_x);
    loading_bar_bkg_01->SetXYInitialToCurrentPos();
    loading_bar_bkg_02->SetXYInitialToCurrentPos();
    loading_bar_bkg_03->SetXYInitialToCurrentPos();
    loading_bar_white->SetXYInitialToCurrentPos();
    loading_bar_red->SetXYInitialToCurrentPos();
}

// ea: 0x00582BD0
void IGOHQProgressBarWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    loading_bar_bkg_01->FormatHUDForSplitScreen(viewport, old_viewport, 0,
                                                0.0f, 0.0f);
    loading_bar_bkg_02->FormatHUDForSplitScreen(viewport, old_viewport, 0,
                                                0.0f, 0.0f);
    loading_bar_bkg_03->FormatHUDForSplitScreen(viewport, old_viewport, 0,
                                                0.0f, 0.0f);
    loading_bar_white->FormatHUDForSplitScreen(viewport, old_viewport, 0,
                                               0.0f, 0.0f);
    loading_bar_red->FormatHUDForSplitScreen(viewport, old_viewport, 0,
                                             0.0f, 0.0f);
    m_pRadioIcon->FormatForSplitScreen(viewport, old_viewport);
}

// ============================================================================
// IGOInGameScoreWidget
// ============================================================================

// ea: 0x00566340
IGOInGameScoreWidget::IGOInGameScoreWidget(int client)
{
    mClient = client;
    is_shown = true;
    force_appear = false;
    m_pAlliesFlagIcon = nullptr;
    m_pAxisFlagIcon = nullptr;
    m_pAlliesScoreText = nullptr;
    m_pAxisScoreText = nullptr;
    m_AlliesScore = 0;
    m_AxisScore = 0;
    m_Draw = true;
}

// ea: 0x00566380
IGOInGameScoreWidget::~IGOInGameScoreWidget()
{
    if (m_pAlliesFlagIcon != nullptr)
        delete m_pAlliesFlagIcon;
    if (m_pAxisFlagIcon != nullptr)
        delete m_pAxisFlagIcon;
    if (m_pAlliesScoreText != nullptr)
        delete m_pAlliesScoreText;
    if (m_pAxisScoreText != nullptr)
        delete m_pAxisScoreText;
}

// ea: 0x00598540
void IGOInGameScoreWidget::Init(PanelFile* panel)
{
    m_pAlliesFlagIcon = panel->GetPointer("sb_allied_icon");
    m_pAxisFlagIcon = panel->GetPointer("sb_axis_icon");
    m_pAlliesScoreText = panel->GetTextPointer("sb_allied_text");
    m_pAxisScoreText = panel->GetTextPointer("sb_axis_text");
    if (mClient > 0)
    {
        m_pAlliesFlagIcon = PanelQuad::Clone(m_pAlliesFlagIcon);
        m_pAxisFlagIcon = PanelQuad::Clone(m_pAxisFlagIcon);
        m_pAlliesScoreText =
            (FEText*)mem_heap_malloc(0x70u);
        if (m_pAlliesScoreText != nullptr)
            m_pAlliesScoreText = new (m_pAlliesScoreText) FEText();
        else
            m_pAlliesScoreText = nullptr;
        m_pAxisScoreText = (FEText*)mem_heap_malloc(0x70u);
        if (m_pAxisScoreText != nullptr)
            m_pAxisScoreText = new (m_pAxisScoreText) FEText();
        else
            m_pAxisScoreText = nullptr;
        m_pAlliesScoreText->CopyFrom(
            panel->GetTextPointer("sb_allied_text"));
        m_pAxisScoreText->CopyFrom(panel->GetTextPointer("sb_axis_text"));
    }
}

// ea: 0x00566400
void IGOInGameScoreWidget::Update(float time_inc)
{
    (void)time_inc;
    if (is_shown
        && dword_F62960[1580 * currCl] != 0
        && cgGlobal.showScore
        && EntityManager::sInst->GetPlayer(currCl)->client->pers.playerState
               == 3)
    {
        if (dword_F62960[1580 * currCl] != -16)
        {
            m_AlliesScore = cgGlobal.teamScores[2];
            m_AxisScore = cgGlobal.teamScores[1];
            char text[8];
            sprintf(text, "%i", m_AlliesScore);
            if (text[0] != 0)
                m_pAlliesScoreText->SetTextNoLocalize(text);
            sprintf(text, "%i", m_AxisScore);
            if (text[0] != 0)
                m_pAxisScoreText->SetTextNoLocalize(text);
            m_Draw = true;
        }
    }
    else
    {
        m_Draw = false;
    }
}

// ea: 0x005664F0
void IGOInGameScoreWidget::Draw()
{
    if (is_shown && m_Draw)
    {
        m_pAlliesFlagIcon->Draw();
        m_pAxisFlagIcon->Draw();
        m_pAlliesScoreText->Draw();
        m_pAxisScoreText->Draw();
    }
}

// ea: 0x00582D00
void IGOInGameScoreWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    m_pAlliesFlagIcon->FattenMeForWidescreen(widescreen, about_x);
    m_pAxisFlagIcon->FattenMeForWidescreen(widescreen, about_x);
}

// ea: 0x005777D0
void IGOInGameScoreWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    m_pAlliesFlagIcon->FormatHUDForSplitScreen(viewport, old_viewport, 5,
                                               0.0f, 22.0f);
    m_pAxisFlagIcon->FormatHUDForSplitScreen(viewport, old_viewport, 5,
                                             0.0f, 32.0f);
    m_pAlliesScoreText->UpdateForHUDSplitScreen(viewport, old_viewport, 5,
                                                0.0f, 0.0f);
    m_pAxisScoreText->UpdateForHUDSplitScreen(viewport, old_viewport, 5,
                                              0.0f, 0.0f);
}

// ============================================================================
// IGORaiseFlagWidget
// ============================================================================

// ea: 0x00566070
IGORaiseFlagWidget::IGORaiseFlagWidget(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    raise_flag_icon = nullptr;
    multiplyer_text = nullptr;
    player_count = 0;
}

// ea: 0x005660A0
IGORaiseFlagWidget::~IGORaiseFlagWidget()
{
    if (raise_flag_icon != nullptr)
        delete raise_flag_icon;
    if (multiplyer_text != nullptr)
        delete multiplyer_text;
}

// ea: 0x005984D0
void IGORaiseFlagWidget::Init(PanelFile* panel)
{
    raise_flag_icon = panel->GetPointer("i_raisingflag");
    multiplyer_text = panel->GetTextPointer("i_raisingflag_text_counter");
    if (mClient > 0)
    {
        raise_flag_icon = PanelQuad::Clone(raise_flag_icon);
        multiplyer_text = multiplyer_text->Clone();
    }
    raise_flag_icon->SetShown(false);
    multiplyer_text->SetShown(false);
}

// ea: 0x00566100
void IGORaiseFlagWidget::Update(float time_inc)
{
    (void)time_inc;
    raise_flag_icon->SetShown(false);
    multiplyer_text->SetShown(false);
    if (is_shown)
    {
        int v3 = dword_F62960[1580 * currCl];
        if (v3 != 0
            && gpBrocAPI->mBrocExports.mCallbackGetFlagBeingContested
                   != nullptr)
        {
            player_count =
                gpBrocAPI->mBrocExports.mCallbackGetFlagBeingContested(
                    *(Broc::entity*)(dword_F62960[1580 * currCl] + 176));
            if (player_count > 0)
            {
                raise_flag_icon->SetShown(true);
                multiplyer_text->SetShown(player_count > 1);
            }
            char text[4];
            sprintf(text, "x%i", player_count);
            if (text[0] != 0)
                multiplyer_text->SetTextNoLocalize(text);
        }
    }
}

// ea: 0x005661D0
void IGORaiseFlagWidget::Draw()
{
    if (is_shown)
    {
        raise_flag_icon->Draw();
        multiplyer_text->Draw();
    }
}

// ea: 0x00577660
void IGORaiseFlagWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    raise_flag_icon->FormatHUDForSplitScreen(viewport, old_viewport, 0,
                                             0.0f, 0.0f);
    multiplyer_text->UpdateForHUDSplitScreen(viewport, old_viewport, 0, 0.0f,
                                             0.0f);
}

// ea: 0x00582C40
void IGORaiseFlagWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    raise_flag_icon->FattenMeForWidescreen(widescreen, about_x);
    multiplyer_text->UpdateForWidescreen(widescreen, (int)about_x);
}

// ============================================================================
// IGOTimerWidget
// ============================================================================

// ea: 0x005661F0
IGOTimerWidget::IGOTimerWidget(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    m_pTimer = nullptr;
    m_DeltaTime = 0.0f;
    m_hour = 0;
    m_min = 0;
    m_sec = 0;
    m_TimerActive = false;
    m_Draw = false;
}

// ea: 0x00566230
IGOTimerWidget::~IGOTimerWidget()
{
    if (m_pTimer != nullptr)
        delete m_pTimer;
}

// ea: 0x00582C70
void IGOTimerWidget::Init(PanelFile* panel)
{
    m_pTimer = panel->GetTextPointer("sb_timer_text");
    if (mClient > 0)
    {
        m_pTimer = (FEText*)mem_heap_malloc(0x70u);
        if (m_pTimer != nullptr)
            m_pTimer = new (m_pTimer) FEText();
        else
            m_pTimer = nullptr;
        m_pTimer->CopyFrom(panel->GetTextPointer("sb_timer_text"));
    }
}

// ea: 0x005762A0
void IGOTimerWidget::Update(float time_inc)
{
    (void)time_inc;
    if (!is_shown
        || dword_F62960[1580 * currCl] == 0
        || EntityManager::sInst->GetPlayer(currCl)->client->pers.playerState
               != 3)
    {
        m_Draw = false;
        return;
    }
    if (cgGlobal.gameTime > 0.0f)
    {
        m_TimeLimit = cgGlobal.gameTime;
        m_StartTime = cgGlobal.gameTimeStartTime;
        cgGlobal.gameTime = 0.0f;
    }
    float v3 = (float)(cgGlobal.time + 999) - m_StartTime;
    m_DeltaTime = v3;
    if (v3 < 0.0f)
    {
        m_Draw = false;
        return;
    }
    if (m_TimeLimit < v3 * 0.001f)
    {
        cgGlobal.gameTime = 0.0f;
        m_TimeLimit = 0.0f;
        m_Draw = false;
        return;
    }
    setTimerValues();
    char text[8];
    if (m_hour != 0)
        sprintf(text, "%i:%02i:%02i", m_hour, m_min, m_sec);
    else
        sprintf(text, "%i:%02i", m_min, m_sec);
    if (text[0] != 0)
        m_pTimer->SetTextNoLocalize(text);
    m_Draw = true;
}

// ea: 0x005662F0
void IGOTimerWidget::setTimerValues()
{
    float v1 = m_DeltaTime * 0.001f;
    int v2 = (int)(m_TimeLimit - v1);
    unsigned int v3 = (unsigned int)v2 / 3600;
    if (v2 / 3600 != 0)
        v2 = (int)(m_TimeLimit - v1) % 3600;
    m_hour = v3;
    m_min = (unsigned int)v2 / 60;
    if (v2 / 60 != 0)
        m_sec = v2 % 60;
    else
        m_sec = v2;
}

// ea: 0x00566280
void IGOTimerWidget::Draw()
{
    if (is_shown && m_Draw)
        m_pTimer->Draw();
}

// ea: 0x005662A0
void IGOTimerWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    m_pTimer->UpdateForWidescreen(widescreen, (int)about_x);
}

// ea: 0x005662C0
void IGOTimerWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    m_pTimer->UpdateForHUDSplitScreen(viewport, old_viewport, 5, 0.0f,
                                      10.0f);
}

// ============================================================================
// IGOGrenadeWidget
// ============================================================================

// ea: 0x00566590
IGOGrenadeWidget::IGOGrenadeWidget(int client)
{
    mClient = client;
    is_shown = true;
    force_appear = false;
    grenadeUS = nullptr;
    grenadeGerman = nullptr;
    grenadeSmokeL = nullptr;
    grenadeSmokeR = nullptr;
    grenadeSticky = nullptr;
    rifleGrenade = nullptr;
    apMine = nullptr;
    ammoLeft = nullptr;
    ammoRight = nullptr;
    ammo_left_val = 0;
    ammo_right_val = 0;
    showLeft = true;
    showRight = true;
    left_draw_time = 0.0f;
    right_draw_time = 0.0f;
}

// ea: 0x005665F0
IGOGrenadeWidget::~IGOGrenadeWidget()
{
    if (grenadeUS != nullptr)
        delete grenadeUS;
    if (grenadeGerman != nullptr)
        delete grenadeGerman;
    if (grenadeSmokeL != nullptr)
        delete grenadeSmokeL;
    if (grenadeSmokeR != nullptr)
        delete grenadeSmokeR;
    if (grenadeSticky != nullptr)
        delete grenadeSticky;
    if (rifleGrenade != nullptr)
        delete rifleGrenade;
    if (apMine != nullptr)
        delete apMine;
    if (ammoLeft != nullptr)
        delete ammoLeft;
    if (ammoRight != nullptr)
        delete ammoRight;
}

// ea: 0x00598730
void IGOGrenadeWidget::Init(PanelFile* panel)
{
    grenadeUS = panel->GetPointer("icon_grenade_US_01");
    grenadeGerman = panel->GetPointer("icon_grenade_german_01");
    grenadeSmokeL = panel->GetPointer("icon_grenade_smoke_01");
    grenadeSmokeR = panel->GetPointer("icon_grenade_smoke");
    grenadeSticky = panel->GetPointer("icon_grenade_sticky_01");
    rifleGrenade = panel->GetPointer("icon_grenade_rifle_US");
    apMine = panel->GetPointer("icon_AP_mine");
    ammoLeft = panel->GetTextPointer("text_grenade_left");
    ammoRight = panel->GetTextPointer("text_grenade_right");
    if (mClient > 0)
    {
        grenadeUS = PanelQuad::Clone(grenadeUS);
        grenadeGerman = PanelQuad::Clone(grenadeGerman);
        grenadeSmokeL = PanelQuad::Clone(grenadeSmokeL);
        grenadeSmokeR = PanelQuad::Clone(grenadeSmokeR);
        grenadeSticky = PanelQuad::Clone(grenadeSticky);
        rifleGrenade = PanelQuad::Clone(rifleGrenade);
        apMine = PanelQuad::Clone(apMine);
        ammoLeft = (FEText*)mem_heap_malloc(0x70u);
        if (ammoLeft != nullptr)
            ammoLeft = new (ammoLeft) FEText();
        else
            ammoLeft = nullptr;
        ammoRight = (FEText*)mem_heap_malloc(0x70u);
        if (ammoRight != nullptr)
            ammoRight = new (ammoRight) FEText();
        else
            ammoRight = nullptr;
        ammoLeft->CopyFrom(panel->GetTextPointer("text_grenade_left"));
        ammoRight->CopyFrom(panel->GetTextPointer("text_grenade_right"));
    }
}

// ea: 0x005666B0
void IGOGrenadeWidget::Update(float time_inc)
{
    if (!is_shown
        || EntityManager::sInst->GetPlayer(mClient) == nullptr
        || EntityManager::sInst->GetPlayer(mClient)->client == nullptr)
    {
        return;
    }
    int cgClientBase = dword_F62960[1580 * currCl];
    if (((EntityManager::sInst->GetPlayer(mClient)->client->ps.eFlags
          & 0x100000)
             != 0
         && !BG_AllowPlayerWeaponAtVehiclePos(
             *(int*)(cgClientBase + 1336), *(int*)(cgClientBase + 1332)))
        || dword_F62960[1580 * currCl] == 0
        || EntityManager::sInst->GetPlayer(mClient)->client->pers.playerState
               != 3)
    {
        showLeft = false;
        showRight = false;
        return;
    }
    int v5 = GetPlayerState(currCl).weaponslots[4];
    int weapRight = GetPlayerState(currCl).weaponslots[9];
    if (v5 == 0 && weapRight == 0)
    {
        showLeft = false;
        showRight = false;
        return;
    }
    int GrenadeCount = CG_GetGrenadeCount();
    int SpecialGrenadeCount = CG_GetSpecialGrenadeCount();
    left_draw_time -= time_inc;
    right_draw_time -= time_inc;
    float v11 = g_grenadeFadeTime.value + g_grenadeSolidTime.value;
    if (GrenadeCount != ammo_left_val || force_appear)
        left_draw_time = v11;
    if (SpecialGrenadeCount != ammo_right_val || force_appear)
        right_draw_time = v11;
    ammo_left_val = GrenadeCount;
    ammo_right_val = SpecialGrenadeCount;
    force_appear = false;
    showLeft = GrenadeCount > 0;
    showRight = SpecialGrenadeCount > 0;
    if (showLeft)
    {
        ammoLeft->SetColor(color32(-1));
        if (ammo_left_val > 999)
            ammo_left_val = 999;
        char text[4];
        sprintf(text, "x%i", ammo_left_val);
        if (text[0] != 0)
        {
            ammoLeft->SetTextNoLocalize(text);
            ammoLeft->SetShown(true);
        }
        weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(v5);
        if (strcmp(InfoForWeapon->szRadiantName, "weapon_smokegrenade") == 0)
        {
            grenadeSmokeL->SetVisibility(1.0f);
            grenadeUS->SetVisibility(0.0f);
            grenadeGerman->SetVisibility(0.0f);
            grenadeSticky->SetVisibility(0.0f);
        }
        if (strcmp(InfoForWeapon->szRadiantName, "weapon_fraggrenade") == 0)
        {
            grenadeUS->SetVisibility(1.0f);
            grenadeGerman->SetVisibility(0.0f);
            grenadeSticky->SetVisibility(0.0f);
            grenadeSmokeL->SetVisibility(0.0f);
        }
        else if (strcmp(InfoForWeapon->szRadiantName,
                        "weapon_stielhandgranate")
                 == 0)
        {
            grenadeUS->SetVisibility(0.0f);
            grenadeGerman->SetVisibility(1.0f);
            grenadeSticky->SetVisibility(0.0f);
            grenadeSmokeL->SetVisibility(0.0f);
        }
        else if (strcmp(InfoForWeapon->szRadiantName,
                        "weapon_stickygrenade")
                 == 0)
        {
            grenadeUS->SetVisibility(0.0f);
            grenadeGerman->SetVisibility(0.0f);
            grenadeSmokeL->SetVisibility(0.0f);
            grenadeSticky->SetVisibility(1.0f);
        }
    }
    else
    {
        grenadeSmokeL->SetVisibility(0.0f);
        grenadeUS->SetVisibility(0.0f);
        grenadeGerman->SetVisibility(0.0f);
        grenadeSticky->SetVisibility(0.0f);
        ammoLeft->SetShown(false);
    }
    if (showRight)
    {
        ammoRight->SetColor(color32(-1));
        if (ammo_right_val > 999)
            ammo_right_val = 999;
        char text[4];
        sprintf(text, "x%i", ammo_right_val);
        if (text[0] != 0)
        {
            ammoRight->SetTextNoLocalize(text);
            ammoRight->SetShown(true);
        }
        const char* v18 = BG_GetInfoForWeapon(weapRight)->szRadiantName;
        if (strcmp(v18, "weapon_m1garand_RG") == 0
            || strcmp(v18, "weapon_k98_RG") == 0)
        {
            grenadeSmokeR->SetVisibility(0.0f);
            rifleGrenade->SetVisibility(1.0f);
            apMine->SetVisibility(0.0f);
        }
        else if (strcmp(v18, "weapon_mine") == 0)
        {
            grenadeSmokeR->SetVisibility(0.0f);
            rifleGrenade->SetVisibility(0.0f);
            apMine->SetVisibility(1.0f);
        }
        else if (strcmp(v18, "weapon_smokegrenade") == 0)
        {
            grenadeSmokeR->SetVisibility(1.0f);
            rifleGrenade->SetVisibility(0.0f);
            apMine->SetVisibility(0.0f);
        }
    }
    else
    {
        grenadeSmokeR->SetVisibility(0.0f);
        rifleGrenade->SetVisibility(0.0f);
        apMine->SetVisibility(0.0f);
        ammoRight->SetShown(false);
    }
}

// ea: 0x00566BC0
void IGOGrenadeWidget::Draw()
{
    if (is_shown
        && (showLeft || showRight)
        && EntityManager::sInst->GetPlayer(mClient)->client->pers.playerState
               == 3)
    {
        grenadeUS->Draw();
        grenadeGerman->Draw();
        grenadeSmokeL->Draw();
        grenadeSmokeR->Draw();
        grenadeSticky->Draw();
        rifleGrenade->Draw();
        apMine->Draw();
        ammoLeft->Draw();
        ammoRight->Draw();
    }
}

// ea: 0x00582D70
void IGOGrenadeWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    grenadeUS->FattenMeForWidescreen(widescreen, about_x);
    grenadeGerman->FattenMeForWidescreen(widescreen, about_x);
    grenadeSmokeL->FattenMeForWidescreen(widescreen, about_x);
    grenadeSmokeR->FattenMeForWidescreen(widescreen, about_x);
    grenadeSticky->FattenMeForWidescreen(widescreen, about_x);
    rifleGrenade->FattenMeForWidescreen(widescreen, about_x);
    apMine->FattenMeForWidescreen(widescreen, about_x);
    ammoLeft->UpdateForWidescreen(widescreen, (int)about_x);
    ammoRight->UpdateForWidescreen(widescreen, (int)about_x);
}

// ea: 0x005778A0
void IGOGrenadeWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    grenadeUS->FormatHUDForSplitScreen(viewport, old_viewport, 10, 0.0f,
                                       -16.0f);
    grenadeGerman->FormatHUDForSplitScreen(viewport, old_viewport, 10, 0.0f,
                                           -16.0f);
    grenadeSmokeL->FormatHUDForSplitScreen(viewport, old_viewport, 10, 0.0f,
                                           -16.0f);
    grenadeSmokeR->FormatHUDForSplitScreen(viewport, old_viewport, 10, 0.0f,
                                           -16.0f);
    grenadeSticky->FormatHUDForSplitScreen(viewport, old_viewport, 10, 0.0f,
                                           -16.0f);
    rifleGrenade->FormatHUDForSplitScreen(viewport, old_viewport, 10, -10.0f,
                                          -16.0f);
    apMine->FormatHUDForSplitScreen(viewport, old_viewport, 10, -10.0f,
                                    -16.0f);
    ammoLeft->UpdateForHUDSplitScreen(viewport, old_viewport, 10, -10.0f,
                                      -8.0f);
    ammoRight->UpdateForHUDSplitScreen(viewport, old_viewport, 10, 3.0f,
                                       -8.0f);
}

// ============================================================================
// IGOSpecialWeaponWidget
// ============================================================================

// ea: 0x00567820
IGOSpecialWeaponWidget::IGOSpecialWeaponWidget(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    artillery = nullptr;
    health = nullptr;
    ammo = nullptr;
    percent = 0.0f;
    hadAmmo = false;
    timeForNormalSize = 0;
    scale = 1.0f;
}

// ea: 0x00598DE0
void IGOSpecialWeaponWidget::Init(PanelFile* panel)
{
    artillery = panel->GetPointer("special_artillery");
    health = panel->GetPointer("special_health");
    ammo = panel->GetPointer("special_ammo");
    if (mClient > 0)
    {
        artillery = PanelQuad::Clone(
            panel->GetPointer("special_artillery"));
        health = PanelQuad::Clone(panel->GetPointer("special_health"));
        ammo = PanelQuad::Clone(panel->GetPointer("special_ammo"));
    }
}

// ea: 0x00567860
void IGOSpecialWeaponWidget::Update(float time_inc)
{
    (void)time_inc;
    if (!is_shown)
        return;
    percent = 0.0f;
    if (!cgGlobal.teamGame)
        return;
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player->sentient == nullptr)
        return;
    Client* client = Player->client;
    if (client->pers.playerState != 3)
        return;
    int v5 = client->ps.weaponslots[9];
    if (client->ps.weaponslots[9] != 0)
    {
        int v6;
        if (BG_WeaponIsClipOnly(client->ps.weaponslots[9]))
            v6 = Player->client->ps.ammoclip[BG_ClipForWeapon(v5)];
        else
            v6 = Player->client->ps.ammo[BG_AmmoForWeapon(v5)];
        if (v6 != 0)
        {
            bool hadAmmo = this->hadAmmo;
            percent = 1.0f;
            if (!hadAmmo)
                timeForNormalSize = cgGlobal.time + 2000;
            this->hadAmmo = true;
            if (timeForNormalSize - cgGlobal.time > 2000)
                timeForNormalSize = 0;
            scale =
                (((float)(timeForNormalSize - cgGlobal.time) * 0.0005f)
                 * 0.25f)
                + 1.0f;
            if (scale < 1.0f)
                scale = 1.0f;
            return;
        }
        this->hadAmmo = false;
    }
    scale = 1.0f;
    int v10 = dword_F641A0[1580 * currCl];
    if (v10 > cgGlobal.time)
        percent =
            1.0f
            - ((float)(v10 - cgGlobal.time)
               / (float)dword_F641A4[1580 * currCl]);
    if (percent > 1.0f)
        percent = 1.0f;
    if (percent < 0.0f)
        percent = 0.0f;
}

// ea: 0x005832D0
void IGOSpecialWeaponWidget::Draw()
{
    if (is_shown
        && percent >= 0.00001f
        && (((EntityManager::sInst->GetPlayer(currCl)->client->ps.eFlags
              & 0x100000)
                 == 0)
            || BG_AllowPlayerWeaponAtVehiclePos(
                *(int*)(dword_F62960[1580 * currCl] + 1336),
                *(int*)(dword_F62960[1580 * currCl] + 1332))))
    {
        Entity* Player = EntityManager::sInst->GetPlayer(currCl);
        if (Player->sentient != nullptr
            && Player->client->pers.playerState == 3)
        {
            int v4 = dword_F6419C[1580 * currCl];
            PanelQuad* quad;
            switch (v4)
            {
            case 6:
                quad = artillery;
                break;
            case 3:
                quad = health;
                break;
            case 4:
            case 5:
                quad = ammo;
                break;
            default:
                return;
            }
            if (quad != nullptr)
            {
                int window = unk_F6A284[802 * currCl];
                float x_scale = View::GetXScalingForHUD(window) * scale;
                float y_scale = View::GetYScalingForHUD(window) * scale;
                quad->ScaleAbsoluteCenter(x_scale, y_scale);
                quad->Mask(1.0f, TOP_MASK, 1.0f);
                quad->SetAlpha(0.25f);
                quad->Draw();
                quad->Mask(
                    (percent / (percentToTrimBottom + percentToTrimTop + 1.0f))
                        + percentToTrimBottom,
                    TOP_MASK, 1.0f);
                float alpha = 1.0f;
                if (percent <= 0.999f)
                    alpha = 0.8f;
                quad->SetAlpha(alpha);
                quad->Draw();
            }
        }
    }
}

// ea: 0x005834B0
void IGOSpecialWeaponWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    artillery->FattenMeForWidescreen(widescreen, about_x);
    health->FattenMeForWidescreen(widescreen, about_x);
    ammo->FattenMeForWidescreen(widescreen, about_x);
}

// ea: 0x00577B40
void IGOSpecialWeaponWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    artillery->FormatHUDForSplitScreen(viewport, old_viewport, 2, 0.0f,
                                       -25.0f);
    health->FormatHUDForSplitScreen(viewport, old_viewport, 2, 0.0f,
                                    -25.0f);
    ammo->FormatHUDForSplitScreen(viewport, old_viewport, 2, 0.0f, -25.0f);
}

// ============================================================================
// IGOTankLoadingWidget
// ============================================================================

// ea: 0x00567420
IGOTankLoadingWidget::IGOTankLoadingWidget(int client)
{
    mClient = client;
    is_shown = true;
    force_appear = false;
    on = nullptr;
    off = nullptr;
    is_on = true;
}

// ea: 0x00598B00
void IGOTankLoadingWidget::Init(PanelFile* panel)
{
    on = panel->GetPointer("tankshells03");
    off = panel->GetPointer("tankshells02");
    if (mClient > 0)
    {
        on = PanelQuad::Clone(on);
        off = PanelQuad::Clone(off);
    }
}

// ea: 0x00588AF0
void IGOTankLoadingWidget::Update(float time_inc)
{
    (void)time_inc;
    if (is_shown && on != nullptr)
    {
        DbLinkedHandle<EntityHandleDb, Entity> h = GetPlayersTank();
        if (h.mHandle.mVal != 0)
            is_on = gpBrocAPI->mIsTurretReady(h.mHandle.mVal);
    }
}

// ea: 0x00567450
void IGOTankLoadingWidget::Draw()
{
    if (is_shown
        && on != nullptr
        && EntityManager::sInst->GetPlayer(currCl) != nullptr
        && EntityManager::sInst->GetPlayer(currCl)->client != nullptr
        && EntityManager::sInst->GetPlayer(currCl)->client->pers.playerState
               == 3
        && (GetPlayerState(currCl).eFlags & 0x100000) != 0)
    {
        if (is_on)
            on->Draw();
        else
            off->Draw();
    }
}

// ea: 0x005831B0
void IGOTankLoadingWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    on->FattenMeForWidescreen(widescreen, about_x);
    off->FattenMeForWidescreen(widescreen, about_x);
}

// ea: 0x00577A70
void IGOTankLoadingWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    on->FormatHUDForSplitScreen(viewport, old_viewport, 2, 0.0f, 0.0f);
    off->FormatHUDForSplitScreen(viewport, old_viewport, 2, 0.0f, 0.0f);
}

// ============================================================================
// IGOTankReticleWidget
// ============================================================================

// ea: 0x00567500
IGOTankReticleWidget::IGOTankReticleWidget(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    reticle = nullptr;
    tic[0] = nullptr;
    tic[1] = nullptr;
    tic[2] = nullptr;
    tic[3] = nullptr;
    ticCount = 0;
    currentTic = 0;
    currentAlpha = 0.0f;
}

// ea: 0x00598B50
void IGOTankReticleWidget::Init(PanelFile* panel)
{
    reticle = panel->GetPointer("tank_reticle");
    tic[0] = panel->GetPointer("tank_tic1");
    tic[1] = panel->GetPointer("tank_tic2");
    tic[2] = panel->GetPointer("tank_tic3");
    tic[3] = panel->GetPointer("tank_tic4");
    if (mClient > 0)
    {
        reticle = PanelQuad::Clone(reticle);
        tic[0] = PanelQuad::Clone(tic[0]);
        tic[1] = PanelQuad::Clone(tic[1]);
        tic[2] = PanelQuad::Clone(tic[2]);
        tic[3] = PanelQuad::Clone(tic[3]);
    }
    float x = reticle->GetCenterX();
    float y = reticle->GetCenterY();
    float tic_x_offset[4];
    float tic_y_offset[4];
    for (int i = 0; i < 4; ++i)
    {
        tic_x_offset[i] = x - tic[i]->GetCenterX();
        tic_y_offset[i] = y - tic[i]->GetCenterY();
    }
    float ya = 240.0f - (y - reticle->GetHeight() * 0.5f);
    float v22 = reticle->GetCenterY() + ya;
    float v23 = reticle->GetCenterX() + 320.0f - x;
    reticle->SetCenterPos(v23, v22);
    for (int j = 0; j < 4; ++j)
    {
        ticXPosition[j] = reticle->GetCenterX() - tic_x_offset[j];
        ticYPosition[j] = reticle->GetCenterY() - tic_y_offset[j];
        tic[j]->SetCenterPos(ticXPosition[j], ticYPosition[j]);
    }
    ticCount = 4;
}

// ea: 0x00588B40
void IGOTankReticleWidget::Update(float time_inc)
{
    (void)time_inc;
    if (!is_shown || reticle == nullptr)
        return;
    DbLinkedHandle<EntityHandleDb, Entity> h = GetPlayersTank();
    unsigned int mVal = h.mHandle.mVal;
    unsigned int v4 = mVal & 0xFFF;
    if (v4 < 0x540
        && mVal >> 12 == (unsigned int)EntityHandleDb::sInst.mElements[v4].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
        if (mObject != nullptr)
        {
            scr_vehicle_t* scr_vehicle = (scr_vehicle_t*)mObject->scr_vehicle;
            if (scr_vehicle != nullptr)
            {
                weaponFileInfo_t* InfoForWeapon =
                    BG_GetInfoForWeapon(mObject->s.weapon);
                int v9 = InfoForWeapon->iFireTime + 2000;
                currentTic = -1;
                int fireTime = scr_vehicle->fireTime;
                int ticTime = v9 / ticCount;
                if (fireTime > 0)
                    currentTic = ticCount - fireTime / ticTime - 1;
                if (currentTic >= 0)
                {
                    for (int i = 0; i < 4; ++i)
                    {
                        if (i > currentTic)
                            tic[i]->SetAlpha(1.0f);
                        if (i < currentTic)
                            tic[i]->SetAlpha(0.0f);
                    }
                    float vehiclea =
                        (float)(scr_vehicle->fireTime
                                + ticTime * (currentTic - ticCount + 1))
                        / (float)ticTime;
                    currentAlpha = vehiclea;
                    tic[currentTic]->SetAlpha(vehiclea);
                }
            }
        }
    }
}

// ea: 0x00567530
void IGOTankReticleWidget::Draw()
{
    if (is_shown
        && reticle != nullptr
        && EntityManager::sInst->GetPlayer(currCl) != nullptr
        && EntityManager::sInst->GetPlayer(currCl)->client != nullptr
        && EntityManager::sInst->GetPlayer(currCl)->client->pers.playerState
               == 3
        && (GetPlayerState(currCl).eFlags & 0x100000) != 0
        && GetPlayerState(currCl).vehType != 1)
    {
        if (GetPlayerState(currCl).vehPos != 1)
        {
            reticle->Draw();
            if (currentTic >= 0)
            {
                for (int i = 0; i < 4; ++i)
                    tic[i]->Draw();
            }
        }
    }
}

// ea: 0x005831E0
void IGOTankReticleWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    reticle->FattenMeForWidescreen(widescreen, about_x);
    tic[0]->FattenMeForWidescreen(widescreen, about_x);
    tic[1]->FattenMeForWidescreen(widescreen, about_x);
    tic[2]->FattenMeForWidescreen(widescreen, about_x);
    tic[3]->FattenMeForWidescreen(widescreen, about_x);
}

// ea: 0x00583230
void IGOTankReticleWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    reticle->FormatForSplitScreen(viewport, old_viewport);
    tic[0]->FormatForSplitScreen(viewport, old_viewport);
    tic[1]->FormatForSplitScreen(viewport, old_viewport);
    tic[2]->FormatForSplitScreen(viewport, old_viewport);
    tic[3]->FormatForSplitScreen(viewport, old_viewport);
}

// ============================================================================
// IGOHeadIcons (render-heavy Update/Draw deferred to renderer batch)
// ============================================================================

const char* sHeadIconNames[8] = {
    "i_head_rank_1_w", "i_head_rank_3_w", "i_downed_friend_w",
    "i_spotted_sniper_w", "i_ammo_box_w", "voip_line_01_icon",
    "voip_line_03_icon", "voip_line_01_text",
};

// ea: 0x00568F70
IGOHeadIcons::IGOHeadIcons(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    memset(mHeadIcons, 0, sizeof(mHeadIcons));
    memset(mPlayers, 0, sizeof(mPlayers));
}

// ea: 0x00568F90
IGOHeadIcons::~IGOHeadIcons()
{
}

// ea: 0x0059A8A0
void IGOHeadIcons::Init(PanelFile* panel)
{
    float u[4] = {0.0f, 1.0f, 0.0f, 1.0f};
    float v[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    for (int i = 0; i < 8; ++i)
    {
        PanelQuad* Pointer = panel->GetPointer(sHeadIconNames[i]);
        mHeadIcons[i].icon = Pointer;
        if (g_femanager.GetDefaultPQ() != Pointer)
        {
            Pointer->quadBlendModeType = 1691321856;
            mHeadIcons[i].icon->SetSectionUV(0, u, v);
            mHeadIcons[i].height =
                (uint8_t)mHeadIcons[i].icon->GetWidth();
            mHeadIcons[i].alpha =
                mHeadIcons[i].icon->GetColor().c.a;
        }
    }
    for (int i = 0; i < 16; ++i)
    {
        mPlayers[i].show = false;
        mPlayers[i].index = 0;
    }
}

// ea: 0x00583D00
void IGOHeadIcons::UpdateWidescreen(bool widescreen, float about_x)
{
    for (int i = 0; i < 8; ++i)
        mHeadIcons[i].icon->FattenMeForWidescreen(widescreen, about_x);
}

// ea: 0x00583D30
void IGOHeadIcons::UpdateSplitScreen(int viewport, int old_viewport)
{
    for (int i = 0; i < 8; ++i)
        mHeadIcons[i].icon->FormatForSplitScreen(viewport, old_viewport);
}

// ea: 0x0058B070
void IGOHeadIcons::Draw()
{
    static int sInitFlags = 0;
    static unsigned int bip_head_hash = 0;
    static unsigned int turret_hash = 0;
    if ((sInitFlags & 1) == 0)
    {
        sInitFlags |= 1;
        bip_head_hash = AeHash("BIP01 HEAD");
    }
    if ((sInitFlags & 2) == 0)
    {
        sInitFlags |= 2;
        turret_hash = AeHash("tag_turret");
    }
    if (!is_shown
        || !cgGlobal.teamGame
        || EntityManager::sInst->GetPlayer(mClient) == nullptr
        || EntityManager::sInst->GetPlayer(mClient)->sentient == nullptr)
    {
        return;
    }
    Entity* Player = EntityManager::sInst->GetPlayer(mClient);
    if (Player == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOHeadIcons.cpp";
        AeAssert::gCurrentLine = 242;
        AeAssert::gCurrentExpr = "localPlayer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Local player is not valid"))
            __debugbreak();
    }
    if (Player->client->ps.pm_type >= 6)
        return;
    float distAbovePlayer = (float)mp_headIconDistAbovePlayer.integer;
    float distAboveVehicle = (float)mp_headIconDistAboveVehicle.integer;
    float iconHeight = (float)mp_headIconHeight.integer;
    float minScreenSize = (float)mp_headIconMinScreenSize.integer;
    for (int i = 0; i < 16; ++i)
    {
        if (!mPlayers[i].show)
            continue;
        Entity* v6 = EntityManager::sInst->GetPlayer(i);
        if (v6 == nullptr || v6->sentient == nullptr
            || v6->client == nullptr)
        {
            continue;
        }
        float posX, posY, posZ;
        if (!mPlayers[i].showVehicleIcon
            || (v6->client->ps.eFlags & 0x100000) == 0)
        {
            if (v6->mDObj != nullptr)
            {
                DObjSkelMat mat;
                if (G_DObjGetWorldTagMatrix(v6, bip_head_hash, &mat) != 0)
                {
                    posX = mat.origin[0];
                    posY = mat.origin[1];
                    posZ = mat.origin[2] + distAbovePlayer;
                    goto draw_icon;
                }
            }
            posX = v6->r.currentOrigin.v.m128_f32[0];
            posY = v6->r.currentOrigin.v.m128_f32[1];
            posZ = v6->r.currentOrigin.v.m128_f32[2] + 72.0f;
        }
        else
        {
            Entity* owner = EntityHandleDb::sInst.GetObject(
                v6->r.mOwner.mHandle.mVal);
            if (owner == nullptr)
                continue;
            if (owner->scr_vehicle == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\IGOHeadIcons.cpp";
                AeAssert::gCurrentLine = 284;
                AeAssert::gCurrentExpr = "vehicle->scr_vehicle";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Players owner is not a vehicle"))
                    __debugbreak();
            }
            if (owner->mDObj != nullptr)
            {
                DObjSkelMat mat;
                if (G_DObjGetWorldTagMatrix(owner, turret_hash, &mat) != 0)
                {
                    posX = mat.origin[0];
                    posY = mat.origin[1];
                    posZ = mat.origin[2] + distAboveVehicle;
                    goto draw_icon;
                }
            }
            posX = owner->r.currentOrigin.v.m128_f32[0];
            posY = owner->r.currentOrigin.v.m128_f32[1];
            posZ = owner->r.currentOrigin.v.m128_f32[2] + 140.0f;
        }
    draw_icon:
        {
            math::Position3 in1;
            in1.v.m128_f32[0] = posX;
            in1.v.m128_f32[1] = posY;
            in1.v.m128_f32[2] = posZ;
            in1.v.m128_f32[3] = 0.0f;
            math::Position3 proj1;
            nglProjectPoint(&proj1, &in1, nglBuildScene);
            if (proj1.v.m128_f32[2] < 1.0f)
                continue;
            math::Position3 in2;
            in2.v.m128_f32[0] = posX;
            in2.v.m128_f32[1] = posY;
            in2.v.m128_f32[2] = posZ + iconHeight;
            in2.v.m128_f32[3] = 0.0f;
            math::Position3 proj2;
            nglProjectPoint(&proj2, &in2, nglBuildScene);
            if (proj2.v.m128_f32[2] < 1.0f)
                continue;
            float screenHeight =
                proj1.v.m128_f32[1] - proj2.v.m128_f32[1];
            if (minScreenSize > screenHeight)
                screenHeight = minScreenSize;
            int window = unk_F6A284[802 * mClient];
            float halfW = View::GetXScalingForHUD(window) * screenHeight
                          * 0.5f;
            float halfH = View::GetYScalingForHUD(window) * screenHeight
                          * 0.5f;
            PanelQuad* icon = mHeadIcons[mPlayers[i].index].icon;
            icon->SetZvalueAbs(proj1.v.m128_f32[2]);
            icon->SetPos(proj1.v.m128_f32[0] - halfW,
                         proj1.v.m128_f32[1] - halfH,
                         proj1.v.m128_f32[0] + halfW,
                         proj1.v.m128_f32[1] + halfH);
            color32 col;
            col.c.b = 255;
            col.c.g = 255;
            col.c.r = 255;
            col.c.a = (uint8_t)mPlayers[i].alpha;
            icon->SetColor(col);
            icon->Draw();
        }
    }
}

// ea: 0x0058AC90
void IGOHeadIcons::Update(float time_inc)
{
    (void)time_inc;
    if (!is_shown
        || !cgGlobal.teamGame
        || EntityManager::sInst->GetPlayer(mClient) == nullptr
        || EntityManager::sInst->GetPlayer(mClient)->sentient == nullptr)
    {
        return;
    }
    Entity* Player = EntityManager::sInst->GetPlayer(mClient);
    if (Player == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOHeadIcons.cpp";
        AeAssert::gCurrentLine = 80;
        AeAssert::gCurrentExpr = "localPlayer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Local player is not valid"))
            __debugbreak();
    }
    if (Player->client->ps.pm_type >= 6)
        return;
    int localTeam = Player->sentient->eTeam;
    for (int idx = 0; idx < 16; ++idx)
    {
        HeadIconsPlayer* p = &mPlayers[idx];
        p->show = false;
        bool showVehicleIcon = false;
        Entity* v5 = EntityManager::sInst->GetPlayer(idx);
        if (v5 == nullptr || v5 == Player)
            continue;
        sentient_s* sentient = v5->sentient;
        if (sentient == nullptr)
            continue;
        Client* client = v5->client;
        if (client->pers.connected != 2 /* CON_CONNECTED */)
            continue;
        int team = sentient->eTeam;
        int playerState = client->pers.playerState;
        int rank = client->pers.rank;
        float v22 = 1.0f;
        int minAlphaDist = 0;
        int maxAlphaDist = 0;
        if (playerState == 4)
        {
            if (*(int*)((char*)Player + 596 + 1904) != 3)
                continue;
            maxAlphaDist = mp_headIconReviveMaxAlphaDist.integer;
            minAlphaDist = mp_headIconReviveMinAlphaDist.integer;
            rank = 4;
            int respawnUntilTime = client->ps.respawnUntilTime;
            if (respawnUntilTime != 0)
            {
                int v12 = respawnUntilTime - level.time;
                if (v12 < 0)
                    continue;
                if (v12 < 5000)
                    v22 = v12 * 0.0002f;
            }
        }
        else
        {
            if (playerState != 3)
                continue;
            if ((client->ps.eFlags & 0x100000) != 0
                && client->ps.vehType == 2 && client->ps.vehPos == 0)
            {
                Entity* owner = EntityHandleDb::sInst.GetObject(
                    v5->r.mOwner.mHandle.mVal);
                if (owner == nullptr || owner->scr_vehicle == nullptr)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\IGOHeadIcons.cpp";
                    AeAssert::gCurrentLine = 168;
                    AeAssert::gCurrentExpr =
                        "*player->r.mOwner && player->r.mOwner->scr_vehicle";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(
                            "Invalid vehicle in IGOHeadIcons::Update"))
                        __debugbreak();
                }
                showVehicleIcon = true;
                if (team != localTeam && IsVehicleSpotted(owner))
                    rank = 7;
            }
            else if (team == localTeam
                     && MultiplayerMgr::sInst->IsPlayerTalking(v5))
            {
                rank = 3;
            }
        }
        if (team != localTeam)
        {
            if (rank <= 4)
                continue;
        }
        else
        {
            if (rank > 4)
                continue;
        }
        float v14 = mHeadIcons[rank].alpha * v22;
        float additional_alpha_scalar = v14;
        if (maxAlphaDist > 0)
        {
            float dx = Player->r.currentOrigin.v.m128_f32[0]
                       - v5->r.currentOrigin.v.m128_f32[0];
            float dy = Player->r.currentOrigin.v.m128_f32[1]
                       - v5->r.currentOrigin.v.m128_f32[1];
            float dz = Player->r.currentOrigin.v.m128_f32[2]
                       - v5->r.currentOrigin.v.m128_f32[2];
            float dist = sqrtf(dx * dx + dy * dy + dz * dz);
            if (dist <= maxAlphaDist)
            {
                if (dist > minAlphaDist)
                {
                    float a = mHeadIcons[rank].alpha;
                    additional_alpha_scalar =
                        a
                        - (((dist - minAlphaDist)
                            / (maxAlphaDist - minAlphaDist))
                           * a);
                }
                v14 = additional_alpha_scalar;
                p->show = true;
                p->showVehicleIcon = showVehicleIcon;
                p->alpha = v14;
                p->index = rank;
            }
        }
        else
        {
            p->show = true;
            p->showVehicleIcon = showVehicleIcon;
            p->alpha = v14;
            p->index = rank;
        }
    }
}

// ============================================================================
// IGOItemIcons (render-heavy Draw deferred to renderer batch)
// ============================================================================

const char* sItemIconNames[2] = {
    "i_ammo_box_w", "voip_line_01_icon",
};

// ea: 0x005691B0
IGOItemIcons::IGOItemIcons(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    memset(mItemIcons, 0, sizeof(mItemIcons));
}

// ea: 0x005691D0
IGOItemIcons::~IGOItemIcons()
{
}

// ea: 0x0059AA40
void IGOItemIcons::Init(PanelFile* panel)
{
    float u[4] = {0.0f, 1.0f, 0.0f, 1.0f};
    float v[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    for (int i = 0; i < 2; ++i)
    {
        PanelQuad* Pointer = panel->GetPointer(sItemIconNames[i]);
        mItemIcons[i].icon = Pointer;
        if (g_femanager.GetDefaultPQ() != Pointer)
        {
            Pointer->quadBlendModeType = 1691321856;
            mItemIcons[i].icon->SetSectionUV(0, u, v);
            mItemIcons[i].height =
                (uint8_t)mItemIcons[i].icon->GetWidth();
            mItemIcons[i].alpha =
                mItemIcons[i].icon->GetColor().c.a;
        }
    }
}

// ea: 0x005691E0
void IGOItemIcons::Update(float time_inc)
{
    (void)time_inc;
}

// ea: 0x005691F0 (private per-item draw helper)
void IGOItemIcons::Draw(Entity* pEnt, const math::Position3& playerPosition,
                        int iconIndex)
{
    if (pEnt->s.pos.trType != 0 /* TR_STATIONARY */)
        return;
    float fMinIconScreenSize = mItemIcons[iconIndex].alpha;
    int maxAlphaDist = 0;
    if (mp_itemIconMaxAlphaDist.integer > 0)
    {
        float dx = playerPosition.v.m128_f32[0]
                   - pEnt->r.currentOrigin.v.m128_f32[0];
        float dy = playerPosition.v.m128_f32[1]
                   - pEnt->r.currentOrigin.v.m128_f32[1];
        float dz = playerPosition.v.m128_f32[2]
                   - pEnt->r.currentOrigin.v.m128_f32[2];
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        if (dist > (float)mp_itemIconMaxAlphaDist.integer)
            return;
        if (dist > mp_itemIconMinAlphaDist.integer)
        {
            fMinIconScreenSize =
                mItemIcons[iconIndex].alpha
                - (((dist - mp_itemIconMinAlphaDist.integer)
                    / (mp_itemIconMaxAlphaDist.integer
                       - mp_itemIconMinAlphaDist.integer))
                   * mItemIcons[iconIndex].alpha);
        }
    }
    float itemX = pEnt->r.currentOrigin.v.m128_f32[0];
    float itemY = pEnt->r.currentOrigin.v.m128_f32[1];
    float itemZ = pEnt->r.currentOrigin.v.m128_f32[2]
                  + mp_itemIconDistAboveItem.integer;
    math::Position3 proj1;
    math::Position3 in1;
    in1.v.m128_f32[0] = itemX;
    in1.v.m128_f32[1] = itemY;
    in1.v.m128_f32[2] = itemZ;
    in1.v.m128_f32[3] = 0.0f;
    nglProjectPoint(&proj1, &in1, nglBuildScene);
    if (proj1.v.m128_f32[2] < 1.0f)
        return;
    math::Position3 proj2;
    math::Position3 in2;
    in2.v.m128_f32[0] = itemX;
    in2.v.m128_f32[1] = itemY;
    in2.v.m128_f32[2] = itemZ + mp_itemIconHeight.integer;
    in2.v.m128_f32[3] = 0.0f;
    nglProjectPoint(&proj2, &in2, nglBuildScene);
    if (proj2.v.m128_f32[2] < 1.0f)
        return;
    float screenHeight = proj1.v.m128_f32[1] - proj2.v.m128_f32[1];
    if (mp_itemIconMinScreenSize.integer > screenHeight)
        screenHeight = (float)mp_itemIconMinScreenSize.integer;
    int window = unk_F6A284[802 * mClient];
    float halfW = View::GetXScalingForHUD(window) * screenHeight * 0.5f;
    float halfH = View::GetYScalingForHUD(window) * screenHeight * 0.5f;
    PanelQuad* icon = mItemIcons[iconIndex].icon;
    icon->SetZvalueAbs(proj1.v.m128_f32[2]);
    icon->SetPos(proj1.v.m128_f32[0] - halfW,
                 proj1.v.m128_f32[1] - halfH,
                 proj1.v.m128_f32[0] + halfW,
                 proj1.v.m128_f32[1] + halfH);
    color32 col;
    col.c.b = 255;
    col.c.g = 255;
    col.c.r = 255;
    col.c.a = (uint8_t)fMinIconScreenSize;
    icon->SetColor(col);
    icon->Draw();
}

// ea: 0x00569530
void IGOItemIcons::Draw()
{
    if (!is_shown)
        return;
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOItemIcons.cpp";
        AeAssert::gCurrentLine = 150;
        AeAssert::gCurrentExpr = "localPlayer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Local player is not valid"))
            __debugbreak();
    }
    if (Player->client->ps.pm_type >= 6 || Player->sentient == nullptr
        || MultiplayerMgr::sInst->mPeer == nullptr)
    {
        return;
    }
    MpPlayerManagerView* playerManager =
        (MpPlayerManagerView*)MultiplayerMgr::sInst->mPeer->GetPlayerManager();
    for (int j = 0; j < 16; ++j)
    {
        MpPlayerView2* v4 = (MpPlayerView2*)playerManager->GetPlayer(
            (unsigned char)j);
        if (v4 == nullptr || !v4->IsValid())
            continue;
        for (int i = 0; i < 3; ++i)
        {
            Entity* Item =
                v4->mItems.FindItem(kItemTypeSupport, (short)i);
            if (Item != nullptr)
                Draw(Item, Player->r.currentOrigin, 0);
        }
        if (v4->mClientIndex >= 0)
        {
            Entity* v7 = EntityManager::sInst->GetPlayer(v4->mClientIndex);
            if (v7 != nullptr && v7->sentient != nullptr
                && v7->sentient->eTeam == Player->sentient->eTeam)
            {
                for (int k = 0; k < 3; ++k)
                {
                    Entity* v10 =
                        v4->mItems.FindItem(kItemTypeMines, (short)k);
                    if (v10 != nullptr && v10->think != 0x0C)
                        Draw(v10, Player->r.currentOrigin, 1);
                }
            }
        }
    }
}

// ea: 0x00583D90
void IGOItemIcons::UpdateWidescreen(bool widescreen, float about_x)
{
    for (int i = 0; i < 2; ++i)
        mItemIcons[i].icon->FattenMeForWidescreen(widescreen, about_x);
}

// ea: 0x00583DC0
void IGOItemIcons::UpdateSplitScreen(int viewport, int old_viewport)
{
    for (int i = 0; i < 2; ++i)
        mItemIcons[i].icon->FormatForSplitScreen(viewport, old_viewport);
}

// ============================================================================
// IGOVoipList
// ============================================================================

const char* sVoipIconNames[4] = {
    "voip_line_01_icon", "voip_line_03_icon", "voip_line_01_text",
    "voip_line_03_text",
};
const char* sVoipTextNames[4] = {
    "voip_line_01_text", "voip_line_03_text", nullptr, nullptr,
};

// Minimal MPPlayer view for the mTeam/mName fields (full layout in mp.o).
struct MpPlayerView {
    uint8_t _pad0[0x68];
    char    mName[32];   // +0x68
    uint8_t _pad2[0x25C - 0x88];
    int16_t mTeam;       // +0x25C
};

// ea: 0x0059C480
IGOVoipList::IGOVoipList(int client)
    : mListBox(4, 2, 4, true)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
}

// ea: 0x0059AB00
IGOVoipList::~IGOVoipList()
{
    mListBox.~UIListBox();
}

// ea: 0x0059AB50
void IGOVoipList::Init(PanelFile* panel)
{
    mListBox.SetColumnStateCount(1, 2);
    for (int i = 0; i < 4; ++i)
    {
        PanelQuad* Pointer = panel->GetPointer(sVoipIconNames[i]);
        FEText* TextPointer = panel->GetTextPointer(sVoipTextNames[i]);
        mListBox.SetItem(i, 0, TextPointer, 0);
        mListBox.SetItem(i, 1, Pointer, 1);
    }
}

// ea: 0x0058B680
void IGOVoipList::Update(float time_inc)
{
    if (MultiplayerMgr::sInst->mPeer == nullptr)
        return;
    MPPlayerManager* PlayerManager =
        MultiplayerMgr::sInst->mPeer->GetPlayerManager();
    MPPlayer* LocalPlayer = PlayerManager->GetLocalPlayer(mClient);
    int local_team = 2;
    if (LocalPlayer != nullptr)
        local_team = ((MpPlayerView*)LocalPlayer)->mTeam;
    int row = 0;
    for (int i = 0; i < 16 && row < 4; ++i)
    {
        MPPlayer* Player = PlayerManager->GetPlayer(i);
        if (Player != nullptr
            && !Player->IsLocalPlayer()
            && (!cgGlobal.teamGame
                || local_team == ((MpPlayerView*)Player)->mTeam)
            && MultiplayerMgr::sInst->mPeer->IsPlayerTalking(Player, 0))
        {
            mListBox.SetText(row, 0, ((MpPlayerView*)Player)->mName);
            mListBox.SetItemState(row, 1, 1);
            ++row;
        }
    }
    for (int i = row; i < 4; ++i)
        mListBox.ClearRow(i);
    mListBox.Update(time_inc);
}

// ea: 0x005696B0
void IGOVoipList::Draw()
{
    if (is_shown)
        mListBox.Draw();
}

// ea: 0x00579530
void IGOVoipList::UpdateWidescreen(bool widescreen, float about_x)
{
    (void)widescreen;
    (void)about_x;
}

// ea: 0x00579540
void IGOVoipList::UpdateSplitScreen(int viewport, int old_viewport)
{
    (void)viewport;
    (void)old_viewport;
}

// ============================================================================
// IGOHintWidget (Update deferred pieces use full extern set below)
// ============================================================================

// ea: 0x00568AF0
IGOHintWidget::IGOHintWidget(int client)
{
    mClient = client;
    force_appear = false;
    is_shown = true;
    memset(icons, 0, sizeof(icons));
    text = nullptr;
    dont_draw = false;
    wide_weapon = false;
    current_icon = -1;
    last_icon = -2;
}

// ea: 0x0059A2E0
void IGOHintWidget::Init(PanelFile* panel)
{
    if (strcmp(panel->mName, "hud_mp.panel") == 0)
    {
        icons[5] = panel->GetPointer("ai_driver");
        icons[6] = panel->GetPointer("ai_flag_pickup");
        icons[7] = panel->GetPointer("ai_gunner_position");
        icons[8] = panel->GetPointer("ai_mantel_tank");
        icons[9] = panel->GetPointer("ai_passenger");
        if (mpviewport > 0)
        {
            icons[5] = PanelQuad::Clone(icons[5]);
            icons[6] = PanelQuad::Clone(icons[6]);
            icons[7] = PanelQuad::Clone(icons[7]);
            icons[8] = PanelQuad::Clone(icons[8]);
            icons[9] = PanelQuad::Clone(icons[9]);
        }
        ++mpviewport;
    }
    else
    {
        icons[1] = panel->GetPointer("hint_usable");
        icons[0] = panel->GetPointer("hint_health");
        icons[2] = panel->GetPointer("hud_ammo2.tga");
        icons[3] = panel->GetPointer("hint_usable");
        icons[4] = panel->GetPointer("hint_usable_MG");
        FEText* TextPointer = panel->GetTextPointer("Press[Use]");
        if (text != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOHintWidget.cpp";
            AeAssert::gCurrentLine = 52;
            AeAssert::gCurrentExpr = "!text";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("no!"))
                __debugbreak();
        }
        text = (FEMultiLineText*)mem_heap_malloc(0xA8u);
        if (text != nullptr)
        {
            color32 col = TextPointer->GetColor();
            text = new (text) FEMultiLineText(
                TextPointer->GetFont(), TextPointer->GetY(), 0.0f, 0,
                (panel_layer)TextPointer->GetScaleX(), 0.0f, 0, (int)col.i,
                col);
        }
        text->SetNumLines(3);
        if (mClient > 0)
        {
            icons[1] = PanelQuad::Clone(icons[1]);
            icons[0] = PanelQuad::Clone(icons[0]);
            icons[2] = PanelQuad::Clone(icons[2]);
            icons[3] = PanelQuad::Clone(icons[3]);
            icons[4] = PanelQuad::Clone(icons[4]);
            text = (FEMultiLineText*)mem_heap_malloc(0xA8u);
            if (text != nullptr)
            {
                color32 col2 = TextPointer->GetColor();
                text = new (text) FEMultiLineText(
                    TextPointer->GetFont(), TextPointer->GetY(), 0.0f, 0,
                    (panel_layer)TextPointer->GetScaleX(), 0.0f, 0,
                    (int)col2.i, col2);
            }
            else
            {
                text = nullptr;
            }
            text->SetNumLines(3);
        }
    }
    memset(current_icon_nudge, 0, sizeof(current_icon_nudge));
}

// ea: 0x0059A5E0
void IGOHintWidget::SetWeaponsPQs(PanelFile* panel, PanelFile* panel2)
{
    if (icons[10] != nullptr)
        return;
    int i = 1;
    if (BG_GetNumWeapons() >= 1)
    {
        int iconIndex = 11;
        do
        {
            weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(i);
            if (InfoForWeapon->szHudIcon[0] != 0)
            {
                char tmp[128];
                int v6 = 0;
                while (v6 < 124)
                {
                    char v7 = InfoForWeapon->szHudIcon[v6];
                    if (v7 == 0)
                        break;
                    tmp[v6] = v7 == '$' ? '_' : v7;
                    ++v6;
                }
                tmp[v6] = 0;
                if (panel2 == nullptr
                    || (icons[iconIndex] = panel2->GetPointer(tmp),
                        icons[iconIndex] == g_femanager.default_pq))
                {
                    icons[iconIndex] = panel->GetPointer(tmp);
                    if (icons[iconIndex] == g_femanager.default_pq)
                    {
                        strcat(tmp, ".tga");
                        icons[iconIndex] = panel->GetPointer(tmp);
                        if (icons[iconIndex] == g_femanager.default_pq)
                        {
                            AeAssert::gCurrentAuthor = AeAssert::COD3;
                            AeAssert::gCurrentFile =
                                "c:\\cod\\code\\game\\IGOHintWidget.cpp";
                            AeAssert::gCurrentLine = 141;
                            AeAssert::gCurrentExpr = nullptr;
                            if (!AeAssert::IsIgnored()
                                && AeAssert::Warning(
                                    "could not find pickup hud icon named %s, "
                                    "using hand icon instead",
                                    tmp))
                                __debugbreak();
                            icons[iconIndex] = nullptr;
                        }
                    }
                }
            }
            ++i;
            ++iconIndex;
        } while (i <= BG_GetNumWeapons());
    }
    for (int j = 0; j < 138; ++j)
    {
        if (icons[j] != nullptr)
            icons[j]->SetColor(color32(-1));
    }
}

// ea: 0x00568B50
void IGOHintWidget::Draw()
{
    if (IsShown() && !dont_draw)
    {
        if ((unsigned int)current_icon <= 0x89)
            icons[current_icon]->Draw();
        text->Draw();
    }
}

// ea: 0x005792B0
void IGOHintWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    for (int i = 0; i < 138; ++i)
    {
        if (icons[i] != nullptr)
            icons[i]->FormatHUDForSplitScreen(viewport, old_viewport, 0,
                                              0.0f, 0.0f);
    }
    text->UpdateForSplitScreen(viewport, old_viewport);
}

// ea: 0x00583C70
void IGOHintWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    for (int i = 0; i < 138; ++i)
    {
        if (icons[i] != nullptr)
            icons[i]->FattenMeForWidescreen(widescreen, about_x);
    }
    if (widescreen)
        text->SetLineSpacing(20);
    else
        text->SetScale(1.1f);
}

// ea: 0x00578AD0
void IGOHintWidget::Update(float time_inc)
{
    (void)time_inc;
    if (!IsShown())
        return;
    int integer = cg_cursorHints.integer;
    int v6 = 1580 * mClient;
    if (integer == 0 || (v6 = 1580 * mClient, dword_F62960[v6] == 0))
    {
        dont_draw = true;
        return;
    }
    int cgBase = dword_F62960[v6];
    if (dword_F6355C[v6] == 0 && *(int*)(cgBase + 1208) != 0)
    {
        dword_F63F60[v6] = cgGlobal.time;
        dword_F63F64[1580 * mClient] = cg_hintFadeTime.integer;
        dword_F63F5C[1580 * mClient] =
            *(int*)(dword_F62960[1580 * mClient] + 1208);
        dword_F63F68[1580 * mClient] =
            *(int*)(dword_F62960[1580 * mClient] + 1212);
        dword_F63F6C[1580 * mClient] =
            *(int*)(dword_F62960[1580 * mClient] + 1216);
    }
    int v8 = 1580 * mClient;
    int v9 = dword_F63F5C[v8];
    if (v9 <= 1)
    {
        dont_draw = true;
        return;
    }
    int v10 = dword_F63F60[v8];
    float v11 = (float)(v10 + dword_F63F64[v8] - cgGlobal.time);
    if (v11 <= 0.0f)
    {
        dword_F63F5C[v8] = 0;
        dont_draw = true;
        return;
    }
    float str = 1.0f;
    if (integer == 2)
        str = (float)(v10 % 1000) * 0.01f;
    else if (integer < 2)
        str = (sinf(cgGlobal.time * 0.0066666668f) + 1.0f) * 5.0f;
    float alpha = 1.0f;
    if (v11 < 100.0f)
        alpha = v11 * 0.01f;
    int last_icon = this->last_icon;
    if (v9 == last_icon)
    {
        icons[last_icon]->SetAlpha(alpha);
        float v14 = str;
        if (wide_weapon)
            v14 = str * 2.0f;
        icons[last_icon]->ScaleAbsoluteCenter(v14, v14);
        text->SetAlpha(alpha);
        return;
    }
    char new_text[256];
    new_text[0] = 0;
    const char* activate_key = g_femanager.IGO->activate_key;
    wide_weapon = false;
    int v18 = dword_F63F5C[v8];
    if (v18 >= 17 && v18 <= 144)
    {
        int weapon = v18 - 16;
        current_icon = v18 - 16 + 10;
        if (icons[current_icon] == nullptr)
            current_icon = 1;
        weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(weapon);
        if (WeaponSlot(InfoForWeapon) == 2)
        {
            if (BG_GetInfoForWeapon(
                    GetPlayerState(currCl).weaponslots[3])->bDoNotDrop
                != 0)
            {
                dont_draw = true;
                return;
            }
        }
        if (InfoForWeapon->bWideListIcon != 0)
            wide_weapon = true;
        const char* display = cg_weapons[weapon].pszTranslatedDisplayName;
        const char* other =
            cg_weapons[GetPlayerState(currCl).weaponslots[2]]
                .pszTranslatedDisplayName;
        const char* fmt = STBManager::sInst->GetSTBString(
            "INGAME_SWAP_WEAPONS_PS2MP");
        sprintf(new_text, fmt, other, display);
        goto label_81;
    }
    if (v18 >= 145 && v18 <= 272)
        goto label_34;
    if (v18 >= 273 && v18 <= 279)
    {
        current_icon = 3;
        static const char* kitNames[7] = {
            "MPGAME_ASSAULT", "MPGAME_INFANTRY", "MPGAME_RIFLEMAN",
            "MPGAME_MEDIC",   "MPGAME_SUPPORT",  "MPGAME_ANTIARMOR",
            "MPGAME_SCOUT",
        };
        const char* kit =
            STBManager::sInst->GetSTBString(kitNames[v18 - 273]);
        const char* fmt =
            STBManager::sInst->GetSTBString("MPGAME_PICKUP_KIT");
        sprintf(new_text, fmt, kit);
        goto label_81;
    }
    if (v18 >= 3 && v18 <= 6)
    {
        switch (v18)
        {
        case 3: current_icon = 9; break;
        case 4: current_icon = 5; break;
        case 5: current_icon = 7; break;
        case 6: current_icon = 8; break;
        }
        const char* v29 =
            CG_ConfigString(dword_F63F6C[v8] + 628);
        const char* v30 =
            STBManager::sInst->GetSTBString(v29);
        const char* LMGKey = activate_key;
        if (v30 == nullptr)
            v30 = v29;
        sprintf(new_text, v30, LMGKey);
        goto label_81;
    }
    if (v18 == 12)
    {
        current_icon = 4;
        int Key = KeyInfo::GetKey("+speed", currCl);
        if (Key == -1)
            Key = KeyInfo::GetKey("toggle cl_run", currCl);
        const char* LMGKey;
        const char* v30;
        switch (Key)
        {
        case 215:
        case 216:
            LMGKey = g_femanager.IGO->GetLMGKey();
            v30 = STBManager::sInst->GetSTBString(
                "INGAME_XBOX_LMG_MOUNTPOINT_PULL");
            break;
        case 219:
            LMGKey = g_femanager.IGO->GetLMGKey();
            v30 = STBManager::sInst->GetSTBString(
                "INGAME_XBOX_LMG_MOUNTPOINT_CLICK_R");
            break;
        case 220:
            LMGKey = g_femanager.IGO->GetLMGKey();
            v30 = STBManager::sInst->GetSTBString(
                "INGAME_XBOX_LMG_MOUNTPOINT_CLICK_L");
            break;
        default:
            LMGKey = g_femanager.IGO->GetLMGKey();
            v30 = STBManager::sInst->GetSTBString(
                "INGAME_PS2_LMG_MOUNTPOINT");
            break;
        }
        sprintf(new_text, v30, LMGKey);
        goto label_81;
    }
    if (dword_F63F6C[v8] == -1)
    {
        if (v18 == 13)
        {
            const char* LMGKey = activate_key;
            current_icon = 0;
            const char* v30 = STBManager::sInst->GetSTBString(
                "INGAME_HEALTH_PICKUP");
            sprintf(new_text, v30, LMGKey);
            goto label_81;
        }
        if (v18 == 2)
            current_icon = 1;
        goto label_81;
    }
    current_icon = 1;
    {
        const char* v33 =
            STBManager::sInst->GetSTBString((unsigned int)dword_F63F6C[v8]);
        if (v33 != nullptr)
        {
            sprintf(new_text, "%s", v33);
            goto label_81;
        }
        const char* v34 = CG_ConfigString(dword_F63F6C[1580 * mClient] + 628);
        const char* str2 = v34;
        if (v34 != nullptr && v34[0] != 0)
        {
            if (strcmp(v34, "Press [USE] to plant charge.") == 0)
            {
                const char* v35 = STBManager::sInst->GetSTBString(
                    "GELA_PLANT_CHARGES");
                if (v35 != nullptr)
                {
                    strcpy(new_text, v35);
                    goto label_81;
                }
            }
            const char* v30 = STBManager::sInst->GetSTBString(str2);
            const char* LMGKey = activate_key;
            if (v30 == nullptr)
                v30 = str2;
            sprintf(new_text, v30, LMGKey);
            goto label_81;
        }
        new_text[0] = 0;
        goto label_81;
    }
label_34:
    current_icon = -1;
label_81:
    text->SetTextBox(new_text, 450, -1082130432);
    bool oneLine = text->GetLineNum() <= 1;
    int ci = current_icon;
    if (oneLine)
    {
        if ((unsigned int)ci <= 0x89 && current_icon_nudge[ci] == 1)
        {
            icons[ci]->SetCenterPos(icons[ci]->GetCenterX(),
                                    icons[ci]->GetCenterY() - 10.0f);
            current_icon_nudge[ci] = 0;
        }
    }
    else if ((unsigned int)ci <= 0x89 && current_icon_nudge[ci] == 0)
    {
        icons[ci]->SetCenterPos(icons[ci]->GetCenterX(),
                                icons[ci]->GetCenterY() + 10.0f);
        current_icon_nudge[ci] = 1;
    }
    if (ci >= 0)
        icons[ci]->SetAlpha(alpha);
    text->SetAlpha(alpha);
    dont_draw = false;
}

// ============================================================================
// IGOJeepMapWidget
// ============================================================================

const float IGOJeepMapWidget::mapTopRight[3][2] = {
    {32767.0f, -16264.0f},
    {25952.0f, -28320.0f},
    {23632.0f, -20288.0f},
};
const float IGOJeepMapWidget::mapWideHeight[3][2] = {
    {49072.0f, 49072.0f},
    {49072.0f, 49072.0f},
    {49072.0f, 49072.0f},
};
const char* IGOJeepMapWidget::mLevelMapName[2] = {
    "SP_nightDrop_DriveMap",
    "SP_fuelPlant_DriveMap",
};

// ea: 0x005689B0
IGOJeepMapWidget::IGOJeepMapWidget(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    icon = nullptr;
    mTextureSetted = false;
}

// ea: 0x0059A250
void IGOJeepMapWidget::Init(PanelFile* panel)
{
    u[0] = 0.0f;
    u[1] = 0.0f;
    v[0] = 0.0f;
    v[2] = 0.0f;
    u[2] = 1.0f;
    u[3] = 1.0f;
    v[1] = 1.0f;
    v[3] = 1.0f;
    hudRange = 0.1f;
    icon = panel->GetPointer("SP_jeepmap");
    icon->SetSectionUV(0, u, v);
    icon->SetZvalueAbs(icon->GetZvalue() + 50.0f);
}

// ea: 0x005689E0
void IGOJeepMapWidget::SetLevelMap(int levelIndex, TPakId pakId)
{
    if (levelIndex <= 2)
    {
        mapSizeX[0] =
            mapTopRight[levelIndex][0] - mapWideHeight[levelIndex][0];
        mapSizeX[1] = mapTopRight[levelIndex][0];
        mapSizeX[2] = mapWideHeight[levelIndex][0];
        static const float sDF38C8[6] = {
            -16264.0f, 25952.0f, -28320.0f, 23632.0f, -20288.0f, 49072.0f,
        };
        static const float sDF38E0[6] = {
            49072.0f, 49072.0f, 49072.0f, 49072.0f, 49072.0f, 0.0f,
        };
        mapSizeY[0] = sDF38C8[2 * levelIndex];
        mapSizeY[1] =
            sDF38E0[2 * levelIndex] + sDF38C8[2 * levelIndex];
        mapSizeY[2] = sDF38E0[2 * levelIndex];
        tlFixedString name(mLevelMapName[levelIndex]);
        icon->SetTexture(cdGetTexture(pakId, name));
    }
}

// ea: 0x00568A90
void IGOJeepMapWidget::WithinMap(float x, float y, float& scaleX,
                                 float& scaleY)
{
    scaleX = (mapSizeX[1] - x) / mapSizeX[2];
    scaleY = (mapSizeY[1] - y) / mapSizeY[2];
}

// ea: 0x00578A90
void IGOJeepMapWidget::Update(float time_inc)
{
    (void)time_inc;
    if (!is_shown || icon == nullptr)
        return;
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player == nullptr || Player->client == nullptr)
        return;
    float v4 = (mapSizeX[1] - Player->r.currentOrigin.v.m128_f32[1])
               / mapSizeX[2];
    float v5 = (mapSizeY[1] - Player->r.currentOrigin.v.m128_f32[0])
               / mapSizeY[2];
    float v6 = v4 - hudRange;
    if (v6 >= 0.0f)
    {
        if (v6 > 1.0f)
            v6 = 1.0f;
    }
    else
    {
        v6 = 0.0f;
    }
    u[0] = v6;
    u[1] = v6;
    float v7 = hudRange + v4;
    if (v7 >= 0.0f)
    {
        if (v7 > 1.0f)
            v7 = 1.0f;
    }
    else
    {
        v7 = 0.0f;
    }
    float v8 = v5 - hudRange;
    u[2] = v7;
    u[3] = v7;
    float v9;
    if (v8 >= 0.0f)
    {
        v9 = 1.0f;
        if (v8 <= 1.0f)
            v9 = v8;
    }
    else
    {
        v9 = 0.0f;
    }
    float v10 = hudRange + v5;
    v[0] = v9;
    if (v10 >= 0.0f)
    {
        if (v10 > 1.0f)
            v10 = 1.0f;
    }
    else
    {
        v10 = 0.0f;
    }
    v[1] = v10;
    v[2] = v9;
    v[3] = v10;
    icon->SetSectionUV(0, u, v);
}

// ea: 0x00568AD0
void IGOJeepMapWidget::Draw()
{
    if (is_shown && icon != nullptr)
        icon->Draw();
}

// ea: 0x00583C40
void IGOJeepMapWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    if (icon != nullptr)
    {
        icon->FattenMeForWidescreen(widescreen, about_x);
        icon->FattenMeForWidescreen(widescreen, about_x);
    }
}

// ============================================================================
// IGOWarStatusWidget
// ============================================================================

// ea: 0x005679F0
IGOWarStatusWidget::IGOWarStatusWidget(int client)
{
    mClient = client;
    is_shown = true;
    force_appear = false;
    for (int i = 0; i < 5; ++i)
    {
        m_pObjectiveFrameUS.m_elements[i] = nullptr;
        m_pObjectiveFrameGerman.m_elements[i] = nullptr;
        m_pObjectiveGerman.m_elements[i] = nullptr;
        m_pObjectiveUS.m_elements[i] = nullptr;
        m_pIconGerman.m_elements[i] = nullptr;
        m_pIconUS.m_elements[i] = nullptr;
    }
    iconWidth = 0.0f;
    iconHeight = 0.0f;
    neutralWidth = 0.0f;
    neutralHeight = 0.0f;
    centerX = 0.0f;
    centerY = 0.0f;
    zoomPct = 0.0f;
    lastFlag = 0;
}

// ea: 0x00577B90
IGOWarStatusWidget::~IGOWarStatusWidget()
{
    for (int i = 0; i < 5; ++i)
    {
        if (m_pObjectiveFrameUS.m_elements[i] != nullptr)
            delete m_pObjectiveFrameUS.m_elements[i];
        if (m_pObjectiveFrameGerman.m_elements[i] != nullptr)
            delete m_pObjectiveFrameGerman.m_elements[i];
        if (m_pObjectiveGerman.m_elements[i] != nullptr)
            delete m_pObjectiveGerman.m_elements[i];
        if (m_pObjectiveUS.m_elements[i] != nullptr)
            delete m_pObjectiveUS.m_elements[i];
        if (m_pIconGerman.m_elements[i] != nullptr)
            delete m_pIconGerman.m_elements[i];
        if (m_pIconUS.m_elements[i] != nullptr)
            delete m_pIconUS.m_elements[i];
    }
}

// ea: 0x00598E70
void IGOWarStatusWidget::Init(PanelFile* panel)
{
    char szGeometry[32];
    for (int i = 0; i < 5; ++i)
    {
        snprintf(szGeometry, 30, "war_icon_objective_frame_%02d", i + 1);
        m_pObjectiveFrameUS.m_elements[i] =
            panel->GetPointer(szGeometry);
        if (m_pObjectiveFrameUS.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOWidget.cpp";
            AeAssert::gCurrentLine = 2531;
            AeAssert::gCurrentExpr = "m_pObjectiveFrameUS[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("m_pObjectiveFrameUS[i] not valid"))
                __debugbreak();
        }
        snprintf(szGeometry, 30, "war_icon_obj_frame_grm_%02d", i + 1);
        m_pObjectiveFrameGerman.m_elements[i] =
            panel->GetPointer(szGeometry);
        if (m_pObjectiveFrameGerman.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOWidget.cpp";
            AeAssert::gCurrentLine = 2536;
            AeAssert::gCurrentExpr = "m_pObjectiveFrameGerman[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                    "m_pObjectiveFrameGerman[i] not valid"))
                __debugbreak();
        }
        snprintf(szGeometry, 30, "war_icon_objective_german_%02d", i + 1);
        m_pObjectiveGerman.m_elements[i] = panel->GetPointer(szGeometry);
        if (m_pObjectiveGerman.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOWidget.cpp";
            AeAssert::gCurrentLine = 2541;
            AeAssert::gCurrentExpr = "m_pObjectiveGerman[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("m_pObjectiveGerman[i] not valid"))
                __debugbreak();
        }
        snprintf(szGeometry, 30, "war_icon_objective_us_%02d", i + 1);
        m_pObjectiveUS.m_elements[i] = panel->GetPointer(szGeometry);
        if (m_pObjectiveUS.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOWidget.cpp";
            AeAssert::gCurrentLine = 2546;
            AeAssert::gCurrentExpr = "m_pObjectiveUS[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("m_pObjectiveUS[i] not valid"))
                __debugbreak();
        }
        snprintf(szGeometry, 30, "war_icon_german_%02d", i + 1);
        m_pIconGerman.m_elements[i] = panel->GetPointer(szGeometry);
        if (m_pIconGerman.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOWidget.cpp";
            AeAssert::gCurrentLine = 2551;
            AeAssert::gCurrentExpr = "m_pIconGerman[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("m_pIconGerman[i] not valid"))
                __debugbreak();
        }
        snprintf(szGeometry, 30, "war_icon_us_%02d", i + 1);
        m_pIconUS.m_elements[i] = panel->GetPointer(szGeometry);
        if (m_pIconUS.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOWidget.cpp";
            AeAssert::gCurrentLine = 2556;
            AeAssert::gCurrentExpr = "m_pIconUS[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("m_pIconUS[i] not valid"))
                __debugbreak();
        }
        m_pObjectiveFrameUS.m_elements[i]->SetShown(true);
        m_pObjectiveFrameGerman.m_elements[i]->SetShown(true);
        m_pObjectiveGerman.m_elements[i]->SetShown(true);
        m_pObjectiveUS.m_elements[i]->SetShown(true);
        m_pIconGerman.m_elements[i]->SetShown(true);
        m_pIconUS.m_elements[i]->SetShown(true);
    }
    if (mClient > 0)
    {
        for (int i = 0; i < 5; ++i)
        {
            m_pObjectiveFrameUS.m_elements[i] =
                PanelQuad::Clone(m_pObjectiveFrameUS.m_elements[i]);
            m_pObjectiveFrameGerman.m_elements[i] =
                PanelQuad::Clone(m_pObjectiveFrameGerman.m_elements[i]);
            m_pObjectiveGerman.m_elements[i] =
                PanelQuad::Clone(m_pObjectiveGerman.m_elements[i]);
            m_pObjectiveUS.m_elements[i] =
                PanelQuad::Clone(m_pObjectiveUS.m_elements[i]);
            m_pIconGerman.m_elements[i] =
                PanelQuad::Clone(m_pIconGerman.m_elements[i]);
            m_pIconUS.m_elements[i] =
                PanelQuad::Clone(m_pIconUS.m_elements[i]);
        }
    }
}

// ea: 0x00567A90
void IGOWarStatusWidget::Update(float time_inc)
{
    (void)time_inc;
}

// ea: 0x005834F0
void IGOWarStatusWidget::DrawFlag(int iFlag, int iIndexAdjustedFlag,
                                  int iContestedFlag, int numFlags,
                                  int myTeam, int notMyTeam,
                                  float capturePct)
{
    (void)numFlags;
    (void)notMyTeam;
    PanelQuad* quadContestedFlagFrame;
    if (iFlag == iContestedFlag
        && (gpBrocAPI->mBrocExports.mCallbackGetFlagBreatherTime() == 0
            || cgGlobal.time % 1000 > 500))
    {
        float fRenderCapturedFlagTransitionPercentage =
            (capturePct * 0.625f) + 0.2f;
        int v10 = gpBrocAPI->mBrocExports.mCallbackGetTeamCapturingFlag();
        if (myTeam == 1)
            quadContestedFlagFrame =
                m_pObjectiveFrameGerman.m_elements[iIndexAdjustedFlag];
        else
            quadContestedFlagFrame =
                m_pObjectiveFrameUS.m_elements[iIndexAdjustedFlag];
        PanelQuad* v12;
        if (v10 == -1)
        {
            v12 = m_pObjectiveGerman.m_elements[iIndexAdjustedFlag];
        }
        else
        {
            if (v10 != 1)
                goto label_13;
            v12 = m_pObjectiveUS.m_elements[iIndexAdjustedFlag];
        }
        if (v12 != nullptr)
        {
            quadContestedFlagFrame->Mask(
                1.0f - fRenderCapturedFlagTransitionPercentage,
                BOTTOM_MASK, 1.0f);
            v12->Mask(fRenderCapturedFlagTransitionPercentage, TOP_MASK,
                      1.0f);
            v12->Draw();
        label_14:
            quadContestedFlagFrame->Draw();
            goto label_15;
        }
    label_13:
        quadContestedFlagFrame->Mask(1.0f, BOTTOM_MASK, 1.0f);
        goto label_14;
    }
label_15:
    {
        int v13 = gpBrocAPI->mBrocExports.mCallbackGetTeamControllingFlag(
            iFlag);
        PanelQuad* icon;
        if (v13 == -1)
        {
            icon = m_pIconGerman.m_elements[iIndexAdjustedFlag];
        }
        else
        {
            if (v13 != 1)
                return;
            icon = m_pIconUS.m_elements[iIndexAdjustedFlag];
        }
        if (icon != nullptr)
            icon->Draw();
    }
}

// ea: 0x00588C90
void IGOWarStatusWidget::Draw()
{
    if (is_shown && MPUIInterface::mServerParams.mGameType == 0)
    {
        if (gpBrocAPI->mBrocExports.mCallbackGetFlagCount != nullptr
            && gpBrocAPI->mBrocExports.mCallbackGetTeamControllingFlag
                   != nullptr
            && gpBrocAPI->mBrocExports.mCallbackGetFlagBeingCaptured
                   != nullptr
            && gpBrocAPI->mBrocExports.mCallbackGetTeamCapturingFlag
                   != nullptr
            && gpBrocAPI->mBrocExports.mCallbackGetFlagBreatherTime
                   != nullptr
            && gpBrocAPI->mBrocExports.mCallbackGetCapturingFlagPercent
                   != nullptr)
        {
            Entity* Player = GetPlayer(currCl);
            if (Player->client->pers.playerState == 3)
            {
                sentient_s* sentient = Player->sentient;
                int myTeam = 0;
                int notMyTeam = 0;
                if (sentient != nullptr)
                {
                    if (sentient->eTeam == TEAM_AXIS)
                    {
                        myTeam = 1;
                        notMyTeam = 2;
                    }
                    else
                    {
                        myTeam = 2;
                        notMyTeam = 1;
                    }
                }
                int numFlags =
                    gpBrocAPI->mBrocExports.mCallbackGetFlagCount();
                if (numFlags != 0)
                {
                    int iContestedFlag =
                        gpBrocAPI->mBrocExports
                            .mCallbackGetFlagBeingCaptured();
                    float v6 = (float)gpBrocAPI->mBrocExports
                                   .mCallbackGetCapturingFlagPercent();
                    int v7 = (5 - numFlags) / 2;
                    float capturePct = fabsf(v6 * 0.01f);
                    if (numFlags > 0)
                    {
                        int i = 4 - v7;
                        int flag = 0;
                        for (;;)
                        {
                            int idx = myTeam == 1 ? i : flag + v7;
                            DrawFlag(flag++, idx, iContestedFlag, numFlags,
                                     myTeam, notMyTeam, capturePct);
                            --i;
                            if (flag >= numFlags)
                                break;
                            v7 = (5 - numFlags) / 2;
                        }
                    }
                }
            }
        }
    }
}

// ea: 0x00583640
void IGOWarStatusWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    for (int i = 0; i < 5; ++i)
    {
        m_pObjectiveFrameUS.m_elements[i]->FattenMeForWidescreen(
            widescreen, about_x);
        m_pObjectiveFrameGerman.m_elements[i]->FattenMeForWidescreen(
            widescreen, about_x);
        m_pObjectiveGerman.m_elements[i]->FattenMeForWidescreen(
            widescreen, about_x);
        m_pObjectiveUS.m_elements[i]->FattenMeForWidescreen(widescreen,
                                                            about_x);
        m_pIconGerman.m_elements[i]->FattenMeForWidescreen(widescreen,
                                                           about_x);
        m_pIconUS.m_elements[i]->FattenMeForWidescreen(widescreen,
                                                       about_x);
    }
}

// ea: 0x00577FC0
void IGOWarStatusWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    for (int i = 0; i < 5; ++i)
    {
        m_pObjectiveFrameUS.m_elements[i]->FormatForSplitScreen(viewport,
                                                                old_viewport);
        m_pObjectiveFrameGerman.m_elements[i]->FormatForSplitScreen(
            viewport, old_viewport);
        m_pObjectiveGerman.m_elements[i]->FormatForSplitScreen(
            viewport, old_viewport);
        m_pObjectiveUS.m_elements[i]->FormatForSplitScreen(viewport,
                                                           old_viewport);
        m_pIconGerman.m_elements[i]->FormatForSplitScreen(viewport,
                                                          old_viewport);
        m_pIconUS.m_elements[i]->FormatForSplitScreen(viewport,
                                                      old_viewport);
    }
}

// ============================================================================
// IGOGrenadeCookWidget
// ============================================================================

// ea: 0x00566C50
IGOGrenadeCookWidget::IGOGrenadeCookWidget(int client)
{
    is_shown = true;
    force_appear = false;
    mClient = client;
    dont_draw = true;
    grenadeTime[0] = nullptr;
    grenadeTime[1] = nullptr;
    grenadeTime[2] = nullptr;
    grenadeTime[3] = nullptr;
    grenadeTime[4] = nullptr;
    grenadeTime[5] = nullptr;
    grenadeRing = nullptr;
    fuseRemaining = -1.0f;
    fuseTotal = -1.0f;
    crossHair = gSaveGameData[LocalClient::ClientToPort(client)]
                    .mStubData.mCrosshair;
}

// ea: 0x00566CE0
IGOGrenadeCookWidget::~IGOGrenadeCookWidget()
{
    for (int i = 0; i < 6; ++i)
    {
        if (grenadeTime[i] != nullptr)
            delete grenadeTime[i];
    }
    if (grenadeRing != nullptr)
        delete grenadeRing;
}

// ea: 0x00598D70
void IGOGrenadeCookWidget::Init(PanelFile* panel)
{
    grenadeTime[0] = panel->GetPointer("GrenadeCook1");
    grenadeTime[1] = panel->GetPointer("GrenadeCook2");
    grenadeTime[2] = panel->GetPointer("GrenadeCook3");
    grenadeTime[3] = panel->GetPointer("GrenadeCook4");
    grenadeTime[4] = panel->GetPointer("GrenadeCook5");
    grenadeTime[5] = panel->GetPointer("GrenadeCook6");
    grenadeRing = panel->GetPointer("GrenadeCookring");
    if (mClient > 0)
    {
        grenadeTime[0] = PanelQuad::Clone(grenadeTime[0]);
        grenadeTime[1] = PanelQuad::Clone(grenadeTime[1]);
        grenadeTime[2] = PanelQuad::Clone(grenadeTime[2]);
        grenadeTime[3] = PanelQuad::Clone(grenadeTime[3]);
        grenadeTime[4] = PanelQuad::Clone(grenadeTime[4]);
        grenadeTime[5] = PanelQuad::Clone(grenadeTime[5]);
        grenadeRing = PanelQuad::Clone(grenadeRing);
    }
    float ringX = grenadeRing->GetCenterX();
    float ringY = grenadeRing->GetCenterY();
    int window = unk_F6A284[802 * mClient];
    float x = View::GetCurrentXPos(320.0f, window) - ringX;
    float y = View::GetCurrentYPos(240.0f, window) - ringY;
    for (int i = 0; i < 6; ++i)
    {
        grenadeTime[i]->SetCenterPos(grenadeTime[i]->GetCenterX() + x,
                                     grenadeTime[i]->GetCenterY() + y);
        grenadeTime[i]->SetAlpha(0.6f);
    }
    grenadeRing->SetCenterPos(grenadeRing->GetCenterX() + x,
                              grenadeRing->GetCenterY() + y);
    grenadeRing->SetAlpha(0.6f);
}

// ea: 0x00566D50
void IGOGrenadeCookWidget::SetFuse(float total, float remain)
{
    fuseRemaining = remain;
    fuseTotal = total;
}

// ea: 0x00566D70
void IGOGrenadeCookWidget::Update(float time_inc)
{
    (void)time_inc;
    if (!is_shown)
        return;
    if (fuseRemaining < 0.0f || fuseTotal < 0.0f)
        goto label_24;
    if (EntityManager::sInst->GetPlayer(currCl) == nullptr
        || EntityManager::sInst->GetPlayer(currCl)->client == nullptr)
    {
        return;
    }
    if (((GetPlayerState(currCl).eFlags & 0x100000) != 0
         && !BG_AllowPlayerWeaponAtVehiclePos(
             GetPlayerState(currCl).vehType, GetPlayerState(currCl).vehPos))
        || dword_F62960[1580 * currCl] == 0)
    {
    label_24:
        dont_draw = true;
        return;
    }
    for (int i = 0; i < 6; ++i)
        grenadeTime[i]->SetVisibility(1.0f);
    float percentLeft = fuseRemaining / fuseTotal;
    for (int i = 0; i < 6; ++i)
        grenadeTime[i]->SetColor(color32(1694433280));
    if (percentLeft < 0.83f)
        grenadeTime[0]->SetVisibility(0.0f);
    if (percentLeft < 0.67f)
        grenadeTime[1]->SetVisibility(0.0f);
    if (percentLeft < 0.5f)
        grenadeTime[2]->SetVisibility(0.0f);
    if (percentLeft < 0.33f)
        grenadeTime[3]->SetVisibility(0.0f);
    if (percentLeft < 0.17f)
        grenadeTime[4]->SetVisibility(0.0f);
    dont_draw = false;
}

// ea: 0x00566F50
void IGOGrenadeCookWidget::Draw()
{
    if (is_shown && !dont_draw)
    {
        for (int i = 0; i < 6; ++i)
            grenadeTime[i]->Draw();
        grenadeRing->Draw();
    }
}

// ea: 0x00582DF0
void IGOGrenadeCookWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    for (int i = 0; i < 6; ++i)
        grenadeTime[i]->FattenMeForWidescreen(widescreen, about_x);
    grenadeRing->FattenMeForWidescreen(widescreen, about_x);
}

// ea: 0x00582E40
void IGOGrenadeCookWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    for (int i = 0; i < 6; ++i)
        grenadeTime[i]->FormatForSplitScreen(viewport, old_viewport);
    grenadeRing->FormatForSplitScreen(viewport, old_viewport);
}

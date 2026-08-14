// ============================================================================
// igo_widgets.cpp - IGO widget classes (shell.o IGOWidget.cpp family)
// IGOHealthWidget / IGOTankHealthWidget / IGOVoteWidget / IGORowboatWidget
// ============================================================================

#include "game/shell/shell_types.h"
#include "game/client_types.h"
#include "game/player_types.h"

extern void* mem_heap_malloc(unsigned int size);  // core.o
extern int currCl;                                // ?currCl@@3HA @ 0xF1579C
extern DbLinkedHandle<EntityHandleDb, Entity> GetPlayersTank();
extern FEManager g_femanager;

// Binary cgGlobal_t starts with frametime at +0x00 (cg.o @ 0xF5FE30).
struct cgGlobal_t {
    int frametime;
    int time;       // +0x04
    int oldTime;    // +0x08
};
extern cgGlobal_t cgGlobal;

extern PlayerState& GetPlayerState(int idx);       // ?GetPlayerState@@YAAAVPlayerState@@H@Z
extern vmCvar_t g_stanceFadeTime;   // ?g_stanceFadeTime@@3UvmCvar_t@@A @ 0xEAC288
extern vmCvar_t g_stanceSolidTime;  // ?g_stanceSolidTime@@3UvmCvar_t@@A @ 0xEAE1C8
extern int unk_F6A284[];            // @ 0xF6A284 (per-client viewport block)

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

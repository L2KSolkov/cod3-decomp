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

// Binary cgGlobal_t starts with frametime at +0x00 (cg.o @ 0xF5FE30).
struct cgGlobal_t {
    int frametime;
};
extern cgGlobal_t cgGlobal;

// game.o / core.o externs (link /FORCE-tolerated until those objects land)
class InteractionController {
public:
    static InteractionController* Inst(int instance);  // ?Inst@InteractionController@@SAPAV1@H@Z
    void SetRenderText(const char* text, int x, int y, float scale,
                       float alpha,
                       int index);  // ?SetRenderText@InteractionController@@QAEXPBDHHMMH@Z
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

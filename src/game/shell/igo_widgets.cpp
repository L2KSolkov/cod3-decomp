// ============================================================================
// igo_widgets.cpp - IGO widget classes (shell.o IGOWidget.cpp family)
// IGOHealthWidget / IGOTankHealthWidget / IGOVoteWidget / IGORowboatWidget
// ============================================================================

#include "game/shell/shell_types.h"
#include "game/cg/cg_global.h"
#include "game/client_types.h"
#include "game/player_types.h"
#include "game/actor_types.h"
#include "game/logic/g_weaponfuncs.h"
#include "core/tlFixedString.h"

extern void* mem_heap_malloc(unsigned int size);  // core.o
extern int currCl;                                // ?currCl@@3HA @ 0xF1579C
extern int cg_aWeaponSelectTime[4];               // ?cg_aWeaponSelectTime@@3PAHA
extern DbLinkedHandle<EntityHandleDb, Entity> GetPlayersTank();
extern FEManager g_femanager;
extern int dword_F62960[];                        // @ 0xF62960 (cg client base)

extern PlayerState& GetPlayerState(int idx);       // ?GetPlayerState@@YAAAVPlayerState@@H@Z
vmCvar_t g_stanceFadeTime = {};
vmCvar_t g_stanceSolidTime = {};
extern float unk_F6A284[];          // @ 0xF6A284 (per-client viewport block)
extern Entity* GetPlayer(int idx);  // ?GetPlayer@@YAPAVEntity@@H@Z

// ea: 0x005AF000
bool ValidPlayerState(int idx)
{
    Entity* player = GetPlayer(idx);
    return player != nullptr && player->client != nullptr;
}

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

static inline int WeaponSlot(weaponFileInfo_t* w)
{
    return w->slot;
}
static inline int WeaponClass(weaponFileInfo_t* w)
{
    return w->weapClass;
}
extern weaponFileInfo_t* BG_GetInfoForWeapon(int iWeapon);  // game.o
extern int BG_ClipForWeapon(int iWeapon);       // game.o
extern int BG_AmmoForWeapon(int iWeapon);       // game.o
extern int BG_WeaponIsClipOnly(int iWeapon);    // game.o
extern bool BG_AllowPlayerWeaponAtVehiclePos(int vehType, int vehPos);  // game.o
extern int CG_GetGrenadeCount();                // cg.o
extern int CG_GetSpecialGrenadeCount();         // cg.o
vmCvar_t g_grenadeFadeTime = {};
vmCvar_t g_grenadeSolidTime = {};
extern int dword_F6419C[];  // @ 0xF6419C (special weapon type)
extern int dword_F641A0[];  // @ 0xF641A0 (special weapon end time)
extern int dword_F641A4[];  // @ 0xF641A4 (special weapon duration)
float percentToTrimBottom = 0.1f;  // @ 0xDF4460
float percentToTrimTop = 0.2f;     // @ 0xDF445C
extern int dword_F6355C[];   // @ 0xF6355C
int dword_F63F5C[4 * 1580] = {};   // @ 0xF63F5C (hint icon)
int dword_F63F60[4 * 1580] = {};   // @ 0xF63F60 (hint start time)
int dword_F63F64[4 * 1580] = {};   // @ 0xF63F64 (hint fade time)
int dword_F63F68[4 * 1580] = {};   // @ 0xF63F68
int dword_F63F6C[4 * 1580] = {};   // @ 0xF63F6C
vmCvar_t cg_cursorHints = {};
vmCvar_t cg_hintFadeTime = {};
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
int mpviewport = 0;          // @ 0xF3A574
extern const char* CG_ConfigString(unsigned int index);  // cg.o
extern int BG_GetNumWeapons();  // game.o
extern bool IsVehicleSpotted(Entity* vehicle);  // g.o
extern double VectorDistance(const float* const v1,
                                  const float* const v2);  // core.o
struct nglScene;
extern math::Position3* nglProjectPoint(math::Position3* result,
                                        const math::Position3* In,
                                        nglScene* Scene);  // ngl/ngl_scene.h
extern nglScene* nglBuildScene;  // render
extern "C" unsigned int AeHash(const char* str);  // core/ae_hash.cpp

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
    // ea: 0x00755140 (MPPlayerItems::FindItem)
    Entity* FindItem(EDroppedItemTypes item, short id)
    {
        ae_vector<sDroppedItem>* list;
        switch (item)
        {
        case kItemTypeSupport:
            list = &mDroppedSupport;
            break;
        case kItemTypeMines:
            list = &mDroppedMines;
            break;
        case kItemTypeKits:
            list = &mDroppedKits;
            break;
        default:
            list = &mDroppedWeapons;
            break;
        }
        if (list->mSize <= id)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/MPPlayerItems.cpp";
            AeAssert::gCurrentLine = 118;
            AeAssert::gCurrentExpr = "size > id";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("FindItem: Invalid ID"))
                __debugbreak();
        }
        unsigned int mVal = list->mElements[id].handle.mHandle.mVal;
        unsigned int index = mVal & 0xFFF;
        if (index < 0x540
            && mVal >> 12 == (unsigned int)EntityHandleDb::sInst
                                      .mElements[index].mKey)
            return EntityHandleDb::sInst.mElements[index].mObject;
        return nullptr;
    }
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
    // ea: 0x00735FF0 (MPPlayer::IsValid)
    bool IsValid() const
    {
        return mId < 0x10u && mConnection != nullptr;
    }
};
struct KeyInfoEntry2 {
    int   mState;           // +0x00 (low 2 bits down, high 30 repeats)
    char* mBoundCmdName;    // +0x04
};
template <typename T, int N>
struct ae_array_fixed {
    T m_elements[N];
};
struct KeyInfo {
    static ae_array_fixed<ae_array_fixed<KeyInfoEntry2, 256>, 1> mKeys;
    static int GetKey(const char* boundCmdName, int clnt);  // ?GetKey@KeyInfo@@SAHPBDH@Z
};
struct weaponInfo_s {
    uint8_t _pad[0x84];
    const char* pszTranslatedDisplayName;  // +0x84
    const char* pszTranslatedModename;      // +0x88
};
extern weaponInfo_s cg_weapons[];  // ?cg_weapons@@3PAUweaponInfo_s@@A @ 0xF6AE60
extern float* dword_F63B8C[];              // @ 0xF63B8C
extern char* va(const char* fmt, ...);    // core.o

// Minimal scr_vehicle_t view (full in game/logic/g_local.h); offsets from IDA.
struct vehicleSeat_t {
    int flags;   // +0x00
    DbLinkedHandle<EntityHandleDb, Entity> occupant;  // +0x04
    int boneIndex;  // +0x08
    int weapon;     // +0x0C
    float heat;     // +0x10
    Handle overheatEffect;  // +0x14
    uint8_t gunMounted;  // +0x18
    uint8_t overheating; // +0x19
    uint8_t firing;      // +0x1A
    uint8_t _pad1B;      // +0x1B
};
struct ScrVehicleLerped {
    math::Position3 mBodyPosition;   // +0x00
    math::Position3 mTurretAngles;   // +0x10
    math::Position3 mGunnerAngles;   // +0x20
    float mSteeringAngle;            // +0x30
    float mHatchAngleRight;          // +0x34
    float mHatchAngleLeft;           // +0x38
    float _pad3C;                    // +0x3C
};
struct scr_vehicle_t {  // 0x750 bytes
    uint8_t _pad[0x170];
    DbLinkedHandle<EntityHandleDb, Entity> mEntity;       // +0x170
    DbLinkedHandle<EntityHandleDb, Entity> mPhysicsOwner; // +0x174
    int16_t infoIdx;    // +0x178
    int16_t waitNode;   // +0x17A
    float   waitSpeed;  // +0x17C
    int     fireTime;   // +0x180
    uint8_t _pad2[0x1E0 - 0x184];
    vehicleSeat_t seats[11];  // +0x1E0 (308 bytes)
    uint8_t _pad3[0x3E0 - (0x1E0 + 11 * 28)];
    ScrVehicleLerped current;  // +0x3E0
    ScrVehicleLerped next;     // +0x420
};
static inline int16_t ScrVehicleInfoIdx(scr_vehicle_t* v)
{
    return v->infoIdx;
}

struct vehicle_info_t {
    uint8_t _pad[0x22];
    int16_t subtype;  // +0x22
    uint8_t _pad2[0x48 - 0x24];
    int hudIndex;     // +0x48
};
extern vehicle_info_t* VEH_GetVehicleInfo(int iIndex);  // g.o
extern float dword_F63CB4[];  // @ 0xF63CB4
float COMPASS_STOP_OFFSET = 0.0001f;  // @ 0xDF4464
extern int cg_aWeaponSelect[];  // ?cg_aWeaponSelect@@3PAHA @ 0xF5D078
vmCvar_t g_ammoFadeTime = {};
vmCvar_t g_ammoSolidTime = {};
extern int Com_BitCheck(const int* const array, int bitNum);  // bg_weapons.cpp
extern int BG_GetTotalAmmoReserve(const PlayerState* pPS,
                                  int iWeaponIndex);  // game.o
extern const float vectoyaw(const float* const vec);      // core.o
extern const float AngleNormalize360(float angle);  // core.o
extern float dword_F63560[];  // @ 0xF63560 (client origin x)
extern float dword_F63564[];  // @ 0xF63564 (client origin y)
extern float dword_F63568[];  // @ 0xF63568 (client origin z)
extern float dword_F63640[];  // @ 0xF63640 (client view height)
extern float dword_F63C50[];  // @ 0xF63C50 (screen x0)
extern float dword_F63C54[];  // @ 0xF63C54 (screen y0)
extern float dword_F63C58[];  // @ 0xF63C58 (screen w)
extern float dword_F63C5C[];  // @ 0xF63C5C (screen h)
extern float unk_F63634[];    // @ 0xF63634 (client yaw)
extern float unk_F6A280[];    // @ 0xF6A280 (previous viewport)
int dword_F64198[4 * 1580] = {};    // @ 0xF64198
vmCvar_t gCvarShowVehMap = {};
extern int gRenderCG_2D;      // ?gRenderCG_2D@@3HA (g.o)
extern bool IsPlayerFullySeatedInVehicle(Entity* player);  // g.o

extern char* Key_KeynumToString(int keynum, int bTranslate);  // cl.o

// ngl / view render helpers
struct nglScene;
enum nglSceneParamType : int { NGLSCENE_DEFAULTS = 0 };
extern nglScene* nglListBeginScene(nglSceneParamType ParamSource);  // ngl/ngl_scene.h
extern void nglSetClearFlags(unsigned int ClearFlags);
extern void nglListEndScene();
namespace View {
void SetViewportClipping(int clientIndex);  // cg.o
}
extern int Q_stricmp(const char* s1, const char* s2);  // g.o
extern vmCvar_t cg_widescreen;  // cg.o

// IGOCompassWidget cvars/data (verified VAs)
vmCvar_t g_compassFadeTime = {};
vmCvar_t g_compassSolidTime = {};
vmCvar_t cg_hudObjectiveRingTime = {};
vmCvar_t cg_hudObjectiveNumRings = {};
vmCvar_t cg_hudCompassMinRange = {};
vmCvar_t cg_hudObjectiveMaxRange = {};
vmCvar_t cg_hudCompassMaxRange = {};
vmCvar_t cg_hudObjectiveMinAlpha = {};
vmCvar_t cg_hudCompassMinRadius = {};
extern vmCvar_t cg_hudCompassSize;        // @ 0xF60528
extern vmCvar_t cg_hudCompassSpringyPointers;  // @ 0xF5CAD8
vmCvar_t cg_hudObjectiveMinHeight = {};
vmCvar_t cg_hudObjectiveMaxHeight = {};
extern vmCvar_t mp_objectiveSize;         // @ 0xEABD78
extern vmCvar_t mp_objectiveFarAlpha;     // @ 0xEAC8B8
vmCvar_t gCvarShowEnemy = {};
extern float dword_F63C70[];  // @ 0xF63C70 (client origin x)
extern float dword_F63C74[];  // @ 0xF63C74 (client origin y)
extern float dword_F63C78[];  // @ 0xF63C78 (client origin z)
extern int   dword_F64140[];  // @ 0xF64140
extern int   iLastCompassTime[];  // @ 0xF3A4C4
extern int   dword_F62964[];  // @ 0xF62964
extern float unk_F6A2B0[];    // @ 0xF6A2B0 (objective world data block)
extern const float AngleSubtract(float a1, float a2);   // core.o
extern const float AngleNormalize180(float angle);       // core.o
extern double VectorNormalize2D(float* const v);    // core.o
extern bool  IsVehicleTank(Entity* ent);           // g.o
extern Client g_clients[16];                       // g.o
extern team_t Sentient_EnemyTeam(team_t eTeam);    // mp_actors.o
extern void __fastcall Sentient_GetOrigin(const sentient_s* pSelf,
                                          float* const vOriginOut);  // mp_actors.o
extern bool G_GetTankIndex(DbLinkedHandle<EntityHandleDb, Entity> entity,
                           int* index, bool* enemy);  // g.o
extern void AddLeanToPosition(float* const vPosition, float fViewYaw,
                              float fLeanFrac, float fViewRoll,
                              float fLeanDist);      // g.o

struct ObjectiveDataView {
    unsigned int worldState;   // +0x00
    unsigned int unk4;         // +0x04
    unsigned int handle;       // +0x08 (DbLinkedHandle raw value)
    int          state;        // +0x0C
    float        origin[3];    // +0x10
    float        ring_time;    // +0x1C
};

// IGOCompassWidget data arrays (shell.o, copied from IDA)
const char* sObjectiveIconNames[27] = {
    "i_guy_bad_c", "i_tank_enemy_c", "i_objective_c_ring", "i_objective_c_up",
    "i_downed_friend_c", "i_flag_allied_c_up", "i_flag_axis_c",
    "i_flag_axis_c_down", "i_neutral_allied_c_up", "i_neutral_axis_c",
    "i_neutral_axis_c_down", "i_HQ_captured_c_up", "i_incoming_artillery_c",
    "i_incoming_artillery_c", nullptr, "i_tank_enemy_w", nullptr, nullptr,
    "i_downed_friend_w", nullptr, "i_flag_axis_w", nullptr, nullptr,
    "i_neutral_axis_w", nullptr, nullptr, nullptr,
};
const char* sWorldIconNames[27] = {
    nullptr, "i_tank_enemy_w", nullptr, nullptr, "i_downed_friend_w",
    nullptr, "i_flag_axis_w", nullptr, nullptr, "i_neutral_axis_w", nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, "i_head_rank_2_w", "i_head_VOIP_w", "i_spotted_enemy_w",
    "i_tank_enemy_w", "i_AP_mine_w", "voip_line_02_icon",
    "voip_line_04_icon",
};
const int sObjectiveIconRotateable[5] = {0, 1, 2, 3, 8};


struct level_locals_t {  // minimal view; offsets from IDA (full 0x2688 bytes)
    void*          clients;      // +0x00
    int            num_entities; // +0x04
    void*          sentients;    // +0x08
    scr_vehicle_t* vehicles;     // +0x0C
    uint8_t        _pad[0x9C - 0x10];
    int            time;         // +0x9C
    uint8_t        _pad2[0xC10 - 0xA0];
    uint16_t       MaxVehicles;  // +0xC10
};
extern level_locals_t level;        // ?level@@3Ulevel_locals_t@@A @ 0xEC9650

bool IsPlayerSpotted(Entity* player)
{
    if (player == nullptr || player->client == nullptr)
        return false;
    int spotTime = player->client->ps.spotTime;
    return spotTime != 0 && spotTime + 15000 >= level.time;
}

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
float GetPreviousHUDXPos(float pos, int window, int justification,
                         float width);    // cg.o
float GetPreviousHUDYPos(float pos, int window, int justification,
                         float height);   // cg.o
float GetCurrentHUDXPos(float pos, int window, int justification,
                        float width);     // cg.o
float GetCurrentHUDYPos(float pos, int window, int justification,
                        float height);    // cg.o
}

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
    draw_flash = false;
}

IGOHealthWidget::~IGOHealthWidget()
{
    delete flash;
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

IGOVoteWidget::~IGOVoteWidget()
{
    delete vote;
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
    PanelQuad* compass = g_femanager.IGO->compassWidget[mClient]->compass;
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

IGORankWidget::~IGORankWidget()
{
    delete friendlyRanks[0];
    delete friendlyRanks[1];
    delete friendlyRanks[2];
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
    PanelQuad* compass = g_femanager.IGO->compassWidget[mClient]->compass;
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

// ea: 0x005B7630
IGOWeaponNameWidget::~IGOWeaponNameWidget()
{
    if (name != nullptr)
        delete name;
    if (background != nullptr)
        delete background;
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

// ea: 0x00567040
void IGOWeaponNameWidget::Update(float time_inc)
{
    (void)time_inc;
    if (!IsShown()
        || EntityManager::sInst->GetPlayer(currCl) == nullptr
        || EntityManager::sInst->GetPlayer(currCl)->client == nullptr)
    {
        return;
    }

    float alpha = (float)(cg_aWeaponSelectTime[currCl] - cgGlobal.time + 1800);
    if (alpha < 0.0f
        || (EntityManager::sInst->GetPlayer(currCl)->client->ps.eFlags
            & 0x100000) != 0)
    {
        dont_draw = true;
        return;
    }

    int weapon = cg_aWeaponSelect[currCl];
    weaponFileInfo_t* info =
        reinterpret_cast<weaponFileInfo_t*>(dword_F63B8C[1580 * currCl]);
    if (weapon >= 0 && weapon < BG_GetNumWeapons()
        && Com_BitCheck(GetPlayerState(currCl).weapons, weapon) != 0)
    {
        info = BG_GetInfoForWeapon(cg_aWeaponSelect[currCl]);
    }

    if (info == nullptr || info->index == 0)
    {
        dont_draw = true;
        return;
    }

    float visibility = alpha * 0.0099999998f;
    if (alpha >= 100.0f)
        visibility = 1.0f;
    dont_draw = false;
    name->SetAlpha(visibility);
    background->SetAlpha(visibility);

    if (last_weapon_index != info->index)
    {
        int index = info->index;
        const char* text;
        if (*info->szModeName != 0)
        {
            text = va("%s / %s", cg_weapons[index].pszTranslatedDisplayName,
                      cg_weapons[index].pszTranslatedModename);
        }
        else
        {
            text = va("%s", cg_weapons[index].pszTranslatedDisplayName);
        }
        name->SetTextNoLocalize(text);
        float width = name->GetWidth(nullptr) + 16.0f;
        float x = 545.0f - width;
        View::GetCurrentHUDXPos(x, (int)unk_F6A284[802 * currCl], 0, 0.0f);
        float y = View::GetCurrentHUDYPos(
            365.0f, (int)unk_F6A284[802 * currCl], 0, 0.0f);
        View::GetCurrentHUDXPos(545.0f, (int)unk_F6A284[802 * currCl], 0,
                                0.0f);
        float bottom = View::GetCurrentHUDYPos(
            385.0f, (int)unk_F6A284[802 * currCl], 0, 0.0f);
        background->SetPos(width, y, y, bottom);
    }
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
    draw_time = 0.0f;
}

IGOAmmoWidget::~IGOAmmoWidget()
{
    delete clipAmmo;
    delete totalAmmo;
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
    if (is_shown)
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
                                      -4.0f);
    totalAmmo->UpdateForHUDSplitScreen(viewport, old_viewport, 10, 5.0f,
                                       -4.0f);
    frame->FormatHUDForSplitScreen(viewport, old_viewport, 10, 0.0f, 0.0f);
}

// ea: 0x00588510
void IGOAmmoWidget::Update(float time_inc)
{
    if (!is_shown
        || EntityManager::sInst->GetPlayer(currCl) == nullptr
        || EntityManager::sInst->GetPlayer(currCl)->client == nullptr)
    {
        return;
    }
    if (((EntityManager::sInst->GetPlayer(currCl)->client->ps.eFlags
          & 0x100000)
             != 0
         && !BG_AllowPlayerWeaponAtVehiclePos(
             GetPlayerState(currCl).vehType, GetPlayerState(currCl).vehPos))
        || dword_F62960[1580 * currCl] == 0
        || EntityManager::sInst->GetPlayer(currCl)->client->pers.playerState
               != 3)
    {
        dont_draw = true;
        return;
    }
    int weapon;
    if (cg_aWeaponSelect[currCl] >= 0
        && cg_aWeaponSelect[currCl] < BG_GetNumWeapons()
        && Com_BitCheck(GetPlayerState(currCl).weapons,
                        cg_aWeaponSelect[currCl]))
    {
        weapon = cg_aWeaponSelect[currCl];
    }
    else
    {
        weapon =
            EntityManager::sInst->GetPlayer(currCl)->s.weapon;
    }
    if (weapon == 0)
    {
        dont_draw = true;
        return;
    }
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(weapon);
    if (WeaponClass(InfoForWeapon) == 5 /* WEAPCLASS_GRENADE */
        && (WeaponSlot(InfoForWeapon) == 9 /* WEAPSLOT_SPECIAL */
            || WeaponSlot(InfoForWeapon) == 8 /* WEAPSLOT_SATCHEL */))
    {
        return;
    }
    draw_time -= time_inc;
    float total_draw_time = g_ammoFadeTime.value + g_ammoSolidTime.value;
    bool lowAmmo = false;
    int ammo_update_val =
        BG_GetTotalAmmoReserve(&GetPlayerState(currCl), weapon);
    bool draw_ammo = false;
    int clip_update_val = -1;
    if (!BG_WeaponIsClipOnly(weapon))
    {
        int clipIdx = BG_ClipForWeapon(weapon);
        clip_update_val = GetPlayerState(currCl).ammoclip[clipIdx];
        draw_ammo = clip_update_val != clip_val;
    }
    clip_val = clip_update_val;
    const char* szRadiantName = InfoForWeapon->szRadiantName;
    bool v18;
    if (strcmp(szRadiantName, "weapon_panzerschreck") == 0
        || strcmp(szRadiantName, "weapon_bazooka") == 0)
    {
        v18 = ammo_val <= InfoForWeapon->iClipSize;
    }
    else
    {
        if (clip_update_val == 1)
        {
            lowAmmo = true;
            goto label_29;
        }
        v18 = ammo_val <= InfoForWeapon->iClipSize;
    }
    if (!v18)
    {
    label_29:
        int v19 = ammo_update_val;
        if (ammo_update_val != ammo_val || draw_ammo || lowAmmo
            || force_appear)
        {
            force_appear = false;
            draw_time = total_draw_time;
        }
        bool draw_clip = clip_update_val >= 0;
        draw_ammo = v19 >= 0;
        v18 = clip_update_val <= 999;
        ammo_val = v19;
        if (!v18)
            clip_val = 999;
        if (v19 > 999)
            ammo_val = 999;
        if (WeaponClass(InfoForWeapon) == 8 /* WEAPCLASS_SPOTTER */)
        {
            draw_clip = false;
            if (InfoForWeapon->bADSFire == 0)
                draw_ammo = false;
        }
        v18 = draw_time >= 0.0f;
        char text[32];
        text[0] = 0;
        if (!v18)
            draw_time = 0.0f;
        frame->SetVisibility(1.0f);
        color32 lowColor;
        if (lowAmmo)
        {
            lowColor.c.b = 50;
            lowColor.c.g = 0x8C;
            lowColor.c.r = 0x8B;
        }
        else
        {
            lowColor.c.b = 0x9B;
            lowColor.c.g = 0xB8;
            lowColor.c.r = 0xFF;
        }
        lowColor.c.a = 255;
        clipAmmo->SetColor(lowColor);
        totalAmmo->SetColor(lowColor);
        if (draw_clip)
        {
            if (!draw_ammo)
            {
                sprintf(text, "%i", clip_val);
                clipAmmo->SetTextNoLocalize(text);
                totalAmmo->SetTextNoLocalize(defaultFileName);
                dont_draw = false;
                return;
            }
            sprintf(text, "%i", clip_val);
            clipAmmo->SetTextNoLocalize(text);
            sprintf(text, "%i", ammo_val);
        }
        else
        {
            if (!draw_ammo)
            {
                dont_draw = false;
                return;
            }
            sprintf(text, "%i", ammo_val);
            clipAmmo->SetTextNoLocalize(defaultFileName);
        }
        totalAmmo->SetTextNoLocalize(text);
        dont_draw = false;
    }
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
                                                0.0f, 22.0f);
    m_pAxisScoreText->UpdateForHUDSplitScreen(viewport, old_viewport, 5,
                                              0.0f, 32.0f);
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
    unsigned int v3 = v2 / 3600;
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
                                      -16.0f);
    ammoRight->UpdateForHUDSplitScreen(viewport, old_viewport, 10, 3.0f,
                                       -16.0f);
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
    hadAmmo = false;
    timeForNormalSize = 0;
    scale = 1.0f;
}

IGOSpecialWeaponWidget::~IGOSpecialWeaponWidget()
{
    delete artillery;
    delete health;
    delete ammo;
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
    MPPlayerManager* playerManager =
        MultiplayerMgr::sInst->mPeer->GetPlayerManager();
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
    "voip_line_01_icon", "voip_line_02_icon", "voip_line_03_icon",
    "voip_line_04_icon",
};
const char* sVoipTextNames[4] = {
    "voip_line_01_text", "voip_line_02_text", "voip_line_03_text",
    "voip_line_04_text",
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

IGOHintWidget::~IGOHintWidget()
{
    delete text;
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
                (panel_layer)TextPointer->GetScaleX(), 0.0f, 0, 0,
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
                    0, col2);
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
                            if (strcmp(tmp, "hud_smokegrenade.tga") != 0)
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
                            }
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
    zoomPct = 0.0f;
    lastFlag = -1;
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
    if (viewport == 3)
        viewport = 5;
    else if (viewport == 4)
        viewport = 7;
    if (old_viewport == 3)
        old_viewport = 5;
    else if (old_viewport == 4)
        old_viewport = 7;

    float war_width = -30.0f;
    for (int i = 0; i < 5; ++i)
    {
        m_pObjectiveFrameUS.m_elements[i]->FormatHUDForSplitScreen(
            viewport, old_viewport, 4, war_width, 20.0f);
        m_pObjectiveFrameGerman.m_elements[i]->FormatHUDForSplitScreen(
            viewport, old_viewport, 4, war_width, 20.0f);
        m_pObjectiveGerman.m_elements[i]->FormatHUDForSplitScreen(
            viewport, old_viewport, 4, war_width, 20.0f);
        m_pObjectiveUS.m_elements[i]->FormatHUDForSplitScreen(
            viewport, old_viewport, 4, war_width, 20.0f);
        m_pIconGerman.m_elements[i]->FormatHUDForSplitScreen(
            viewport, old_viewport, 4, war_width, 20.0f);
        m_pIconUS.m_elements[i]->FormatHUDForSplitScreen(
            viewport, old_viewport, 4, war_width, 20.0f);
        war_width += 15.0f;
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

// ============================================================================
// IGOTankIconWidget
// ============================================================================

// ea: 0x00567AA0
IGOTankIconWidget::IGOTankIconWidget(int client)
{
    mClient = client;
    force_appear = false;
    is_shown = true;
    mCompassWidth = 0.0f;
    mLastBaseAngles = 0.0f;
    mLastTurretAngles = 0.0f;
    base[0] = nullptr;
    base[1] = nullptr;
    base[2] = nullptr;
    base[3] = nullptr;
    turret[0] = nullptr;
    turret[1] = nullptr;
    turret[2] = nullptr;
    turret[3] = nullptr;
    memset(occupants, 0, sizeof(occupants));
}

// ea: 0x00599D10
void IGOTankIconWidget::Init(PanelFile* panel)
{
    base[0] = panel->GetPointer("sherman_icon_body");
    turret[0] = panel->GetPointer("sherman_icon_turret");
    base[1] = panel->GetPointer("PanzerV_icon_body");
    turret[1] = panel->GetPointer("PanzerV_icon_turret");
    base[2] = panel->GetPointer("horch_icon_body");
    base[3] = panel->GetPointer("horch_icon_body");
    occupants[0][0][0] = panel->GetPointer("sherman_position_01_off");
    occupants[0][0][1] = panel->GetPointer("sherman_position_01_on");
    occupants[0][1][0] = panel->GetPointer("sherman_position_02_off");
    occupants[0][1][1] = panel->GetPointer("sherman_position_02_on");
    occupants[1][0][0] = panel->GetPointer("PanzerV_position_01_off");
    occupants[1][0][1] = panel->GetPointer("PanzerV_position_01_on");
    occupants[1][1][0] = panel->GetPointer("PanzerV_position_02_off");
    occupants[1][1][1] = panel->GetPointer("PanzerV_position_02_on");
    occupants[3][0][0] = panel->GetPointer("horch_position_01_off");
    occupants[3][0][1] = panel->GetPointer("horch_position_01_on");
    occupants[3][1][0] = panel->GetPointer("horch_position_03_off");
    occupants[3][1][1] = panel->GetPointer("horch_position_03_on");
    occupants[3][2][0] = panel->GetPointer("horch_position_02_off");
    occupants[3][2][1] = panel->GetPointer("horch_position_02_on");
    occupants[2][0][0] = PanelQuad::Clone(occupants[3][0][0]);
    occupants[2][0][1] = PanelQuad::Clone(occupants[3][0][1]);
    occupants[2][1][0] = PanelQuad::Clone(occupants[3][1][0]);
    occupants[2][1][1] = PanelQuad::Clone(occupants[3][1][1]);
    occupants[2][2][0] = PanelQuad::Clone(occupants[3][2][0]);
    occupants[2][2][1] = PanelQuad::Clone(occupants[3][2][1]);
    if (mClient > 0)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (base[i] != nullptr)
                base[i] = PanelQuad::Clone(base[i]);
            if (turret[i] != nullptr)
                turret[i] = PanelQuad::Clone(turret[i]);
            for (int j = 0; j < 3; ++j)
            {
                if (occupants[i][j][0] != nullptr)
                    occupants[i][j][0] =
                        PanelQuad::Clone(occupants[i][j][0]);
                if (occupants[i][j][1] != nullptr)
                    occupants[i][j][1] =
                        PanelQuad::Clone(occupants[i][j][1]);
            }
        }
    }
    for (int i = 0; i < 4; ++i)
    {
        if (base[i] != nullptr)
            base[i]->SetXYInitialToCurrentPos();
        if (turret[i] != nullptr)
            turret[i]->SetXYInitialToCurrentPos();
        for (int j = 0; j < 3; ++j)
        {
            if (occupants[i][j][0] != nullptr)
                occupants[i][j][0]->SetXYInitialToCurrentPos();
            if (occupants[i][j][1] != nullptr)
                occupants[i][j][1]->SetXYInitialToCurrentPos();
        }
    }
}

// ea: 0x00588DF0
void IGOTankIconWidget::Update(float time_inc)
{
    (void)time_inc;
    DbLinkedHandle<EntityHandleDb, Entity> ent = GetPlayersTank();
    unsigned int v4 = ent.mHandle.mVal & 0xFFF;
    if (v4 < 0x540
        && ent.mHandle.mVal >> 12
               == (unsigned int)EntityHandleDb::sInst.mElements[v4].mKey
        && EntityHandleDb::sInst.mElements[v4].mObject != nullptr)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
        scr_vehicle_t* vehicle = (scr_vehicle_t*)mObject->scr_vehicle;
        if (VEH_GetVehicleInfo(ScrVehicleInfoIdx(vehicle))->subtype == 2)
        {
            SetShown(false);
            return;
        }
        SetShown(true);
        if (is_shown)
        {
            vehicle_info_t* VehicleInfo =
                VEH_GetVehicleInfo(ScrVehicleInfoIdx(vehicle));
            int v8 = VehicleInfo != nullptr ? VehicleInfo->hudIndex : 0;
            mVehicleType = v8;
            float* v9 = &dword_F63CB4[1580 * currCl];
            float v10 = *v9
                        - mObject->r.currentAngles.v.m128_f32[1];
            float turretAngle =
                (v10 - vehicle->next.mTurretAngles.v.m128_f32[1])
                * 0.017453292f;
            if (COMPASS_STOP_OFFSET <= fabsf(*v9 - mLastBaseAngles)
                || (turret[v8] != nullptr
                    && COMPASS_STOP_OFFSET
                           <= fabsf(
                               vehicle->next.mTurretAngles.v.m128_f32[1]
                               - mLastTurretAngles)))
            {
                mLastBaseAngles = *v9;
                mLastTurretAngles =
                    vehicle->next.mTurretAngles.v.m128_f32[1];
                Rotate(base[v8], v10 * 0.017453292f);
                if (turret[mVehicleType] != nullptr)
                    Rotate(turret[mVehicleType], turretAngle);
                for (int i = 0; i < 3; ++i)
                {
                    PanelQuad** pOff =
                        &occupants[mVehicleType][i][0];
                    if (*pOff != nullptr)
                    {
                        DbLinkedHandle<EntityHandleDb, Entity>& occupant =
                            vehicle->seats[i].occupant;
                        unsigned int v15 = occupant.mHandle.mVal & 0xFFF;
                        bool occupied =
                            v15 < 0x540
                            && occupant.mHandle.mVal >> 12
                           == (unsigned int)EntityHandleDb::sInst
                                          .mElements[v15].mKey
                            && EntityHandleDb::sInst.mElements[v15].mObject
                                   != nullptr;
                        (*pOff)->SetShown(!occupied);
                        PanelQuad* pOn = occupants[mVehicleType][i][1];
                        pOn->SetShown(occupied);
                        float angle = v10 * 0.017453292f;
                        if (i == 1)
                            angle = turretAngle;
                        Rotate(occupied ? pOn : *pOff, angle);
                    }
                }
            }
        }
    }
}

// ea: 0x00567BE0
void IGOTankIconWidget::Draw()
{
    if (is_shown
        && EntityManager::sInst->GetPlayer(currCl)->client->pers.playerState
               == 3
        && (EntityManager::sInst->GetPlayer(currCl)->client->ps.eFlags
            & 0x100000)
               != 0)
    {
        base[mVehicleType]->Draw();
        if (turret[mVehicleType] != nullptr)
            turret[mVehicleType]->Draw();
        for (int i = 0; i < 3; ++i)
        {
            if (occupants[mVehicleType][i][0] != nullptr)
                occupants[mVehicleType][i][0]->Draw();
            if (occupants[mVehicleType][i][1] != nullptr)
                occupants[mVehicleType][i][1]->Draw();
        }
    }
}

// ea: 0x00567B00
void IGOTankIconWidget::Rotate(PanelQuad* quad, float angle)
{
    quad->ResetToInitialXY();
    int window = unk_F6A284[802 * mClient];
    float x_pos = View::GetCurrentHUDXPos(quad->GetCenterX(), window, 9,
                                          0.0f);
    int window2 = unk_F6A284[802 * mClient];
    float y_pos = View::GetCurrentHUDYPos(quad->GetCenterY(), window2, 9,
                                          0.0f);
    quad->Rotate(angle, true);
    quad->SetCenterPos(x_pos, y_pos);
    float yScale = View::GetYScalingForHUD(unk_F6A284[802 * mClient]);
    float xScale = View::GetXScalingForHUD(unk_F6A284[802 * mClient]);
    quad->Scale(x_pos, y_pos, xScale, yScale, true);
}

// ea: 0x00567CC0
int IGOTankIconWidget::GetVehicleIndex(scr_vehicle_t* vehicle) const
{
    vehicle_info_t* VehicleInfo =
        VEH_GetVehicleInfo(ScrVehicleInfoIdx(vehicle));
    if (VehicleInfo != nullptr)
        return VehicleInfo->hudIndex;
    return 0;
}

// ea: 0x005782A0
void IGOTankIconWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    PanelQuad* compass = g_femanager.IGO->compassWidget[mClient]->compass;
    float width;
    if (compass != nullptr)
        width = compass->GetInitialWidth() * 0.5f;
    else
        width = 0.0f;
    mCompassWidth = width;
    for (int i = 0; i < 4; ++i)
    {
        if (base[i] != nullptr)
        {
            base[i]->ResetToInitialXY();
            base[i]->FormatHUDForSplitScreen(viewport, old_viewport, 9,
                                             mCompassWidth, 0.0f);
        }
        if (turret[i] != nullptr)
        {
            turret[i]->ResetToInitialXY();
            turret[i]->FormatHUDForSplitScreen(viewport, old_viewport, 9,
                                               mCompassWidth, 0.0f);
        }
        for (int j = 0; j < 3; ++j)
        {
            if (occupants[i][j][0] != nullptr)
            {
                occupants[i][j][0]->ResetToInitialXY();
                occupants[i][j][0]->FormatHUDForSplitScreen(
                    viewport, old_viewport, 9, mCompassWidth, 0.0f);
            }
            if (occupants[i][j][1] != nullptr)
            {
                occupants[i][j][1]->ResetToInitialXY();
                occupants[i][j][1]->FormatHUDForSplitScreen(
                    viewport, old_viewport, 9, mCompassWidth, 0.0f);
            }
        }
    }
}

// ea: 0x00583890
void IGOTankIconWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    for (int i = 0; i < 4; ++i)
    {
        if (base[i] != nullptr)
        {
            base[i]->ResetToInitialXY();
            base[i]->FattenMeForWidescreen(widescreen, about_x);
        }
        if (turret[i] != nullptr)
        {
            turret[i]->ResetToInitialXY();
            turret[i]->FattenMeForWidescreen(widescreen, about_x);
        }
        for (int j = 0; j < 3; ++j)
        {
            if (occupants[i][j][0] != nullptr)
            {
                occupants[i][j][0]->ResetToInitialXY();
                occupants[i][j][0]->FattenMeForWidescreen(widescreen,
                                                          about_x);
            }
            if (occupants[i][j][1] != nullptr)
            {
                occupants[i][j][1]->ResetToInitialXY();
                occupants[i][j][1]->FattenMeForWidescreen(widescreen,
                                                          about_x);
            }
        }
    }
}

// ============================================================================
// IGOGrenadeIndicator
// ============================================================================

// ea: 0x00579310
IGOGrenadeIndicator::IGOGrenadeIndicator(int client)
{
    mClient = client;
    is_shown = true;
    force_appear = false;
    memset(mActiveGrenadeList, 0, sizeof(mActiveGrenadeList));
    mArrowOffset = 0.0f;
    mMineIcon = nullptr;
    mGrenadeIcon = nullptr;
    mGrenadeArrow = nullptr;
    mGrenadeHold = nullptr;
    mCurrentGrenadeIcon = nullptr;
}

// ea: 0x00568BA0
IGOGrenadeIndicator::~IGOGrenadeIndicator()
{
    if (mMineIcon != nullptr)
    {
        delete mMineIcon;
        mMineIcon = nullptr;
    }
    if (mGrenadeIcon != nullptr)
    {
        delete mGrenadeIcon;
        mGrenadeIcon = nullptr;
    }
    if (mGrenadeArrow != nullptr)
    {
        delete mGrenadeArrow;
        mGrenadeArrow = nullptr;
    }
    if (mGrenadeHold != nullptr)
    {
        delete mGrenadeHold;
        mGrenadeHold = nullptr;
    }
}

// ea: 0x0059A780
void IGOGrenadeIndicator::Init(PanelFile* panel)
{
    if (strcmp(panel->mName, "hud_mp.panel") == 0)
    {
        PanelQuad* Pointer = panel->GetPointer("AP_mine_proximity");
        mMineIcon = Pointer;
        if (mClient > 0)
            mMineIcon = PanelQuad::Clone(Pointer);
    }
    else
    {
        mGrenadeIcon = panel->GetPointer("grenade_proximity_icon");
        mGrenadeArrow = panel->GetPointer("grenade_proximity_arrow");
        mGrenadeHold = panel->GetPointer("grenade_hold");
        if (mClient > 0)
        {
            mGrenadeIcon = PanelQuad::Clone(mGrenadeIcon);
            mGrenadeArrow = PanelQuad::Clone(mGrenadeArrow);
            mGrenadeHold = PanelQuad::Clone(mGrenadeHold);
        }
        mCurrentGrenadeIcon = mGrenadeIcon;
        float iconX = 0.0f, iconY = 0.0f;
        float arrowX = 0.0f, arrowY = 0.0f;
        mGrenadeIcon->GetCenterPos(iconX, iconY);
        mGrenadeArrow->GetCenterPos(arrowX, arrowY);
        mArrowOffset = VectorDistance(&arrowX, &iconX);
    }
}

// ea: 0x00568C30
bool IGOGrenadeIndicator::ValidHudGrenade(
    const Entity* grenade, float splashRadius,
    float (&grenadeOffset)[3], float& grenadeDistanceSquared) const
{
    if ((grenade->s.pos.trDelta[0] * grenade->s.pos.trDelta[0])
            + (grenade->s.pos.trDelta[1] * grenade->s.pos.trDelta[1])
            + (grenade->s.pos.trDelta[2] * grenade->s.pos.trDelta[2])
        > 1.0f)
    {
        return false;
    }
    int base = 1580 * currCl;
    grenadeOffset[0] = grenade->s.pos.trBase[0] - dword_F63560[base];
    grenadeOffset[1] = grenade->s.pos.trBase[1] - dword_F63564[base];
    float v5 = grenade->s.pos.trBase[2] - dword_F63568[base];
    grenadeOffset[2] = v5;
    if (v5 < -104.0f || v5 > dword_F63640[base] + 104.0f)
        return false;
    float v6 = (v5 * v5) + (grenadeOffset[1] * grenadeOffset[1])
               + (grenadeOffset[0] * grenadeOffset[0]);
    grenadeDistanceSquared = v6;
    if (grenade->splashMethodOfDeath == 6)
    {
        if (v6
            > ((grenade->r.maxs.v.m128_f32[0] + 72.0f)
               * (grenade->r.maxs.v.m128_f32[0] + 72.0f)))
            return false;
    }
    else if (v6 > splashRadius * splashRadius)
    {
        return false;
    }
    return true;
}

// ea: 0x00568D70
float IGOGrenadeIndicator::CalcGrenadeAlpha(
    float grenadeDistanceSquared, float splashInnerRadius,
    float splashOutterRadius) const
{
    float v4;
    if ((splashInnerRadius * splashInnerRadius) >= 0.0f)
    {
        v4 = ((splashOutterRadius * splashOutterRadius)
              - (splashInnerRadius * splashInnerRadius)
              - (grenadeDistanceSquared
                 - (splashInnerRadius * splashInnerRadius)))
             / ((splashOutterRadius * splashOutterRadius)
                - (splashInnerRadius * splashInnerRadius));
    }
    else
    {
        v4 = 1.0f;
    }
    float v5 = v4 * 0.6f;
    if (v5 >= 0.1f)
        return v5;
    return 0.1f;
}

// ea: 0x00568DE0
void IGOGrenadeIndicator::DrawGrenadeIcon(float sinYaw, float cosYaw,
                                          float alpha) const
{
    int base = 1580 * currCl;
    mCurrentGrenadeIcon->SetCenterPos(
        (dword_F63C58[base] * 0.5f) + dword_F63C50[base]
            - (sinYaw * 60.0f),
        (dword_F63C5C[base] * 0.5f) + dword_F63C54[base]
            - (cosYaw * 60.0f));
    mCurrentGrenadeIcon->SetAlpha(alpha);
    mCurrentGrenadeIcon->Draw();
}

// ea: 0x00568E80
void IGOGrenadeIndicator::DrawGrenadeArrow(float yaw, float sinYaw,
                                           float cosYaw, float alpha) const
{
    int base = 1580 * currCl;
    mGrenadeArrow->SetCenterPos(
        (dword_F63C58[base] * 0.5f) + dword_F63C50[base]
            - ((mArrowOffset + 60.0f) * sinYaw),
        (dword_F63C5C[base] * 0.5f) + dword_F63C54[base]
            - ((mArrowOffset + 60.0f) * cosYaw));
    mGrenadeArrow->Rotate((0.0f - yaw) * 3.1415927f * 0.0055555557f,
                          true);
    mGrenadeArrow->SetAlpha(alpha);
    mGrenadeArrow->Draw();
}

// ea: 0x00568F50
bool IGOGrenadeIndicator::CanBePickUp()
{
    return false;
}

// ea: 0x00568F60
void IGOGrenadeIndicator::SetDefaultIcon()
{
    mCurrentGrenadeIcon = mGrenadeIcon;
}

// ea: 0x005793A0
void IGOGrenadeIndicator::DrawGrenade(const Entity* grenade)
{
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(grenade->s.weapon);
    mCurrentGrenadeIcon = mGrenadeIcon;
    if (InfoForWeapon == nullptr)
        return;
    float grenadeOffset[3];
    float grenadeDistanceSquared;
    if (!ValidHudGrenade(grenade, (float)InfoForWeapon->iExplosionRadius,
                         grenadeOffset, grenadeDistanceSquared))
    {
        return;
    }
    float angle = vectoyaw(grenadeOffset) - unk_F63634[1580 * currCl];
    float yaw = AngleNormalize360(angle);
    float radians = yaw * 3.1415927f * 0.0055555557f;
    float sinYaw, cosYaw;
    FastSinCos(radians, &sinYaw, &cosYaw);
    float alpha;
    if (InfoForWeapon->type == 10 /* WEAPTYPE_NUM */)
    {
        float v6 = grenade->r.maxs.v.m128_f32[0]
                   * grenade->r.maxs.v.m128_f32[0];
        float v8;
        if (v6 >= 0.0f)
        {
            float v7 = ((grenade->r.maxs.v.m128_f32[0] + 72.0f)
                        * (grenade->r.maxs.v.m128_f32[0] + 72.0f))
                       - v6;
            v8 = (v7 - (grenadeDistanceSquared - v6)) / v7;
        }
        else
        {
            v8 = 1.0f;
        }
        float v9 = v8 * 0.6f;
        alpha = v9 >= 0.1f ? v9 : 0.1f;
        mCurrentGrenadeIcon = mMineIcon;
    }
    else
    {
        float v10 = (float)(InfoForWeapon->iExplosionRadius
                            * InfoForWeapon->iExplosionRadius);
        float v11 = ((v10 - grenadeDistanceSquared) / v10) * 0.6f;
        alpha = v11 >= 0.1f ? v11 : 0.1f;
    }
    DrawGrenadeIcon(sinYaw, cosYaw, alpha);
    DrawGrenadeArrow(yaw, sinYaw, cosYaw, alpha);
}

// ea: 0x0058A9A0
void IGOGrenadeIndicator::Update(float time_inc)
{
    (void)time_inc;
    for (int i = 0; i < 10; ++i)
    {
        unsigned int v4 = mActiveGrenadeList[i].mHandle.mVal & 0xFFF;
        if (v4 >= 0x540
            || mActiveGrenadeList[i].mHandle.mVal >> 12
                   != (unsigned int)EntityHandleDb::sInst.mElements[v4].mKey
            || EntityHandleDb::sInst.mElements[v4].mObject == nullptr)
        {
            mActiveGrenadeList[i].mHandle.mVal = 0;
        }
    }
}

// ea: 0x0058AAA0
void IGOGrenadeIndicator::Draw()
{
    for (int i = 0; i < 10; ++i)
    {
        unsigned int v4 = mActiveGrenadeList[i].mHandle.mVal & 0xFFF;
        if (v4 < 0x540
            && mActiveGrenadeList[i].mHandle.mVal >> 12
                   == (unsigned int)EntityHandleDb::sInst.mElements[v4].mKey)
        {
            Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
            if (mObject != nullptr && mObject->think != 0x0C)
                DrawGrenade(mObject);
        }
    }
    if (MultiplayerMgr::sInst->mPeer == nullptr)
        return;
    MPPlayerManager* playerManager =
        MultiplayerMgr::sInst->mPeer->GetPlayerManager();
    if (mClient >= 16)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EntityManager.h";
        AeAssert::gCurrentLine = 19;
        AeAssert::gCurrentExpr = "idx<16";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
            __debugbreak();
    }
    if (EntityManager::sInst->mPlayers[mClient] == nullptr)
        return;
    for (int j = 0; j < 16; ++j)
    {
        MPPlayer* Player = playerManager->GetPlayer(j);
        MpPlayerView2* pv = (MpPlayerView2*)Player;
        if (Player == nullptr || !pv->IsValid())
            continue;
        if (pv->mClientIndex >= 0)
            EntityManager::sInst->GetPlayer(
                pv->mClientIndex);
        for (int k = 0; k < 3; ++k)
        {
            Entity* Item = pv->mItems.FindItem(kItemTypeMines, (short)k);
            if (Item != nullptr && Item->think != 0x0C)
                DrawGrenade(Item);
        }
    }
}

// ea: 0x0058ABE0
void IGOGrenadeIndicator::AddActiveGrenade(const Entity* grenade)
{
    for (int i = 0; i < 10; ++i)
    {
        unsigned int v4 = mActiveGrenadeList[i].mHandle.mVal & 0xFFF;
        if (v4 >= 0x540
            || mActiveGrenadeList[i].mHandle.mVal >> 12
                   != (unsigned int)EntityHandleDb::sInst.mElements[v4].mKey
            || EntityHandleDb::sInst.mElements[v4].mObject == nullptr)
        {
            mActiveGrenadeList[i].mHandle.mVal =
                grenade->mHandle.mHandle.mVal;
            return;
        }
    }
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOGrenadeIndicator.cpp";
    AeAssert::gCurrentLine = 240;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert(
            "Maximum number of grenade indicator widgets (%d) reached", 10))
        __debugbreak();
}

// ea: 0x00583CD0
void IGOGrenadeIndicator::UpdateWidescreen(bool widescreen, float about_x)
{
    mCurrentGrenadeIcon->FattenMeForWidescreen(widescreen, about_x);
    mGrenadeArrow->FattenMeForWidescreen(widescreen, about_x);
}

// ============================================================================
// IGOFrontEnd
// ============================================================================

// ea: 0x0059CA80
IGOFrontEnd::IGOFrontEnd()
{
    compassWidget[0] =
        (IGOCompassWidget*)mem_heap_malloc(0x1488u);
    if (compassWidget[0] != nullptr)
        compassWidget[0] =
            new (compassWidget[0]) IGOCompassWidget(0);
    else
        compassWidget[0] = nullptr;

    hintWidget[0] = (IGOHintWidget*)mem_heap_malloc(0x46Cu);
    if (hintWidget[0] != nullptr)
        hintWidget[0] = new (hintWidget[0]) IGOHintWidget(0);
    else
        hintWidget[0] = nullptr;

    jeepMapWidget[0] = (IGOJeepMapWidget*)mem_heap_malloc(0x50u);
    if (jeepMapWidget[0] != nullptr)
        jeepMapWidget[0] = new (jeepMapWidget[0]) IGOJeepMapWidget(0);
    else
        jeepMapWidget[0] = nullptr;

    stanceWidget[0] = (IGOStanceWidget*)mem_heap_malloc(0x38u);
    if (stanceWidget[0] != nullptr)
        stanceWidget[0] = new (stanceWidget[0]) IGOStanceWidget(0);
    else
        stanceWidget[0] = nullptr;

    healthWidget[0] = (IGOHealthWidget*)mem_heap_malloc(0x28u);
    if (healthWidget[0] != nullptr)
        healthWidget[0] = new (healthWidget[0]) IGOHealthWidget(0);
    else
        healthWidget[0] = nullptr;

    ammoWidget[0] = (IGOAmmoWidget*)mem_heap_malloc(0x28u);
    if (ammoWidget[0] != nullptr)
        ammoWidget[0] = new (ammoWidget[0]) IGOAmmoWidget(0);
    else
        ammoWidget[0] = nullptr;

    weaponNameWidget[0] = (IGOWeaponNameWidget*)mem_heap_malloc(0x1Cu);
    if (weaponNameWidget[0] != nullptr)
        weaponNameWidget[0] =
            new (weaponNameWidget[0]) IGOWeaponNameWidget(0);
    else
        weaponNameWidget[0] = nullptr;

    grenadeWidget[0] = (IGOGrenadeWidget*)mem_heap_malloc(0x44u);
    if (grenadeWidget[0] != nullptr)
        grenadeWidget[0] = new (grenadeWidget[0]) IGOGrenadeWidget(0);
    else
        grenadeWidget[0] = nullptr;

    grenadeCookWidget[0] = (IGOGrenadeCookWidget*)mem_heap_malloc(0x34u);
    if (grenadeCookWidget[0] != nullptr)
        grenadeCookWidget[0] =
            new (grenadeCookWidget[0]) IGOGrenadeCookWidget(0);
    else
        grenadeCookWidget[0] = nullptr;

    hintText[0] = nullptr;

    mGrenadeIndicator[0] = (IGOGrenadeIndicator*)mem_heap_malloc(0x4Cu);
    if (mGrenadeIndicator[0] != nullptr)
        mGrenadeIndicator[0] =
            new (mGrenadeIndicator[0]) IGOGrenadeIndicator(0);
    else
        mGrenadeIndicator[0] = nullptr;

    mTankReticleWidget[0] = (IGOTankReticleWidget*)mem_heap_malloc(0x4Cu);
    if (mTankReticleWidget[0] != nullptr)
        mTankReticleWidget[0] =
            new (mTankReticleWidget[0]) IGOTankReticleWidget(0);
    else
        mTankReticleWidget[0] = nullptr;

    tankLoadingWidget[0] = (IGOTankLoadingWidget*)mem_heap_malloc(0x18u);
    if (tankLoadingWidget[0] != nullptr)
        tankLoadingWidget[0] =
            new (tankLoadingWidget[0]) IGOTankLoadingWidget(0);
    else
        tankLoadingWidget[0] = nullptr;

    tankIconWidget[0] = (IGOTankIconWidget*)mem_heap_malloc(0x9Cu);
    if (tankIconWidget[0] != nullptr)
        tankIconWidget[0] = new (tankIconWidget[0]) IGOTankIconWidget(0);
    else
        tankIconWidget[0] = nullptr;

    mTimerWidget[0] = (IGOTimerWidget*)mem_heap_malloc(0x24u);
    if (mTimerWidget[0] != nullptr)
        mTimerWidget[0] = new (mTimerWidget[0]) IGOTimerWidget(0);
    else
        mTimerWidget[0] = nullptr;

    mGameScoreWidget[0] = (IGOInGameScoreWidget*)mem_heap_malloc(0x28u);
    if (mGameScoreWidget[0] != nullptr)
        mGameScoreWidget[0] =
            new (mGameScoreWidget[0]) IGOInGameScoreWidget(0);
    else
        mGameScoreWidget[0] = nullptr;

    mRankWidget[0] = (IGORankWidget*)mem_heap_malloc(0x20u);
    if (mRankWidget[0] != nullptr)
        mRankWidget[0] = new (mRankWidget[0]) IGORankWidget(0);
    else
        mRankWidget[0] = nullptr;

    mVoteWidget[0] = (IGOVoteWidget*)mem_heap_malloc(0x10u);
    if (mVoteWidget[0] != nullptr)
        mVoteWidget[0] = new (mVoteWidget[0]) IGOVoteWidget(0);
    else
        mVoteWidget[0] = nullptr;

    mSpecialWeaponWidget[0] = (IGOSpecialWeaponWidget*)mem_heap_malloc(0x28u);
    if (mSpecialWeaponWidget[0] != nullptr)
        mSpecialWeaponWidget[0] =
            new (mSpecialWeaponWidget[0]) IGOSpecialWeaponWidget(0);
    else
        mSpecialWeaponWidget[0] = nullptr;

    mWarStatusWidget[0] = (IGOWarStatusWidget*)mem_heap_malloc(0xA4u);
    if (mWarStatusWidget[0] != nullptr)
        mWarStatusWidget[0] =
            new (mWarStatusWidget[0]) IGOWarStatusWidget(0);
    else
        mWarStatusWidget[0] = nullptr;

    mRaiseFlagWidget[0] = (IGORaiseFlagWidget*)mem_heap_malloc(0x18u);
    if (mRaiseFlagWidget[0] != nullptr)
        mRaiseFlagWidget[0] =
            new (mRaiseFlagWidget[0]) IGORaiseFlagWidget(0);
    else
        mRaiseFlagWidget[0] = nullptr;

    mHQProgressBarWidget[0] = (IGOHQProgressBarWidget*)mem_heap_malloc(0x28u);
    if (mHQProgressBarWidget[0] != nullptr)
        mHQProgressBarWidget[0] =
            new (mHQProgressBarWidget[0]) IGOHQProgressBarWidget(0);
    else
        mHQProgressBarWidget[0] = nullptr;

    mHeadIcons[0] = (IGOHeadIcons*)mem_heap_malloc(0x10Cu);
    if (mHeadIcons[0] != nullptr)
        mHeadIcons[0] = new (mHeadIcons[0]) IGOHeadIcons(0);
    else
        mHeadIcons[0] = nullptr;

    mItemIcons[0] = (IGOItemIcons*)mem_heap_malloc(0x1Cu);
    if (mItemIcons[0] != nullptr)
        mItemIcons[0] = new (mItemIcons[0]) IGOItemIcons(0);
    else
        mItemIcons[0] = nullptr;

    mVoipList[0] = (IGOVoipList*)mem_heap_malloc(0xB8u);
    if (mVoipList[0] != nullptr)
        mVoipList[0] = new (mVoipList[0]) IGOVoipList(0);
    else
        mVoipList[0] = nullptr;

    actionHintWidget[0] = (IGOActionHintWidget*)mem_heap_malloc(0x20u);
    if (actionHintWidget[0] != nullptr)
        actionHintWidget[0] =
            new (actionHintWidget[0]) IGOActionHintWidget(0);
    else
        actionHintWidget[0] = nullptr;

    tankHealthWidget = (IGOTankHealthWidget*)mem_heap_malloc(0x20u);
    if (tankHealthWidget != nullptr)
        tankHealthWidget = new (tankHealthWidget) IGOTankHealthWidget(0);
    else
        tankHealthWidget = nullptr;

    panel = nullptr;
    iconsPanel = nullptr;
    mpPanel = nullptr;
    activate_key = (char*)mem_heap_malloc(8u);
    activate_key[0] = 0;
    run_key = (char*)mem_heap_malloc(8u);
    run_key[0] = 0;
    speed_key = (char*)mem_heap_malloc(8u);
    speed_key[0] = 0;
    key_bindings_set = false;
    previous_splitscreen = 0;
    previous_widescreen = 0;
    actionHintTimer[0] = -1;
    current_type[0] = HUD_TYPE_NORMAL;
    hintTimer[0] = -1.0f;
}

// ea: 0x00564F70
IGOFrontEnd::~IGOFrontEnd()
{
    if (compassWidget[0] != nullptr)
        delete compassWidget[0];
    if (hintWidget[0] != nullptr)
        delete hintWidget[0];
    if (jeepMapWidget[0] != nullptr)
        delete jeepMapWidget[0];
    if (stanceWidget[0] != nullptr)
        delete stanceWidget[0];
    if (healthWidget[0] != nullptr)
        delete healthWidget[0];
    if (ammoWidget[0] != nullptr)
        delete ammoWidget[0];
    if (weaponNameWidget[0] != nullptr)
        delete weaponNameWidget[0];
    if (grenadeWidget[0] != nullptr)
        delete grenadeWidget[0];
    if (grenadeCookWidget[0] != nullptr)
        delete grenadeCookWidget[0];
    if (hintText[0] != nullptr)
        delete hintText[0];
    if (tankLoadingWidget[0] != nullptr)
        delete tankLoadingWidget[0];
    if (tankIconWidget[0] != nullptr)
        delete tankIconWidget[0];
    if (mTankReticleWidget[0] != nullptr)
        delete mTankReticleWidget[0];
    if (mGrenadeIndicator[0] != nullptr)
        delete mGrenadeIndicator[0];
    if (mTimerWidget[0] != nullptr)
        delete mTimerWidget[0];
    if (mGameScoreWidget[0] != nullptr)
        delete mGameScoreWidget[0];
    if (mRankWidget[0] != nullptr)
        delete mRankWidget[0];
    if (mVoteWidget[0] != nullptr)
        delete mVoteWidget[0];
    if (mSpecialWeaponWidget[0] != nullptr)
        delete mSpecialWeaponWidget[0];
    if (mRaiseFlagWidget[0] != nullptr)
        delete mRaiseFlagWidget[0];
    if (mHQProgressBarWidget[0] != nullptr)
        delete mHQProgressBarWidget[0];
    if (mWarStatusWidget[0] != nullptr)
        delete mWarStatusWidget[0];
    if (mHeadIcons[0] != nullptr)
        delete mHeadIcons[0];
    if (mItemIcons[0] != nullptr)
        delete mItemIcons[0];
    if (mVoipList[0] != nullptr)
        delete mVoipList[0];
    if (actionHintWidget[0] != nullptr)
        delete actionHintWidget[0];
    if (tankHealthWidget != nullptr)
        delete tankHealthWidget;
    mem_heap_free(activate_key);
    mem_heap_free(run_key);
    mem_heap_free(speed_key);
}

// ea: 0x00565110
void IGOFrontEnd::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    panel = nullptr;
    if (hintText[0] != nullptr)
        delete hintText[0];
    hintText[0] = nullptr;
}

// ea: 0x00565140
void IGOFrontEnd::UpdateInScene(float time_inc)
{
    if (mHeadIcons[currCl] != nullptr)
        mHeadIcons[currCl]->Update(time_inc);
    if (mItemIcons[currCl] != nullptr)
        mItemIcons[currCl]->Update(time_inc);
}

// ea: 0x00565180
void IGOFrontEnd::SetTutorialText(int ref, int viewport)
{
    if (viewport != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOFrontEnd.cpp";
        AeAssert::gCurrentLine = 361;
        AeAssert::gCurrentExpr = "viewport >= 0 && viewport < 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "SetTutorialText called for a non local player"))
            __debugbreak();
    }
    if (viewport == 0)
    {
        if (ref == -1)
        {
            hintText[0]->SetTextNoLocalize(defaultFileName);
            hintTimer[0] = -1.0f;
        }
        else
        {
            const char* STBString =
                STBManager::sInst->GetSTBString((unsigned int)ref);
            if (STBString != nullptr)
                hintText[0]->SetTextBox(STBString, 400, -1082130432);
            else
                hintText[0]->SetText("STRING NOT FOUND!");
            hintTimer[0] = 0.0f;
        }
    }
}

// ea: 0x00565260
void IGOFrontEnd::SetActionHint(int ref, int viewport)
{
    if (viewport != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOFrontEnd.cpp";
        AeAssert::gCurrentLine = 387;
        AeAssert::gCurrentExpr = "viewport >= 0 && viewport < 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "SetActionHint called for a non local player"))
            __debugbreak();
    }
    if (viewport == 0)
    {
        if (ref == -1)
        {
            actionHintText[0] = 0;
            actionHintTimer[0] = -1;
        }
        else
        {
            actionHintText[0] = ref;
            actionHintTimer[0] = level.time;
        }
    }
}

// ea: 0x00565300
void IGOFrontEnd::DrawHint(int viewport)
{
    if (dword_F64198[1580 * currCl] == 0 && hintTimer[viewport] > -1.0f)
    {
        if (hintText[viewport] != nullptr)
            hintText[viewport]->Draw();
    }
}

// ea: 0x00565350
void IGOFrontEnd::Draw(int client)
{
    if (gRenderCG_2D == 0)
        return;
    if (g_femanager.mDontDrawHud)
        return;
    if (EntityManager::sInst->GetPlayer(client) != nullptr
        && EntityManager::sInst->GetPlayer(client)->client != nullptr)
    {
        Entity* Player = EntityManager::sInst->GetPlayer(client);
        if (!IsPlayerFullySeatedInVehicle(Player))
            return;
    }
    nglListBeginScene(NGLSCENE_DEFAULTS);
    nglSetClearFlags(3u);
    View::SetViewportClipping(client);
    if (compassWidget[currCl] != nullptr)
    {
        if (compassWidget[currCl]->IsShown())
            compassWidget[currCl]->Draw();
    }
    if (gCvarShowVehMap.integer == 1)
    {
        if (jeepMapWidget[currCl] != nullptr
            && jeepMapWidget[currCl]->IsShown())
            jeepMapWidget[currCl]->Draw();
    }
    else if (stanceWidget[currCl] != nullptr
             && stanceWidget[currCl]->IsShown())
    {
        stanceWidget[currCl]->Draw();
    }
    if (healthWidget[currCl] != nullptr && healthWidget[currCl]->IsShown())
        healthWidget[currCl]->Draw();
    if (ammoWidget[currCl] != nullptr && ammoWidget[currCl]->IsShown())
        ammoWidget[currCl]->Draw();
    if (weaponNameWidget[currCl] != nullptr
        && weaponNameWidget[currCl]->IsShown())
        weaponNameWidget[currCl]->Draw();
    if (hintWidget[currCl] != nullptr && hintWidget[currCl]->IsShown())
        hintWidget[currCl]->Draw();
    if (grenadeWidget[currCl] != nullptr && grenadeWidget[currCl]->IsShown())
        grenadeWidget[currCl]->Draw();
    if (tankIconWidget[currCl] != nullptr
        && tankIconWidget[currCl]->IsShown())
        tankIconWidget[currCl]->Draw();
    if (grenadeCookWidget[currCl] != nullptr
        && grenadeCookWidget[currCl]->IsShown())
        grenadeCookWidget[currCl]->Draw();
    if (mGrenadeIndicator[currCl] != nullptr)
        mGrenadeIndicator[currCl]->Draw();
    if (mTankReticleWidget[currCl] != nullptr)
        mTankReticleWidget[currCl]->Draw();
    if (mTimerWidget[currCl] != nullptr)
        mTimerWidget[currCl]->Draw();
    if (mGameScoreWidget[currCl] != nullptr)
        mGameScoreWidget[currCl]->Draw();
    if (mRankWidget[currCl] != nullptr)
        mRankWidget[currCl]->Draw();
    if (mVoteWidget[currCl] != nullptr)
        mVoteWidget[currCl]->Draw();
    if (mSpecialWeaponWidget[currCl] != nullptr)
        mSpecialWeaponWidget[currCl]->Draw();
    if (tankLoadingWidget[currCl] != nullptr)
        tankLoadingWidget[currCl]->Draw();
    if (mRaiseFlagWidget[currCl] != nullptr)
        mRaiseFlagWidget[currCl]->Draw();
    if (mHQProgressBarWidget[currCl] != nullptr)
        mHQProgressBarWidget[currCl]->Draw();
    if (mWarStatusWidget[currCl] != nullptr)
        mWarStatusWidget[currCl]->Draw();
    if (mVoipList[currCl] != nullptr)
        mVoipList[currCl]->Draw();
    if (actionHintWidget[currCl] != nullptr)
        actionHintWidget[currCl]->Draw();
    nglListEndScene();
}

// ea: 0x00565660
void IGOFrontEnd::Draw3DWorldSpace()
{
    if (mHeadIcons[currCl] != nullptr)
        mHeadIcons[currCl]->Draw();
    if (mItemIcons[currCl] != nullptr)
        mItemIcons[currCl]->Draw();
}

// ea: 0x005771D0
void IGOFrontEnd::Draw3DScreenSpace()
{
    IGOCompassWidget* v2 = compassWidget[currCl];
    if (v2 != nullptr && v2->IsShown())
        compassWidget[currCl]->Draw3DObjectiveLocations();
}

// ea: 0x00565690
void IGOFrontEnd::UpdateSplitScreen()
{
    int v1 = unk_F6A284[0];
    int v2 = unk_F6A280[0];
    if (unk_F6A284[0] == unk_F6A280[0])
        return;
    if (hintText[0] != nullptr)
        hintText[0]->UpdateForSplitScreen(v1, v2);
    if (hintWidget[0] != nullptr)
        hintWidget[0]->UpdateSplitScreen(v1, v2);
    if (mTimerWidget[0] != nullptr)
        mTimerWidget[0]->UpdateSplitScreen(v1, v2);
    if (mGameScoreWidget[0] != nullptr)
        mGameScoreWidget[0]->UpdateSplitScreen(v1, v2);
    if (compassWidget[0] != nullptr)
        compassWidget[0]->UpdateSplitScreen(v1, v2);
    if (stanceWidget[0] != nullptr)
        stanceWidget[0]->UpdateSplitScreen(v1, v2);
    if (ammoWidget[0] != nullptr)
        ammoWidget[0]->UpdateSplitScreen(v1, v2);
    if (mRaiseFlagWidget[0] != nullptr)
        mRaiseFlagWidget[0]->UpdateSplitScreen(v1, v2);
    if (mHQProgressBarWidget[0] != nullptr)
        mHQProgressBarWidget[0]->UpdateSplitScreen(v1, v2);
    if (mWarStatusWidget[0] != nullptr)
        mWarStatusWidget[0]->UpdateSplitScreen(v1, v2);
    if (mVoipList[0] != nullptr)
        mVoipList[0]->UpdateSplitScreen(v1, v2);
    if (healthWidget[0] != nullptr)
        healthWidget[0]->UpdateSplitScreen(v1, v2);
    if (weaponNameWidget[0] != nullptr)
        weaponNameWidget[0]->UpdateSplitScreen(v1, v2);
    if (grenadeWidget[0] != nullptr)
        grenadeWidget[0]->UpdateSplitScreen(v1, v2);
    if (grenadeCookWidget[0] != nullptr)
        grenadeCookWidget[0]->UpdateSplitScreen(v1, v2);
    if (mRankWidget[0] != nullptr)
        mRankWidget[0]->UpdateSplitScreen(v1, v2);
    if (mVoteWidget[0] != nullptr)
        mVoteWidget[0]->UpdateSplitScreen(v1, v2);
    if (mSpecialWeaponWidget[0] != nullptr)
        mSpecialWeaponWidget[0]->UpdateSplitScreen(v1, v2);
    if (mHeadIcons[0] != nullptr)
        mHeadIcons[0]->UpdateSplitScreen(v1, v2);
    if (mItemIcons[0] != nullptr)
        mItemIcons[0]->UpdateSplitScreen(v1, v2);
    if (mTankReticleWidget[0] != nullptr)
        mTankReticleWidget[0]->UpdateSplitScreen(v1, v2);
    if (mGrenadeIndicator[0] != nullptr)
        mGrenadeIndicator[0]->UpdateSplitScreen(v1, v2);
    if (tankIconWidget[0] != nullptr)
        tankIconWidget[0]->UpdateSplitScreen(v1, v2);
    if (tankLoadingWidget[0] != nullptr)
        tankLoadingWidget[0]->UpdateSplitScreen(v1, v2);
    if (actionHintWidget[0] != nullptr)
        actionHintWidget[0]->UpdateSplitScreen(v1, v2);
}

// ea: 0x00565920
void IGOFrontEnd::ResetWidgets()
{
    compassWidget[0]->SetShown(true);
    compassWidget[0]->DrawObjectivesOnly = false;
    hintWidget[0]->SetShown(true);
    jeepMapWidget[0]->SetShown(true);
    stanceWidget[0]->SetShown(true);
    healthWidget[0]->SetShown(true);
    ammoWidget[0]->SetShown(true);
    weaponNameWidget[0]->SetShown(true);
    grenadeWidget[0]->SetShown(true);
    grenadeCookWidget[0]->SetShown(true);
    tankLoadingWidget[0]->SetShown(false);
    tankIconWidget[0]->SetShown(false);
    mTankReticleWidget[0]->SetShown(true);
    mTimerWidget[0]->SetShown(true);
    mGameScoreWidget[0]->SetShown(true);
    mRankWidget[0]->SetShown(true);
    mVoteWidget[0]->SetShown(true);
    mSpecialWeaponWidget[0]->SetShown(true);
    mHQProgressBarWidget[0]->SetShown(true);
    mRaiseFlagWidget[0]->SetShown(true);
    mWarStatusWidget[0]->SetShown(true);
    mVoipList[0]->SetShown(true);
    mHeadIcons[0]->SetShown(true);
    mItemIcons[0]->SetShown(true);
    tankIconWidget[0]->SetShown(true);
    tankLoadingWidget[0]->SetShown(true);
    actionHintWidget[0]->SetShown(true);
    tankHealthWidget->SetShown(false);
}

// ea: 0x00565A40
void IGOFrontEnd::TurnOffMostWidgets()
{
    compassWidget[0]->SetShown(false);
    compassWidget[0]->DrawObjectivesOnly = false;
    jeepMapWidget[0]->SetShown(false);
    stanceWidget[0]->SetShown(false);
    healthWidget[0]->SetShown(false);
    ammoWidget[0]->SetShown(false);
    weaponNameWidget[0]->SetShown(false);
    grenadeWidget[0]->SetShown(false);
    grenadeCookWidget[0]->SetShown(false);
    hintWidget[0]->SetShown(false);
    tankLoadingWidget[0]->SetShown(false);
    tankIconWidget[0]->SetShown(false);
    mTankReticleWidget[0]->SetShown(false);
    mTimerWidget[0]->SetShown(false);
    mGameScoreWidget[0]->SetShown(false);
    mRankWidget[0]->SetShown(false);
    mSpecialWeaponWidget[0]->SetShown(false);
    mRaiseFlagWidget[0]->SetShown(false);
    mHQProgressBarWidget[0]->SetShown(false);
    mWarStatusWidget[0]->SetShown(false);
    mVoipList[0]->SetShown(false);
    mVoteWidget[0]->SetShown(false);
    mHeadIcons[0]->SetShown(false);
    mItemIcons[0]->SetShown(false);
    actionHintWidget[0]->SetShown(false);
    tankHealthWidget->SetShown(false);
}

// ea: 0x00565B50
void IGOFrontEnd::SetForLiberatorBomber()
{
    TurnOffMostWidgets();
}

// ea: 0x00565B60
void IGOFrontEnd::SetForLiberatorGround()
{
    compassWidget[0]->SetShown(true);
    compassWidget[0]->DrawObjectivesOnly = true;
    hintWidget[0]->SetShown(true);
    jeepMapWidget[0]->SetShown(true);
    stanceWidget[0]->SetShown(true);
    healthWidget[0]->SetShown(true);
    ammoWidget[0]->SetShown(false);
    weaponNameWidget[0]->SetShown(false);
    grenadeWidget[0]->SetShown(false);
    grenadeCookWidget[0]->SetShown(false);
    tankLoadingWidget[0]->SetShown(false);
    tankIconWidget[0]->SetShown(false);
    tankHealthWidget->SetShown(false);
}

// ea: 0x00565BF0
void IGOFrontEnd::SetForTunisia()
{
    ResetWidgets();
    stanceWidget[0]->SetShown(false);
    healthWidget[0]->SetShown(false);
    grenadeWidget[0]->SetShown(false);
    grenadeCookWidget[0]->SetShown(false);
    tankLoadingWidget[0]->SetShown(true);
    tankIconWidget[0]->SetShown(true);
    tankHealthWidget->SetShown(true);
}

// ea: 0x00565C40
void IGOFrontEnd::SetHUDType(hud_type ht, int viewport)
{
    if (ht != current_type[viewport])
    {
        current_type[viewport] = ht;
        if (ht == HUD_TYPE_NORMAL)
        {
            ResetWidgets();
        }
        else if (ht == HUD_TYPE_TUNISIA)
        {
            SetForTunisia();
        }
    }
}

// ea: 0x00577200
void IGOFrontEnd::UpdateWidescreen(bool widescreen)
{
    if (compassWidget[0] != nullptr)
        compassWidget[0]->UpdateWidescreen(widescreen, 50.0f);
    if (jeepMapWidget[0] != nullptr)
        jeepMapWidget[0]->UpdateWidescreen(widescreen, 50.0f);
    if (stanceWidget[0] != nullptr)
        stanceWidget[0]->UpdateWidescreen(widescreen, 50.0f);
    if (healthWidget[0] != nullptr)
        healthWidget[0]->UpdateWidescreen(widescreen, 512.0f);
    if (ammoWidget[0] != nullptr)
        ammoWidget[0]->UpdateWidescreen(widescreen, 512.0f);
    if (weaponNameWidget[0] != nullptr)
        weaponNameWidget[0]->UpdateWidescreen(widescreen, 512.0f);
    if (hintWidget[0] != nullptr)
        hintWidget[0]->UpdateWidescreen(widescreen, 320.0f);
    if (tankHealthWidget != nullptr)
        tankHealthWidget->UpdateWidescreen(widescreen, 512.0f);
    if (tankLoadingWidget[0] != nullptr)
        tankLoadingWidget[0]->UpdateWidescreen(widescreen, 512.0f);
    if (grenadeWidget[0] != nullptr)
        grenadeWidget[0]->UpdateWidescreen(widescreen, 512.0f);
    if (grenadeCookWidget[0] != nullptr)
        grenadeCookWidget[0]->UpdateWidescreen(widescreen, 320.0f);
    if (tankIconWidget[0] != nullptr)
        tankIconWidget[0]->UpdateWidescreen(widescreen, 50.0f);
    if (mGrenadeIndicator[0] != nullptr)
        mGrenadeIndicator[0]->UpdateWidescreen(widescreen, 320.0f);
    if (mTankReticleWidget[0] != nullptr)
        mTankReticleWidget[0]->UpdateWidescreen(widescreen, 320.0f);
    if (mTimerWidget[0] != nullptr)
        mTimerWidget[0]->UpdateWidescreen(widescreen, 50.0f);
    if (mGameScoreWidget[0] != nullptr)
        mGameScoreWidget[0]->UpdateWidescreen(widescreen, 50.0f);
    if (mRankWidget[0] != nullptr)
        mRankWidget[0]->UpdateWidescreen(widescreen, 50.0f);
    if (mVoteWidget[0] != nullptr)
        mVoteWidget[0]->UpdateWidescreen(widescreen, 512.0f);
    if (mSpecialWeaponWidget[0] != nullptr)
        mSpecialWeaponWidget[0]->UpdateWidescreen(widescreen, 512.0f);
    if (mRaiseFlagWidget[0] != nullptr)
        mRaiseFlagWidget[0]->UpdateWidescreen(widescreen, 320.0f);
    if (mHQProgressBarWidget[0] != nullptr)
        mHQProgressBarWidget[0]->UpdateWidescreen(widescreen, 50.0f);
    if (mWarStatusWidget[0] != nullptr)
        mWarStatusWidget[0]->UpdateWidescreen(widescreen, 320.0f);
    if (mVoipList[0] != nullptr)
        mVoipList[0]->UpdateWidescreen(widescreen, 320.0f);
    if (mHeadIcons[0] != nullptr)
        mHeadIcons[0]->UpdateWidescreen(widescreen, 320.0f);
    if (mItemIcons[0] != nullptr)
        mItemIcons[0]->UpdateWidescreen(widescreen, 320.0f);
    if (actionHintWidget[0] != nullptr)
        actionHintWidget[0]->UpdateWidescreen(widescreen, 320.0f);
}

// ea: 0x00577420
void IGOFrontEnd::FindKeyBindings()
{
    for (int i = 0; i < 256; ++i)
    {
        const char* mBoundCmdName =
            KeyInfo::mKeys.m_elements[currCl].m_elements[i].mBoundCmdName;
        if (activate_key[0] == 0 && Q_stricmp(mBoundCmdName, "+activate") == 0)
        {
            Q_strncpyz(activate_key, Key_KeynumToString(i, 1), 8);
        }
        else if (run_key[0] == 0
                 && Q_stricmp(mBoundCmdName, "toggle cl_run") == 0)
        {
            Q_strncpyz(run_key, Key_KeynumToString(i, 1), 8);
        }
        else if (speed_key[0] == 0
                 && Q_stricmp(mBoundCmdName, "+speed") == 0)
        {
            Q_strncpyz(speed_key, Key_KeynumToString(i, 1), 8);
        }
    }
    key_bindings_set = true;
}

// ea: 0x005775A0
void IGOFrontEnd::SetFuse(float total, float remain, int client)
{
    IGOGrenadeCookWidget* v4 = grenadeCookWidget[client];
    if (v4 != nullptr)
    {
        v4->fuseRemaining = remain;
        v4->fuseTotal = total;
    }
}

// ea: 0x005821F0
void IGOFrontEnd::Update(float time_inc)
{
    if (!key_bindings_set)
        FindKeyBindings();
    if (compassWidget[currCl] != nullptr)
        compassWidget[currCl]->Update(time_inc);
    if (gCvarShowVehMap.integer != 1)
    {
        if (stanceWidget[currCl] != nullptr)
            stanceWidget[currCl]->Update(time_inc);
        jeepMapWidget[currCl]->mTextureSetted = false;
    }
    else
    {
        if (!jeepMapWidget[currCl]->mTextureSetted)
        {
            jeepMapWidget[currCl]->mTextureSetted = true;
            int levelIndex = -1;
            char mapname[64];
            Cvar_VariableStringBuffer("mapname", mapname, 64);
            if (strcmp(mapname, "nightdrop") == 0)
                levelIndex = 0;
            else if (strcmp(mapname, "hostage") == 0)
                levelIndex = 1;
            else if (strcmp(mapname, "fuelplant") == 0)
                levelIndex = 2;
            jeepMapWidget[currCl]->SetLevelMap(levelIndex, (TPakId)-1);
        }
        if (jeepMapWidget[currCl] != nullptr)
            jeepMapWidget[currCl]->Update(time_inc);
    }
    if (healthWidget[currCl] != nullptr)
        healthWidget[currCl]->Update(time_inc);
    if (ammoWidget[currCl] != nullptr)
        ammoWidget[currCl]->Update(time_inc);
    if (weaponNameWidget[currCl] != nullptr)
        weaponNameWidget[currCl]->Update(time_inc);
    if (hintWidget[currCl] != nullptr)
        hintWidget[currCl]->Update(time_inc);
    if (grenadeWidget[currCl] != nullptr)
        grenadeWidget[currCl]->Update(time_inc);
    if (grenadeCookWidget[currCl] != nullptr)
        grenadeCookWidget[currCl]->Update(time_inc);
    if (tankIconWidget[currCl] != nullptr)
        tankIconWidget[currCl]->Update(time_inc);
    if (previous_widescreen != cg_widescreen.integer)
    {
        UpdateWidescreen(cg_widescreen.integer != 0);
        previous_widescreen = cg_widescreen.integer;
    }
    if (hintTimer[currCl] > -1.0f)
    {
        hintTimer[currCl] += time_inc;
        if (hintTimer[currCl] >= 8.0f)
            hintTimer[currCl] = -1.0f;
    }
    if (current_type[currCl] == HUD_TYPE_TUNISIA)
    {
        if (tankHealthWidget != nullptr)
            tankHealthWidget->Update(time_inc);
        if (tankLoadingWidget[currCl] != nullptr)
            tankLoadingWidget[currCl]->Update(time_inc);
    }
    if (mTankReticleWidget[currCl] != nullptr)
        mTankReticleWidget[currCl]->Update(time_inc);
    if (mGrenadeIndicator[currCl] != nullptr)
        mGrenadeIndicator[currCl]->Update(time_inc);
    if (mTimerWidget[currCl] != nullptr)
        mTimerWidget[currCl]->Update(time_inc);
    if (mGameScoreWidget[currCl] != nullptr)
        mGameScoreWidget[currCl]->Update(time_inc);
    if (mRankWidget[currCl] != nullptr)
        mRankWidget[currCl]->Update(time_inc);
    if (mVoteWidget[currCl] != nullptr)
        mVoteWidget[currCl]->Update(time_inc);
    if (mSpecialWeaponWidget[currCl] != nullptr)
        mSpecialWeaponWidget[currCl]->Update(time_inc);
    if (tankLoadingWidget[currCl] != nullptr)
        tankLoadingWidget[currCl]->Update(time_inc);
    if (mRaiseFlagWidget[currCl] != nullptr)
        mRaiseFlagWidget[currCl]->Update(time_inc);
    if (mHQProgressBarWidget[currCl] != nullptr)
        mHQProgressBarWidget[currCl]->Update(time_inc);
    if (mWarStatusWidget[currCl] != nullptr)
        mWarStatusWidget[currCl]->Update(time_inc);
    if (actionHintWidget[currCl] != nullptr)
        actionHintWidget[currCl]->Update(time_inc);
    if (mVoipList[currCl] != nullptr)
        mVoipList[currCl]->Update(time_inc);
}

// ea: 0x00590AF0
void IGOFrontEnd::AddActiveGrenade(const Entity* grenade)
{
    mGrenadeIndicator[0]->AddActiveGrenade(grenade);
}

// ea: 0x0059C0F0
void IGOFrontEnd::SetPanelFile(PanelFile* pf)
{
    bool icon_panel = false;
    if (_stricmp(pf->mName, "hud_menu.panel") == 0)
    {
        panel = pf;
        pf->PostUnmashFixup(PANEL_LAYER_IGO);
    }
    else if (_stricmp(pf->mName, "hud_icons.panel") == 0)
    {
        iconsPanel = pf;
        pf->PostUnmashFixup(PANEL_LAYER_COMPASS_ICONS);
        icon_panel = true;
    }
    else if (_stricmp(pf->mName, "hud_mp.panel") == 0)
    {
        mpPanel = pf;
        pf->PostUnmashFixup(PANEL_LAYER_IGO);
        if (tankIconWidget[0] != nullptr)
            tankIconWidget[0]->Init(pf);
        if (hintWidget[0] != nullptr)
            hintWidget[0]->Init(pf);
        if (grenadeWidget[0] != nullptr)
            grenadeWidget[0]->Init(pf);
        if (mTimerWidget[0] != nullptr)
            mTimerWidget[0]->Init(pf);
        if (mGameScoreWidget[0] != nullptr)
            mGameScoreWidget[0]->Init(pf);
        if (mHQProgressBarWidget[0] != nullptr)
            mHQProgressBarWidget[0]->Init(pf);
        if (mRaiseFlagWidget[0] != nullptr)
            mRaiseFlagWidget[0]->Init(pf);
        if (mGrenadeIndicator[0] != nullptr)
            mGrenadeIndicator[0]->Init(pf);
        if (mVoipList[0] != nullptr)
            mVoipList[0]->Init(pf);
        return;
    }
    if (compassWidget[0] != nullptr)
        compassWidget[0]->Init(pf);
    if (icon_panel)
    {
        if (mHeadIcons[0] != nullptr)
            mHeadIcons[0]->Init(pf);
        if (mItemIcons[0] != nullptr)
            mItemIcons[0]->Init(pf);
    }
    else
    {
        if (stanceWidget[0] != nullptr)
            stanceWidget[0]->Init(pf);
        if (healthWidget[0] != nullptr)
            healthWidget[0]->Init(pf);
        if (ammoWidget[0] != nullptr)
            ammoWidget[0]->Init(pf);
        if (weaponNameWidget[0] != nullptr)
            weaponNameWidget[0]->Init(pf);
        if (grenadeCookWidget[0] != nullptr)
            grenadeCookWidget[0]->Init(pf);
        if (hintWidget[0] != nullptr)
            hintWidget[0]->Init(pf);
        if (tankLoadingWidget[0] != nullptr)
            tankLoadingWidget[0]->Init(pf);
        if (mTankReticleWidget[0] != nullptr)
            mTankReticleWidget[0]->Init(pf);
        if (mGrenadeIndicator[0] != nullptr)
            mGrenadeIndicator[0]->Init(pf);
        if (mRankWidget[0] != nullptr)
            mRankWidget[0]->Init(pf);
        if (mVoteWidget[0] != nullptr)
            mVoteWidget[0]->Init(panel);
        if (mSpecialWeaponWidget[0] != nullptr)
            mSpecialWeaponWidget[0]->Init(pf);
        if (mWarStatusWidget[0] != nullptr)
            mWarStatusWidget[0]->Init(pf);
        if (actionHintWidget[0] != nullptr)
            actionHintWidget[0]->Init(pf);
        if (tankHealthWidget != nullptr)
            tankHealthWidget->Init(pf);
        FEText* TextPointer = panel->GetTextPointer("HintString");
        if (hintText[0] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOFrontEnd.cpp";
            AeAssert::gCurrentLine = 239;
            AeAssert::gCurrentExpr = "!hintText[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("no!"))
                __debugbreak();
        }
        hintText[0] = (FEMultiLineText*)mem_heap_malloc(0xA8u);
        if (hintText[0] != nullptr)
        {
            color32 col = TextPointer->GetColor();
            hintText[0] = new (hintText[0]) FEMultiLineText(
                TextPointer->GetFont(), TextPointer->GetY(), 0.0f, 0,
                    (panel_layer)TextPointer->GetScaleX(), 0.0f, 0, 0,
                col);
        }
        hintText[0]->SetNumLines(2);
        hintText[0]->SetNoFlash(color32(-1));
    }
}

// ea: 0x0059C460
void IGOFrontEnd::UpdateAfterWeaponsLoaded()
{
    if (hintWidget[currCl] != nullptr)
        hintWidget[currCl]->SetWeaponsPQs(panel, iconsPanel);
}

// ea: 0x005AED10
char* IGOFrontEnd::GetActivateKey()
{
    return activate_key;
}

// ea: 0x005AED20
const char* IGOFrontEnd::GetLMGKey()
{
    const char* result = run_key;
    if (*result != 0)
        return speed_key;
    return result;
}

// ea: 0x005AED40
int IGOFrontEnd::GetActionHintTimer(int viewport)
{
    return actionHintTimer[viewport];
}

// ea: 0x005AED60
void IGOFrontEnd::SetActionHintTimer(int time, int viewport)
{
    actionHintTimer[viewport] = time;
}

// ea: 0x005AED80
int IGOFrontEnd::GetActionHintText(int viewport)
{
    return actionHintText[viewport];
}

// ea: 0x005AED20 (inline COMDAT)
// GetLMGKey is defined above with its shell.o address.

// ============================================================================
// IGOCompassWidget
// ============================================================================

IGOCompassWidget::IGOFriendly::IGOFriendly()
{
    draw = false;
}

IGOCompassWidget::IGOEnemy::IGOEnemy()
{
    draw = false;
}

// ea: 0x005783F0
IGOCompassWidget::IGOCompassWidget(int client)
{
    mClient = client;
    is_shown = true;
    force_appear = false;
    mDrawVehMap = false;
    compass = nullptr;
    pointer = nullptr;
    frame = nullptr;
    compass_speed = 0.0f;
    compass_yaw = 0.0f;
    DrawObjectivesOnly = false;
    memset(friendlies, 0, sizeof(friendlies));
    memset(gEnemies, 0, sizeof(gEnemies));
    memset(enemyTanks, 0, sizeof(enemyTanks));
    memset(tanks, 0, sizeof(tanks));
    memset(objectives, 0, sizeof(objectives));
    memset(objectiveIcons, 0, sizeof(objectiveIcons));
    memset(worldIcons, 0, sizeof(worldIcons));
    m_hideCompassStarActive = 0;
    m_hideCompassStarIndex = 0;
    m_hideUpdatedText = 0;
    m_hideUpdatedTextIndex = 0;
    global_alpha = g_compassFadeTime.value + g_compassSolidTime.value;
    draw_time = 1.0f;
    mViewport = 0;
    last_player_pos[0] = 0.0f;
    last_player_pos[1] = 0.0f;
    last_player_pos[2] = 0.0f;
    last_player_angles[0] = 0.0f;
    last_player_angles[1] = 0.0f;
    last_player_angles[2] = 0.0f;
}

// ea: 0x00567CF0
IGOCompassWidget::~IGOCompassWidget()
{
}

// ea: 0x00599FA0
void IGOCompassWidget::Init(PanelFile* panel, bool bIconPanel)
{
    if (bIconPanel)
    {
        float u[4] = {0.0f, 1.0f, 0.0f, 1.0f};
        float v[4] = {0.0f, 0.0f, 1.0f, 1.0f};
        for (int i = 0; i < 27; ++i)
        {
            PanelQuad* Pointer = panel->GetPointer(sObjectiveIconNames[i]);
            objectiveIcons[i] = Pointer;
            if (i == 25 || i == 26)
                objectiveIcons[i] = PanelQuad::Clone(Pointer);
            PanelQuad* v8 = panel->GetPointer(sWorldIconNames[i]);
            worldIcons[i].icon = v8;
            if (g_femanager.GetDefaultPQ() != v8)
            {
                v8->quadBlendModeType = 1691321856;
                worldIcons[i].icon->SetSectionUV(0, u, v);
                worldIcons[i].height = (uint8_t)worldIcons[i].icon->GetWidth();
                worldIcons[i].alpha = worldIcons[i].icon->GetColor().c.a;
            }
        }
    }
    else
    {
        compass = panel->GetPointer("compass");
        pointer = panel->GetPointer("compassneedle");
        frame = panel->GetPointer("compassglass");
        frame->SetZvalueAbs(frame->GetZvalue() + 20.0f);
    }
    DrawObjectivesOnly = false;
    mDrawVehMap = false;
    for (int j = 0; j < 25; ++j)
        tanks[j].draw = false;
    if (mClient > 0)
    {
        compass = PanelQuad::Clone(compass);
        pointer = PanelQuad::Clone(pointer);
        frame = PanelQuad::Clone(frame);
        if (bIconPanel)
        {
            for (int k = 0; k < 27; ++k)
            {
                objectiveIcons[k] = PanelQuad::Clone(objectiveIcons[k]);
                worldIcons[k].icon = PanelQuad::Clone(worldIcons[k].icon);
            }
        }
    }
    if (bIconPanel)
    {
        for (int m = 0; m < 5; ++m)
        {
            if (objectiveIcons[sObjectiveIconRotateable[m]] != nullptr)
                objectiveIcons[sObjectiveIconRotateable[m]]
                    ->SetXYInitialToCurrentPos();
        }
    }
    else
    {
        compass->SetXYInitialToCurrentPos();
        pointer->SetXYInitialToCurrentPos();
        frame->SetXYInitialToCurrentPos();
    }
    global_alpha = 1.0f;
    draw_time = 1.0f;
    last_player_pos[0] = 0.0f;
    last_player_pos[1] = 0.0f;
    last_player_pos[2] = 0.0f;
    last_player_angles[0] = 0.0f;
    last_player_angles[1] = 0.0f;
    last_player_angles[2] = 0.0f;
}

// ea: 0x00590BF0
void IGOCompassWidget::Update(float time_inc)
{
    if (!is_shown)
        return;
    float fade_time = g_compassFadeTime.value;
    float v3 = g_compassFadeTime.value + g_compassSolidTime.value;
    if (!force_appear
        && dword_F63C70[1580 * currCl] == last_player_pos[0]
        && dword_F63C74[1580 * currCl] == last_player_pos[1]
        && dword_F63C78[1580 * currCl] == last_player_pos[2])
    {
        if (GetPlayer(mClient) == nullptr
            || GetPlayer(mClient)->r.currentAngles.v.m128_f32[0]
                   != last_player_angles[0]
            || GetPlayer(mClient)->r.currentAngles.v.m128_f32[1]
                   != last_player_angles[1]
            || GetPlayer(mClient)->r.currentAngles.v.m128_f32[2]
                   != last_player_angles[2])
        {
            goto label_12;
        }
        v3 = draw_time - time_inc;
    }
    draw_time = v3;
label_12:
    force_appear = false;
    last_player_pos[0] = dword_F63C70[1580 * currCl];
    last_player_pos[1] = dword_F63C74[1580 * currCl];
    last_player_pos[2] = dword_F63C78[1580 * currCl];
    if (EntityManager::sInst->GetPlayer(mClient) != nullptr)
    {
        Entity* p = EntityManager::sInst->GetPlayer(mClient);
        last_player_angles[0] = p->r.currentAngles.v.m128_f32[0];
        last_player_angles[1] = p->r.currentAngles.v.m128_f32[1];
        last_player_angles[2] = p->r.currentAngles.v.m128_f32[2];
    }
    float v8 = 0.0f;
    if (draw_time >= 0.0f)
    {
        if (draw_time <= fade_time)
            v8 = draw_time / fade_time;
        else
            v8 = 1.0f;
    }
    bool DrawObjectivesOnly = this->DrawObjectivesOnly;
    global_alpha = v8;
    global_alpha = 1.0f;
    if (DrawObjectivesOnly)
    {
        if (gCvarShowVehMap.integer != 0)
            UpdateObjectivesABS();
        else
            UpdateObjectives();
    }
    else if (gCvarShowVehMap.integer != 0)
    {
        UpdateObjectivesABS();
        UpdateEnemiesABS();
        UpdateTanksABS();
    }
    else
    {
        UpdateCompassRotation();
        UpdateFriendlies();
        UpdateObjectives();
        UpdateEnemies();
        UpdateTanks();
    }
}

// ea: 0x00590E90
void IGOCompassWidget::Draw()
{
    if (is_shown
        && EntityManager::sInst->GetPlayer(mClient)->client->pers.playerState
               == 3)
    {
        if (DrawObjectivesOnly)
        {
            DrawObjectives();
        }
        else
        {
            if (gCvarShowVehMap.integer == 1)
            {
                DrawVehcile();
            }
            else
            {
                compass->SetAlpha(global_alpha);
                compass->Draw();
                pointer->SetAlpha(global_alpha);
                pointer->Draw();
                frame->SetAlpha(global_alpha);
                frame->Draw();
                DrawFriendlies();
            }
            DrawObjectives();
            DrawEnemies();
            DrawTanks();
        }
    }
}

// ea: 0x00567D00
void IGOCompassWidget::Draw3DObjective(int index)
{
    int worldState = objectives[index].worldState;
    float height = worldIcons[worldState].height;
    if (mp_objectiveSize.integer > 0)
        height = (float)mp_objectiveSize.integer;
    if (worldIcons[worldState].icon == g_femanager.default_pq || worldState >= 24)
        return;
    int base = 3208 * mClient + 176 * index;
    float* objBlock = &unk_F6A2B0[base];
    if (objBlock[0] < 0.0f)
        return;
    float d[3] = {objBlock[4], objBlock[5], objBlock[6]};
    if (sqrtf(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]) < 0.2f)
        return;
    Entity* Player = EntityManager::sInst->GetPlayer(mClient);
    float pos[3] = {objBlock[4], objBlock[5], objBlock[6] + objBlock[1]};
    math::Position3 in;
    in.v.m128_f32[0] = pos[0];
    in.v.m128_f32[1] = pos[1];
    in.v.m128_f32[2] = pos[2];
    in.v.m128_f32[3] = 0.0f;
    math::Position3 proj;
    nglProjectPoint(&proj, &in, nglBuildScene);
    if (proj.v.m128_f32[2] < 0.0f)
        return;
    int window = unk_F6A284[802 * mClient];
    float halfW = View::GetXScalingForHUD(window) * height * 0.5f;
    float fullH = View::GetYScalingForHUD(window) * height;
    float alpha = worldIcons[worldState].alpha;
    if (mp_objectiveFarAlpha.integer > 0)
        alpha = (float)mp_objectiveFarAlpha.integer;
    PanelQuad* icon = worldIcons[worldState].icon;
    icon->SetZvalueAbs(0.0f);
    icon->SetPos(proj.v.m128_f32[0] - halfW,
                 proj.v.m128_f32[1] - fullH,
                 proj.v.m128_f32[0] + halfW, proj.v.m128_f32[1]);
    color32 col;
    col.c.b = 255;
    col.c.g = 255;
    col.c.r = 255;
    col.c.a = (uint8_t)alpha;
    icon->SetColor(col);
    icon->Draw();
}

// ea: 0x00567F80
void IGOCompassWidget::Draw3DObjectiveLocations()
{
    if (EntityManager::sInst->GetPlayer(mClient)->client->pers.playerState
        == 0)
    {
        return;
    }
    for (int i = 0; i < 16; ++i)
    {
        if (objectives[i].draw
            && worldIcons[objectives[i].worldState].icon->IsShown())
            Draw3DObjective(i);
    }
}

// ea: 0x00567FE0
void IGOCompassWidget::UpdateCompassDial()
{
    if (compass == nullptr)
        return;
    compass->ResetToInitialXY();
    float x_pos = View::GetCurrentHUDXPos(compass->GetCenterX(), mViewport, 1,
                                          0.0f);
    float y_pos = View::GetCurrentHUDYPos(compass->GetCenterY(), mViewport, 1,
                                          0.0f);
    compass->Rotate(compass_yaw * 0.017453292f, true);
    compass->SetCenterPos(x_pos, y_pos);
    compass->Scale(x_pos, y_pos,
                   View::GetXScalingForHUD(mViewport),
                   View::GetYScalingForHUD(mViewport), true);
}

// ea: 0x005680D0
void IGOCompassWidget::UpdateCompassFrame()
{
    if (frame != nullptr)
    {
        frame->ResetToInitialXY();
        float x_pos = View::GetCurrentHUDXPos(frame->GetCenterX(), mViewport,
                                              1, 0.0f);
        float y_pos = View::GetCurrentHUDYPos(frame->GetCenterY(), mViewport,
                                              1, 0.0f);
        frame->SetCenterPos(x_pos, y_pos);
        frame->Scale(x_pos, y_pos, View::GetXScalingForHUD(mViewport),
                     View::GetYScalingForHUD(mViewport), true);
    }
    if (pointer != nullptr)
    {
        pointer->ResetToInitialXY();
        float x_pos = View::GetCurrentHUDXPos(pointer->GetCenterX(),
                                              mViewport, 1, 0.0f);
        float y_pos = View::GetCurrentHUDYPos(pointer->GetCenterY(),
                                              mViewport, 1, 0.0f);
        pointer->SetCenterPos(x_pos, y_pos);
        pointer->Scale(x_pos, y_pos, View::GetXScalingForHUD(mViewport),
                       View::GetYScalingForHUD(mViewport), true);
    }
}

// ea: 0x00568250
void IGOCompassWidget::DrawEnemies()
{
    if (gCvarShowEnemy.integer != 1)
        return;
    for (int i = 0; i < 32; ++i)
    {
        IGOEnemy& e = gEnemies[i];
        if (e.draw && e.alpha >= 0.00001f && (e.flags & 2) != 0)
        {
            PanelQuad* v3 = objectiveIcons[0];
            v3->ResetToInitialXY();
            float yaw = AngleNormalize360(dword_F63CB4[1580 * currCl]
                                          - e.last_yaw);
            v3->SetCenterPos(e.x, e.y);
            v3->Rotate(yaw * 3.1415927f * 0.0055555557f, true);
            v3->SetAlpha(global_alpha * e.alpha);
            v3->Draw();
        }
    }
}

// ea: 0x00568340
void IGOCompassWidget::DrawFriendlies()
{
    for (int i = 0; i < 32; ++i)
    {
        IGOFriendly& f = friendlies[i];
        if (f.draw)
        {
            PanelQuad* v3 = objectiveIcons[(f.flags & 0x20) != 0 ? 8 : 1];
            v3->SetZvalueAbs(15.0f);
            v3->ResetToInitialXY();
            float yaw = AngleNormalize360(dword_F63CB4[1580 * currCl]
                                          - f.last_yaw);
            v3->SetCenterPos(f.x, f.y);
            if ((f.flags & 0x20) == 0)
                v3->Rotate(yaw * 3.1415927f * 0.0055555557f, true);
            v3->SetAlpha(f.alpha * global_alpha);
            v3->Draw();
        }
    }
}

// ea: 0x00568440
void IGOCompassWidget::DrawTanks()
{
    for (int i = 0; i < 25; ++i)
    {
        IGOFriendly& t = tanks[i];
        if (t.draw)
        {
            int v3 = (t.flags & 2) != 0 ? 2 : 3;
            float yaw = AngleNormalize360(dword_F63CB4[1580 * currCl]
                                          - t.last_yaw);
            objectiveIcons[v3]->SetZvalueAbs(15.0f);
            objectiveIcons[v3]->ResetToInitialXY();
            objectiveIcons[v3]->SetCenterPos(t.x, t.y);
            objectiveIcons[v3]->Rotate(yaw * 3.1415927f * 0.0055555557f,
                                       true);
            objectiveIcons[v3]->SetAlpha(t.alpha * global_alpha);
            objectiveIcons[v3]->Draw();
        }
    }
}

// ea: 0x00568540
void IGOCompassWidget::DrawObjectives()
{
    for (int i = 0; i < 17; ++i)
    {
        IGOObjective& o = objectives[i];
        if (!o.draw)
            continue;
        PanelQuad* v4;
        if (o.up)
            v4 = objectiveIcons[o.state + 1];
        else if (o.down)
            v4 = objectiveIcons[o.state + 2];
        else
            v4 = objectiveIcons[o.state];
        if (v4 != nullptr)
        {
            v4->SetCenterPos(o.x, o.y);
            v4->SetAlpha(o.alpha * global_alpha);
            v4->Draw();
            v4->SetZvalueAbs(30.0f);
        }
        if (o.draw_ring)
        {
            objectiveIcons[4]->ResetToInitialXY();
            objectiveIcons[4]->SetAlpha(o.ring_alpha * global_alpha);
            objectiveIcons[4]->SetCenterPos(o.x, o.y);
            objectiveIcons[4]->Scale(o.ring_scale, true);
            objectiveIcons[4]->SetZvalueAbs(0.0f);
            objectiveIcons[4]->Draw();
        }
    }
}

// ea: 0x00568660
void IGOCompassWidget::CalculateRing(IGOMapObject* mo, float ring_time)
{
    IGOObjective* obj = (IGOObjective*)mo;
    obj->draw_ring = false;
    if (cg_hudObjectiveNumRings.integer <= 0
        || cg_hudObjectiveRingTime.integer <= 0 || ring_time <= -1.0f
        || cgGlobal.time >= cg_hudObjectiveRingTime.integer + (int)ring_time)
    {
        return;
    }
    float v4 = (float)cg_hudObjectiveRingTime.integer
               / (float)cg_hudObjectiveNumRings.integer;
    if (v4 <= 0.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOCompassWidget.cpp";
        AeAssert::gCurrentLine = 1358;
        AeAssert::gCurrentExpr = "ringLen > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (cgGlobal.time < (int)ring_time)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOCompassWidget.cpp";
        AeAssert::gCurrentLine = 1359;
        AeAssert::gCurrentExpr = "cgGlobal.time >= ring_time";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float time = (float)cgGlobal.time - ring_time;
    int v3 = 0;
    float i = time;
    while (i > v4)
    {
        i -= v4;
        ++v3;
    }
    if (i < 0.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOCompassWidget.cpp";
        AeAssert::gCurrentLine = 1369;
        AeAssert::gCurrentExpr = "time >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        i = time;
    }
    float v6 = i / v4;
    obj->ring_alpha = v6 <= 0.5f ? v6 * 2.0f : 1.0f - ((v6 - 0.5f) * 2.0f);
    obj->ring_scale = v6;
    if (v3 != 0)
        obj->ring_scale = v6 * 0.5f;
    obj->draw_ring = true;
}

// ea: 0x00568850
void IGOCompassWidget::SetHideCompassStar(int active, int index)
{
    m_hideCompassStarActive = active;
    m_hideCompassStarIndex = index;
}

// ea: 0x00568870
int IGOCompassWidget::IsCompassStarHidden(int index)
{
    return m_hideCompassStarActive != 0 && m_hideCompassStarIndex == index;
}

// ea: 0x005688A0
void IGOCompassWidget::SetHideUpdatedText(int active, int objectiveIndex)
{
    m_hideUpdatedText = active;
    m_hideUpdatedTextIndex = objectiveIndex;
}

// ea: 0x005688C0
int IGOCompassWidget::IsUpdatedTextHidden(int index)
{
    return m_hideUpdatedText != 0 && m_hideUpdatedTextIndex == index;
}

// ea: 0x005688F0
int IGOCompassWidget::ObjectiveStateIndexFromString(const char* name)
{
    if (name == nullptr || *name == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOCompassWidget.cpp";
        AeAssert::gCurrentLine = 1540;
        AeAssert::gCurrentExpr = "name && name[0]";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid objective name"))
            __debugbreak();
    }
    int v1 = 0;
    while (strcmp(sObjectiveIconNames[v1], name) != 0)
    {
        if (++v1 >= 27)
            return 27;
    }
    return v1;
}

// ea: 0x005689A0
void IGOCompassWidget::UpdateVehcile()
{
}

// ea: 0x00578580
void IGOCompassWidget::UpdateCompassRotation()
{
    float fTargetYaw =
        AngleNormalize360(dword_F63CB4[1580 * currCl]
                          - *(float*)&dword_F64140[1580 * currCl]);
    int v3 = iLastCompassTime[currCl];
    if (v3 > cgGlobal.time || (cgGlobal.time - v3) > 500)
    {
        iLastCompassTime[currCl] = cgGlobal.time;
    }
    else
    {
        int time = cgGlobal.time;
        iLastCompassTime[currCl] = cgGlobal.time;
        int v5 = time - v3;
        float fYawOffset = AngleSubtract(compass_yaw, fTargetYaw);
        float v6 = fYawOffset;
        if (v5 <= 0)
        {
            compass_yaw = AngleNormalize360(v6 + fTargetYaw);
            UpdateCompassDial();
            return;
        }
        while (1)
        {
            int v7;
            if (v5 <= 5)
            {
                v7 = v5;
                v5 = 0;
            }
            else
            {
                v7 = 5;
                v5 -= 5;
            }
            float fTimeStep = v7 * 0.001f;
            if (fabsf(fYawOffset) < 0.25f && fabsf(compass_speed) < 1.0f)
                break;
            fYawOffset = AngleNormalize180((fTimeStep * compass_speed) + v6);
            v6 = fYawOffset;
            if (fYawOffset <= 0.0f)
            {
                if (fYawOffset < 0.0f)
                    compass_speed = (fTimeStep * 1000.0f) + compass_speed;
            }
            else
            {
                compass_speed = compass_speed - (fTimeStep * 1000.0f);
            }
            float v8 = compass_speed - ((fTimeStep * compass_speed) * 2.0f);
            compass_speed = v8;
            if (v8 <= 0.0f)
            {
                if (fYawOffset < 0.0f)
                    compass_speed = v8 - ((v8 * fTimeStep) * 3.5f);
                compass_speed = fTimeStep + compass_speed;
                if (compass_speed > 0.0f)
                    compass_speed = 0.0f;
            }
            else
            {
                if (fYawOffset > 0.0f)
                    compass_speed = v8 - ((v8 * fTimeStep) * 3.5f);
                compass_speed = compass_speed - fTimeStep;
                if (compass_speed < 0.0f)
                    compass_speed = 0.0f;
            }
            if (compass_speed > 30000.0f)
                compass_speed = 30000.0f;
            else if (compass_speed < -30000.0f)
                compass_speed = -30000.0f;
            if (v5 <= 0)
            {
                compass_yaw = AngleNormalize360(v6 + fTargetYaw);
                UpdateCompassDial();
                return;
            }
        }
    }
    compass_yaw = fTargetYaw;
    compass_speed = 0.0f;
}

// ea: 0x00578800
void IGOCompassWidget::CheckpointRestart()
{
    m_hideCompassStarActive = 0;
    m_hideCompassStarIndex = 0;
    m_hideUpdatedText = 0;
    m_hideUpdatedTextIndex = 0;
}

// ea: 0x00578820
void IGOCompassWidget::CalculateMapObjectABS(IGOMapObject* mo, float dist,
                                             float yaw, float ring_time,
                                             bool is_objective)
{
    int value = cg_hudCompassMinRange.value;
    float v10 = cg_hudObjectiveMaxRange.value;
    int v12 = cg_hudCompassMaxRange.value;
    float v13 = dist;
    if (dist > v10 || (v10 = (float)v12, v12 > dist))
        v13 = v10;
    float v14 = cg_hudObjectiveMaxRange.value - v12;
    float alpha;
    if (v14 == 0.0f)
        alpha = 0.0f;
    else
        alpha = (((v13 - v12) / v14)
                 * (cg_hudObjectiveMinAlpha.value - 1.0f))
                + 1.0f;
    float v15 = 4900.0f;
    if (dist > 4900.0f || (v15 = (float)value, value > dist))
        dist = v15;
    float radius_scale = 0.0f;
    if ((v12 - value) != 0.0f)
        radius_scale = dist * 0.01122449f;
    float sinYaw, cosYaw;
    FastSinCos(yaw * 3.1415927f * 0.0055555557f, &sinYaw, &cosYaw);
    mo->x = frame->GetCenterX()
            - View::GetXScalingForHUD(mViewport) * sinYaw * radius_scale;
    mo->y = frame->GetCenterY()
            - View::GetYScalingForHUD(mViewport) * cosYaw * radius_scale;
    mo->alpha = alpha;
    mo->draw = true;
    if (is_objective)
        CalculateRing(mo, ring_time);
}

// ea: 0x00583AA0
void IGOCompassWidget::CalculateMapObject(IGOMapObject* mo, float dist,
                                          float yaw, float ring_time,
                                          bool is_objective)
{
    int value = cg_hudCompassMinRange.value;
    float v11 = cg_hudObjectiveMaxRange.value;
    int v13 = cg_hudCompassMaxRange.value;
    float v14 = dist;
    if (dist > v11 || (v11 = (float)v13, v13 > dist))
        v14 = v11;
    float v15 = cg_hudObjectiveMaxRange.value - v13;
    float alpha;
    if (v15 == 0.0f)
        alpha = 0.0f;
    else
        alpha = (((v14 - v13) / v15)
                 * (cg_hudObjectiveMinAlpha.value - 1.0f))
                + 1.0f;
    float v16 = (float)v13;
    if (dist > v13 || (v16 = (float)value, value > dist))
        dist = v16;
    float v17 = (float)(v13 - value);
    float radius_scale = 0.0f;
    if (v17 != 0.0f)
    {
        radius_scale =
            (((((dist - value) / v17)
               * (1.0f - cg_hudCompassMinRadius.value))
              + cg_hudCompassMinRadius.value)
             * cg_hudCompassSize.value)
            * 43.75f;
    }
    float sinYaw, cosYaw;
    FastSinCos(yaw * 3.1415927f * 0.0055555557f, &sinYaw, &cosYaw);
    mo->x = frame->GetCenterX()
            - View::GetXScalingForHUD(mViewport) * sinYaw * radius_scale;
    mo->y = frame->GetCenterY()
            - View::GetYScalingForHUD(mViewport) * cosYaw * radius_scale;
    mo->alpha = alpha;
    mo->draw = true;
    if (is_objective)
        CalculateRing(mo, ring_time);
}

// ea: 0x00583970
void IGOCompassWidget::UpdateSplitScreen(int viewport, int old_viewport)
{
    mViewport = viewport;
    UpdateCompassDial();
    UpdateCompassFrame();
    for (int i = 0; i < 27; ++i)
    {
        if (objectiveIcons[i] != nullptr)
            objectiveIcons[i]->FormatHUDForSplitScreen(viewport, old_viewport,
                                                       0, 0.0f, 0.0f);
        if (worldIcons[i].icon != nullptr)
            worldIcons[i].icon->FormatForSplitScreen(viewport, old_viewport);
    }
    for (int j = 0; j < 5; ++j)
    {
        if (objectiveIcons[sObjectiveIconRotateable[j]] != nullptr)
            objectiveIcons[sObjectiveIconRotateable[j]]
                ->SetXYInitialToCurrentPos();
    }
}

// ea: 0x00583A10
void IGOCompassWidget::UpdateWidescreen(bool widescreen, float about_x)
{
    UpdateCompassDial();
    UpdateCompassFrame();
    for (int i = 0; i < 27; ++i)
    {
        if (objectiveIcons[i] != nullptr)
            objectiveIcons[i]->FattenMeForWidescreen(widescreen, about_x);
        if (worldIcons[i].icon != nullptr)
            worldIcons[i].icon->FattenMeForWidescreen(widescreen, 0.0f);
    }
    for (int j = 0; j < 5; ++j)
    {
        if (objectiveIcons[sObjectiveIconRotateable[j]] != nullptr)
            objectiveIcons[sObjectiveIconRotateable[j]]
                ->SetXYInitialToCurrentPos();
    }
}

// ea: 0x005890A0
void IGOCompassWidget::UpdateFriendlies()
{
    if (dword_F62964[1580 * currCl] == 0)
        return;
    Entity* localPlayer = EntityManager::sInst->GetPlayer(mClient);
    int team = 0;
    if (localPlayer != nullptr && localPlayer->sentient != nullptr)
        team = localPlayer->sentient->eTeam;
    if (!cgGlobal.teamGame)
        return;
    for (int i = 0; i < 16; ++i)
    {
        Entity* Player = EntityManager::sInst->GetPlayer(i);
        if (Player == nullptr || Player->client == nullptr
            || Player->sentient == nullptr || localPlayer == Player
            || team != Player->sentient->eTeam)
        {
            continue;
        }
        int playerState = Player->client->pers.playerState;
        if (playerState == 0 || playerState == 2
            || (playerState == 1
                && *(int*)((char*)localPlayer + 596 + 1904) != 3))
        {
            continue;
        }
        int idx = Player->client - g_clients;
        if (idx < 0)
            continue;
        IGOFriendly& f = friendlies[idx];
        f.last_update = cgGlobal.time;
        f.last_yaw = Player->s.lerpAngles.v.m128_f32[1];
        f.last_pos[0] = Player->s.lerpOrigin.v.m128_f32[0];
        f.last_pos[1] = Player->s.lerpOrigin.v.m128_f32[1];
        if (!Player->IsInSnapshot())
            return;
        f.flags &= ~0x20;
        if (*(int*)((char*)Player + 596 + 36) >= 6
            || *(int*)((char*)Player + 596 + 1908) == 5)
        {
            f.flags |= 0x20;
        }
        if (*(int*)((char*)localPlayer + 596 + 1904) == 3
            && *(int*)((char*)Player + 596 + 1908) == 1)
        {
            f.flags |= 0x20;
        }
        Client* cl = Player->client;
        Entity* mOwner = nullptr;
        unsigned int mVal = Player->r.mOwner.mHandle.mVal;
        unsigned int v6 = mVal & 0xFFF;
        if (v6 < 0x540
            && mVal >> 12
                   == (unsigned int)EntityHandleDb::sInst.mElements[v6].mKey)
            mOwner = EntityHandleDb::sInst.mElements[v6].mObject;
        if ((*(int*)((char*)cl + 0xF4) & 0x100000) != 0 && mOwner != nullptr)
        {
            if (!IsVehicleTank(mOwner))
                f.last_yaw = mOwner->r.currentAngles.v.m128_f32[1];
        }
        else
        {
            f.last_yaw = Player->r.currentAngles.v.m128_f32[1];
        }
    }
    int clientBase = dword_F62964[1580 * currCl];
    if (*(int*)(clientBase + 0x520) != 0)
    {
        int idx = *(int*)(clientBase + 0x520) & 0x3F;
        IGOFriendly& f = friendlies[idx];
        f.last_update = cgGlobal.time;
        float v28 =
            (float)(4 * ((*(int*)(clientBase + 0x520) >> 6) & 0x1FF) - 1020);
        float v29 =
            (float)(4 * ((*(int*)(clientBase + 0x520) >> 15) & 0x1FF) - 1020);
        if (v28 == 1024.0f || v28 == -1020.0f || v29 == 1024.0f
            || v29 == -1020.0f)
        {
            float pos[2] = {v28, v29};
            VectorNormalize2D(pos);
            f.last_pos[0] = pos[0];
            f.last_pos[1] = pos[1];
        }
        else
        {
            float pos[3];
            pos[0] = *(float*)(clientBase + 0x10);
            pos[1] = *(float*)(clientBase + 0x14);
            pos[2] =
                *(float*)(clientBase + 0x18) + *(float*)(clientBase + 0xF0);
            AddLeanToPosition(pos, *(float*)(clientBase + 0xE4),
                              *(float*)(clientBase + 0x5C), 16.0f, 20.0f);
            f.last_pos[0] = pos[0] + v28;
            f.last_pos[1] = pos[1] + v29;
        }
        f.last_yaw =
            (float)*(signed char*)(clientBase + 0x523) * 1.40625f;
    }
    float compass_yaw =
        cg_hudCompassSpringyPointers.integer != 0
            ? this->compass_yaw
            : dword_F63CB4[1580 * currCl];
    for (int i = 0; i < 32; ++i)
    {
        IGOFriendly& f = friendlies[i];
        f.draw = false;
        if (f.last_update > cgGlobal.time)
            f.last_update = 0;
        if (f.last_update < cgGlobal.time - 800)
            continue;
        if (!((SmokeGrenadeMgr*)SmokeGrenadeMgr::sInst)
                 ->PointCanSeePoint(&dword_F63C70[1580 * currCl],
                                    f.last_pos, 0.4f))
        {
            f.draw = false;
            continue;
        }
        float yaw;
        float dist;
        if (fabsf(f.last_pos[0]) > 1.0f || fabsf(f.last_pos[1]) > 1.0f)
        {
            float delta[2] = {f.last_pos[0] - dword_F63C70[1580 * currCl],
                              f.last_pos[1] - dword_F63C74[1580 * currCl]};
            yaw = AngleNormalize360(vectoyaw(delta) - compass_yaw);
            dist = sqrtf(delta[1] * delta[1] + delta[0] * delta[0]);
        }
        else
        {
            yaw = AngleNormalize360(vectoyaw(f.last_pos) - compass_yaw);
            dist = cg_hudCompassMaxRange.value;
        }
        CalculateMapObject(&f, dist, yaw, -1.0f, false);
    }
}

// ea: 0x005895F0
void IGOCompassWidget::UpdateEnemies()
{
    if (gCvarShowEnemy.integer != 1
        || dword_F62964[1580 * currCl] == 0)
    {
        return;
    }
    Entity* Player = EntityManager::sInst->GetPlayer(mClient);
    if (Player == nullptr || Player->sentient == nullptr)
        return;
    int enemyTeam = Sentient_EnemyTeam(Player->sentient->eTeam);
    memset(gEnemies, 0, sizeof(gEnemies));
    int count = 0;
    for (int i = 0; i < 16; ++i)
    {
        Entity* v6 = EntityManager::sInst->GetPlayer(i);
        if (v6 == nullptr || v6->client == nullptr
            || v6->sentient == nullptr || Player == v6)
        {
            continue;
        }
        if (enemyTeam != v6->sentient->eTeam && cgGlobal.teamGame)
            continue;
        if (count >= 32)
            break;
        IGOEnemy& e = gEnemies[count++];
        e.last_update = cgGlobal.time;
        e.last_yaw = v6->s.lerpAngles.v.m128_f32[1];
        float origin[3];
        Sentient_GetOrigin(v6->sentient, origin);
        e.last_pos[0] = origin[0];
        e.last_pos[1] = origin[1];
        e.flags = 2;
        e.last_shot_time = v6->sentient->lastShotTime;
    }
    float compass_yaw =
        cg_hudCompassSpringyPointers.integer != 0
            ? this->compass_yaw
            : dword_F63CB4[1580 * currCl];
    for (int i = 0; i < 32; ++i)
    {
        IGOEnemy& e = gEnemies[i];
        if (e.last_update == 0)
            continue;
        float yaw;
        float dist;
        if (fabsf(e.last_pos[0]) > 1.0f || fabsf(e.last_pos[1]) > 1.0f)
        {
            float delta[2] = {e.last_pos[0] - dword_F63C70[1580 * currCl],
                              e.last_pos[1] - dword_F63C74[1580 * currCl]};
            yaw = AngleNormalize360(vectoyaw(delta) - compass_yaw);
            dist = sqrtf(delta[1] * delta[1] + delta[0] * delta[0]);
        }
        else
        {
            yaw = AngleNormalize360(vectoyaw(e.last_pos) - compass_yaw);
            dist = cg_hudCompassMaxRange.value;
        }
        CalculateMapObject(&e, dist, yaw, -1.0f, false);
        float v16 = (level.time - e.last_shot_time) * 0.0005f;
        e.alpha = v16 <= 1.0f ? 1.0f - v16 : 0.0f;
    }
}

// ea: 0x00589880
void IGOCompassWidget::UpdateObjectives()
{
    for (int i = 0; i < 17; ++i)
    {
        ObjectiveDataView* v4 =
            (ObjectiveDataView*)&unk_F6A2B0[802 * mClient + 44 * i];
        IGOObjective& o = objectives[i];
        o.draw = false;
        unsigned int v5 = v4->handle;
        unsigned int v6 = v5 & 0xFFF;
        Entity* mObject = nullptr;
        if (v6 < 0x540
            && v5 >> 12 == (unsigned int)EntityHandleDb::sInst.mElements[v6].mKey)
            mObject = EntityHandleDb::sInst.mElements[v6].mObject;
        if (mObject != nullptr)
        {
            v4->origin[0] = mObject->r.currentOrigin.v.m128_f32[0];
            v4->origin[1] = mObject->r.currentOrigin.v.m128_f32[1];
            v4->origin[2] = mObject->r.currentOrigin.v.m128_f32[2];
        }
        if (v4->origin[0] != 0.0f || v4->origin[1] != 0.0f
            || v4->origin[2] != 0.0f)
        {
            float delta = v4->origin[0] - dword_F63C70[1580 * currCl];
            float v19 = v4->origin[1] - dword_F63C74[1580 * currCl];
            float delta_z = v4->origin[2] - dword_F63C78[1580 * currCl]
                            + dword_F63640[1580 * currCl];
            float compass_yaw =
                cg_hudCompassSpringyPointers.integer != 0
                    ? this->compass_yaw
                    : dword_F63CB4[1580 * currCl];
            float obj_yaw = AngleNormalize360(vectoyaw(&delta)
                                              - compass_yaw);
            float dist = sqrtf(delta * delta + v19 * v19);
            CalculateMapObject(&o, dist, obj_yaw, v4->ring_time, true);
            o.state = v4->state;
            o.worldState = v4->worldState;
            o.up = false;
            o.down = false;
            if (delta_z <= cg_hudObjectiveMaxHeight.value)
            {
                if (cg_hudObjectiveMinHeight.value > delta_z)
                    o.down = true;
            }
            else
            {
                o.up = true;
            }
        }
    }
}

// ea: 0x00589AD0
void IGOCompassWidget::UpdateTanks()
{
    if (dword_F62964[1580 * currCl] == 0 || level.vehicles == nullptr)
        return;
    for (int i = 0; i < level.MaxVehicles; ++i)
    {
        unsigned int v3 =
            *(unsigned int*)&level.vehicles[i].mEntity.mHandle.mVal & 0xFFF;
        if (v3 >= 0x540
            || *(unsigned int*)&level.vehicles[i].mEntity.mHandle.mVal >> 12
                   != (unsigned int)EntityHandleDb::sInst.mElements[v3].mKey)
        {
            continue;
        }
        Entity* mObject = EntityHandleDb::sInst.mElements[v3].mObject;
        if (mObject == nullptr)
            continue;
        int index = 0;
        bool enemy = false;
        if (!G_GetTankIndex(mObject->mHandle, &index, &enemy))
            continue;
        if (gCvarShowEnemy.integer == 0 && enemy)
            continue;
        IGOFriendly& t = tanks[index];
        t.last_update = cgGlobal.time;
        t.last_pos[0] = mObject->s.lerpOrigin.v.m128_f32[0];
        t.last_pos[1] = mObject->s.lerpOrigin.v.m128_f32[1];
        t.last_yaw = mObject->s.lerpAngles.v.m128_f32[1];
        t.flags &= ~2;
        if (enemy)
            t.flags |= 2;
    }
    if (*(int*)(dword_F62964[1580 * currCl] + 1316) != 0)
    {
        int idx = *(int*)(dword_F62964[1580 * currCl] + 1316) & 0x3F;
        IGOFriendly& t = tanks[idx];
        t.last_update = cgGlobal.time;
        int v9 = dword_F62964[1580 * currCl];
        float v10 = (float)(4 * ((*(int*)(v9 + 1316) >> 6) & 0x1FF) - 1020);
        float v11 = (float)(4 * ((*(int*)(v9 + 1316) >> 15) & 0x1FF) - 1020);
        if (v10 == 1024.0f || v10 == -1020.0f || v11 == 1024.0f
            || v11 == -1020.0f)
        {
            float v[2] = {v10, v11};
            VectorNormalize2D(v);
            t.last_pos[0] = v[0];
            t.last_pos[1] = v[1];
        }
        else
        {
            t.last_pos[0] = *(float*)(v9 + 16) + v10;
            t.last_pos[1] = *(float*)(v9 + 20) + v11;
        }
        t.last_yaw = (float)*(signed char*)(v9 + 0x527) * 1.40625f;
    }
    float compass_yaw =
        cg_hudCompassSpringyPointers.integer != 0
            ? this->compass_yaw
            : dword_F63CB4[1580 * currCl];
    for (int i = 0; i < 25; ++i)
    {
        IGOFriendly& t = tanks[i];
        t.draw = false;
        if (t.last_update > cgGlobal.time)
            t.last_update = 0;
        if (t.last_update < cgGlobal.time - 800)
            continue;
        if (!((SmokeGrenadeMgr*)SmokeGrenadeMgr::sInst)
                 ->PointCanSeePoint(&dword_F63C70[1580 * currCl],
                                    t.last_pos, 0.4f))
        {
            t.draw = false;
            continue;
        }
        float yaw;
        float dist;
        if (fabsf(t.last_pos[0]) > 1.0f || fabsf(t.last_pos[1]) > 1.0f)
        {
            float delta[2] = {t.last_pos[0] - dword_F63C70[1580 * currCl],
                              t.last_pos[1] - dword_F63C74[1580 * currCl]};
            yaw = AngleNormalize360(vectoyaw(delta) - compass_yaw);
            dist = sqrtf(delta[1] * delta[1] + delta[0] * delta[0]);
        }
        else
        {
            yaw = AngleNormalize360(vectoyaw(t.last_pos) - compass_yaw);
            dist = cg_hudCompassMaxRange.value;
        }
        CalculateMapObject(&t, dist, yaw, -1.0f, false);
    }
}

// ea: 0x00589F40
void IGOCompassWidget::DrawVehcile()
{
    if (gCvarShowEnemy.integer == 0)
        return;
    PanelQuad* v4 = objectiveIcons[1];
    v4->SetZvalueAbs(15.0f);
    v4->ResetToInitialXY();
    unsigned int mVal =
        EntityManager::sInst->GetPlayer(mClient)->r.mOwner.mHandle.mVal;
    unsigned int v6 = mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v6 < 0x540
        && mVal >> 12 == (unsigned int)EntityHandleDb::sInst.mElements[v6].mKey)
        mObject = EntityHandleDb::sInst.mElements[v6].mObject;
    float v8 = 0.0f;
    if (mObject != nullptr)
        v8 = 0.0f - mObject->r.currentAngles.v.m128_f32[1];
    v4->Rotate((v8 * 3.1415927f) * 0.0055555557f, true);
    v4->SetCenterPos(frame->GetCenterX(), frame->GetCenterY());
    v4->Draw();
}

// ea: 0x0058A020
void IGOCompassWidget::UpdateObjectivesABS()
{
    for (int i = 0; i < 17; ++i)
    {
        ObjectiveDataView* v3 =
            (ObjectiveDataView*)&unk_F6A2B0[802 * mClient + 44 * i];
        IGOObjective& o = objectives[i];
        o.draw = false;
        if (v3->origin[0] != 0.0f || v3->origin[1] != 0.0f
            || v3->origin[2] != 0.0f)
        {
            Entity* localPlayer = EntityManager::sInst->GetPlayer(mClient);
            Entity* mObject = nullptr;
            if (localPlayer != nullptr)
            {
                unsigned int v6 = localPlayer->r.mOwner.mHandle.mVal & 0xFFF;
                if (v6 < 0x540
                    && localPlayer->r.mOwner.mHandle.mVal >> 12
                           == (unsigned int)EntityHandleDb::sInst
                                  .mElements[v6].mKey)
                    mObject = EntityHandleDb::sInst.mElements[v6].mObject;
            }
            float delta, v15, delta_z;
            if (mObject != nullptr)
            {
                delta = v3->origin[0]
                        - mObject->r.currentOrigin.v.m128_f32[0];
                v15 = v3->origin[1]
                      - mObject->r.currentOrigin.v.m128_f32[1];
                delta_z = v3->origin[2]
                          - mObject->r.currentOrigin.v.m128_f32[2];
            }
            else
            {
                delta = v15 = delta_z = 0.0f;
            }
            float obj_yaw = AngleNormalize360(vectoyaw(&delta));
            float dist = sqrtf(delta * delta + v15 * v15);
            CalculateMapObjectABS(&o, dist, obj_yaw, v3->ring_time, true);
            o.state = v3->state;
            o.up = false;
            o.down = false;
            if (delta_z <= cg_hudObjectiveMaxHeight.value)
            {
                if (cg_hudObjectiveMinHeight.value > delta_z)
                    o.down = true;
            }
            else
            {
                o.up = true;
            }
        }
    }
}

// ea: 0x0058A230
void IGOCompassWidget::UpdateEnemiesABS()
{
    if (gCvarShowEnemy.integer != 1
        || dword_F62964[1580 * currCl] == 0)
    {
        return;
    }
    Entity* Player = EntityManager::sInst->GetPlayer(mClient);
    if (Player == nullptr || Player->sentient == nullptr)
        return;
    int enemyTeam = Sentient_EnemyTeam(Player->sentient->eTeam);
    memset(gEnemies, 0, sizeof(gEnemies));
    int count = 0;
    for (int i = 0; i < 16; ++i)
    {
        Entity* v7 = EntityManager::sInst->GetPlayer(i);
        if (v7 == nullptr || v7->client == nullptr
            || v7->sentient == nullptr || Player == v7
            || enemyTeam != v7->sentient->eTeam
            || !IsPlayerSpotted(v7))
        {
            continue;
        }
        if (count >= 32)
            break;
        IGOEnemy& e = gEnemies[count++];
        e.last_update = cgGlobal.time;
        e.last_yaw = v7->s.lerpAngles.v.m128_f32[1];
        float origin[3];
        Sentient_GetOrigin(v7->sentient, origin);
        e.last_pos[0] = origin[0];
        e.last_pos[1] = origin[1];
        e.flags = 0;
        e.last_shot_time = v7->sentient->lastShotTime;
    }
    for (int i = 0; i < 32; ++i)
    {
        IGOEnemy& e = gEnemies[i];
        if (e.last_update == 0)
            continue;
        float yaw;
        float dist;
        if (fabsf(e.last_pos[0]) > 1.0f || fabsf(e.last_pos[1]) > 1.0f)
        {
            Entity* owner = nullptr;
            Entity* p = EntityManager::sInst->GetPlayer(mClient);
            if (p != nullptr)
            {
                unsigned int mVal = p->r.mOwner.mHandle.mVal;
                unsigned int v15 = mVal & 0xFFF;
                if (v15 < 0x540
                    && mVal >> 12
                           == (unsigned int)EntityHandleDb::sInst
                                  .mElements[v15].mKey)
                    owner = EntityHandleDb::sInst.mElements[v15].mObject;
            }
            float delta[2] = {e.last_pos[0], e.last_pos[1]};
            if (owner != nullptr)
            {
                delta[0] -= owner->r.currentOrigin.v.m128_f32[0];
                delta[1] -= owner->r.currentOrigin.v.m128_f32[1];
            }
            yaw = AngleNormalize360(vectoyaw(delta));
            dist = sqrtf(delta[1] * delta[1] + delta[0] * delta[0]);
        }
        else
        {
            yaw = AngleNormalize360(vectoyaw(e.last_pos));
            dist = cg_hudCompassMaxRange.value;
        }
        CalculateMapObjectABS(&e, dist, yaw, -1.0f, false);
        float v17 = (level.time - e.last_shot_time) * 0.0005f;
        e.alpha = v17 <= 1.0f ? 1.0f - v17 : 0.0f;
    }
}

// ea: 0x0058A4F0
void IGOCompassWidget::UpdateTanksABS()
{
    if (dword_F62964[1580 * currCl] == 0 || level.vehicles == nullptr)
        return;
    for (int i = 0; i < level.MaxVehicles; ++i)
    {
        unsigned int v4 =
            *(unsigned int*)&level.vehicles[i].mEntity.mHandle.mVal & 0xFFF;
        if (v4 >= 0x540
            || *(unsigned int*)&level.vehicles[i].mEntity.mHandle.mVal >> 12
                   != (unsigned int)EntityHandleDb::sInst.mElements[v4].mKey)
        {
            continue;
        }
        Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
        if (mObject == nullptr)
            continue;
        int index = 0;
        bool enemy = false;
        if (!G_GetTankIndex(mObject->mHandle, &index, &enemy))
            continue;
        if (gCvarShowEnemy.integer == 0 && enemy)
            continue;
        IGOFriendly& t = tanks[index];
        t.last_update = cgGlobal.time;
        t.last_pos[0] = mObject->s.lerpOrigin.v.m128_f32[0];
        t.last_pos[1] = mObject->s.lerpOrigin.v.m128_f32[1];
        t.last_yaw = mObject->s.lerpAngles.v.m128_f32[1];
        t.flags &= ~2;
        if (enemy)
            t.flags |= 2;
    }
    if (*(int*)(dword_F62964[1580 * currCl] + 1316) != 0)
    {
        int idx = *(int*)(dword_F62964[1580 * currCl] + 1316) & 0x3F;
        IGOFriendly& t = tanks[idx];
        t.last_update = cgGlobal.time;
        int v8 = dword_F62964[1580 * currCl];
        float v9 = (float)(4 * ((*(int*)(v8 + 1316) >> 6) & 0x1FF) - 1020);
        float v10 = (float)(4 * ((*(int*)(v8 + 1316) >> 15) & 0x1FF) - 1020);
        if (v9 == 1024.0f || v9 == -1020.0f || v10 == 1024.0f
            || v10 == -1020.0f)
        {
            float v[2] = {v9, v10};
            VectorNormalize2D(v);
            t.last_pos[0] = v[0];
            t.last_pos[1] = v[1];
        }
        else
        {
            t.last_pos[0] = *(float*)(v8 + 16) + v9;
            t.last_pos[1] = *(float*)(v8 + 20) + v10;
        }
        t.last_yaw = (float)*(signed char*)(v8 + 0x527) * 1.40625f;
    }
    Entity* Player = EntityManager::sInst->GetPlayer(mClient);
    for (int i = 0; i < 25; ++i)
    {
        IGOFriendly& t = tanks[i];
        t.draw = false;
        if (t.last_update > cgGlobal.time)
            t.last_update = 0;
        if (t.last_update < cgGlobal.time - 800)
            continue;
        if (!((SmokeGrenadeMgr*)SmokeGrenadeMgr::sInst)
                 ->PointCanSeePoint(&dword_F63C70[1580 * currCl],
                                    t.last_pos, 0.4f))
        {
            t.draw = false;
            continue;
        }
        float yaw;
        float dist;
        if (fabsf(t.last_pos[0]) > 1.0f || fabsf(t.last_pos[1]) > 1.0f)
        {
            float delta[2] = {t.last_pos[0], t.last_pos[1]};
            if (Player != nullptr)
            {
                unsigned int mVal = Player->r.mOwner.mHandle.mVal;
                unsigned int v18 = mVal & 0xFFF;
                Entity* owner = nullptr;
                if (v18 < 0x540
                    && mVal >> 12
                           == (unsigned int)EntityHandleDb::sInst
                                  .mElements[v18].mKey)
                    owner = EntityHandleDb::sInst.mElements[v18].mObject;
                if (owner != nullptr)
                {
                    delta[0] -= owner->r.currentOrigin.v.m128_f32[0];
                    delta[1] -= owner->r.currentOrigin.v.m128_f32[1];
                }
            }
            yaw = AngleNormalize360(vectoyaw(delta));
            dist = sqrtf(delta[1] * delta[1] + delta[0] * delta[0]);
        }
        else
        {
            yaw = AngleNormalize360(vectoyaw(t.last_pos));
            dist = cg_hudCompassMaxRange.value;
        }
        CalculateMapObjectABS(&t, dist, yaw, -1.0f, false);
    }
}

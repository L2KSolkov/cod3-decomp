// ============================================================================
// sv_stubs.h â€” minimal cross-object types used by sv.o functions.
// These types belong to OTHER (unported) game objects; only the fields that
// sv.o touches are declared here so the server engine compiles standalone.
// Full definitions arrive when those objects are ported.
// ============================================================================

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <intrin.h>
#include "engine/broc_types.h"
#include "game/game_types.h"
#include "core/ae_array.h"
#include "core/ae_fixed_string.h"
#include "bd/bdSession.h"

// bd headers pull windows.h via bdReferencable.h; undo its GetObjectA macro
// so HandleDb::GetObject keeps the binary's mangled name.
#undef GetObject

// Forward enum declarations used by mp.o member decls (full defs in
// g_local.h / mp_types.h).
enum itemType_t : int;
enum EDroppedItemTypes : int;

struct cdl_object_t;  // full definition in game/logic/g_local.h

// font_index - FE font selection enum (also defined in ui_types.h; guarded
// because sv_stubs.h is included by many TUs that cannot take ui_types.h).
#ifndef COD3_FONT_INDEX_DEFINED
#define COD3_FONT_INDEX_DEFINED
enum font_index {
    FONT_GARAMOND = 0,
    FONT_GEMFONTONE = 1,
    FONT_BUTTON = 2,
    FONT_ARIAL14 = 3,
    FONT_BIG = 4,
    FONT_NORMAL = 5,
    FONT_IMPACT = 6,
    FONT_HELVETICA_BOLD = 7,
    NUM_FONTS = 8,
    INVALID_FONT = 9,
};
#endif

// EPakType - pak type enum (global enum; kPakTypeGlobal == 0)
enum EPakType { kPakTypeGlobal = 0 };

// nsl sound types (enums in the binary; verified via W4 mangling)
enum nslSourceID : int { NSL_SOURCE_ID_INVALID = -1 };
enum nslWaveID : int { NSL_WAVE_ID_INVALID = -1 };
enum nslBankID : int { NSL_BANK_ID_INVALID = -1 };

// ============================================================================
// TPakId â€” pak archive id enum
// ============================================================================
// ============================================================================
// DCGSet Ã¢â‚¬â€ collision model (opaque; only fields SV_SetBrushModel touches)
// The release decompile views the entity as a DCGSet*; fields below mirror the
// offsets the disassembly reads (ent[2] = +0x20, ent[5] = +0x50, ent[6] = +0x60).
// Full definition arrives when the collision object is ported.
// ============================================================================
class DCGSet {
public:
    // Verified against disasm (TestInLeaf 0x623D40, TempBoxModel 0x618670,
    // CM_ModelBounds 0x6093D0, TempDCGSet ctor 0x638800): 112 bytes total.
    uint16_t nboxes;                   // +0x00
    uint16_t nbrushes;                 // +0x02
    int      objects_m_count;          // +0x04
    void*    objects_m_elements;       // +0x08
    int      brushes_m_count;          // +0x0C
    void*    brushes_m_elements;       // +0x10
    int      gjk_brushes_m_count;      // +0x14
    void*    gjk_brushes_m_elements;   // +0x18
    int      brush_sides_m_count;      // +0x1C
    void*    brush_sides_m_elements;   // +0x20
    int      brush_verts_m_count;      // +0x24
    void*    brush_verts_m_elements;   // +0x28
    uint32_t _2C;                      // +0x2C
    math::Position3 min;               // +0x30
    math::Position3 max;               // +0x40
    math::Position3 center;            // +0x50
    float    f60;                      // +0x60
    float    f64;                      // +0x64
    int      id;                       // +0x68
    int      _6C;                      // +0x6C

    int get_contents() const;          // ?get_contents@DCGSet@@QBEHXZ
    unsigned int size() const;              // ?size@DCGSet@@QBEIXZ (g.o 0x4AEE40)
    const cdl_object_t& get_object(unsigned short index) const;  // ?get_object@DCGSet@@QBEABUcdl_object_t@@G@Z (g.o 0x4AEE50)
};
static_assert(sizeof(DCGSet) == 0x70, "DCGSet size mismatch");
static_assert(offsetof(DCGSet, objects_m_count) == 0x04, "DCGSet::objects offset");
static_assert(offsetof(DCGSet, min) == 0x30, "DCGSet::min offset");
static_assert(offsetof(DCGSet, max) == 0x40, "DCGSet::max offset");
static_assert(offsetof(DCGSet, id) == 0x68, "DCGSet::id offset");

// ============================================================================
// StubData â€” per-controller MP save/profile data (1216 bytes) â€” verified IDA
// ============================================================================
struct StubData {
    char    mProfileName[16];          // +0x000
    int     mSaveGameSlot;             // +0x010
    int     mLevelReached;             // +0x014
    int     mNextLevel;                // +0x018
    int     mLanguage;                 // +0x01C
    int     mDifficulty;               // +0x020
    bool    mGameComplete;             // +0x024
    bool    mShowEnding;               // +0x025
    int     mSec;                      // +0x028
    int     mMin;                      // +0x02C
    int     mHour;                     // +0x030
    int     mDay;                      // +0x034
    bool    mSubtitles;                // +0x038
    bool    mCrosshair;                // +0x039
    bool    mFriendlyTags;             // +0x03A
    int     mTankStyle;                // +0x03C
    bool    mAdsToggle;                // +0x040
    bool    mInvertAim;                // +0x041
    bool    mVibration;                // +0x042
    bool    mStickyAim;                // +0x043
    int     mHorizontalSensitivity;    // +0x044
    int     mVerticalSensitivity;      // +0x048
    int     mControllerButtonConfiguration;  // +0x04C
    int     mControllerStickConfiguration;   // +0x050
    bool    mRatioIs4by3;              // +0x054
    bool    mResolutionIs480p;         // +0x055
    int     mChannels;                 // +0x058
    int     mVolume;                   // +0x05C
    int     mMusicVolume;              // +0x060
    int     mEffectVolume;             // +0x064
    bool    mViewedCredit;             // +0x068
    bool    mGameCompleted;            // +0x069
    int     mViewedSmg;                // +0x06C
    int     mViewedRifle;              // +0x070
    int     mViewedHmg;                // +0x074
    int     mViewedPistol;             // +0x078
    int     mViewedGrenade;            // +0x07C
    int     mViewedAssault;            // +0x080
    int     mViewedSniper;             // +0x084
    int     mViewedCrewServed;         // +0x088
    int     mViewedAntiTank;           // +0x08C
    int     mViewedDemolition;         // +0x090
    int     mViewedLand;               // +0x094
    int     mViewedAir;                // +0x098
    int     mViewedSea;                // +0x09C
    int     mViewedArtillery;          // +0x0A0
    int     mViewedAntiAir;            // +0x0A4
    int     mViewedRocket;             // +0x0A8
    int     mViewedCharacters;         // +0x0AC
    int     mViewedArt[14];            // +0x0B0
    int     mViewedMovies;             // +0x0E8
    int     mMaxPlayerCntPreference;   // +0x0EC
    int     mGameModePreference;       // +0x0F0
    int     mMapPreference;            // +0x0F4
    int     mAutoTeamBalancePreference;// +0x0F8
    int     mTeamDamagePreference;     // +0x0FC
    bool    mSaved;                    // +0x100
    int     mSaveId;                   // +0x104
    uint8_t savedState[516];           // +0x108 (_XONLINE_LOGON_STATE, opaque)
    int     loginMethod;               // +0x30C (_UIX_LOGON_TYPE)
    int     liveState;                 // +0x310 (ELiveState)
    uint8_t savedInvite[156];          // +0x314 (_XONLINE_ACCEPTED_GAMEINVITE, opaque)
    int     mControllerPort;           // +0x3B0
    bool    savedStateIsValid;         // +0x3B4
    int     lastLoginCode;             // +0x3B8 (HRESULT)
    bool    mReturnToMain;             // +0x3BC
    bool    appearOnline;              // +0x3BD
    bool    mbWasInvited;              // +0x3BE
    bool    mDisableSave;              // +0x3BF
    char    mCmdLine[256];             // +0x3C0

    void Init();                     // ?Init@StubData@@QAEXXZ (shell.o)
    void LoadData(StubData* input);  // ?LoadData@StubData@@QAEXPAU1@@Z
    void LoadDataLastMinFix(StubData* input);  // shell.o
    void ApplyDifficulty();          // ?ApplyDifficulty@StubData@@QAEXXZ
    void ApplyStubOptions();         // ?ApplyStubOptions@StubData@@QAEXXZ
};
static_assert(sizeof(StubData) == 0x4C0, "StubData size mismatch");
static_assert(offsetof(StubData, mControllerPort) == 0x3B0, "StubData::mControllerPort offset mismatch");
static_assert(offsetof(StubData, mSec) == 0x28, "StubData::mSec offset mismatch");
static_assert(offsetof(StubData, mInvertAim) == 0x41, "StubData::mInvertAim offset mismatch");

// ============================================================================
// SaveGameData â€” MP save game bundle (7156 bytes) â€” verified IDA
// ============================================================================
struct SaveGameData {
    StubData mStubData;         // +0x000
    bool     mCheckpointSaveExists;    // +0x4C0
    int      ammo[92];                 // +0x4C4
    int      ammoclip[92];             // +0x634
    int      weapons[2];               // +0x7A4
    char     weaponslots[10];          // +0x7AC
    int      weaponrechamber[2];       // +0x7B8
    int      weapon;                   // +0x7C0
    float    mPlayerHealth;            // +0x7C4
    float    mPlayerOrientation[3];    // +0x7C8
    float    mPlayerPosition[3];       // +0x7D4
    uint8_t  mFriendlies[896];         // +0x7E0
    int      mFriendlyCount;           // +0xB60
    char     mEvent[32];               // +0xB64
    char     mCurrentMapName[32];      // +0xB84
    char     mCheckpointName[32];      // +0xBA4
    int      mGameVarCount;            // +0xBC4
    uint8_t  mGameVars[3072];          // +0xBC8
    int      mSavedExploderCount;      // +0x17C8
    uint8_t  mCheckpointScriptExploded[1024];  // +0x17CC
    char     m_title_prefix[12];       // +0x1BCC
    char     m_version_number[26];     // +0x1BD8

    void Init();                       // ?Init@SaveGameData@@QAEXXZ
    void LoadData(SaveGameData* input);  // ?LoadData@SaveGameData@@QAEXPAU1@@Z
};
static_assert(sizeof(SaveGameData) == 7156, "SaveGameData size mismatch");
static_assert(offsetof(SaveGameData, ammo) == 0x4C4,
              "SaveGameData::ammo offset mismatch");
static_assert(offsetof(SaveGameData, mFriendlies) == 0x7E0,
              "SaveGameData::mFriendlies offset mismatch");

// MP player / entity manager minimal views (fields used by SV_PostConnect)
class MPPlayer;
struct MPPlayerManager;
class MPPeer;

// ============================================================================
// ServerTime â€” server clock (20 bytes) â€” verified IDA
// ============================================================================
class ServerTime {
public:
    unsigned int mNumTicksElapsed;  // +0x00
    int          mTickMSec;         // +0x04
    float        mTickDelta;        // +0x08
    float        mTickDeltaInv;     // +0x0C
    float        mElapsedTime;      // +0x10

    static ServerTime sInst;        // ?sInst@ServerTime@@0V1@A
    static ServerTime* Inst();      // ?Inst@ServerTime@@SAPAV1@XZ
    int GetTickMSec() const;        // ?GetTickMSec@ServerTime@@QBEHXZ
    float GetTickDelta() const;     // ?GetTickDelta@ServerTime@@QBEMXZ
};
static_assert(sizeof(ServerTime) == 0x14, "ServerTime size mismatch");

// ============================================================================
// FEManager â€” front-end manager (1012 bytes; opaque, only sv.o fields shown)
// ============================================================================
class PanelFileUser;
struct IGOFrontEnd;
class FEMenuSystem;
struct DialogMenuSystem;
struct InGameMenuSystem;
struct AARMenuSystem {
    void** menus;           // +0x04 (FEMenu** array)
    uint8_t _pad08[0x1C - 0x08];
    void (*gap1C)(void* self, float a2);  // +0x1C (shell.o AAR update slot)
    bool IsSystemActive();  // ?IsSystemActive@AARMenuSystem@@QAE_NXZ (shell.o; stub)
    void Update(float time_inc);  // mp.o
    void Draw();                  // mp.o
    void UpdateSplitScreen();     // mp.o
    void ActivateMenu(int menu);  // ?ActivateMenu@AARMenuSystem@@QAEXH@Z (shell.o; stub)
    bool GetPanelFileUsers(const char* name,
                           ae_sized_array<PanelFileUser*, 12>& array);  // mp.o
};
struct ProfileManager;
class PanelQuad;
class PanelFile;
class nglFont;
struct ControllerDisconnectedMenu;

struct FEManager {
public:
    // +0x00 vftable (1 ptr)
    nglFont* fonts[4];                  // +0x04
    IGOFrontEnd* IGO;                   // +0x14
    ControllerDisconnectedMenu* ControllerDisconnected;  // +0x18
    FEMenuSystem* fems;                 // +0x1C
    ProfileManager* mProfileManager;    // +0x20
    PanelQuad* default_pq;              // +0x24
    float   pause_menu_timer;           // +0x28
    bool    fontsLoaded[4];             // +0x2C
    bool    start_on;                   // +0x30
    bool    debug_mode;                 // +0x31
    bool    forceMovieExit;             // +0x32
    bool    skipFE;                     // +0x33
    bool    enablePause;                // +0x34
    bool    IGO_active;                 // +0x35
    bool    inGame;                     // +0x36
    bool    menuMovieRunning;           // +0x37
    bool    legalMoviesFinished;        // +0x38
    bool    skipAllLegalMovies;         // +0x39
    bool    skipAllMovies;              // +0x3A
    bool    renderMovieOnly;            // +0x3B
    bool    mDontDrawHud;               // +0x3C
    char    loadLevel[128];             // +0x3D
    float   saveTime;                   // +0xC0
    DialogMenuSystem* mDMS[1];          // +0xC4
    InGameMenuSystem* mIGMS[1];          // +0xC8
    AARMenuSystem* mAARS;               // +0xCC
    uint8_t mPanelArray[800];           // +0xD0 (100 x sPanelPakData, opaque)
    int     mNumPanels;                 // +0x3F0
    // +0x3F4 .. 0x3F4 remaining pad
    void UpdateLoadingMenu(float percentDone);  // ?UpdateLoadingMenu@FEManager@@QAEXM@Z
    nglFont* GetFont(font_index f);  // ?GetFont@FEManager@@QAEPAVnglFont@@W4font_index@@@Z (shell.o)
    font_index FindFont(const char* font_filename,
                        bool checkfileext);  // ?FindFont@FEManager@@QAE?AW4font_index@@PBD_N@Z (shell.o 0x5852E0)
    PanelQuad* GetDefaultPQ();               // ?GetDefaultPQ@FEManager@@QAEPAVPanelQuad@@XZ (shell.o 0x593D90)
    void SetFont(nglFont* f,
                 const char* font_filename);  // ?SetFont@FEManager@@QAEXPAVnglFont@@PBD@Z (shell.o 0x58DD60)
    void HandlePanelFilePointer(const char* name, PanelFile* pf,
                                TPakId pakId);  // ?HandlePanelFilePointer@FEManager@@QAEXPBDPAVPanelFile@@W4TPakId@@@Z (shell.o 0x58DE00)
    static ae_fixed_string<32, unsigned char>
        font_name_array[4];  // ?font_name_array@FEManager@@0PAV?$ae_fixed_string@$0CA@E@@A @ 0xF382C0
    void SetInGameMenusActive(bool active,
                              int client);  // ?SetInGameMenusActive@FEManager@@QAEX_NH@Z (sv.o 0x51E1A0)
    InGameMenuSystem* GetIGMS(int client);   // ?GetIGMS@FEManager@@QAEPAVInGameMenuSystem@@H@Z (shell.o 0x57F410)
    DialogMenuSystem* GetDMS(int client);    // ?GetDMS@FEManager@@QAEPAVDialogMenuSystem@@H@Z
    void UpdateIGO(float time_inc);           // ?UpdateIGO@FEManager@@QAEXM@Z (cl.o 0x528390)
    void UpdateInSceneIGO(float time_inc);    // ?UpdateInSceneIGO@FEManager@@QAEXM@Z (cl.o 0x5283A0)
    bool AARMenusActive();                    // ?AARMenusActive@FEManager@@QAE_NXZ (cl.o 0x5283B0)
    bool FrontEndMenusActive(int client);     // ?FrontEndMenusActive@FEManager@@QAE_NH@Z (cl.o 0x5283C0)
    void SetFrontEndMenusActive(bool active,
                                int client);  // ?SetFrontEndMenusActive@FEManager@@QAEX_NH@Z (cl.o 0x5283E0)
    AARMenuSystem* GetAARS();                 // ?GetAARS@FEManager@@QAEPAVAARMenuSystem@@XZ (cl.o 0x528400)

    // FEManager.cpp family (shell.o)
    FEManager();                          // 0x593C90
    virtual ~FEManager();                 // 0x56F040
    void LoadFonts();                     // 0x56F0C0
    void ReleaseFonts();                  // 0x56F0D0
    void ReleaseFont(font_index f);       // 0x56F0E0
    void LoadFont(font_index i);          // 0x57D720
    void UpdateFrontEnd(float time_inc);  // 0x56F0F0
    void UpdateAARMenus(float time_inc);  // 0x56F1B0
    void UpdateInGameMenus(float time_inc);  // 0x56F230
    void DrawIGO(int client);             // 0x56F280
    void Draw3DWorldSpace();              // 0x56F330
    void DrawLoadingDots();               // 0x56F3B0
    void DrawDebugDiscError();            // 0x56F440
    void PlayFadeInOranScreen();          // 0x56F5F0
    void DrawFrontEnd();                  // 0x56F820
    void DrawInGameMenus();               // 0x56F8A0
    void DrawAARMenus();                  // 0x56F980
    void PrepareLoadingMenus();           // 0x56FA20
    void ReleaseInGameMenus();            // 0x56FA30
    void UpdateSplitScreen();             // 0x56FAE0
    void Draw3DScreenSpace();             // 0x57D770
    void DrawLoadingScreen(int alpha);    // 0x57D7A0
    void DrawControllerError();           // 0x57DA30
    void UpdateButtonFontForLanguage();   // 0x585260
    void ReleaseFrontEnd();               // 0x585450
    nglFont* GetFont(font_index f,
                     float scale);       // 0x585490
    void InitDialogMenuSystem();          // 0x5958A0
    void LoadFrontEnd();                  // 0x59ACD0
    void LoadInGameMenus();               // 0x59AD70
    void InitIGO();                       // 0x59D0C0
    void DrawDiscError();                 // 0x57D8A0
    bool DMSMenusActiveAnyClient();       // ?DMSMenusActiveAnyClient@FEManager@@QAE_NXZ
protected:
    void GetPanelFileUsers(const char* name,
                           ae_sized_array<PanelFileUser*, 12>& array);  // 0x5854D0
};
static_assert(sizeof(FEManager) == 0x3F4, "FEManager size mismatch");

// ============================================================================
// Opaque singleton managers (fields used by sv.o only)
// ============================================================================
class AeThread;  // scr.o (AeThread.cpp; full type in g_scr.cpp)
class AeThreadManager {
public:
    uint8_t _pad[0x858];
    void*   mThreadExecuting;  // +0x858 (scr.o 0x5BC880)
    uint8_t _pad2[2148 - 0x85C];
    static AeThreadManager sInst;   // ?sInst@AeThreadManager@@0V1@A
    static AeThreadManager* Inst(); // ?Inst@AeThreadManager@@SAPAV1@XZ (g.o 0x4A6340)
    void KillAllThreads();
    void UnloadScript(void* p);  // ?UnloadScript@AeThreadManager@@QAEXPAX@Z (scr.o 0x5C1F30)
    void Execute(float deltaT);     // ?Execute@AeThreadManager@@QAEXM@Z
    void AddNotify(EntityNotify* notify);  // ?AddNotify@AeThreadManager@@QAEXPAVEntityNotify@@@Z (g.o 0x4B2170)
    // scr.o methods (AeThread.cpp; layout overlays defined in g_scr.cpp)
    AeThreadManager();                         // ??0AeThreadManager@@QAE@XZ (scr.o 0x5DB100)
    void AddThread(AeThread* t);               // ?AddThread@AeThreadManager@@QAEXPAVAeThread@@@Z (scr.o 0x5C95F0)
    void DebugThread(unsigned int threadId);   // ?DebugThread@AeThreadManager@@QAEXI@Z (scr.o 0x5C97C0)
    void ReleaseHandle(AeThread* t);           // ?ReleaseHandle@AeThreadManager@@QAEXPAVAeThread@@@Z (scr.o 0x5C9770)
    void ReleaseHandle(Handle h);              // ?ReleaseHandle@AeThreadManager@@QAEXVHandle@@@Z (scr.o 0x5C97A0)
    Handle AssignHandle(AeThread* t);          // ?AssignHandle@AeThreadManager@@QAE?AVHandle@@PAVAeThread@@@Z (scr.o 0x5DB160)
private:
    void KillThread(AeThread* t);              // ?KillThread@AeThreadManager@@AAEXPAVAeThread@@@Z (scr.o 0x5DC120)
    void ProcessScriptNotifys();               // ?ProcessScriptNotifys@AeThreadManager@@AAEXXZ (scr.o 0x5C96D0)
public:
};
static_assert(sizeof(AeThreadManager) == 2148, "AeThreadManager size mismatch");

class MultiplayerMgr {
public:
    class MPEntityHandle {
    public:
        int mVal;
    };  // +0x00 opaque
    MPPeer* mPeer;                  // +0x00
    uint8_t _pad[0x35 - 0x4];
    bool    mRankedGame;            // +0x35
    uint8_t _pad2[0x40 - 0x36];
    bool    mLinkCheckEnabled;      // +0x40 (field used by SV_Map_f)
    uint8_t _pad3[0x50 - 0x41];
    static MultiplayerMgr* sInst;   // ?sInst@MultiplayerMgr@@2PAV1@A
    static MultiplayerMgr* Inst();  // ?Inst@MultiplayerMgr@@SAPAV1@XZ (g.o 0x4A9780)
    MPPeer* GetPeer();              // ?GetPeer@MultiplayerMgr@@QAEPAVMPPeer@@XZ (g.o 0x4A9790)
    static struct kuju_sTime* getLocalTime(MultiplayerMgr* self);  // mp.o
    bool IsVoteOngoing();           // ?IsVoteOngoing@MultiplayerMgr@@QAE_NXZ
    bool IsPlayerTalking(Entity* player);  // ?IsPlayerTalking@MultiplayerMgr@@QAE_NPAVEntity@@@Z (mp.o)
    void setEnableLinkCheck(bool enabled);  // ?setEnableLinkCheck@MultiplayerMgr@@QAEX_N@Z (sv.o 0x528020)
    bool getEnableLinkCheck();              // ?getEnableLinkCheck@MultiplayerMgr@@QAE_NXZ (sv.o 0x528030)
    void ExitLevel();
    void StartDevServer();
    void HandleDiscError();                    // ?HandleDiscError@MultiplayerMgr@@QAEXXZ (mp.o 0x72C4A0)
    int  AddTestClient();                      // ?AddTestClient@MultiplayerMgr@@QAEHXZ (mp.o 0x72C4E0)
    bool oneOffCheckLinkStatus();              // ?oneOffCheckLinkStatus@MultiplayerMgr@@QAE_NXZ (mp.o 0x72C6F0)
    bool FromLobby();                          // ?FromLobby@MultiplayerMgr@@QAE_NXZ (mp.o 0x72C7D0)
    float getSendInterval() const;             // ?getSendInterval@MultiplayerMgr@@QBEMXZ (mp.o 0x72C4D0)
    bool getLinkStatus();                      // ?getLinkStatus@MultiplayerMgr@@QAE_NXZ (mp.o 0x72C780)
    void setSendInterval(int ms);              // ?setSendInterval@MultiplayerMgr@@QAEXH@Z (mp.o 0x72C4B0)
    void shutdownVoiceSubsystem();             // ?shutdownVoiceSubsystem@MultiplayerMgr@@QAEXXZ (mp.o 0x7511E0)
    EDroppedItemTypes GetDroppedItemType(itemType_t item);  // ?GetDroppedItemType@MultiplayerMgr@@QAE?AW4EDroppedItemTypes@@W4itemType_t@@@Z (mp.o 0x72C4F0)
    void MapRestart();                                  // ?MapRestart@MultiplayerMgr@@QAEXXZ (mp.o)
    void RoundOver(int condition, int team);            // ?RoundOver@MultiplayerMgr@@QAEXHH@Z (mp.o)
    void NextRound(bool allowChange);                   // ?NextRound@MultiplayerMgr@@QAEX_N@Z (mp.o)
    void SendRespawnRequest(unsigned int clientID);     // ?SendRespawnRequest@MultiplayerMgr@@QAEXI@Z (mp.o)
    void SendGameScore(int alliesScore, int axisScore); // ?SendGameScore@MultiplayerMgr@@QAEXHH@Z (mp.o)
    void AreaCaptured(int netIndex, int itemType, int hostOnly);  // ?AreaCaptured@MultiplayerMgr@@QAEXHHH@Z (mp.o)
    void EnterGame();                                   // ?EnterGame@MultiplayerMgr@@QAEXXZ (mp.o)
    void PlayerRespawn(Entity* player,
                       const math::Position3& position,
                       const math::Dir3& angles,
                       int team);  // ?PlayerRespawn@MultiplayerMgr@@QAEXPAVEntity@@ABVPosition3@math@@ABVDir3@4@H@Z (mp.o)
    const char* GetPlayerName(Entity* player) const;  // ?GetPlayerName@MultiplayerMgr@@QBEPBDPBVEntity@@@Z (mp.o)
    void SpotEntity(Entity* ent);               // ?SpotEntity@MultiplayerMgr@@QAEXPAVEntity@@@Z
    void PlayerDamage(Entity* hitEntity, Entity* attacker,
                      const math::Position3& position, const math::Dir3& normal,
                      int weapon, float damage, unsigned char mod, int dflags,
                      EHitLocation hitLocation);
    void VehicleDamage(Entity* hitEntity, Entity* attacker,
                       const math::Position3& position, const math::Dir3& normal,
                       float damage, int weapon, unsigned char mod, int dflags);
    void VehicleDeath(Entity* hitEntity, Entity* killer, int weapon, int mod);
    void ProjectileExplosion(Entity* projectile, int weapon,
                             const math::Position3& position, const math::Dir3& normal,
                             unsigned char surfaceType, Entity* owner);
    void PlayerDead(Entity* player, Entity* inflictor, Entity* attacker,
                    int damage, int mod, int weapon, const float* position,
                    const float* dir, int hitLoc);   // ?PlayerDead@MultiplayerMgr@@QAEXPAVEntity@@00HHHQBM1H@Z
    void AttemptToRevivePlayer(Entity* player, Entity* medic);  // ?AttemptToRevivePlayer@MultiplayerMgr@@QAEXPAVEntity@@0@Z
    void RegisterDroppedItem(int itemType, Entity* item, Entity* owner, int a4);  // ?RegisterDroppedItem@MultiplayerMgr@@QAEXW4EDroppedItemTypes@@PAVEntity@@1H@Z
    MPEntityHandle RegisterDroppedItem(int itemType, Entity* item, Entity* owner);  // ?RegisterDroppedItem@MultiplayerMgr@@QAE?AVMPEntityHandle@@W4EDroppedItemTypes@@PAVEntity@@1@Z
    void FireMissile(int weapon, const math::Position3& position, const math::Dir3& dir,
                     MultiplayerMgr::MPEntityHandle handle);  // ?FireMissile@MultiplayerMgr@@QAEXHABVPosition3@math@@ABVDir3@3@VMPEntityHandle@@@Z
    void Step(int earlyOutInterval, bool fromThread, bool a_bFromGame);  // ?Step@MultiplayerMgr@@QAEXH_N0@Z
    bool IsLocalPlayer(Entity* player);            // ?IsLocalPlayer@MultiplayerMgr@@QAE_NPAVEntity@@@Z
    bool IsInVehicle(Entity* player);              // ?IsInVehicle@MultiplayerMgr@@QAE_NPAVEntity@@@Z
    bool IsInVehicle(Entity* vehicle, Entity* player);  // ?IsInVehicle@MultiplayerMgr@@QAE_NPAVEntity@@0@Z
    bool ChangeTeam(Entity* player, int team, bool hostOnly,
                    bool autoBalance);  // ?ChangeTeam@MultiplayerMgr@@QAE_NPAVEntity@@H_N1@Z
    void SendInitialGameState(Entity* player);  // ?SendInitialGameState@MultiplayerMgr@@QAEXPAVEntity@@@Z
    void SendVehicleStates(Entity* player);  // ?SendVehicleStates@MultiplayerMgr@@QAEXPAVEntity@@@Z
    void BroadcastVehicleRespawn(Entity* vehicle);  // ?BroadcastVehicleRespawn@MultiplayerMgr@@QAEXPAVEntity@@@Z
    void SendGameState(Entity* player, int currentTime, int timeLimit,
                       int scoreLimit, int roundLimit, bool friendlyFire,
                       bool lastManStanding, bool teamBalance,
                       int respawnTime, int alliesScore, int axisScore,
                       bool roundStarted, int roundOver,
                       int roundCount);  // ?SendGameState@MultiplayerMgr@@QAEXPAVEntity@@HHHH_N11HHH1HH@Z
    void SendGameStateHQ(Entity* player, unsigned int stage,
                         const math::Position3& pA,
                         const math::Position3& pB,
                         unsigned int triggerIndex, bool alliesDefending,
                         bool pointAIsHQ);  // ?SendGameStateHQ@MultiplayerMgr@@QAEXPAVEntity@@IABVPosition3@math@@1I_N2@Z
    void SendGameStateCTF(Entity* player,
                          const math::Position3& allied_flag,
                          const math::Dir3& alliedAngles,
                          Entity* allied_flag_holder,
                          const math::Position3& axis_flag,
                          const math::Dir3& axisAngles,
                          Entity* axis_flag_holder);  // ?SendGameStateCTF@MultiplayerMgr@@QAEXPAVEntity@@ABVPosition3@math@@ABVDir3@4@0120@Z
    void SendGameStateSCF(Entity* player, int currentFlagIndex,
                          const math::Position3& flagPosition,
                          const math::Dir3& flagAngles,
                          Entity* flagHolder);  // ?SendGameStateSCF@MultiplayerMgr@@QAEXPAVEntity@@HABVPosition3@math@@ABVDir3@4@0@Z
    void SendGameStateDOM(Entity* player, int flag0, int flag1, int flag2,
                          int flag3, int flag4);  // ?SendGameStateDOM@MultiplayerMgr@@QAEXPAVEntity@@HHHHH@Z
    void SendGameStateSD(Entity* player, Entity* planter, Entity* defuser,
                         bool planting, const math::Position3& bombPosition,
                         const math::Dir3& bombAngles,
                         int bombTimeLeft);  // ?SendGameStateSD@MultiplayerMgr@@QAEXPAVEntity@@00_NABVPosition3@math@@ABVDir3@4@H@Z
    void SendHostBombRequest(const Entity* player, bool defusing);  // ?SendHostBombRequest@MultiplayerMgr@@QAEXPBVEntity@@_N@Z
    void SendBombExplosion(const Entity* player);  // ?SendBombExplosion@MultiplayerMgr@@QAEXPBVEntity@@@Z
    void SendBombOperation(const Entity* player, bool defusing);  // ?SendBombOperation@MultiplayerMgr@@QAEXPBVEntity@@_N@Z
    void SendBombOperationEvent(const Entity* player, bool defusing,
                                bool success);  // ?SendBombOperationEvent@MultiplayerMgr@@QAEXPBVEntity@@_N1@Z
    void DropWeapon(int weapon, int netIndex, const math::Position3* position,
                    const math::Position3* angles, const math::Dir3* velocity,
                    int clipCount, int ammoCount);  // ?DropWeapon@MultiplayerMgr@@QAEXHHABVPosition3@math@@1ABVDir3@2@HH@Z
    void SpreadFire(Entity* player, float gunPitch, float gunYaw,
                    float* weaponPosition, int weapon, float spread,
                    float coneAngleTangent, int seed);  // ?SpreadFire@MultiplayerMgr@@QAEXPAVEntity@@MMQAMHMHH@Z
    void GetNextDroppedItemID(void* result, int itemType, Entity* owner);  // ?GetNextDroppedItemID@MultiplayerMgr@@QAEXAAVMPEntityHandle@@W4EDroppedItemTypes@@PAVEntity@@@Z
    int  GetDroppedItemType(int itemType);   // ?GetDroppedItemType@MultiplayerMgr@@QAE?AW4EDroppedItemTypes@@W4itemType_t@@@Z
    MPEntityHandle FindDroppedItemID(int itemType, Entity* item, Entity* owner);  // ?FindDroppedItemID@MultiplayerMgr@@QAE?AVMPEntityHandle@@W4EDroppedItemTypes@@PAVEntity@@1@Z
    void PickupItem(int netIndex, int itemType, Entity* player, bool scriptFrom);  // ?PickupItem@MultiplayerMgr@@QAEXHHPAVEntity@@_N@Z
    void DropItem(int itemType, const math::Position3* position,
                  const math::Dir3* angles, const math::Dir3* velocity,
                  int netIndex, bool scriptFrom, int typeIndex);  // ?DropItem@MultiplayerMgr@@QAEXW4EDroppedItemTypes@@ABVPosition3@math@@ABVDir3@2@2H_NH@Z
    void ApplyLocalPhysicsToVehicle(Entity* vehicle, math::Position3* position,
                                    math::Position3* angles, float* velocity);  // ?ApplyLocalPhysicsToVehicle@MultiplayerMgr@@QAEXPAVEntity@@AAVPosition3@math@@1QAM@Z
    void AttemptToGetInVehicle(Entity* vehicle, Entity* player, int seatIdx,
                               int entryIdx);  // ?AttemptToGetInVehicle@MultiplayerMgr@@QAEXPAVEntity@@0HH@Z
    void AttemptVehicleSeatChange(Entity* vehicle, Entity* player, int newSeatIdx);  // ?AttemptVehicleSeatChange@MultiplayerMgr@@QAEXPAVEntity@@0H@Z
    void GetOutOfVehicle(Entity* vehicle, int seatIdx);  // ?GetOutOfVehicle@MultiplayerMgr@@QAEXPAVEntity@@H@Z
    void VehicleFireMissile(Entity* vehEnt, int weapon,
                            const math::Position3* position,
                            const math::Dir3* dir);  // ?VehicleFireMissile@MultiplayerMgr@@QAEXPAVEntity@@HABVPosition3@math@@ABVDir3@4@@Z
    void FireArtillery(Entity* attacker, int weapon,
                       const math::Position3* position, int seed, bool fire);  // ?FireArtillery@MultiplayerMgr@@QAEXPAVEntity@@HABVPosition3@math@@H_N@Z
    void VehicleMantled(Entity* vehicle, Entity* killer);  // ?VehicleMantled@MultiplayerMgr@@QAEXPAVEntity@@0@Z
    void AnimEvent(int animEvent);  // ?AnimEvent@MultiplayerMgr@@QAEXH@Z
    bool IsHost();                  // ?IsHost@MultiplayerMgr@@QAE_NXZ
    void SwapWeapon(int weapon, int netIndex, int clipCount, int ammoCount);  // ?SwapWeapon@MultiplayerMgr@@QAEXHHHH@Z
    void SwapKit(int playerClass, int netIndex);  // ?SwapKit@MultiplayerMgr@@QAEXHH@Z
    void SetPlayerPos(const Entity* player, float* pos);  // ?SetPlayerPos@MultiplayerMgr@@QAEXPBVEntity@@QAM@Z
    void LevelLoaded();  // ?LevelLoaded@MultiplayerMgr@@QAEXXZ
    void BulletHit(const math::Position3& position, const math::Dir3& normal,
                   unsigned char surfaceType, unsigned char weapon,
                   Entity* hitEntity);  // ?BulletHit@MultiplayerMgr@@QAEXABVPosition3@math@@ABVDir3@3@EEPAVEntity@@@Z
    void BulletHitPlayer(Entity* hitEntity, Entity* attackerEntity,
                         const math::Position3& position,
                         const math::Dir3& normal, unsigned char surfaceType,
                         unsigned char weapon, short damage,
                         unsigned char damageFlags, unsigned char mod,
                         int hitLocation);  // ?BulletHitPlayer@MultiplayerMgr@@QAEXPAVEntity@@0ABVPosition3@math@@ABVDir3@4@EEFEEH@Z
    void MeleeHit(Entity* hitEntity, Entity* attackerEntity,
                  const math::Position3& position, const math::Dir3& normal,
                  unsigned char surfaceType, short damage, unsigned char mod,
                  int hitLocation);  // ?MeleeHit@MultiplayerMgr@@QAEXPAVEntity@@0ABVPosition3@math@@ABVDir3@4@EFEH@Z
};
static_assert(sizeof(MultiplayerMgr) == 80, "MultiplayerMgr size mismatch");

struct SmokeGrenadeInfo {
    float mTime;   // +0x00
    void* mEffect; // +0x04
    bool  bHit[4]; // +0x08
};

struct SceneEntity;  // streamer.o (opaque)
struct SceneBank;    // streamer.o (opaque)

// ae_vector<SmokeGrenadeInfo> - 12 bytes
struct SmokeGrenadeInfoList {
    SmokeGrenadeInfo* mElements;  // +0x00
    int mSize;                    // +0x04
    int mCapacity;                // +0x08
};

class SmokeGrenadeMgr {
public:
    SmokeGrenadeInfoList mSmokeGrenadeInfoList;  // +0x00
    static void* sInst;  // ?sInst@SmokeGrenadeMgr@@2PAV1@A @ 0xF049B4
    static SmokeGrenadeMgr* Inst();  // ?Inst@SmokeGrenadeMgr@@SAPAV1@XZ (g.o 0x4A83B0)
    float CalcOpacity(const SmokeGrenadeInfo& smokeGrenInfo) const;  // ?CalcOpacity@SmokeGrenadeMgr@@IBEMABUSmokeGrenadeInfo@@@Z (game2.o 0x4FA0E0)
    bool PointCanSeePoint(const float* startPoint, const float* endPoint,
                          float visThreshold);  // ?PointCanSeePoint@SmokeGrenadeMgr@@QAE_NQBM0M@Z (game2.o 0x4FA220)
    bool EntityCanSeePoint(const class Entity* ent, const float* endPoint,
                           float visThreshold);  // ?EntityCanSeePoint@SmokeGrenadeMgr@@QAE_NPBVEntity@@QBMM@Z (game2.o 0x4FA400)
    bool EntityCanSeeEntity(const class Entity* ent, const class Entity* targEnt,
                            float visThreshold);  // ?EntityCanSeeEntity@SmokeGrenadeMgr@@QAE_NPBVEntity@@0M@Z (game2.o 0x4FA460)
    void Update(float deltaT);  // ?Update@SmokeGrenadeMgr@@QAEXM@Z (game2.o 0x4F9CC0)
    void AddSmokeGrenade(const SmokeGrenadeInfo* smokeGrenInfo);  // ?AddSmokeGrenade@SmokeGrenadeMgr@@QAEXABUSmokeGrenadeInfo@@@Z (game2.o 0x4FFBD0)
    void ReInitialize();  // ?ReInitialize@SmokeGrenadeMgr@@QAEXXZ (game2.o 0x504C50)
};
static_assert(sizeof(SmokeGrenadeMgr) == 0xC, "SmokeGrenadeMgr size mismatch");

// Camera mode enums (binary values from codmp_xboxr.xbe.h)
enum EVehicleCameraMode : int32_t {
    VEH_MODE_FIRSTPERSON = 0x0,
    VEH_MODE_CHASECAM = 0x1,
    VEH_MODE_HLO = 0x2,
    VEH_MODE_STRAFE = 0x3,
    VEH_MODE_MAX = 0x4,
};

enum ECameraModes : int32_t {
    CAM_NORMAL_FIRST = 0x0,
    CAM_NORMAL_THIRD = 0x1,
    CAM_VEHICLE_FIRST = 0x2,
    CAM_VEHICLE_THIRD = 0x3,
    CAM_VEHICLE_TANK = 0x4,
    CAM_VEHICLE_TANK_COMMANDER = 0x5,
    CAM_VEHICLE_GUNNER = 0x6,
    CAM_VEHICLE_PASSENGER = 0x7,
    CAM_VEHICLE_ANIMATING = 0x8,
    CAM_LINKED = 0x9,
    CAM_TURRET = 0xA,
    CAM_INTERMISSION = 0xB,
    CAM_SCENE_ANIMATED = 0xC,
    CAM_INTERACTION_LOCKED = 0xD,
    CAM_INTERACTION_FREE = 0xE,
    CAM_MP_DEATH_CAMERA = 0xF,
};

// SetEnvironment(0x602B90) is a no-op in the binary; values unverified.
enum ESoundEnvironment {
    kEnvironmentIndoor = 0,
};

class SoundDevice {
public:
    class Sound {
    public:
        int     mSource;         // +0x00 (nslSourceID; NSL_SOURCE_ID_INVALID == -1)
        int     mWave;           // +0x04 (nslWaveID; NSL_WAVE_ID_INVALID == -1)
        bool    mPaused;         // +0x08
        bool    mAutoRelease;    // +0x09
        float   mPitch;          // +0x0C
        float   mVolume;         // +0x10
        float   mMinRange;       // +0x14
        float   mMaxRange;       // +0x18
        float   mGroupVolume;    // +0x1C
        Handle  mEntHandle;      // +0x20
        Handle  mHandle;         // +0x24
        void*   mPoPtr;          // +0x28
        HashString mDialogNotify;// +0x2C
        float   mDebugPos[3];    // +0x30 (stride 0x3C, verified SetPosition)
        Sound();                 // ??0Sound@SoundDevice@@QAE@XZ (game.o 0x612A10)
        void Reset();            // ?Reset@Sound@SoundDevice@@QAEXXZ (game.o 0x6129C0)
        float GetPlaybackPosition() const;  // ?GetPlaybackPosition@Sound@SoundDevice@@QBEMXZ
        void  SetReverb(bool on);           // ?SetReverb@Sound@SoundDevice@@QAEX_N@Z
        float GetVolume() const;            // ?GetVolume@Sound@SoundDevice@@QBEMXZ
        const char* GetSourceName() const;  // ?GetSourceName@Sound@SoundDevice@@QBEPBDXZ
        bool  IsQueuing() const;            // ?IsQueuing@Sound@SoundDevice@@QBE_NXZ
        bool  IsQueued() const;             // ?IsQueued@Sound@SoundDevice@@QBE_NXZ
        bool  IsPlaying() const;            // ?IsPlaying@Sound@SoundDevice@@QBE_NXZ
        bool  IsPaused() const;             // ?IsPaused@Sound@SoundDevice@@QBE_NXZ
        bool  IsFinished() const;           // ?IsFinished@Sound@SoundDevice@@QBE_NXZ
        bool  IsLooped() const;             // ?IsLooped@Sound@SoundDevice@@QBE_NXZ
        float GetLength() const;            // ?GetLength@Sound@SoundDevice@@QBEMXZ
        ae_fixed_string<1024, unsigned short> GetDebugString() const;  // ?GetDebugString@Sound@SoundDevice@@QBE?AV?$ae_fixed_string@$0EAA@G@@XZ (game.o 0x6216F0)
        void PlayQueued();                  // ?PlayQueued@Sound@SoundDevice@@QAEXXZ
        void Pause();                       // ?Pause@Sound@SoundDevice@@QAEXXZ
        void Unpause();                     // ?Unpause@Sound@SoundDevice@@QAEXXZ
        void DampenGuard();                 // ?DampenGuard@Sound@SoundDevice@@QAEXXZ
        ~Sound();                           // ??1Sound@SoundDevice@@QAE@XZ (game.o 0x62C020)
        void Stop();                        // ?Stop@Sound@SoundDevice@@QAEXXZ (game.o 0x62C0D0)
        void SetVolume(float vol);          // ?SetVolume@Sound@SoundDevice@@QAEXM@Z (game.o 0x62C210)
        void SetPitch(float pitch);         // ?SetPitch@Sound@SoundDevice@@QAEXM@Z (game.o 0x62C2D0)
        void SetRange(float min, float max);// ?SetRange@Sound@SoundDevice@@QAEXMM@Z (game.o 0x62C380)
        void SetPosition(const math::Position3& pos);  // ?SetPosition@Sound@SoundDevice@@QAEXABVPosition3@math@@@Z (game.o 0x62C470)
        void SetVelocity(const math::Dir3& vel);      // ?SetVelocity@Sound@SoundDevice@@QAEXABVDir3@math@@@Z (game.o 0x62C630)
        void SetPoPtr(const math::Mat43* poPtr);      // ?SetPoPtr@Sound@SoundDevice@@QAEXPBVMat43@math@@@Z (game.o 0x63A040)
        void Queue(nslWaveID wave, float vol, float pitch, float minrange,
                   float maxrange, const math::Position3* pos,
                   const math::Dir3* vel, bool autoRelease,
                   DbLinkedHandle<EntityHandleDb, Entity> entHandle,
                   bool mImportant);                  // ?Queue@Sound@SoundDevice@@QAEXW4nslWaveID@@MMMMABVPosition3@math@@ABVDir3@5@_NV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@3@Z (game.o 0x6399D0)
        void Play(nslWaveID wave, float vol, float pitch, float minrange,
                  float maxrange, const math::Position3* pos,
                  const math::Dir3* vel, bool autoRelease,
                  DbLinkedHandle<EntityHandleDb, Entity> entHandle,
                  bool mImportant);                    // ?Play@Sound@SoundDevice@@QAEXW4nslWaveID@@MMMMABVPosition3@math@@ABVDir3@5@_NV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@3@Z (game.o 0x639CE0)
        void Update();                      // ?Update@Sound@SoundDevice@@QAEXXZ (game.o 0x62C7A0)
    };
    class SoundHandleDb {
    public:
        struct DbElement {
            Sound* mObject;  // +0x00
            int    mKey;     // +0x04
        };
        uint8_t   _pad[0x40];
        DbElement mElements[0x200];  // +0x40
        static SoundHandleDb sInst;         // ?sInst@SoundHandleDb@SoundDevice@@0V12@A @ 0xF50D10
        // HandleDb<Sound,512,SizedHandle<12,20>> inline methods (COMDAT in
        // binary; ported from ea 0x6627B0/0x660710/0x661D80).
        Handle AllocateHandle() {
            Handle result;
            for (int i = 0; i < 0x200; ++i) {
                if ((_pad[i >> 3] & (1u << (i & 7))) == 0)
                    continue;
                _pad[i >> 3] &= (uint8_t)~(1u << (i & 7));
                result.mVal = (unsigned int)((mElements[i].mKey << 12) | i);
                return result;
            }
            result.mVal = 0xFFFFFFFFu;
            return result;
        }
        void BindObjectToHandle(Handle handle, Sound* obj) {
            unsigned int idx = handle.mVal & 0xFFF;
            if (idx < 0x200
                && (unsigned int)mElements[idx].mKey == (handle.mVal >> 12))
                mElements[idx].mObject = obj;
        }
        void ReleaseHandle(Handle h) {
            if (h.mVal == 0)
                return;
            unsigned int idx = h.mVal & 0xFFF;
            if (idx < 0x200
                && (unsigned int)mElements[idx].mKey == (h.mVal >> 12)) {
                _pad[idx >> 3] |= (uint8_t)(1u << (idx & 7));
                mElements[idx].mObject = nullptr;
                ++mElements[idx].mKey;
            }
        }
    };
private:
    nslBankID SyncLoadBank(const char* filename);  // ?SyncLoadBank@SoundDevice@@AAE?AW4nslBankID@@PBD@Z (game.o 0x6024A0)
    void UpdateListener();                         // ?UpdateListener@SoundDevice@@AAEXXZ (game.o 0x603AC0)
    int  GetFreeSlot();                            // ?GetFreeSlot@SoundDevice@@AAEHXZ (game.o 0x63A060)
public:
    struct CrossFadeInfo {
        Handle mSound1;         // +0x00
        Handle mSound2;         // +0x04
        float  mAdjustVolume1;  // +0x08
        float  mAdjustVolume2;  // +0x0C
        float  mRemainingTime;  // +0x10
    };
    enum EOutputMode {
        kMono = 0,
        kStereo = 1,
        kHeadPhones = 2,
        kSurround = 3,
    };
    Sound mSounds[512];              // +0x00 (0x3C stride)
    int   mNumberOfListeners;        // +0x7800
    void*   mNslBuffer;              // +0x7804 (NSL work buffer)
    uint8_t mNslParams[0x44];        // +0x7808 (nslInitParams, 0x44 bytes)
    nslBankID mMainBank;             // +0x784C
    float mVolScale;                 // +0x7850
    bool  mUpdateReverb;             // +0x7854
    uint8_t _pad7855[0x7858 - 0x7855];
    unsigned int mTargetReverb[14];  // +0x7858
    unsigned int mCurrentReverb[14]; // +0x7890
    unsigned int mDeltaReverb[14];   // +0x78C8 (read by UpdateReverb only)
    float mRemainingReverbBlendTime; // +0x7900
    CrossFadeInfo mCrossFadeInfo[16];// +0x7904 (16 * 20 bytes)
    uint8_t _pad7A44[0x7A50 - 0x7A44];
    float mBusPitchTargetPitch;      // +0x7A50
    float mBusPitchRemainingTime;    // +0x7A54
    float mBusPitchDeltaPitch;       // +0x7A58
    float mBusPitchCurrentPitch;     // +0x7A5C
    float mBusVolumeTargetVolume;    // +0x7A60
    float mBusVolumeRemainingTime;   // +0x7A64
    float mBusVolumeDeltaVolume;     // +0x7A68
    float mBusVolumeCurrentVolume;   // +0x7A6C
    cvar_t* mShowStreams;            // +0x7A70
    cvar_t* mShowListenerPosition;   // +0x7A74
    cvar_t* mShowEmitterPosition;    // +0x7A78
    float mDebugListenerPosition[3];  // +0x7A7C
    float mDebugListenerForward[3];   // +0x7A88
    float mDebugListenerUp[3];        // +0x7A94
    static SoundDevice* sInst;      // ?sInst@SoundDevice@@2PAV1@A
    static SoundDevice* Inst();     // ?Inst@SoundDevice@@SAPAV1@XZ (g.o 0x4A8520)
    SoundDevice();                  // ??0SoundDevice@@QAE@XZ (game.o 0x6397F0)
    ~SoundDevice();                 // ??1SoundDevice@@QAE@XZ (game.o 0x646610)
    nslWaveID FindWave(const char* name);  // ?FindWave@SoundDevice@@QAE?AW4nslWaveID@@PBD@Z (game.o 0x612980)
    float GetWaveDuration(nslWaveID wave);  // ?GetWaveDuration@SoundDevice@@QAEMW4nslWaveID@@@Z (game.o 0x6025A0)
    void ScaleVolume(float scale);          // ?ScaleVolume@SoundDevice@@QAEXM@Z (game.o 0x602A10)
    void PauseAllSounds();                  // ?PauseAllSounds@SoundDevice@@QAEXXZ (game.o 0x602A80)
    Sound* GetSoundFromSourceId(nslSourceID id);  // ?GetSoundFromSourceId@SoundDevice@@QAEPAVSound@1@W4nslSourceID@@@Z
    void DampenAllSounds(float level);      // ?DampenAllSounds@SoundDevice@@QAEXM@Z (game.o 0x602B60)
    void UndampenAllSounds();               // ?UndampenAllSounds@SoundDevice@@QAEXXZ (game.o 0x602B80)
    void SetEnvironment(ESoundEnvironment env);  // ?SetEnvironment@SoundDevice@@QAEXW4ESoundEnvironment@@@Z (game.o 0x602B90)
    void UpdateReverb(float deltaTime);     // ?UpdateReverb@SoundDevice@@QAEXM@Z (game.o 0x603640)
    bool BusVolumeIsName(const char* name); // ?BusVolumeIsName@SoundDevice@@QAE_NPBD@Z (game.o 0x603820)
    void BusPitchFade(const char* busName, float pitch, float time);  // game.o 0x603840
    void BusVolumeFade(const char* busName, float volume, float time);  // game.o 0x6038C0
    void BusPitchAddBus(const char* busName);   // game.o 0x603920
    void BusVolumeAddBus(const char* busName);  // game.o 0x603940
    void BusPitchRemoveBus(const char* busName);// game.o 0x603960
    void BusVolumeRemoveBus(const char* busName);// game.o 0x603980
    void UpdateBusPitchFade(float deltaTime);   // game.o 0x6039A0
    void UpdateBusVolumeFade(float deltaTime);  // game.o 0x603A30
    float GetGroupVolume(const char* group) const;  // ?GetGroupVolume@SoundDevice@@QBEMPBD@Z (game.o 0x603D90)
    void SetOutputMode(EOutputMode mode);       // game.o 0x603DD0
    EOutputMode GetOutputMode() const;          // game.o 0x603E30
    void DebugRender();                         // ?DebugRender@SoundDevice@@QAEXXZ (game.o 0x62CD90)
    static void SingletonDebugRender();         // ?SingletonDebugRender@SoundDevice@@SAXXZ (game.o 0x6629D0)
    void FrameAdvance(float delta);             // ?FrameAdvance@SoundDevice@@QAEXM@Z (game.o 0x63A5A0)
    Sound* GetSoundForHandle(DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> handle);  // game.o 0x621670
    const Sound* GetSoundForHandle(DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> handle) const;  // game.o 0x6216B0
    void UnpauseAllSounds();                // ?UnpauseAllSounds@SoundDevice@@QAEXXZ (game.o 0x602AE0)
    int GetNumberOfListeners();             // ?GetNumberOfListeners@SoundDevice@@QAEHXZ (game.o 0x602B10)
    void SetNumberOfListeners(int listeners);  // ?SetNumberOfListeners@SoundDevice@@QAEXH@Z (game.o 0x602B20)
    bool IsSoundReady();                    // ?IsSoundReady@SoundDevice@@QAE_NXZ (game.o 0x602B40)
    void SetListenerVectors(int listener, const math::Position3& position,
                            const math::Dir3& front,
                            const math::Dir3& up);  // ?SetListenerVectors@SoundDevice@@QAEXHABVPosition3@math@@ABVDir3@3@1@Z (game.o 0x612A70)
    void StopAllSounds();
    void ReleaseSound(Sound* s);      // ?ReleaseSound@SoundDevice@@QAEXPAVSound@1@@Z (game.o 0x62C9B0)
    void ReleaseSound(DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> s);  // ?ReleaseSound@SoundDevice@@QAEXV?$DbLinkedHandle@VSoundHandleDb@SoundDevice@@VSound@2@@@@Z (game.o 0x62C9C0)
    void StopAllSoundsNotPaused();    // ?StopAllSoundsNotPaused@SoundDevice@@QAEXXZ (game.o 0x62CA10)
    void UpdateCrossFade(float deltaTime);  // ?UpdateCrossFade@SoundDevice@@QAEXM@Z (game.o 0x62CA40)
    void CrossFade(unsigned int sound1, unsigned int sound2,
                   float crossFadeTime);    // ?CrossFade@SoundDevice@@QAEXIIM@Z (game.o 0x62CBE0)
    void SetReverb(const char* preset, bool immediate);  // ?SetReverb@SoundDevice@@QAEXPBD_N@Z (game.o 0x9F20A0)
    DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> PlaySound(
        const char* name, DbLinkedHandle<EntityHandleDb, Entity> ent,
        bool a4, bool a5, const math::Position3& pos, const math::Dir3& dir,
        float a8, float a9, float a10,
        float a11);  // ?PlaySound@SoundDevice@@QAE?AV?$DbLinkedHandle@VSoundHandleDb@SoundDevice@@VSound@2@@@PBDV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@_N2ABVPosition3@math@@ABVDir3@5@MMMM@Z
    DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> PlaySound(
        nslWaveID id, DbLinkedHandle<EntityHandleDb, Entity> ent,
        bool a4, bool a5, const math::Position3& pos, const math::Dir3& dir,
        float a8, float a9, float a10,
        float a11);  // ?PlaySound@SoundDevice@@QAE?AV?$DbLinkedHandle@VSoundHandleDb@SoundDevice@@VSound@2@@@W4nslWaveID@@V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@_N2ABVPosition3@math@@ABVDir3@6@MMMM@Z (game.o 0x63A260)
    DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> QueueSound(
        nslWaveID id, DbLinkedHandle<EntityHandleDb, Entity> entHandle,
        bool mImportant, bool autoRelease, const math::Position3& pos,
        const math::Dir3& vel, float vol, float pitch, float min,
        float max);  // ?QueueSound@SoundDevice@@QAE?AV?$DbLinkedHandle@VSoundHandleDb@SoundDevice@@VSound@2@@@W4nslWaveID@@V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@_N2ABVPosition3@math@@ABVDir3@6@MMMM@Z (game.o 0x63A0D0)
};
static_assert(sizeof(SoundDevice) == 31392, "SoundDevice size mismatch");

// ============================================================================
// Additional cross-object singleton managers (fields used by sv.o only)
// ============================================================================
// SCheckpointGameVar - checkpoint game variable (12 bytes) - verified IDA
struct SCheckpointGameVar {
    unsigned int mHashVarName;  // +0x00
    unsigned int mVal;          // +0x04
    unsigned int mDataSize;     // +0x08
};

// ae_vector<T> - dynamic array (12 bytes) - verified against IDA
template <typename T>
struct CheckpointVector {
    T*  mElements;  // +0x00
    int mCapacity;  // +0x04
    int mSize;      // +0x08
};

class CheckpointMgr {
public:
    bool         mUsingCheckpoints;       // +0x00 (bool)
    uint8_t      _pad01[3];               // +0x01
    int          mPlayerHealth;           // +0x04
    Broc::string mWeapons[6];             // +0x08
    int          mWeaponAmmo[6];          // +0x20
    int          mWeaponClipAmmo[6];      // +0x38
    int          ammo[92];                // +0x50 (0x170 bytes)
    int          ammoclip[92];            // +0x1C0 (0x170 bytes)
    int          weapons[2];              // +0x330
    char         weaponslots[10];         // +0x338
    int          weaponrechamber[2];      // +0x344
    int          weapon;                  // +0x34C
    float        mTimeRemainingForHudText;// +0x350
    bool         mCurrentlySavingCheckpoint;  // +0x354
    bool         mCheckpointFromStorage;      // +0x355
    struct ExplodedArray {
        unsigned short mElements[256];    // +0x00
        int            m_size;            // +0x200
    };
    ExplodedArray mCurrentScriptExploded; // +0x358
    float        mPlayerOrientation[3];   // +0x55C
    math::Position3 mOrigin;              // +0x568 (checkpoint player origin)
    bool         mCheckpointSaveExists;   // +0x574 (bool)
    static CheckpointMgr* Inst();                  // ?Inst@CheckpointMgr@@SAPAV1@XZ (g.o 0x4A9920)
    bool CheckpointSaveExists();                   // ?CheckpointSaveExists@CheckpointMgr@@QAE_NXZ (g.o 0x4A9930)
    bool IsRestoringCheckpoint() const;            // ?IsRestoringCheckpoint@CheckpointMgr@@QBE_NXZ (g.o 0x4A9940)
    const float (&GetPlayerPosition() const)[3];   // ?GetPlayerPosition@CheckpointMgr@@QBEAAY02$$CBMXZ (g.o 0x4A9960)
    struct SEntitySaveInfo {
        char  mTargetname[32];            // +0x00
        float mOrientation[3];            // +0x20
        float mOrigin[3];                 // +0x2C
    } mFriendlies[16];                    // +0x578
    int          mFriendlyCount;          // +0x8F8
    Broc::string mEvent;                  // +0x8FC (Broc::string)
    Broc::string mCurrentMapName;         // +0x900 (Broc::string)
    CheckpointVector<SCheckpointGameVar> mGameVars;  // +0x904 (12 bytes)
    ExplodedArray mCheckpointScriptExploded;        // +0x910
    int          mCheckpointIndex;        // +0xB14
    static CheckpointMgr* sInst;          // ?sInst@CheckpointMgr@@2PAV1@A
    CheckpointMgr();                      // ??0CheckpointMgr@@QAE@XZ (game.o 0x6220C0)
    ~CheckpointMgr();                     // ??1CheckpointMgr@@QAE@XZ (game.o 0x6221F0)
    void ClearSavedCheckpointData();       // ?ClearSavedCheckpointData@CheckpointMgr@@QAEXXZ (game.o 0x640E60)
    void SaveCheckpoint(const char* checkpointName, bool calledFromScript);  // ?SaveCheckpoint@CheckpointMgr@@QAEXPBD_N@Z
    void LoadCheckpointFromStubData();     // ?LoadCheckpointFromStubData@CheckpointMgr@@QAEXXZ (game.o 0x632120)
    const char* GetCheckpointMapName();    // ?GetCheckpointMapName@CheckpointMgr@@QAEPBDXZ (sv.o 0x528190)
    void SetCheckpointCvar();             // ?SetCheckpointCvar@CheckpointMgr@@QAEXXZ
    void RestoreExplodedExploders();      // ?RestoreExplodedExploders@CheckpointMgr@@QAEXXZ
    void RestoreSceneEntity(Entity* pEnt);  // ?RestoreSceneEntity@CheckpointMgr@@QAEXPAVEntity@@@Z (game.o 0x609010)
    bool SceneEntityWasDeletedBeforeCheckpoint(SceneEntity* pSceneEnt,
                                               SceneBank* pScnBank);  // ?SceneEntityWasDeletedBeforeCheckpoint@CheckpointMgr@@QAE_NPAVSceneEntity@@PAVSceneBank@@@Z (game.o 0x6180F0)
    void RestorePlayerHealth();           // ?RestorePlayerHealth@CheckpointMgr@@QAEXXZ (game.o 0x609240)
    void SetTosserValues();               // ?SetTosserValues@CheckpointMgr@@QAEXXZ (game.o 0x609340)
    void RestoreScriptExploders();        // ?RestoreScriptExploders@CheckpointMgr@@QAEXXZ (game.o 0x609350)
    bool Restart();                       // ?Restart@CheckpointMgr@@QAE_NXZ (game.o 0x609330)
    void ReInit();                        // ?ReInit@CheckpointMgr@@QAEXXZ (game.o 0x631490)
    void ClearGameVars();                 // ?ClearGameVars@CheckpointMgr@@QAEXXZ (game.o 0x632110)
    void SetEvent(const char* checkpointName);  // ?SetEvent@CheckpointMgr@@QAEXPBD@Z (game.o 0x618120)
    void RestoreLastCheckpoint();         // ?RestoreLastCheckpoint@CheckpointMgr@@QAEXXZ (game.o 0x618130)
    bool GetGameVar(unsigned int hashVarName, unsigned int* val,
                    unsigned int dataSize);  // ?GetGameVar@CheckpointMgr@@QAE_NIPAI I@Z (game.o 0x6182C0)
    void SetGameVar(unsigned int hashVarName, unsigned int* val,
                    unsigned int dataSize);  // ?SetGameVar@CheckpointMgr@@QAEXIPAI I@Z (game.o 0x622290)
    bool ExploderCheckpointExploded(int exploderId);  // ?ExploderCheckpointExploded@CheckpointMgr@@QAE_NH@Z (game.o 0x622330)
    bool PrecludeExploderPiece(const char* exploderType,
                               int exploderId);  // ?PrecludeExploderPiece@CheckpointMgr@@QAE_NPBDH@Z (game.o 0x622370)
};
// size not asserted (opaque; verified fields at +0x00/+0x04/+0x50/+0x1C0/
// +0x330/+0x338/+0x344/+0x34C/+0x350/+0x358/+0x574/+0x904/+0x910 from
// checkpointmgr.cpp disasm)

struct PakInfoNode;
class PanelFileUser;
class NumBanks {
public:
    struct Ps3Banks {
        float main;  // +0x00
        float lram;  // +0x04
    };
    struct GcBanks {
        float main;  // +0x00
        float aram;  // +0x04
    };
    float    ps2;              // +0x00
    Ps3Banks ps3;              // +0x04
    float    xbox;             // +0x0C
    float    xenon;            // +0x10
    float    pcx;              // +0x14
    GcBanks  gc;               // +0x18
};

class PakManager {
public:
    uint8_t _pad[0x28];
    unsigned int mEnabled;             // +0x28
    static PakManager* sInst;            // ?sInst@PakManager@@2PAV1@A
    void* mProgressCallback;             // +0x2C
    uint8_t _pad30[0x70 - 0x30];
    struct { void* m_head; void* m_end; } mActivePaks;  // +0x70
    TPakId FindPakId(EPakType t) const;  // ?FindPakId@PakManager@@QBE?AW4TPakId@@W4EPakType@@@Z
    void* MemAlloc(TPakId id, unsigned int size, bool bUseActorHeap);  // ?MemAlloc@PakManager@@QAEPAXW4TPakId@@I_N@Z
    bool IsLoaded(TPakId id) const;  // ?IsLoaded@PakManager@@QBE_NW4TPakId@@@Z
    bool IsUnloaded(TPakId id) const;  // ?IsUnloaded@PakManager@@QBE_NW4TPakId@@@Z (streamer.o)
    bool IsLoading(TPakId id) const;   // ?IsLoading@PakManager@@QBE_NW4TPakId@@@Z (streamer.o)
    void FillBanks();                    // ?FillBanks@PakManager@@QAEXXZ
    void UnloadAll();                    // ?UnloadAll@PakManager@@QAEXXZ
    void SetSoundProgress(float t);      // ?SetSoundProgress@PakManager@@QAEXM@Z (streamer.o 0x665710)
    void ResetPriorities(bool user_distances_also);  // ?ResetPriorities@PakManager@@QAEX_N@Z
    void SetUserDistance(const PakInfoNode* cpak, float dist);  // ?SetUserDistance@PakManager@@QAEXPBUPakInfoNode@@M@Z
    void* CrazyTempMemBorrow(unsigned int align, unsigned int size);  // ?CrazyTempMemBorrow@PakManager@@QAEPAXII@Z (streamer.o)
    void  CrazyTempMemGiveBack(void* ptr);  // ?CrazyTempMemGiveBack@PakManager@@QAEXPAX@Z (streamer.o)
    const PakInfoNode* GetPakInfo(TPakId pakId) const;  // ?GetPakInfo@PakManager@@QBEPBUPakInfoNode@@W4TPakId@@@Z
    const PakInfoNode* GetPakInfo(const char* long_name) const;  // ?GetPakInfo@PakManager@@QBEPBUPakInfoNode@@PBD@Z
    bool IsUnloading(TPakId id) const;   // ?IsUnloading@PakManager@@QBE_NW4TPakId@@@Z
    void MemFree(TPakId id, void* ptr, bool bUseActorHeap);  // ?MemFree@PakManager@@QAEXW4TPakId@@PAX_N@Z
    TPakId GetGlobalPakId() const;       // ?GetGlobalPakId@PakManager@@QBE?AW4TPakId@@XZ (core.o)
    TPakId GetTopContext() const;        // ?GetTopContext@PakManager@@QBE?AW4TPakId@@XZ (streamer.o)
    void* MemAlign(TPakId id, unsigned int align, unsigned int size);  // ?MemAlign@PakManager@@QAEPAXW4TPakId@@II@Z
    void PushContext(TPakId id);         // ?PushContext@PakManager@@QAEXW4TPakId@@@Z (streamer.o)
    TPakId PopContext();                 // ?PopContext@PakManager@@QAE?AW4TPakId@@XZ (streamer.o)
    void Update(bool calledFromMovie);   // ?Update@PakManager@@QAEX_N@Z (streamer.o)
    const PakInfoNode* SyncLoadFLI(EPakType t, const char* path);  // ?SyncLoadFLI@PakManager@@QAEPBUPakInfoNode@@W4EPakType@@PBD@Z
    TPakId SyncLoadPak(const PakInfoNode* cpak);  // ?SyncLoadPak@PakManager@@QAE?AW4TPakId@@PBUPakInfoNode@@@Z
    TPakId SyncLoadPak(EPakType t, const char* path, NumBanks banks);  // ?SyncLoadPak@PakManager@@QAE?AW4TPakId@@W4EPakType@@PBDVNumBanks@@@Z
    void SyncUnloadPak(TPakId id);       // ?SyncUnloadPak@PakManager@@QAEXW4TPakId@@@Z
    void ClearUserDistance(const PakInfoNode* cpak);  // ?ClearUserDistance@PakManager@@QAEXPBUPakInfoNode@@@Z
    void SetProgressCallback(void (*cb)(float));  // ?SetProgressCallback@PakManager@@QAEXP6AXM@Z@Z
};
// size not asserted (opaque; mActivePaks at +0x70)

// ============================================================================
// EntityHandleDb â€” entity handle database (opaque; only element lookup used)
// HandleDb<Entity,1344,SizedHandle<12,20>>::DbElement = { int mKey; Entity* mObject; }
// ============================================================================
struct EntityHandleDbDbElement {
    Entity*        mObject; // +0x00
    int            mKey;    // +0x04
};
// ae_sized_array<Entity*, 4096> â€” fixed-capacity array (16388 bytes)
struct AeSizedEntityArray {
    Entity*     m_elements[4096];   // +0x00
    int         m_size;             // +0x4000
};
static_assert(sizeof(AeSizedEntityArray) == 16388, "AeSizedEntityArray size mismatch");

class EntityHandleDb {
public:
    uint8_t  _pad[0xA8];                 // HandleDb BitSet<1344> (168 bytes)
    EntityHandleDbDbElement mElements[0x540];  // +0xA8 (1344 * 8 = 10752)
    void     (*mDebugCallback)(int, Entity*);  // +0x2AA8
    AeSizedEntityArray mActiveList;      // +0x2AAC (16388 bytes)
    static EntityHandleDb sInst;         // ?sInst@EntityHandleDb@@0V1@A
    EntityHandleDb();                    // ??0EntityHandleDb@@QAE@XZ (g.o 0x4B3D50)
    static EntityHandleDb* Inst();       // ?Inst@EntityHandleDb@@SAPAV1@XZ (g.o 0x4A9D40)
    Entity* Find(int fieldofs, HashString match);  // ?Find@EntityHandleDb@@QAEPAVEntity@@HVHashString@@@Z (g.o 0x4B0F80)
    const ae_sized_array<Entity*, 4096>& GetActiveList() const;  // ?GetActiveList@EntityHandleDb@@QBEABV?$ae_sized_array@PAVEntity@@$0BAAA@@@XZ (g.o 0x4A9D50)
    Entity* GetObject(int idx) const;    // ?GetObject@?$HandleDb@VEntity@@$0FEA@V?$SizedHandle@$0M@$0BE@@@@@QBEPAVEntity@@H@Z (g.o 0x4B0DA0)
    void BindObjectToHandle(Handle handle, Entity* obj);  // ?BindObjectToHandle@?$HandleDb@VEntity@@$0FEA@V?$SizedHandle@$0M@$0BE@@@@@QAEXVHandle@@PAVEntity@@@Z (g.o 0x4B0E20)
    void Init();                         // ?Init@EntityHandleDb@@QAEXXZ
    void AssignHandle(Entity& e);        // ?AssignHandle@EntityHandleDb@@QAEXAAVEntity@@@Z
    // Inline handle lookup (used at every call site in the binary)
    Entity* GetObject(unsigned int val) const {
        unsigned int idx = val & 0xFFF;
        if (idx < 0x540 && val >> 12 == (unsigned int)mElements[idx].mKey)
            return mElements[idx].mObject;
        return NULL;
    }
    Entity* Find(int fieldofs, HashString match) const;  // ?Find@EntityHandleDb@@QBEPAVEntity@@HVHashString@@@Z
    void Find(int fieldOfs, unsigned short match, ae_sized_array<Entity*, 4096>& results) const;  // ?Find@EntityHandleDb@@QBEXGAAV?$ae_sized_array@PAVEntity@@$0BAAA@@@@Z
    void Find(int fieldOfs, HashString match, ae_sized_array<Entity*, 4096>& results) const;      // ?Find@EntityHandleDb@@QBEXVHashString@@AAV?$ae_sized_array@PAVEntity@@$0BAAA@@@@Z
    void Find(int fieldOfs, const Broc::string& match, ae_sized_array<Entity*, 4096>& results) const;  // ?Find@EntityHandleDb@@QBEXHABVstring@Broc@@AAV?$ae_sized_array@PAVEntity@@$0BAAA@@@@Z
    ae_sized_array<Entity*, 4096>::const_iterator Find(
        int fieldofs, unsigned short match,
        ae_sized_array<Entity*, 4096>::const_iterator begin,
        ae_sized_array<Entity*, 4096>::const_iterator end) const;  // ?Find@EntityHandleDb@@QBE?AVconst_iterator@?$ae_sized_array@PAVEntity@@$0BAAA@@@HGV23@0@Z
    ae_sized_array<Entity*, 4096>::const_iterator Find(
        int fieldofs, HashString match,
        ae_sized_array<Entity*, 4096>::const_iterator begin,
        ae_sized_array<Entity*, 4096>::const_iterator end) const;  // ?Find@EntityHandleDb@@QBE?AVconst_iterator@?$ae_sized_array@PAVEntity@@$0BAAA@@@HVHashString@@V23@1@Z
    void Release(Entity& e);                    // ?Release@EntityHandleDb@@QAEXAAVEntity@@@Z
    Entity* Find(int fieldofs, unsigned short match) const;  // ?Find@EntityHandleDb@@QBEPAVEntity@@HG@Z
    Entity* Find(int fieldofs, const Broc::string& match) const;  // ?Find@EntityHandleDb@@QBEPAVEntity@@HABVstring@Broc@@@Z
    ae_sized_array<Entity*, 4096>::const_iterator Find(
        int fieldofs, Broc::string match,
        ae_sized_array<Entity*, 4096>::const_iterator begin,
        ae_sized_array<Entity*, 4096>::const_iterator end) const;  // ?Find@EntityHandleDb@@QBE?AVconst_iterator@?$ae_sized_array@PAVEntity@@$0BAAA@@@HVstring@Broc@@V23@1@Z
    void Compact();                         // ?Compact@EntityHandleDb@@QAEXXZ

private:
    void Validate();                        // ?Validate@EntityHandleDb@@AAEXXZ

public:
};
static_assert(offsetof(EntityHandleDb, mElements) == 0xA8, "EntityHandleDb::mElements offset mismatch");
static_assert(offsetof(EntityHandleDb, mActiveList) == 0x2AAC, "EntityHandleDb::mActiveList offset mismatch");
static_assert(sizeof(EntityHandleDb) == 27312, "EntityHandleDb size mismatch");

// ============================================================================
// XModelManager â€” model manager (opaque)
// ============================================================================
struct XModel;
class XModelManager {
public:
    uint8_t _pad[4];
    static XModelManager* sInst;           // ?sInst@XModelManager@@2PAV1@A
    static XModelManager* Inst();          // ?Inst@XModelManager@@SAPAV1@XZ
    IVPointer<XModel> GetXModel(TPakId pak_id, const char* name);
};
static_assert(sizeof(XModelManager) == 4, "XModelManager size mismatch (opaque)");

// ============================================================================
// EntityManager â€” entity factory (opaque; only sv.o fields used)
// ============================================================================
struct AssetBankSet {
    virtual ~AssetBankSet();          // ??1AssetBankSet@@UAE@XZ (streamer.o 0x675850)
    AssetBankSet();                   // ??0AssetBankSet@@QAE@XZ (streamer.o 0x6663A0)
};

class EntityManager : public AssetBankSet {
public:
    static EntityManager* sInst;            // ?sInst@EntityManager@@2PAV1@A
    EntityManager();                        // ??0EntityManager@@QAE@XZ (game.o 0x612420)
    virtual ~EntityManager();               // ??1EntityManager@@UAE@XZ (game.o 0x612470)
    static EntityManager* Inst();           // ?Inst@EntityManager@@SAPAV1@XZ (g.o inline)
    Entity* GetPlayer(int idx);             // ?GetPlayer@EntityManager@@QAEPAVEntity@@H@Z (g.o inline)
    Entity* GetWorld();                     // ?GetWorld@EntityManager@@QAEPAVEntity@@XZ (g.o inline)
    bool IsLocalPlayer(Entity* entity);     // ?IsLocalPlayer@EntityManager@@QAE_NPAVEntity@@@Z (game.o)
    Entity* mPlayers[16];                   // +0x04 (player entity handles)
    Entity* mWorld;                         // +0x44
    void SwapPlayers(int eA, int eB);       // ?SwapPlayers@EntityManager@@QAEXHH@Z (game.o)
    void CreatePlayers();                   // ?CreatePlayers@EntityManager@@QAEXXZ (game.o)
    void CreateWorld();                     // ?CreateWorld@EntityManager@@QAEXXZ (game.o)
    void DeleteAllEntities();               // ?DeleteAllEntities@EntityManager@@QAEXXZ (game.o)
    int  GetEntityController(Entity* entity);  // ?GetEntityController@EntityManager@@QAEHPAVEntity@@@Z (game.o)
    int  GetPlayerIndex(Entity* entity);    // ?GetPlayerIndex@EntityManager@@QAEHPAVEntity@@@Z (game.o)
    Entity* GetFirstLocalPlayer();          // ?GetFirstLocalPlayer@EntityManager@@QAEPAVEntity@@XZ (game.o)

private:
    virtual void UnloadBank(TPakId pakId);  // ?UnloadBank@EntityManager@@EAEXW4TPakId@@@Z (game.o)
};
static_assert(offsetof(EntityManager, mPlayers) == 0x04, "EntityManager::mPlayers offset mismatch");
static_assert(offsetof(EntityManager, mWorld) == 0x44, "EntityManager::mWorld offset mismatch");
static_assert(sizeof(EntityManager) == 0x48, "EntityManager size mismatch");

// ============================================================================
// AeAssert â€” assertion system (namespace-style free functions + globals)
// ============================================================================
namespace AeAssert {
    enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, MJK = 4, MJU = 5,
                    MM = 6, TPB = 7, SLB = 8, AC = 9, JSV = 0xA, DK = 0xB,
                    PL = 0xC, DL = 0xD };
    extern ECoderId gCurrentAuthor;  // ?gCurrentAuthor@AeAssert@@3W4ECoderId@1@A
    extern const char* gCurrentFile;  // ?gCurrentFile@AeAssert@@3PBDB
    extern int  gCurrentLine;         // ?gCurrentLine@AeAssert@@3HA
    extern const char* gCurrentExpr;  // ?gCurrentExpr@AeAssert@@3PBDB
    bool IsIgnored(void);
    bool Assert(const char* fmtstring, ...);
    bool Warning(const char* fmtstring, ...);
    bool Error(const char* fmtstring, ...);  // ?Error@AeAssert@@YA_NPBDZZ
}

// ============================================================================
// movie_manager â€” static movie helpers
// ============================================================================
struct PakInfoNode;
struct PakHeader {
    struct Section;
};
enum nvlFrameState : int;
class nvlMovie;
struct nglQuad;

class movie_manager {
public:
    static void RegisterMovies(const PakInfoNode* paks,
                               const PakHeader::Section& section);  // 0x56B430
    static void init_manager();       // 0x56B440
    static void set_quad_pos(float upper_left_x, float upper_left_y,
                             float lower_right_x, float lower_right_y);  // 0x56B650
    static void movie_done(bool clear_backbuffer);  // 0x57C180
    static void shut_down();          // 0x57C240
    static void render();             // 0x5843F0
    static nvlFrameState render2();   // 0x584430
    static void frame_advance();      // 0x58D330
    static bool load_movie(const char* movie_name,
                           const char* sound_name);  // 0x5913B0
    static void PakLoadCallback(float);  // 0x591640
    static void continue_playing_wait_for_keypress();  // 0x591690
    static void load_and_play_movie(const char* movie_name,
                                    const char* sound_name);  // 0x593AF0

    static nvlMovie* theMovie;    // ?theMovie@movie_manager@@2PAVnvlMovie@@A
    static bool preloadDone;      // ?preloadDone@movie_manager@@2_NA
    static bool ignoreLocalizedTrack;  // ?ignoreLocalizedTrack@movie_manager@@2_NA
    static float wait_timer;      // ?wait_timer@movie_manager@@2MA
private:
    static bool force_exit();     // 0x56B450 (CA_N)
    static bool init_movie(const char* movie_name);  // 0x56B4C0 (CA_NPBD)
    static bool dont_play_movies();  // 0x56B640 (CA_N)
    static float delay_timer;     // ?delay_timer@movie_manager@@0MA
    static nglQuad* theQuad;      // ?theQuad@movie_manager@@0PAUnglQuad@@A
    static int lastTick;          // ?lastTick@movie_manager@@0HA
};

class subtitle_manager {
public:
    static void stop();                       // 0x56B6A0
    static int  GetCol(int);                  // 0x56B6B0
    static char* GetLevelName();              // 0x56B6C0
    static void render();                     // 0x57C260
    static bool play_subtitle(const char* tag, char* prefix);  // 0x584460
    static void frame_advance(int time_delta);  // 0x5848C0

    static char* token1;         // ?token1@subtitle_manager@@0PADA
    static char* token2;         // ?token2@subtitle_manager@@0PADA
    static unsigned int m_color; // ?m_color@subtitle_manager@@0IA
    static float m_scale;        // ?m_scale@subtitle_manager@@0MA
    static float timeRef;        // ?timeRef@subtitle_manager@@0MA
    static float timeStart;      // ?timeStart@subtitle_manager@@0MA
    static float timeEnd;        // ?timeEnd@subtitle_manager@@0MA
    static unsigned int posX1;   // ?posX1@subtitle_manager@@0IA
    static unsigned int posY1;   // ?posY1@subtitle_manager@@0IA
    static unsigned int posX2;   // ?posX2@subtitle_manager@@0IA
    static unsigned int posY2;   // ?posY2@subtitle_manager@@0IA
    static char sub_tag[45];     // ?sub_tag@subtitle_manager@@0PADA
    static unsigned int index;   // ?index@subtitle_manager@@0IA
    static char mPrefix[16];     // ?mPrefix@subtitle_manager@@0PADA
    static char sub_text[163];   // ?sub_text@subtitle_manager@@0PADA
    static short m_token1TooLong;  // ?m_token1TooLong@subtitle_manager@@0FA
    static short m_token2TooLong;  // ?m_token2TooLong@subtitle_manager@@0FA
};

extern const char* seps;         // ?seps@@3PBDB @ 0xDF3948

// ============================================================================
// Misc enums / constants
// ============================================================================
enum EThreadOwner {
    kMainThread = 0,
};

enum EGamePhase {
    GAME_PHASE_LOADING = 0,
    GAME_PHASE_INGAME = 1,
    GAME_PHASE_FRONTEND = 2,
};

struct BadPlaceArc;             // defined in game/actor_types.h
enum team_t : int32_t;          // defined in game/actor_types.h

class PathNodeMgr {
public:
    PathNodes::TOC1* mLevelTOC;      // +0x04
    PathNodes::TOC2* mLevelTOC2;     // +0x08
    static PathNodeMgr* sInst;           // ?sInst@PathNodeMgr@@2PAV1@A
    static PathNodeMgr* Inst();          // ?Inst@PathNodeMgr@@SAPAV1@XZ (g.o 0x4A97D0)
    int GetTotalNodeCount() const;       // ?GetTotalNodeCount@PathNodeMgr@@QBEHXZ (g.o 0x4A97E0)
    PathNodeMgr();                               // ??0PathNodeMgr@@QAE@XZ
    void InitPaths();                            // ?InitPaths@PathNodeMgr@@QAEXXZ
    void CleanUpManager();                       // ?CleanUpManager@PathNodeMgr@@QAEXXZ
    void InitLinkCounts(int zoneIndex);          // ?InitLinkCounts@PathNodeMgr@@QAEXH@Z
    void SetCoverNodeStatus(const Broc::string& name, int inValid);  // ?SetCoverNodeStatus@PathNodeMgr@@QAEXABVstring@Broc@@H@Z
    void AttachSentientToChainNode(sentient_s* pSentient,
                                   const Broc::string& targetname);  // ?AttachSentientToChainNode@PathNodeMgr@@QAEXPAUsentient_s@@ABVstring@Broc@@@Z
    void ConnectPathsForEntity(Entity* ent);     // ?ConnectPathsForEntity@PathNodeMgr@@QAEXPAVEntity@@@Z
    void DisconnectPathsForEntity(Entity* ent);  // ?DisconnectPathsForEntity@PathNodeMgr@@QAEXPAVEntity@@@Z
    void NodeList();                             // ?NodeList@PathNodeMgr@@QAEXXZ
    void CheckpointResetNodes();                 // ?CheckpointResetNodes@PathNodeMgr@@QAEXXZ
    void UpdateArcBadPlaceCount(BadPlaceArc* pArc, int a, int b);  // ?UpdateArcBadPlaceCount@PathNodeMgr@@QAEXPAUBadPlaceArc@@HH@Z
    void FindOverlappingNodes();                 // ?FindOverlappingNodes@PathNodeMgr@@QAEXXZ
    void ValidateNode(PathNodes::PathNode* pNode) const;   // ?ValidateNode@PathNodeMgr@@QBEXPAUPathNode@PathNodes@@@Z
    void ValidateAllNodes() const;               // ?ValidateAllNodes@PathNodeMgr@@QBEXXZ
    void CheckLinkLeaks() const;                 // ?CheckLinkLeaks@PathNodeMgr@@QBEXXZ
    bool IsConnectedTo(const PathNodes::PathNode* pNode1,
                       const PathNodes::PathNode* pNode2) const;  // ?IsConnectedTo@PathNodeMgr@@QBE_NPBUPathNode@PathNodes@@0@Z
    bool IsBadPlaceLink(const PathNodes::NodeHandle& iNodeNumFrom,
                        const PathNodes::NodeHandle& iNodeNumTo,
                        team_t eTeam) const;     // ?IsBadPlaceLink@PathNodeMgr@@QBE_NABVNodeHandle@PathNodes@@0W4team_t@@@Z
    bool IsReserveForMe(const PathNodes::PathNode* pNode,
                        const sentient_s* pClaimer) const;  // ?IsReserveForMe@PathNodeMgr@@QBE_NPBUPathNode@PathNodes@@PBUsentient_s@@@Z
    bool IsReserveForOther(const PathNodes::PathNode* pNode,
                           const sentient_s* pClaimer) const;  // ?IsReserveForOther@PathNodeMgr@@QBE_NPBUPathNode@PathNodes@@PBUsentient_s@@@Z
    PathNodes::PathNode* FirstNode(int iTypeFlags);  // ?FirstNode@PathNodeMgr@@QAEPAUPathNode@PathNodes@@H@Z
    PathNodes::PathNode* NextNode(PathNodes::PathNode* pPrevNode,
                                  int iTypeFlags);   // ?NextNode@PathNodeMgr@@QAEPAUPathNode@PathNodes@@PAU23@H@Z
    void DisconnectPath(PathNodes::PathNode* pNode,
                        PathNodes::PathLink* pLink); // ?DisconnectPath@PathNodeMgr@@QAEXPAUPathNode@PathNodes@@PAUPathLink@3@@Z
    void ConnectPath(PathNodes::PathNode* pNode,
                     PathNodes::PathLink* pLink);   // ?ConnectPath@PathNodeMgr@@QAEXPAUPathNode@PathNodes@@PAUPathLink@3@@Z
    void ConnectPath(PathNodes::PathNode* pNode,
                     const PathNodes::NodeHandle& toNodeNum);  // ?ConnectPath@PathNodeMgr@@QAEXPAUPathNode@PathNodes@@ABVNodeHandle@3@@Z
    void DisconnectPath(Entity* ent, PathNodes::PathNode* pNode,
                        PathNodes::PathLink* pLink); // ?DisconnectPath@PathNodeMgr@@QAEXPAVEntity@@PAUPathNode@PathNodes@@PAUPathLink@4@@Z
    int  GetNode(const Broc::string& a, const Broc::string& b,
                 int* c);                           // ?GetNode@PathNodeMgr@@QAEHABVstring@Broc@@0PAH@Z
    int  GetVehicleNodeIndex(const Broc::string& a, const Broc::string& b,
                             int* c, int d);        // ?GetVehicleNodeIndex@PathNodeMgr@@QAEHABVstring@Broc@@0PAHH@Z
    bool NodesVisible(PathNodes::PathNode* p1,
                      PathNodes::PathNode* p2) const;  // ?NodesVisible@PathNodeMgr@@QBE_NPAUPathNode@PathNodes@@0@Z
    int  NodesInCylinder(const float* const origin, float maxDist,
                         float maxHeight, PathNodes::PathSort* nodes,
                         int maxNodes, int typeFlags);  // ?NodesInCylinder@PathNodeMgr@@QAEHQBMMMPAUPathSort@PathNodes@@HH@Z
    PathNodes::PathNode* ChooseChainPos(PathNodes::PathNode* pChainPos,
                                        int iDepthMin, int iDepthMax,
                                        PathNodes::PathNode* pPrevChainPos,
                                        sentient_s* pClaimer,
                                        int refChainNodeIndex);  // ?ChooseChainPos@PathNodeMgr@@QAEPAUPathNode@PathNodes@@PAU23@HH0PAUsentient_s@@H@Z
    PathNodes::PathNode* RunToFirstReserveNode(PathNodes::PathNode* pNode,
                                               sentient_s* pClaimer);  // ?RunToFirstReserveNode@PathNodeMgr@@QAEPAUPathNode@PathNodes@@PAU23@PAUsentient_s@@@Z
    void DissociateSentient(sentient_s* pSentient);  // ?DissociateSentient@PathNodeMgr@@QAEXPAUsentient_s@@@Z
    // PathNodeMgr.h inline (@ 0x4A9860)
    PathNodes::PathNode* GetNode(const PathNodes::NodeHandle& handle)
    {
        uint16_t mValue = handle.mValue;
        if (handle.mValue == 0 || mValue == 0xFFFF)
            return nullptr;
        PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
        if (mLevelTOC == nullptr)
            return nullptr;
        if ((mValue - 1) >= mLevelTOC->mNodeCount)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PathNodeMgr.h";
            AeAssert::gCurrentLine = 257;
            AeAssert::gCurrentExpr =
                "handle.GetZoneIndex() < mLevelTOC->mNodeCount";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        return &this->mLevelTOC->mNodes[(mValue - 1)];
    }
    // PathNodeMgr.h inline (@ 0x78A850)
    const PathNodes::PathNode* GetNode(
        const PathNodes::NodeHandle& handle) const
    {
        uint16_t mValue = handle.mValue;
        if (handle.mValue == 0 || mValue == 0xFFFF)
            return nullptr;
        PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
        if (mLevelTOC == nullptr)
            return nullptr;
        if ((mValue - 1) >= mLevelTOC->mNodeCount)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PathNodeMgr.h";
            AeAssert::gCurrentLine = 281;
            AeAssert::gCurrentExpr =
                "handle.GetZoneIndex() < mLevelTOC->mNodeCount";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        return &this->mLevelTOC->mNodes[(mValue - 1)];
    }
    PathNodes::PathNode* FindChainPos(const float* vOrigin,
                                      PathNodes::PathNode* pPrevChainPos);  // ?FindChainPos@PathNodeMgr@@QAEPAUPathNode@PathNodes@@QBMPAU23@@Z
    bool NodeNumsVisible(const PathNodes::NodeHandle& iNode1,
                         const PathNodes::NodeHandle& iNode2) const;  // ?NodeNumsVisible@PathNodeMgr@@QBE_NABVNodeHandle@PathNodes@@0@Z
    bool ExpandedNodeNumsVisible(const PathNodes::NodeHandle& iNode1,
                                 const PathNodes::NodeHandle& iNode2) const;  // ?ExpandedNodeNumsVisible@PathNodeMgr@@QBE_NABVNodeHandle@PathNodes@@0@Z
protected:
    PathNodes::PathNode* ChooseDesperationNewChainNode(
        int iDepthMin, int iDepthMax, int chainIndex,
        PathNodes::PathNode* pRefPos, sentient_s* pClaimer);  // ?ChooseDesperationNewChainNode@PathNodeMgr@@IAEPAUPathNode@PathNodes@@HHHPAU23@PAUsentient_s@@@Z
    PathNodes::PathNode* GetPreviousChainNodeReserveForMe(
        PathNodes::PathNode* pNode, sentient_s* pClaimer) const;  // ?GetPreviousChainNodeReserveForMe@PathNodeMgr@@IBEPAUPathNode@PathNodes@@PAU23@PAUsentient_s@@@Z
    PathNodes::PathNode* ChooseSubsequentChainNode_r(
        int iDepthMin, int iDepthMax, int parentIndex,
        sentient_s* pClaimer);   // ?ChooseSubsequentChainNode_r@PathNodeMgr@@IAEPAUPathNode@PathNodes@@HHHPAUsentient_s@@@Z
    PathNodes::PathNode* ChooseAnyChainNodeIfDeadEnd(
        int iDepthMin, int iDepthMax, PathNodes::PathNode* pChainPos,
        sentient_s* pClaimer);   // ?ChooseAnyChainNodeIfDeadEnd@PathNodeMgr@@IAEPAUPathNode@PathNodes@@HHPAU23@PAUsentient_s@@@Z
    PathNodes::PathNode* ChoosePreviousChainNode(
        int iDepthMin, int iDepthMax, PathNodes::PathNode* pChainPos,
        sentient_s* pClaimer);   // ?ChoosePreviousChainNode@PathNodeMgr@@IAEPAUPathNode@PathNodes@@HHPAU23@PAUsentient_s@@@Z
    PathNodes::PathNode* ChooseDesperationChainNode(
        int iDepthMin, int iDepthMax, int chainIndex,
        PathNodes::PathNode* pChainPos, sentient_s* pClaimer);  // ?ChooseDesperationChainNode@PathNodeMgr@@IAEPAUPathNode@PathNodes@@HHHPAU23@PAUsentient_s@@@Z
    PathNodes::PathNode* ChooseChainNodeInRange(
        int iDepthMin, int iDepthMax, PathNodes::PathNode* pChainPos,
        int refChainNodeIndex, sentient_s* pClaimer);  // ?ChooseChainNodeInRange@PathNodeMgr@@IAEPAUPathNode@PathNodes@@HHPAU23@HPAUsentient_s@@@Z
    PathNodes::PathNode* ChooseChainPosFromCurrent(
        int iDepthMin, int iDepthMax, PathNodes::PathNode* pChainPos,
        PathNodes::PathNode* pPrevChainPos, bool bReserve,
        sentient_s* pClaimer, int refChainNodeIndex);  // ?ChooseChainPosFromCurrent@PathNodeMgr@@IAEPAUPathNode@PathNodes@@HHPAU23@0_NPAUsentient_s@@H@Z
    bool SetupLevelPaths();                      // ?SetupLevelPaths@PathNodeMgr@@IAE_NXZ
    void NodesInCylinder_r(PathNodes::PathNodeTree* tree);  // ?NodesInCylinder_r@PathNodeMgr@@IAEXPAUPathNodeTree@PathNodes@@@Z
private:
    int FindZoneIndex(TPakId pakId);             // ?FindZoneIndex@PathNodeMgr@@AAEHW4TPakId@@@Z
    int FindZoneIndex(const char* zone);         // ?FindZoneIndex@PathNodeMgr@@AAEHPBD@Z
    void InitScriptVariables(int zoneIndex);     // ?InitScriptVariables@PathNodeMgr@@AAEXH@Z
    void InitScriptVariables();                  // ?InitScriptVariables@PathNodeMgr@@AAEXXZ
    void InitLinkInfoArray();                    // ?InitLinkInfoArray@PathNodeMgr@@AAEXXZ
    PathNodes::PathNodeTree* CreateTree_r(PathNodes::PathNode** treeNodes,
                                          PathNodes::PathNodeTree** tree);  // ?CreateTree_r@PathNodeMgr@@AAEPAUPathNodeTree@PathNodes@@PAPAUPathNode@3@PAPAU23@@Z
    const int NodeVisCacheEntry(const PathNodes::NodeHandle& a,
                                const PathNodes::NodeHandle& b) const;  // ?NodeVisCacheEntry@PathNodeMgr@@ABE?BHABVNodeHandle@PathNodes@@0@Z
    const int ExpandedNodeVisCacheEntry(const PathNodes::NodeHandle& a,
                                        const PathNodes::NodeHandle& b) const;  // ?ExpandedNodeVisCacheEntry@PathNodeMgr@@ABE?BHABVNodeHandle@PathNodes@@0@Z
    int FindChainIndex(const PathNodes::NodeHandle& handle);  // ?FindChainIndex@PathNodeMgr@@AAEHABVNodeHandle@PathNodes@@@Z
    unsigned char* LooseFileSupport(unsigned char* data, TPakId pakId,
                                    const char* fileSuffix, int* sizeInOut);  // ?LooseFileSupport@PathNodeMgr@@AAEPAEPAEW4TPakId@@PBDPAH@Z
    void InitScriptFunctions(int zoneIndex);     // ?InitScriptFunctions@PathNodeMgr@@AAEXH@Z
    void InitScriptFunctions();                  // ?InitScriptFunctions@PathNodeMgr@@AAEXXZ
    void UpdateBestChainNode(int iDepthMin, int iDepthMax,
                             PathNodes::PathNode* pNode,
                             PathNodes::PathNode** ppBestNode,
                             int* piFoundCount,
                             sentient_s* pClaimer) const;  // ?UpdateBestChainNode@PathNodeMgr@@ABEXHHPAUPathNode@PathNodes@@PAPAU23@PAHPAUsentient_s@@@Z
    virtual void UnloadBank(TPakId pakId);       // ?UnloadBank@PathNodeMgr@@EAEXW4TPakId@@@Z
};
static_assert(sizeof(PathNodeMgr) == 12,
              "PathNodeMgr size mismatch (opaque)");

// StreamZoneManager - streaming level cell manager (opaque; real layout
// verified against IDA in pakmanager.cpp: mInitialPosition +0x1A0,
// mLastPosition +0x1B0, mInitialCell +0x1C0, mLastCellNum +0x1C4)
class StreamZoneManager {
public:
    uint8_t _pad[0x1A0];
    math::Position3 mInitialPosition;  // +0x1A0
    uint8_t _pad2[0x1C0 - 0x1B0];
    int          mInitialCell;      // +0x1C0
    int          mLastCellNum;      // +0x1C4
    static StreamZoneManager* sInst;  // ?sInst@StreamZoneManager@@2PAV1@A
    static StreamZoneManager* Inst();  // ?Inst@StreamZoneManager@@SAPAV1@XZ
    int GetLastCellNum() const;     // ?GetLastCellNum@StreamZoneManager@@QBEHXZ
    int GetInitialCell() const;     // ?GetInitialCell@StreamZoneManager@@QBEHXZ
    math::Position3 GetInitialPosition() const;  // ?GetInitialPosition@StreamZoneManager@@QBE?AVPosition3@math@@XZ
    int GetNumZones() const;        // ?GetNumZones@StreamZoneManager@@QBEHXZ
    void Update(int cellNum, const math::Position3* pos, bool forceReset);  // ?Update@StreamZoneManager@@QAEXHABVPosition3@math@@_N@Z
    void CheckpointRestart();       // ?CheckpointRestart@StreamZoneManager@@QAEXXZ
    const PakInfoNode* GetCellPakInfo(int cellIndex);  // ?GetCellPakInfo@StreamZoneManager@@QAEPBUPakInfoNode@@H@Z
};
// ============================================================================
// SceneManager Ã¢â‚¬â€ scene/static-model manager (opaque)
// ============================================================================
class SceneManager {
public:
    uint8_t _pad[4];
    static SceneManager* sInst;          // ?sInst@SceneManager@@2PAV1@A
    InplaceVector<unsigned char>* mPersistantStorage;
    static SceneManager* Inst();                 // ?Inst@SceneManager@@SAPAV1@XZ (g.o 0x4A9E40)
    InplaceVector<unsigned char>* GetPersistantStorage();  // ?GetPersistantStorage@SceneManager@@QAEPAV?$InplaceVector@E@@XZ (g.o 0x4A9E50)
    void ResetAllStaticModels();         // ?ResetAllStaticModels@SceneManager@@QAEXXZ
    void RestartPersistentArray();       // ?RestartPersistentArray@SceneManager@@QAEXXZ
    void InstanceEntities();             // ?InstanceEntities@SceneManager@@QAEXXZ
    void EnableEffect(unsigned int hash);   // ?EnableEffect@SceneManager@@QAEXI@Z (game.o 0x60CBF0)
    void DisableEffect(unsigned int hash);  // ?DisableEffect@SceneManager@@QAEXI@Z (game.o 0x60CB80)
};

#ifndef COD3_FULL_FE_TYPES
class FEMenuSystem {
public:
    virtual void SetActiveMenu(int a2);  // ?SetActiveMenu@FEMenuSystem@@UAEXH@Z
    void**  menus;                       // +0x04 (FEMenu** array)
    uint8_t _pad08[0x1C - 0x08];
    void (*gap1C)(void* self, float a2); // +0x1C (shell.o update slot)
    uint8_t _pad20[0x2A - 0x20];
    bool    is_active;                   // +0x2A
    void SetSystemActive(bool active);  // ?SetSystemActive@FEMenuSystem@@QAEX_N@Z (sv.o 0x51E150)
    bool IsSystemActive();              // ?IsSystemActive@FEMenuSystem@@QAE_NXZ (shell.o; stub)
};
static_assert(sizeof(FEMenuSystem) == 0x2C, "FEMenuSystem size mismatch (opaque)");

#ifndef COD3_FULL_FE_TYPES
class InGameMenuSystem : public FEMenuSystem {
public:
    void SetActiveMenu(int a2);   // ?SetActiveMenu@InGameMenuSystem@@QAEXH@Z (shell.o; stub)
    bool IsSystemActive();        // ?IsSystemActive@InGameMenuSystem@@QAE_NXZ (shell.o; stub)
    int  GetActiveMenu();         // ?GetActiveMenu@InGameMenuSystem@@QAEHXZ (shell.o 0x571370)
    void ActivateMenu(int menu);  // ?ActivateMenu@InGameMenuSystem@@QAEXH@Z (shell.o 0x572FD0)
    virtual bool IsMenuActive(int menu);     // ?IsMenuActive@FEMenuSystem@@UAE_NH@Z (0x570BC0)
    virtual void MakeActive(int index);      // ?MakeActive@FEMenuSystem@@UAEXH@Z (0x570B20)
    virtual void ClearReturnMenu(int menu);  // ?ClearReturnMenu@FEMenuSystem@@UAEXH@Z (0x570BE0)
};
inline bool InGameMenuSystem::IsSystemActive()
{
    return is_active;
}
inline void InGameMenuSystem::SetActiveMenu(int a2)
{
    (void)a2;
}
#endif
#endif

// ============================================================================
// GamePause â€” static pause helpers
// ============================================================================
struct GamePause {
    struct GamePauseData {
        bool mGamePaused[1];    // +0x00
        GamePauseData();        // ??0GamePauseData@GamePause@@QAE@XZ (game.o 0x612680)
    };
    static GamePauseData mData;                 // ?mData@GamePause@@0UGamePauseData@1@A
    static void SetAllPaused(bool paused);      // ?SetAllPaused@GamePause@@SAX_N@Z (game.o 0x612690)
    static void SetGamePaused(int client, bool paused);  // ?SetGamePaused@GamePause@@SAXH_N@Z
    static bool IsGamePaused(int client);       // ?IsGamePaused@GamePause@@SA_NH@Z
};

// ============================================================================
// VehicleNodeAllocator â€” vehicle node manager
// ============================================================================
struct vehicle_node_t;
struct VehicleNodeAllocator {
    uint16_t m_numNodes;        // +0x00
    uint16_t m_numBlocks;       // +0x02
    uint16_t m_currentBlockIndex;  // +0x04
    uint16_t spad;              // +0x06
    void*    m_pNodeBlocks[16]; // +0x08 (64 bytes)

    VehicleNodeAllocator();  // ??0VehicleNodeAllocator@@QAE@XZ (g.o 0x4B0040)
    void FreeAll();   // ?FreeAll@VehicleNodeAllocator@@QAEXXZ
    void Initialize();  // ?Initialize@VehicleNodeAllocator@@QAEXXZ
    vehicle_node_t* AllocNode();  // ?AllocNode@VehicleNodeAllocator@@QAEPAUvehicle_node_t@@XZ
};

// ============================================================================
// IGOFrontEnd â€” in-game overlay front end
// ============================================================================
#ifndef COD3_FULL_FE_TYPES
enum hud_type { kHudTypeNone = 0 };  // full enumerator set from IDA TBD

struct IGOFrontEnd {
    uint8_t _pad0[0x04];
    void*   compassWidget[1];  // +0x04 (IGOCompassWidget*, indexed by client)
    uint8_t _pad08[0x14 - 0x08];
    void*   ammoWidget[4];   // +0x14 (IGOAmmoWidget*, indexed by client)
    uint8_t _pad24[0x44 - 0x24];
    int     actionHintText[1];  // +0x44 (IGOActionHintWidget::text hash)
    uint8_t _pad48[0x84 - 0x48];
    char*   activate_key;       // +0x84
    uint8_t _pad88[0xA0 - 0x88];
    int     actionHintTimer[1];  // +0xA0
    uint8_t _padA4[0xA8 - 0xA4];
    const char* GetLMGKey();     // ?GetLMGKey@IGOFrontEnd@@QAEPBDXZ (shell.o 0x56DE50)
    void Update(float time_inc);       // ?Update@IGOFrontEnd@@QAEXM@Z (shell.o; stub)
    void UpdateInScene(float time_inc);// ?UpdateInScene@IGOFrontEnd@@QAEXM@Z (shell.o; stub)
    void SetTutorialText(int ref, int viewport);  // ?SetTutorialText@IGOFrontEnd@@QAEXHH@Z
    void SetActionHint(int ref, int viewport);    // ?SetActionHint@IGOFrontEnd@@QAEXHH@Z
    void SetFuse(float total, float remain, int client);  // ?SetFuse@IGOFrontEnd@@QAEXMMH@Z
    void AddActiveGrenade(const Entity* grenade);  // ?AddActiveGrenade@IGOFrontEnd@@QAEXPBVEntity@@@Z
    void SetHUDType(hud_type ht, int viewport);  // ?SetHUDType@IGOFrontEnd@@QAEXW4hud_type@@H@Z
    void UpdateAfterWeaponsLoaded();  // ?UpdateAfterWeaponsLoaded@IGOFrontEnd@@QAEXXZ
};
static_assert(sizeof(IGOFrontEnd) == 168, "IGOFrontEnd size mismatch");
#endif

// Camera Ã¢â‚¬â€ camera state (0x1F0 stride) - full layout from cg.o (cg_misc.cpp)
class Camera {
public:
    Camera();  // real in cg_misc.cpp (avoids implicit COMDAT vs real def)
    uint8_t _pad0[0x2C];                 // +0x00 (GlobalEffectNode)
    bool    mDeathRumble;                // +0x2C
    uint8_t _pad0b[0x30 - 0x2D];
    math::Position3 mPrevViewPos;        // +0x30
    math::Position3 mPrevAngles;         // +0x40
    math::Position3 mPrevViewDir;        // +0x50
    float   mPrevFOV;                    // +0x60
    uint8_t _pad1[0x70 - 0x64];
    math::Position3 mPrevAnimatedViewPos;    // +0x70
    math::Position3 mPrevAnimatedAngles;     // +0x80
    math::Position3 mVehPrevAngles;          // +0x90
    int     mVehPrevAnglesTime;              // +0xA0
    uint8_t _pad2[0xB0 - 0xA4];
    math::Position3 mVehPrevOrigin;          // +0xB0
    float   mVehTimeSinceInput;              // +0xC0
    int     mVehInputState;                  // +0xC4
    float   mVehGasPressedTime;              // +0xC8
    float   mSteerYawOffset;                 // +0xCC
    float   mTankPrevious3rdFrac;            // +0xD0
    uint8_t _pad3[0xE0 - 0xD4];
    math::Position3 mTankRelativeAngles;     // +0xE0
    math::Position3 mTweenStartPos;          // +0xF0
    math::Position3 mTweenStartAngles;       // +0x100
    float   mTweenStartFOV;                  // +0x110
    float   mTweenTime;                      // +0x114
    float   mTweenDuration;                  // +0x118
    uint16_t mTweenFlags;                    // +0x11C
    uint8_t _pad4[0x120 - 0x11E];
    math::Position3 mTweenAnimatedStartPos;    // +0x120
    math::Position3 mTweenAnimatedStartAngles; // +0x130
    uint16_t mAnimFlags;                       // +0x140
    uint8_t _pad5[0x144 - 0x142];
    int     mTagCameraIndex;                   // +0x144
    uint8_t _pad6[0x150 - 0x148];
    math::Mat43 mLastTagCamMat;                // +0x150
    int     mCamMode;                          // +0x190 (CAM_VEHICLE_FIRST == 2)
    int     mVehicleCamMode;                   // +0x194
    uint8_t _pad7[0x1A0 - 0x198];
    math::Position3 mVehCamThirdAnglesOffset;  // +0x1A0
    math::Position3 mTweenParentPos;           // +0x1B0
    math::Position3 mTweenParentAngles;        // +0x1C0
    void*   mShake;                            // +0x1D0
    int     mRumbleEffect;                     // +0x1D4
    bool    mDoingFadeOutIn;                   // +0x1D8
    uint8_t _pad8[0x1DC - 0x1D9];
    float   mFadeTime;                         // +0x1DC
    int     mClient;                           // +0x1E0
    uint8_t _pad9[0x1F0 - 0x1E4];
    void Restart();  // ?Restart@Camera@@QAEXXZ
    bool IsTweening();  // ?IsTweening@Camera@@QAE_NXZ (cg.o 0x68EBB0)
    ECameraModes GetCameraMode();                    // ?GetCameraMode@Camera@@QAE?AW4ECameraModes@@XZ (g.o 0x4A9020)
    EVehicleCameraMode GetVehicleCameraMode();       // ?GetVehicleCameraMode@Camera@@QAE?AW4EVehicleCameraMode@@XZ (g.o 0x4A9030)
    float GetLastViewAngles(int axis);               // ?GetLastViewAngles@Camera@@QAEMH@Z (g.o 0x4A9040)
};
static_assert(sizeof(Camera) == 0x1F0, "Camera size mismatch");
extern Camera gCamera[8];  // ?gCamera@@3PAVCamera@@A (cg.o @ 0x1358EF0, stride 0x1F0)

// ============================================================================
// Memory helpers (Z_MallocInternal / Z_FreeInternal / heap)
// ============================================================================
extern "C" void* _Z_MallocInternal(int size);
extern "C" void  _Z_FreeInternal(void* ptr);
extern void* mem_heap_malloc(int alignment, unsigned int size);
extern void  mem_heap_free(void* ptr);

// ============================================================================
// Cross-object globals used by sv.o
// ============================================================================
extern SaveGameData* gSaveGameData;   // ?gSaveGameData@@3PAUSaveGameData@@A
extern FEManager     g_femanager;     // ?g_femanager@@3UFEManager@@A
extern bool          gQuickStart;     // ?gQuickStart@@3_NA
extern bool          gReturnToMenu;   // ?gReturnToMenu@@3_NA
extern cvar_t*       com_sv_running;  // ?com_sv_running@@3PAUcvar_t@@A
extern bool          gGodModeEnabled; // ?gGodModeEnabled@@3_NA
extern bool          gNoClipEnabled;  // ?gNoClipEnabled@@3_NA
extern bool          gIsWorkspaceMap; // ?gIsWorkspaceMap@@3_NA
extern bool          gDoNotPlayCampaignMovies; // ?gDoNotPlayCampaignMovies@@3_NA
extern int           g_networkOwner;  // ?g_networkOwner@@3W4EThreadOwner@@A
extern const char* const defaultFileName;  // ?defaultFileName
extern void          mem_heap_free(void* ptr);
extern int           dword_F6A290[4 * 802];  // Xbox dev/retail flag array @ 0xF6A290

// ============================================================================
// Cross-object functions used by sv.o
// ============================================================================
extern void   Cmd_AddServerCommand(const char* cmd_name, void (*function)());
extern void   SV_DirectConnect(netadr_t from);
extern void   SV_ClientEnterWorld(client_s* client, int restart, int savegame);
extern void   SV_CheckLoadLevel(int savegame);
extern void   SV_SendClientMessages(void);
extern void   CL_ParseGamestate(Broc::string* configstrings);
extern void   CL_ConnectResponse(netadr_t from);
extern int    CL_FirstSnapshot(void);
extern void   PathNodeMgr_InitPaths(void);
extern int    BG_GetNumWeapons(void);
extern void   CG_RegisterWeapon(int weaponNum);
extern void   j_nullsub_86(int phase);
extern void   movie_manager_load_and_play_movie(const char* movie_name, const char* sound_name);
extern   void   FEManager_PlayFadeInOranScreen(void);

// ============================================================================
// MP player / entity manager minimal views (fields used by SV_PostConnect)
// ============================================================================
class MPPlayer {
public:
    unsigned char mId;                 // +0x00
    uint8_t _pad[3];
    bdReference<bdConnection> mConnection;  // +0x04
    int     mClientIndex;   // +0x08
    Entity* GetEntity();    // ?GetEntity@MPPlayer@@QAEPAVEntity@@XZ
    void SetClientIndex(int index);  // ?SetClientIndex@MPPlayer@@QAEXH@Z (sv.o 0x528040)
    int  GetClientIndex();           // ?GetClientIndex@MPPlayer@@QAEHXZ (sv.o 0x528050)
    static unsigned char GetNullId();  // ?GetNullId@MPPlayer@@SAEXZ (mp.o)
    unsigned char GetId() const;       // ?GetId@MPPlayer@@QBEEXZ (mp.o 0x72CEA0)
    void SetId(unsigned char id);      // ?SetId@MPPlayer@@QAEXE@Z (mp.o 0x72CE90)
    bool IsValid() const;              // ?IsValid@MPPlayer@@QBE_NXZ (mp.o 0x735FF0)
    static bool IsValid(unsigned char id);  // ?IsValid@MPPlayer@@SA_NE@Z (mp.o 0x72CE70)
    bool IsConnected() const;          // ?IsConnected@MPPlayer@@QBE_NXZ (mp.o 0x736010)
    void GetAngles(float (&angles)[3]) const;  // ?GetAngles@MPPlayer@@QBEXAAY02M@Z (mp.o 0x72DFD0)

    static int sDebugNetworkUpdates;   // ?sDebugNetworkUpdates@MPPlayer@@2HA (mp.o)
    static int sPauseNetworkUpdates;   // ?sPauseNetworkUpdates@MPPlayer@@2HA (mp.o)
    bool IsLocalPlayer() const;        // ?IsLocalPlayer@MPPlayer@@QBE_NXZ (mp.o)
};
// ?GetEntity@MPPlayer@@QAEPAVEntity@@XZ (mp.o; stub)
inline Entity* MPPlayer::GetEntity()
{
    return nullptr;
}

struct MPPlayerManager {
    MPPlayer* GetPlayer(int id);
    MPPlayer* GetPlayer(unsigned char id);  // ?GetPlayer@MPPlayerManager@@QAEPAVMPPlayer@@E@Z (mp.o)
    MPPlayer* GetLocalPlayer(int nLocalPlayer);  // ?GetLocalPlayer@MPPlayerManager@@QAEPAVMPPlayer@@H@Z (mp.o)
    bool IsGuest(int controller) const;   // ?IsGuest@MPPlayerManager@@QAE_NH@Z (mp.o 0x72F190)
    void SendConsistencyUpdates();        // ?SendConsistencyUpdates@MPPlayerManager@@QAEXXZ (mp.o 0x72F240)
    unsigned char getPlayerIndex(int localPlayer);  // ?getPlayerIndex@MPPlayerManager@@QAEEH@Z (mp.o 0x72ED90)
    bool IsLocalPlayer(const Entity* const entity);  // ?IsLocalPlayer@MPPlayerManager@@QAE_NQBVEntity@@@Z (mp.o 0x72EA80)
    bool AnyLocalPlayers();             // ?AnyLocalPlayers@MPPlayerManager@@QAE_NXZ (mp.o 0x72EA10)
    int  GetLocalId(const MPPlayer* player);  // ?GetLocalId@MPPlayerManager@@QAEHPBVMPPlayer@@@Z (mp.o 0x72EB80)
private:
    void HandleKickPlayer(const bdReceivedMessage& receivedMsg);  // ?HandleKickPlayer@MPPlayerManager@@AAEXABVbdReceivedMessage@@@Z (mp.o 0x72EC30)
};

class MPPeer {
public:
    MPPlayerManager* GetPlayerManager();
    bool IsPlayerTalking(MPPlayer* player,
                         int local_controller);  // ?IsPlayerTalking@MPPeer@@QAE_NPAVMPPlayer@@H@Z (mp.o)
    void DebugPrintTTYSessionInfo();       // ?DebugPrintTTYSessionInfo@MPPeer@@QAEXXZ (mp.o 0x72CAE0)
    bdSession::bdSessionStatus GetSessionStatus() const;  // ?GetSessionStatus@MPPeer@@QAE?AW4bdSessionStatus@bdSession@@XZ (mp.o 0x72C890)
    static void operator delete(void* p);  // ??3MPPeer@@SAXPAX@Z (mp.o 0x72C840)
    static void* operator new(unsigned int s);  // ??2MPPeer@@SAPAXI@Z (mp.o 0x72C820)
    bool IsHost();                          // ?IsHost@MPPeer@@QAE_NXZ (mp.o 0x72C8B0)
    bool IsQosComplete(int qos_handle);     // ?IsQosComplete@MPPeer@@QAE_NH@Z (mp.o 0x72CA00)
    bool IsLocalPlayer(Entity* player);     // ?IsLocalPlayer@MPPeer@@QAE_NPAVEntity@@@Z (mp.o 0x735AA0)
    void shutdownVoiceSubsystem();          // ?shutdownVoiceSubsystem@MPPeer@@QAEXXZ (mp.o 0x745E80)
    void InitializeVehicles();              // ?InitializeVehicles@MPPeer@@QAEXXZ (mp.o 0x72...)
    void AnimEvent(int animEvent);          // ?AnimEvent@MPPeer@@QAEXH@Z (mp.o)
    void SpotEntity(Entity* ent);           // ?SpotEntity@MPPeer@@QAEXPAVEntity@@@Z (mp.o)
    void DebugRender();                     // ?DebugRender@MPPeer@@QAEXXZ (mp.o)
    void LeaveSession();                    // ?LeaveSession@MPPeer@@QAEXXZ (mp.o 0x72C850)
    void CancelJoin();                      // ?CancelJoin@MPPeer@@QAEXXZ (mp.o 0x72C870)
    int  FindAvailableQosSlot();            // ?FindAvailableQosSlot@MPPeer@@QAEHXZ (mp.o 0x72CAC0)
    bool IsQosSuccessful(int qos_handle);   // ?IsQosSuccessful@MPPeer@@QAE_NH@Z (mp.o 0x72CA20)

    static int mRenderDataInfo;        // ?mRenderDataInfo@MPPeer@@2HA (mp.o)
    static int mRenderPlayerInfo;      // ?mRenderPlayerInfo@MPPeer@@2HA (mp.o)
    static int mRenderSessionInfo;     // ?mRenderSessionInfo@MPPeer@@2HA (mp.o)
};

extern EntityManager*   EntityManager_sInst(void);
extern int              currCl;      // ?currCl@@3HA
extern bool             gExitGame;   // ?gExitGame@@3_NA
extern int              dword_F6A290[4 * 802];  // Xbox dev/retail flag array @ 0xF6A290
extern int              dword_F641E0[];

// ============================================================================
// XModel Ã¢â‚¬â€ minimal view for SV_PointTraceToEntity model scan
// ============================================================================
struct XModelLod;
struct nglMesh;
class XModelParts;

// XModelLod - model LOD entry (12 bytes) - verified against IDA
struct XModelLod {
    float        dist;         // +0x00
    InplaceString filename;    // +0x04
    XModelParts* xmodelParts;  // +0x08
};
static_assert(sizeof(XModelLod) == 0x0C, "XModelLod size mismatch");

// XBoneHierarchy - bone hierarchy entry (12 bytes) - verified against IDA
class XBoneHierarchy {
public:
    InplaceString mName;        // +0x00
    unsigned int  mNameHash;    // +0x04
    int           mParentIndex; // +0x08
};
static_assert(sizeof(XBoneHierarchy) == 0x0C, "XBoneHierarchy size mismatch");

// XBoneInfo - bone bounds/offset entry (40 bytes) - verified against IDA
// `class` tag per binary mangling (?PAPAVXBoneInfo in render.o)
class XBoneInfo {
public:
    math::Position3::Packed mBounds[2];  // +0x00
    math::Dir3::Packed      mOffset;     // +0x18
    float                   mRadiusSquared;  // +0x24
};
static_assert(sizeof(XBoneInfo) == 0x28, "XBoneInfo size mismatch");

// XModelParts - model geometry/anim data (0x40 bytes) - verified against IDA
// `class` tag per binary mangling (?PAVXModelParts in cg.o/render.o)
class XModelParts {
public:
    InplaceVector<math::Mat43::Packed> mTransforms;  // +0x00
    InplaceVector<XBoneInfo> mBoneInfos;             // +0x08
    InplaceVector<XBoneHierarchy> mHierarchy;        // +0x10 InplaceVector<XBoneHierarchy>
    void*            mPartClassifications;           // +0x18
    InplaceVector<InplaceString> mMeshNames;         // +0x20
    InplaceVector<nglMesh*> mMeshPtrs;               // +0x28
    int              mNumRootBones;                  // +0x30
    InplaceString    mAnimDefName;                   // +0x34
    void*            mAnimDef;                       // +0x38 nalBaseSkeleton*
    InplaceString    mName;                          // +0x3C
    const char* GetBoneName(unsigned int i) const;   // ?GetBoneName@XModelParts@@QBEPBDI@Z
    int GetNumBones() const;  // ?GetNumBones@XModelParts@@QBEHXZ (g.o 0x4AF100)
};

class XModel {
public:
    uint8_t      _pad0[0x20];  // +0x00
    XModelParts* parts;        // +0x20
    XModelLod*   lod[5];       // +0x24
    uint8_t      _pad28[0x38 - 0x28];
    InplaceVector<struct XModelCollSurf const*> collSurfs;  // +0x38
    uint8_t      _pad3C[0x40 - 0x3C];
    int          contents;     // +0x40
    uint8_t      _pad41[0x44 - 0x41];
    uint16_t     numLods;      // +0x44
    uint16_t     collLod;      // +0x46
    InplaceString name;        // +0x48
    unsigned int iflFrames;    // +0x4C (Bitmask<unsigned int>)
    const char* GetName() const;  // ?GetName@XModel@@QBEPBDXZ
    XModelParts* GetXModelParts(int lodIndex);        // ?GetXModelParts@XModel@@QAEPAVXModelParts@@H@Z
    const XModelParts* GetXModelParts(int lodIndex) const;  // ?GetXModelParts@XModel@@QBEPBVXModelParts@@H@Z
    int GetNumBones(int lodIndex) const;  // ?GetNumBones@XModel@@QBEHH@Z (g.o 0x4AF110)
    static int GetNumBones(XModel* model, int lodIndex);
};

// StaticModel - static world model instance (240 bytes) - verified against
// IDA (CM_TraceStaticModel / CM_LinkStaticModel disasm)
class StaticModel {
public:
    uint8_t      _pad0[0x68];      // +0x00
    XModel*      xmodel;           // +0x68
    uint8_t      _pad6C[0x90 - 0x6C];
    float        origin[3];        // +0x90
    uint8_t      _pad9C[0xA0 - 0x9C];
    float        absmin[3];        // +0xA0
    float        absmax[3];        // +0xAC
    float        invAxis[3][3];    // +0xB8
    StaticModel* nextModel;        // +0xDC
    TPakId       pakId;            // +0xE0
    uint8_t      _padE4[0xF0 - 0xE4];
};
static_assert(sizeof(StaticModel) == 0xF0, "StaticModel size mismatch");
static_assert(offsetof(StaticModel, xmodel) == 0x68,
              "StaticModel::xmodel offset mismatch");
static_assert(offsetof(StaticModel, origin) == 0x90,
              "StaticModel::origin offset mismatch");
static_assert(offsetof(StaticModel, invAxis) == 0xB8,
              "StaticModel::invAxis offset mismatch");
static_assert(offsetof(StaticModel, pakId) == 0xE0,
              "StaticModel::pakId offset mismatch");

// render.o model pose/trace entry points (referenced by CM_TraceStaticModel)
struct DObjSkelMat;
void XModelGetBasePose(IVPointer<XModel> model, DObjSkelMat* mat,
                       DObjSkelMat* modelParentMat);  // render.o 0x6CA670
int  XModelTraceLine(IVPointer<XModel> model, trace_t* results,
                     DObjSkelMat* boneMtxList, const float* localStart,
                     const float* localEnd, int contentmask);  // render.o 0x6CB470

// IVPointer operator bool / operator-> (reconstructed from IDA)
template <typename T>
inline bool IVPointer_IsValid(const IVPointer<T>& p) { return p.mValue != NULL; }
template <typename T>
inline T* IVPointer_Deref(const IVPointer<T>& p) { return p.mValue; }

// ============================================================================
// Client static state (cls) Ã¢â‚¬â€ opaque
// ============================================================================
enum clientStateCA_t {
    CA_DISCONNECTED = 0,
    CA_ACTIVE = 1,
    CA_LOADING = 2,
};

struct cls_t {
    int keyCatchers;
    int state;
    int endgamemenu;
    int cddialog;
    int servername[1024 + 32];
};

extern cls_t cls;  // ?cls@@3Ucls_t@@A
extern int   EntityManager_GetNumPlayers(void);
extern Entity* EntityManager_GetPlayerEntity(int idx);

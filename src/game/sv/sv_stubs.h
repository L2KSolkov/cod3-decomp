// ============================================================================
// sv_stubs.h — minimal cross-object types used by sv.o functions.
// These types belong to OTHER (unported) game objects; only the fields that
// sv.o touches are declared here so the server engine compiles standalone.
// Full definitions arrive when those objects are ported.
// ============================================================================

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "engine/broc_types.h"
#include "game/game_types.h"

// ============================================================================
// TPakId — pak archive id enum
// ============================================================================
enum TPakId { kPakTypeLevel = 0, kPakTypeNone = -1 };

// ============================================================================
// StubData — per-controller MP save/profile data (1216 bytes) — verified IDA
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
    uint8_t loginMethod[4];            // +0x30C (_UIX_LOGON_TYPE)
    uint8_t liveState[4];              // +0x310 (ELiveState)
    uint8_t savedInvite[156];          // +0x314 (_XONLINE_ACCEPTED_GAMEINVITE, opaque)
    int     mControllerPort;           // +0x3B0
    bool    savedStateIsValid;         // +0x3B4
    int     lastLoginCode;             // +0x3B8 (HRESULT)
    bool    mReturnToMain;             // +0x3BC
    bool    appearOnline;              // +0x3BD
    bool    mbWasInvited;              // +0x3BE
    bool    mDisableSave;              // +0x3BF
    char    mCmdLine[256];             // +0x3C0
};
static_assert(sizeof(StubData) == 0x4C0, "StubData size mismatch");
static_assert(offsetof(StubData, mControllerPort) == 0x3B0, "StubData::mControllerPort offset mismatch");
static_assert(offsetof(StubData, mSec) == 0x28, "StubData::mSec offset mismatch");
static_assert(offsetof(StubData, mInvertAim) == 0x41, "StubData::mInvertAim offset mismatch");

// ============================================================================
// SaveGameData — MP save game bundle (7156 bytes) — verified IDA
// ============================================================================
struct SaveGameData {
    StubData mStubData;         // +0x000
    uint8_t  _rest[7156 - sizeof(StubData)];  // remaining fields opaque
};
static_assert(sizeof(SaveGameData) == 7156, "SaveGameData size mismatch");

// ============================================================================
// ServerTime — server clock (20 bytes) — verified IDA
// ============================================================================
class ServerTime {
public:
    unsigned int mNumTicksElapsed;  // +0x00
    int          mTickMSec;         // +0x04
    float        mTickDelta;        // +0x08
    float        mTickDeltaInv;     // +0x0C
    float        mElapsedTime;      // +0x10

    static ServerTime sInst;        // ?sInst@ServerTime@@0V1@A
};
static_assert(sizeof(ServerTime) == 0x14, "ServerTime size mismatch");

// ============================================================================
// FEManager — front-end manager (1012 bytes; opaque, only sv.o fields shown)
// ============================================================================
struct IGOFrontEnd;
struct FEMenuSystem;
struct DialogMenuSystem;
struct InGameMenuSystem;
struct AARMenuSystem;
struct ProfileManager;
struct PanelQuad;
struct nglFont;
struct ControllerDisconnectedMenu;

class FEManager {
public:
    // +0x00 vftable (1 ptr)
    uint8_t _vftable[4];
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
    InGameMenuSystem* mIGMS[1];         // +0xC8
    AARMenuSystem* mAARS;               // +0xCC
    uint8_t mPanelArray[800];           // +0xD0 (100 x sPanelPakData, opaque)
    int     mNumPanels;                 // +0x3F0
    // +0x3F4 .. 0x3F4 remaining pad
};
static_assert(sizeof(FEManager) == 0x3F4, "FEManager size mismatch");

// ============================================================================
// Opaque singleton managers (fields used by sv.o only)
// ============================================================================
struct AeThreadManager {
    uint8_t _pad[2148];
    static AeThreadManager sInst;   // ?sInst@AeThreadManager@@0V1@A
    void KillAllThreads();
};
static_assert(sizeof(AeThreadManager) == 2148, "AeThreadManager size mismatch");

struct MultiplayerMgr {
    uint8_t _pad[0x40];
    bool    mLinkCheckEnabled;      // +0x40 (field used by SV_Map_f)
    uint8_t _pad2[0x50 - 0x41];
    static MultiplayerMgr* sInst;   // ?sInst@MultiplayerMgr@@2PAV1@A
    void ExitLevel();
    void StartDevServer();
};
static_assert(sizeof(MultiplayerMgr) == 80, "MultiplayerMgr size mismatch");

struct SoundDevice {
    uint8_t _pad[31392];
    static SoundDevice* sInst;      // ?sInst@SoundDevice@@2PAV1@A
    void StopAllSounds();
};
static_assert(sizeof(SoundDevice) == 31392, "SoundDevice size mismatch");

// ============================================================================
// Additional cross-object singleton managers (fields used by sv.o only)
// ============================================================================
struct CheckpointMgr {
    bool         mUsingCheckpoints;       // +0x00 (bool)
    bool         mCheckpointSaveExists;   // +0x01 (bool)
    Broc::string mCurrentMapName;         // +0x04 (Broc::string, 4 bytes)
    static CheckpointMgr* sInst;          // ?sInst@CheckpointMgr@@2PAV1@A
    void ClearSavedCheckpointData();
};
static_assert(sizeof(CheckpointMgr) == 8, "CheckpointMgr size mismatch (fields used)");

struct PakManager {
    uint8_t _pad[4];
    static PakManager* sInst;            // ?sInst@PakManager@@2PAV1@A
    void FillBanks();
};
static_assert(sizeof(PakManager) == 4, "PakManager size mismatch (opaque)");

// ============================================================================
// EntityHandleDb — entity handle database (opaque; only element lookup used)
// HandleDb<Entity,1344,SizedHandle<12,20>>::DbElement = { int mKey; Entity* mObject; }
// ============================================================================
struct EntityHandleDbDbElement {
    unsigned short mKey;    // +0x00
    Entity*        mObject; // +0x04
};
class EntityHandleDb {
public:
    uint8_t  _pad[0x2AAC];                 // HandleDb storage (10924 bytes)
    EntityHandleDbDbElement mElements[0x540];  // +0x2AAC (1344 * 8 = 10752)
    uint8_t  _rest[27312 - 0x2AAC - 10752];
    static EntityHandleDb sInst;           // ?sInst@EntityHandleDb@@0V1@A
};
static_assert(sizeof(EntityHandleDb) == 27312, "EntityHandleDb size mismatch");
static_assert(offsetof(EntityHandleDb, mElements) == 0x2AAC, "EntityHandleDb::mElements offset mismatch");

// ============================================================================
// XModelManager — model manager (opaque)
// ============================================================================
struct XModel;
class XModelManager {
public:
    uint8_t _pad[4];
    static XModelManager* sInst;           // ?sInst@XModelManager@@2PAV1@A
    IVPointer<XModel> GetXModel(TPakId pak_id, const char* name);
};
static_assert(sizeof(XModelManager) == 4, "XModelManager size mismatch (opaque)");

// ============================================================================
// EntityManager — entity factory (opaque; only sv.o fields used)
// ============================================================================
struct EntityManager {
    uint8_t _pad[4];
    static EntityManager* sInst;            // ?sInst@EntityManager@@2PAV1@A
    Entity* GetPlayer(int idx);             // ?GetPlayer@EntityManager@@QAEPAVEntity@@H@Z
};
static_assert(sizeof(EntityManager) == 4, "EntityManager size mismatch (opaque)");

// ============================================================================
// AeAssert — assertion system (namespace-style free functions + globals)
// ============================================================================
namespace AeAssert {
    enum ECoderId { COD3 = 0 };
    extern ECoderId gCurrentAuthor;  // ?gCurrentAuthor@AeAssert@@3W4ECoderId@1@A
    extern const char* gCurrentFile;  // ?gCurrentFile@AeAssert@@3PBDB
    extern int  gCurrentLine;         // ?gCurrentLine@AeAssert@@3HA
    extern const char* gCurrentExpr;  // ?gCurrentExpr@AeAssert@@3PBDB
    bool IsIgnored(void);
    bool Assert(const char* fmtstring, ...);
}

// ============================================================================
// movie_manager — static movie helpers
// ============================================================================
struct movie_manager {
    static void load_and_play_movie(const char* movie_name, const char* sound_name);
};

// ============================================================================
// Misc enums / constants
// ============================================================================
enum EThreadOwner {
    kMainThread = 0,
};

enum EGamePhase {
    GAME_PHASE_LOADING = 0,
    GAME_PHASE_INGAME = 1,
};

struct PathNodeMgr {
    uint8_t _pad[4];
    static PathNodeMgr* sInst;           // ?sInst@PathNodeMgr@@2PAV1@A
    void InitPaths();                    // ?InitPaths@PathNodeMgr@@QAEXXZ
};
static_assert(sizeof(PathNodeMgr) == 4, "PathNodeMgr size mismatch (opaque)");

struct FEMenuSystem {
    virtual void SetActiveMenu(int a2);  // ?SetActiveMenu@FEMenuSystem@@UAEXH@Z
};
static_assert(sizeof(FEMenuSystem) == 4, "FEMenuSystem size mismatch (opaque)");

// ============================================================================
// GamePause — static pause helpers
// ============================================================================
struct GamePause {
    static void SetAllPaused(bool paused);      // ?SetAllPaused@GamePause@@SAX_N@Z
    static void SetGamePaused(int client, bool paused);  // ?SetGamePaused@GamePause@@SAXH_N@Z
};

// ============================================================================
// VehicleNodeAllocator — vehicle node manager
// ============================================================================
struct VehicleNodeAllocator {
    void FreeAll();   // ?FreeAll@VehicleNodeAllocator@@QAEXXZ
};

// ============================================================================
// IGOFrontEnd — in-game overlay front end
// ============================================================================
struct IGOFrontEnd {
    uint8_t _pad[168];
    void SetTutorialText(int ref, int viewport);  // ?SetTutorialText@IGOFrontEnd@@QAEXHH@Z
};
static_assert(sizeof(IGOFrontEnd) == 168, "IGOFrontEnd size mismatch");

// ============================================================================
// Memory helpers (Z_MallocInternal / Z_FreeInternal / heap)
// ============================================================================
extern void* _Z_MallocInternal(unsigned int size);
extern void  _Z_FreeInternal(void* ptr);
extern void* mem_heap_malloc(int alignment, unsigned int size);
extern void  mem_heap_free(void* ptr);

// ============================================================================
// Cross-object globals used by sv.o
// ============================================================================
extern SaveGameData* gSaveGameData;   // ?gSaveGameData@@3PAUSaveGameData@@A
extern FEManager     g_femanager;     // ?g_femanager@@3VFEManager@@A
extern bool          gQuickStart;     // ?gQuickStart@@3_NA
extern bool          gReturnToMenu;   // ?gReturnToMenu@@3_NA
extern bool          gGodModeEnabled; // ?gGodModeEnabled@@3_NA
extern bool          gNoClipEnabled;  // ?gNoClipEnabled@@3_NA
extern bool          gIsWorkspaceMap; // ?gIsWorkspaceMap@@3_NA
extern bool          gDoNotPlayCampaignMovies; // ?gDoNotPlayCampaignMovies@@3_NA
extern int           g_networkOwner;  // ?g_networkOwner@@3W4EThreadOwner@@A
extern const char    defaultFileName[];  // ?defaultFileName
extern void          mem_heap_free(void* ptr);
extern int           unk_F6A290;      // Xbox dev/retail flag

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
extern void   CL_FirstSnapshot(void);
extern void   PathNodeMgr_InitPaths(void);
extern int    BG_GetNumWeapons(void);
extern void   CG_RegisterWeapon(int weaponNum);
extern void   j_nullsub_86(int phase);
extern void   movie_manager_load_and_play_movie(const char* movie_name, const char* sound_name);
extern   void   FEManager_PlayFadeInOranScreen(void);

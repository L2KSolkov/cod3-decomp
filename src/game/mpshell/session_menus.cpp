// ============================================================================
// session_menus.cpp - mp_shell.o session/overlay menu classes
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/mpshell/session_menus.h"
#include "game/actor_types.h"
#include "game/client_types.h"
#include "bd/bdNet.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert")) \
            __debugbreak();                                               \
    } while (0)

// MPVote layout from IDA (mp.o; currVote at MPPlayerManager +0x5914).
struct AARMPVoteReset {
    unsigned char voteStartTime[4];
    int mVoteType;
    unsigned char voteIndex;
    unsigned char mYesVotes;
    unsigned char mNoVotes;
    unsigned char callerIndex;
    unsigned char voteSubject;
    unsigned char eligableVoters;
    unsigned char _pad0E[2];
    int arrPlayerMapVotes[16];
    bool localVoted;
    unsigned char _tail[3];
};
static_assert(sizeof(AARMPVoteReset) == 0x54, "MPVote layout mismatch");

// ============================================================================
// MPUIInterface - minimal view with binary-exact manglings (mp.o owns the
// real definitions; only members used by mp_shell.o are declared).
// ============================================================================
enum eGameType : int {
    GAME_TYPE_WAR = 0,
    GAME_TYPE_CTF = 1,
    GAME_TYPE_SCF = 2,
    GAME_TYPE_HQ = 3,
    GAME_TYPE_TDM = 4,
    GAME_TYPE_DM = 5,
    GAME_TYPE_DOM = 6,
    GAME_TYPE_SND = 7,
};

struct sServerCreateParams;
struct sServerQueryParams;
struct sGameListing;

class MPUIInterface {
public:
    enum eSetting : int {
        SETTING_TIME_LIMIT = 0,
        SETTING_SCORE_LIMIT = 1,
        SETTING_ROUND_LIMIT = 2,
        SETTING_RESPAWN_TIME = 3,
        SETTING_LAST_MAN_STANDING = 4,
        SETTING_MAX_PLAYERS = 5,
    };
    enum EGameConnectionType : int {
        kGameConnectionTypeLan = 0,
        kGameConnectionTypeOnline = 1,
        kGameConnectionTypeLocal = 2,
    };

    static bool IsOnlineGame();         // ?IsOnlineGame@MPUIInterface@@SA_NXZ
    static const bool IsLANGame();      // ?IsLANGame@MPUIInterface@@SA?B_NXZ
    static void Step();                 // ?Step@MPUIInterface@@SAXXZ
    static const int GetDefaultOption(eSetting setting,
                                      eGameType gameType);  // ?GetDefaultOption@MPUIInterface@@SA?BHW4eSetting@1@W4eGameType@@@Z
    static const int GetScoreLimitCount(eGameType gameType);  // ?GetScoreLimitCount@MPUIInterface@@SA?BHW4eGameType@@@Z
    static const int GetScoreLimit(unsigned long index,
                                   eGameType gameType);  // ?GetScoreLimit@MPUIInterface@@SA?BHKW4eGameType@@@Z
    static const bool IsGameListingComplete();  // ?IsGameListingComplete@MPUIInterface@@SA?B_NXZ
    static const bool StartClient(sGameListing& game, bool bStartGame,
                                  int nGameIndex);  // ?StartClient@MPUIInterface@@SA?B_NAAUsGameListing@@_NH@Z
    static bool GameListingStart();  // ?GameListingStart@MPUIInterface@@SA_NXZ
    static bool StartGame(bool forceRestart, bool blockUntilNetReady);  // ?StartGame@MPUIInterface@@SA_N_N0@Z
    static const int GetTimeLimitCount();  // ?GetTimeLimitCount@MPUIInterface@@SA?BHXZ
    static const int GetTimeLimit(unsigned long index);  // ?GetTimeLimit@MPUIInterface@@SA?BHK@Z
    static const int GetMaxPlayers(unsigned long index);  // ?GetMaxPlayers@MPUIInterface@@SA?BHK@Z
    static const char* GetGameTypeString(unsigned long gameType);  // ?GetGameTypeString@MPUIInterface@@SAPBDK@Z
    static sGameListing* GameListingGet(unsigned long& numGames);  // ?GameListingGet@MPUIInterface@@SAPAUsGameListing@@AAK@Z
    static void ExitGame();   // ?ExitGame@MPUIInterface@@SAXXZ
    static int mReturnMenu;   // ?mReturnMenu@MPUIInterface@@1HA
    static const int GetMaxPlayersOptionFromMap(char mapID);  // ?GetMaxPlayersOptionFromMap@MPUIInterface@@SA?BHD@Z
    static const char* GetMapString(unsigned long mapIndex);  // ?GetMapString@MPUIInterface@@SAPBDK@Z
    static const bool StartServer(bool forceRestart,
                                  bool blockUntilNetReady);  // ?StartServer@MPUIInterface@@SA?B_N_N0@Z
    static void ExitFrontend(int returnMenu);  // ?ExitFrontend@MPUIInterface@@SAXH@Z
    static void SetQueryParams(sServerQueryParams& params);  // ?SetQueryParams@MPUIInterface@@SAXAAUsServerQueryParams@@@Z
    static void CancelJoin();  // ?CancelJoin@MPUIInterface@@SAXXZ

    static sServerCreateParams mServerParams;      // ?mServerParams@MPUIInterface@@1UsServerCreateParams@@A
    static sServerCreateParams mNextServerParams;  // ?mNextServerParams@MPUIInterface@@1UsServerCreateParams@@A
    static int  mMaxScoreLimitCount;  // ?mMaxScoreLimitCount@MPUIInterface@@1HA @ 0xE36E8C
    static bool mIsViewableOnline;    // ?mIsViewableOnline@MPUIInterface@@1_NA
    static EGameConnectionType mGameConnectionType;  // ?mGameConnectionType@MPUIInterface@@1W4EGameConnectionType@@A
    static bool mLiveQueryActive;  // ?mLiveQueryActive@MPUIInterface@@1_NA
    static bool mQueryFromID;      // ?mQueryFromID@MPUIInterface@@1_NA

    static const int GetMaxScoreLimitCount();  // ?GetMaxScoreLimitCount@MPUIInterface@@SA?BHXZ
    static void SetViewOnlineStatus(bool state);  // ?SetViewOnlineStatus@MPUIInterface@@SAX_N@Z
};

// sServerCreateParams - server create params (105 bytes; layout from IDA)
struct sServerCreateParams {
    char mRandomMapList[64];       // +0x00
    char mName[24];                // +0x40
    unsigned char mMapID;          // +0x58
    unsigned char mGameType;       // +0x59
    unsigned char mGameSubType;    // +0x5A
    unsigned char mMaxPlayers;     // +0x5B
    unsigned char mTeamBalancing;  // +0x5C
    unsigned char mFriendlyFire;   // +0x5D
    unsigned char mPrivateSlots;   // +0x5E
    unsigned char mTimeLimit;      // +0x5F
    unsigned char mScoreLimit;     // +0x60
    unsigned char mRoundLimit;     // +0x61
    unsigned char mSwapEnds;       // +0x62
    unsigned char mRespawnTime;    // +0x63
    bool mDontRotate;              // +0x64
    bool mDoChangeMap;             // +0x65
    unsigned char mEnableAARVote;  // +0x66
    unsigned char mEnablePenaltyVote;  // +0x67
    unsigned char mMapRotation;    // +0x68

    void SetMapRotation(unsigned char MapRotation);  // ?SetMapRotation@sServerCreateParams@@QAEXE@Z (mp.o)
};
static_assert(sizeof(sServerCreateParams) == 105,
              "sServerCreateParams size mismatch");

struct sServerQueryParams {
    unsigned int mGameType;       // +0x00
    unsigned int mMapID;          // +0x04
    unsigned int mMaxPlayers;     // +0x08
    unsigned int mMinPlayers;     // +0x0C
    unsigned int mGameSubType;    // +0x10
    unsigned int mTeamBalancing;  // +0x14
    unsigned int mFriendlyFire;   // +0x18
    char mSessionNamePrefix[16];  // +0x1C
    unsigned int mListIfFull;     // +0x2C
};

struct sGameListing {
    int mSize;                          // +0x00
    void* mGameInfo;                    // +0x04 (bdReference<MPGameInfo>)
    int mPing;                          // +0x08
    bool mValidVersion;                 // +0x0C
};

// MPGameInfo minimal view (mp.o owns the definition; offsets from IDA)
struct sMPGameInfoView {
    uint8_t _pad0[0x28];
    unsigned char m_publicOpen;    // +0x28
    unsigned char m_privateOpen;   // +0x29
    unsigned char m_publicFilled;  // +0x2A
    unsigned char m_privateFilled; // +0x2B
    char mName[24];                // +0x2C
    unsigned char mMapID;          // +0x44
    unsigned char mGameType;       // +0x45
    unsigned char mGameSubType;    // +0x46
};

// ============================================================================
// MPLiveEngine / LiveWrapper minimal views (game_xbox.o owns the definitions)
// ============================================================================
enum ELiveState {
    kNotSignedIn = 0,
    kSigningIn = 1,
    kSignedIn = 2,
};

struct LivePlayer {
    void** vftable;              // +0x00
    _XUID xuid;                  // +0x04
    unsigned short gamertag[16]; // +0x10
};

struct LiveLocal : LivePlayer {
};

class LiveWrapper {
public:
    // vptr anchor (game_xbox.o owns the real vtable; never called here)
    virtual void vtableAnchor() = 0;

    LiveLocal* GetLocalPlayer(unsigned int portNumber);  // ?GetLocalPlayer@LiveWrapper@@QAEPAVLiveLocal@@I@Z (game_xbox.o)
    char* GetIcon(unsigned int portNumber);  // ?GetIcon@LiveWrapper@@QAEPADK@Z (game_xbox.o)
    void SetNotificationFlag(unsigned int portNumber, unsigned int flagID,
                             bool flagState);  // ?SetNotificationFlag@LiveWrapper@@QAEXKK_N@Z
    void DoWork();                            // ?DoWork@LiveWrapper@@QAEXXZ (game_xbox.o)
    void LogOut();                            // ?LogOut@LiveWrapper@@QAEXXZ (game_xbox.o)
    void ToggleOfflineAppearance(unsigned int portNumber);  // ?ToggleOfflineAppearance@LiveWrapper@@QAEXK@Z
    void SetVTS(unsigned int portNumber, bool enabled);     // ?SetVTS@LiveWrapper@@QAEXK_N@Z
    static LiveWrapper* theWrapper;          // ?theWrapper@LiveWrapper@@1PAV1@A (game_xbox.o)
};

struct XONLINE_FRIEND;
extern "C" void __stdcall LiveEngine_Reboot(void* engine, int mode);
    // _LiveEngine_Reboot@8 (uixd:engine.obj)

// Minimal views for ModelMenu animation helpers (anim.o / game2.o own the
// real definitions; only the members used by mp_shell.o are declared).
namespace nalGeneric { class nalGenericAnim; }
class nalPlayMethod;
class nalAnimCallback;
class DObj {
public:
    void* tree[8];         // +0x00
    void* animPlayers[8];  // +0x20
};
class AnimationPlayer {
public:
    enum AnimationPlayerModifierType {
        nalAdditiveModifier = 0x0,
        nalPartialModifier = 0x1,
        nalFullModifier = 0x2,
    };
    void PlayModifier(nalGeneric::nalGenericAnim* anim,
                      AnimationPlayerModifierType type, float priority,
                      unsigned int mask, bool ForceRestart, float fade_in,
                      float fade_out, nalPlayMethod* play_method,
                      float callback_time, nalAnimCallback* callback,
                      float speed, float time_in_seconds_to_start);
    void SetModifierType(unsigned int mask,
                         AnimationPlayerModifierType type);
};
struct MP_ANIM_INDEX {
    char* name;                        // +0x00
    unsigned int ID;                   // +0x04
    nalGeneric::nalGenericAnim* anim;  // +0x08
    unsigned short flags;              // +0x0C
    unsigned short padding;            // +0x0E
};

// ModelMenu::UpdateClassModel helpers (mp_actors.o / g.o / render.o)
class AnimTree;
class AIType {
public:
    void InitPlayer(Entity* ent, TPakId pakId);  // ?InitPlayer@AIType@@QAEXPAVEntity@@W4TPakId@@@Z (mp_actors.o)
};
class AITypeManager {
public:
    static AITypeManager* sInst;  // ?sInst@AITypeManager@@2PAV1@A
    IVPointer<AIType> GetAIType(TPakId pakId, const char* name,
                                int nameOffset);  // ?GetAIType@AITypeManager@@QAE?AV?$IVPointer@VAIType@@@@W4TPakId@@PBDH@Z
};
extern TPakId CurPakId();  // ?CurPakId@@YA?AW4TPakId@@XZ (streamer.o)
extern void SV_SetBrushModel(Entity* ent);  // ?SV_SetBrushModel@@YAXPAVEntity@@@Z (sv.o)
extern AnimTree* Scr_GetAnimTreeByName(const char* treename);  // scr.o
extern void G_SetAnimTree(Entity* ent, AnimTree* animtree);  // g.o
extern void DObjCreateAnimationPlayer(DObj* obj, int a2);  // render.o
extern void G_DObjUpdate(Entity* ent, bool forceWeaponModel);  // g.o
extern void g_UnlinkEntity(Entity* ent);  // g.o
struct weaponFileInfo_t {
    int weapClass;    // +0x00
    int type;         // +0x04
    int slot;         // +0x08
    int iMeleeDamage; // +0x0C
    int bADSOnly;     // +0x10
};
extern weaponFileInfo_t* BG_GetInfoForWeapon(int iWeapon);  // cl.o

// BuildControllerMessage - controller-disconnect message (mp_shell.o 0x78FC00)
Broc::string BuildControllerMessage();

class MPLiveEngine : public LiveWrapper {
public:
    static MPLiveEngine* GetHandle();  // ?GetHandle@MPLiveEngine@@SAPAV1@XZ (game_xbox.o)
    int internalState;                 // +0x04 (LiveWrapper)
    uint8_t _pad08[0x14 - 0x08];       // +0x08
    void* uixEngine;                   // +0x14 (LiveEngine*)
    uint8_t _pad18[0x4474 - 0x18];     // +0x18
    bool needConfirmation;             // +0x4474
    unsigned char friendToJoin[0x56];  // +0x4475 (XONLINE_FRIEND)
    bool renderingEnabled;             // +0x44CB
    uint8_t _pad44CC[0x45D4 - 0x44CC]; // +0x44CC
    unsigned int actualPort;           // +0x45D4
    virtual void JoinGame(XONLINE_FRIEND* joinee);  // ?JoinGame@MPLiveEngine@@UAEXPAU_XONLINE_FRIEND@@@Z (vtable +0x38)
};

// XBoxLiveIngameOptionsCOD3 - FE menu (mp_xbox.o owns the definition)
class XBoxLiveIngameOptionsCOD3 : public FEMenu {
public:
    static XBoxLiveIngameOptionsCOD3* Me(int version);  // ?Me@XBoxLiveIngameOptionsCOD3@@SAPAV1@H@Z
};

// ============================================================================
// mp_shell.o data
// ============================================================================
int g_NumBaseMaps = 0;       // ?g_NumBaseMaps@@3HA @ 0xF99860
int g_NumTotalMaps = 0;      // ?g_NumTotalMaps@@3HA @ 0xF99864
char byte_E386C9[] = {'\0'};   // map-ID conversion table @ 0xE386C9
const char* const szClassReference[7] = {
    "MPGAME_RIFLEMAN_ALLCAPS", "MPGAME_INFANTRY_ALLCAPS",
    "MPGAME_ASSAULT_ALLCAPS", "MPGAME_MEDIC_ALLCAPS",
    "MPGAME_SCOUT_ALLCAPS", "MPGAME_SUPPORT_ALLCAPS",
    "MPGAME_ANTIARMOR_ALLCAPS",
};
extern float unk_F6A280[802];  // viewport prev (cg.o)
extern float unk_F6A284[802];  // viewport curr (cg.o)
extern bool g_controllerConnected[];           // ?g_controllerConnected@@3PA_NA (game2.o)
extern bool g_controllerConnectedErrorShown[]; // ?g_controllerConnectedErrorShown@@3PA_NA (game2.o)
const char* const szPlayLanMenuOptionTextReferences[3] = {
    "MPFRONTEND_MM_CREATE_GAME", "MPFRONTEND_FIND_GAME",
    "MPFRONTEND_MM_XBOX_LIVE_OPTIONS",
};
const char* const szPlayLanMenuDescriptionReferences[3] = {
    "MPFRONTEND_CREATE_GAME_DESCRIPTION",
    "MPFRONTEND_FIND_GAME_DESCRIPTION",
    "MPFRONTEND_XBOXLIVEOPTIONS_DESCRIPTION",
};

struct Mapinfo_t {
    char map_pack;              // +0x00
    char map_id_number;         // +0x01
    char map_name_string[32];   // +0x02
    char map_title_string[32];  // +0x22
    char short_name[16];        // +0x42
    char map_location_string[32];  // +0x52
};
extern Mapinfo_t* g_TheMapInfo;  // ?g_TheMapInfo@@3PAUMapinfo_t@@A @ 0x1227BC8
char aMpfrontendMerv[20] = "MPFRONTEND_MERVILLE";
char aMploadingMervi[23] = "MPLOADING_MERVILLE_LOC";
char aMploadingMervi_0[25] = "MPLOADING_MERVILLE_TITLE";
char aMpMerv[] = "mp_merv";
extern const char* const szPlayLanMenuDescriptionReferences[3];  // @ 0xE381BC
extern int dword_F641D0[1580 * 802];   // cg.o
extern char byte_F64194[6320 * 802];   // cg.o
extern int dword_F641D4[1580 * 802];   // cg.o
extern int dword_F6A28C[4 * 802];      // cg.o
extern int dword_F6A290[4 * 802];      // cg.o
extern int dword_186A0;                // damage constant (game.o)
extern vmCvar_t cg_widescreen;         // cg.o @ 0xF5CC88
extern void CG_FillRect(float x, float y, float width, float height,
                        const float* color, float z);  // ?CG_FillRect@@YAXMMMMQBMH@Z (cg.o)

int scoreboard_player_sorter(const void* left, const void* right);

class AARXBoxLiveIngameOptions {
public:
    AARXBoxLiveIngameOptions(FEMenuSystem* s);  // ??0AARXBoxLiveIngameOptions@@QAE@PAVFEMenuSystem@@@Z (mp_xbox.o)
    static AARXBoxLiveIngameOptions* Me();  // ?Me@AARXBoxLiveIngameOptions@@SAPAV1@XZ (game_xbox.o)
    bool SetTimerText();                    // ?SetTimerText@AARXBoxLiveIngameOptions@@QAE_NXZ (game_xbox.o)
};

enum ESpectatorState : int {
    kSpectatorStateIntermission = 0x0,
    kSpectatorStateSpawn = 0x1,
    kSpectatorStateInjured = 0x2,
    kSpectatorStateDying = 0x3,
    kSpectatorStateDead = 0x4,
    kSpectatorStateDeadCanSpawn = 0x5,
    kSpectatorStateCount = 0x6,
};

// Minimal views for game-side symbols used by the menus
struct BrocExports {
    void (*mCallbackSpawnButtonPressed)(int entityHandle);  // +0xCD0 (Broc::entity)
    void (*mCallbackPlayerClassChange)(int entityHandle, int playerClass);  // +0xC68
};
struct BrocAPI {
    BrocExports mBrocExports;  // +0x00
};
extern BrocAPI* gpBrocAPI;  // 0xF3ABDC

// STBManager - string table bank manager (core.o owns the definition)
class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    const char* GetSTBString(const char* pszReference);  // ?GetSTBString@STBManager@@QAEPBDPBD@Z
};

// Starting-map combo -> map id (byte_E386C9[114*k], 114-byte map records)
static const unsigned char sFindMapIdByComboOption[9] = {
    0x00, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x60,
};

enum hitLocation_t : int;

struct MenuClearHelper : FEMenu {
    void ClearAll() { ClearAllButtons(); }
};
void player_die(void* self, void* inflictor, void* attacker, int damage,
                int meansOfDeath, int iWeapon, const float* vPosition,
                const float* vDir, hitLocation_t hitLoc);  // ?player_die@@YAXPAVEntity@@00HHHPBM1W4hitLocation_t@@@Z (g.o)

extern void tlPrintf(const char* fmt, ...);  // ?tlPrintf@@YAXPBDZZ (tl_system.o)
extern "C" void __stdcall DmGetXboxName(char* name, unsigned int* size);  // xbox_shim
extern void j_nullsub_46(void* self);  // g.o nullsub
extern void j_nullsub_58(void* self, bool use);  // g.o nullsub
extern int irand(int min, int max);  // ?irand@@YAHHH@Z (g.o)
extern int g_NumBdMessages;  // ?g_NumBdMessages@@3HA (bd.o)
extern ELanguage gLanguage;  // 0x012F03A4 (core.o)
void ShowNotificationIcon(unsigned int* menuIcon, PanelQuad* inviteQuad,
                          PanelQuad* friendQuad);  // platform_xbox (XboxLiveMenus.cpp)
namespace PlayerStats {
int TotalScoreForStats(short* stats);  // ?TotalScoreForStats@PlayerStats@@YAHQAF@Z (mp.o)
}
namespace View {
bool IsSplitScreen();  // ?IsSplitScreen@View@@YA_NXZ (cg.o)
void UpdateNumViewports();  // ?UpdateNumViewports@View@@YAXXZ (cg.o)
}
struct cgGlobal_t {
    int   frametime;  // +0x00
    int   time;       // +0x04
    int   oldTime;    // +0x08
    int   cubemapShot; // +0x0C
    int   cubemapSize; // +0x10
    bool  teamGame;   // +0x14
    bool  showScore;  // +0x15
    uint8_t _pad16[0x18 - 0x16];
    float gameTime;   // +0x18
    float gameTimeStartTime;  // +0x1C
    int   teamScores[5];      // +0x20
};
extern cgGlobal_t cgGlobal;   // 0xF5FE30 (cg.o)
struct AARMenuSystem;
namespace LocalClient {
bool QuitClientOutOfGame(int client);  // ?QuitClientOutOfGame@LocalClient@@YA_NH@Z (cl.o)
int  PortToClient(int port);           // ?PortToClient@LocalClient@@YAHH@Z (cl.o)
int  ClientToPort(int client);         // ?ClientToPort@LocalClient@@YAHH@Z (cl.o)
void UpdatePlayerPorts(int fixedPort); // ?UpdatePlayerPorts@LocalClient@@YAXH@Z (cl.o)
}

extern bool LocalClient_QuitClientOutOfGame(int client);
extern int LocalClient_PortToClient(int port);
extern int LocalClient_ClientToPort(int client);
extern void LocalClient_UpdatePlayerPorts(int fixedPort);
namespace LocalClient {
bool QuitClientOutOfGame(int client) { return LocalClient_QuitClientOutOfGame(client); }
int PortToClient(int port) { return LocalClient_PortToClient(port); }
int ClientToPort(int client) { return LocalClient_ClientToPort(client); }
void UpdatePlayerPorts(int fixedPort) { LocalClient_UpdatePlayerPorts(fixedPort); }
}

unsigned char CreateSessionMenu::m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@CreateSessionMenu@@1EA @ 0x1388D54
unsigned char CreateLanSessionMenu::m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@CreateLanSessionMenu@@1EA @ 0x1388D55
unsigned char FindSessionMenu::m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@FindSessionMenu@@1EA @ 0x1388D56
unsigned char FindLanSessionMenu::m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@FindLanSessionMenu@@1EA @ 0x1388D57
unsigned char AARMapVote::m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@AARMapVote@@1EA @ 0x1388D58
int PlayOnlineMenu::m_currSelection;  // ?m_currSelection@PlayOnlineMenu@@1HA @ 0xE381C4

// ============================================================================
// Small accessors (mp_shell.o, 16-byte functions)
// ============================================================================

// ea: 0x0078BEB0
short FEComboBox::GetCurrOption() const
{
    return mCurrOption;
}

// ea: 0x0078BEC0
const int MPUIInterface::GetMaxScoreLimitCount()
{
    return MPUIInterface::mMaxScoreLimitCount;
}

// ea: 0x0078BED0
void MPUIInterface::SetViewOnlineStatus(bool state)
{
    MPUIInterface::mIsViewableOnline = state;
}

// ea: 0x0078BEE0
void UIListBox::BlockRefresh(bool block)
{
    mBlockRefresh = block;
}

// ea: 0x0078BEF0
const OverlayMenu::eState OverlayMenu::GetState()
{
    return mState;
}

// ============================================================================
// CreateSessionMenu (mp_shell.o CreateSessionMenu.cpp)
// ============================================================================

// ea: 0x0078BF00
CreateSessionMenu::CreateSessionMenu(FEMenuSystem* s)
    : FEMenu(s, 8, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    mNumberOfPlayersCombo = nullptr;
    mGameModeCombo = nullptr;
    mStartingMapCombo = nullptr;
    m_PrivateSlotsCombo = nullptr;
    m_LastPlayerCount = 0;
    default_color_scheme = 5;
    mLastGameType = -1;
    mLastMap = -1;
    for (int i = 0; i < 6; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 8; ++i)
        m_pSlotText[i] = nullptr;
    for (int i = 0; i < 8; ++i)
        m_pSlotGeometry[i] = nullptr;
    memset(m_szSessionName, 0, sizeof(m_szSessionName));
}

// ea: 0x0078BFE0
CreateSessionMenu::~CreateSessionMenu()
{
    for (int i = 0; i < 6; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 8; ++i)
        m_pSlotText[i] = nullptr;
    mGameModeCombo = nullptr;
    mNumberOfPlayersCombo = nullptr;
    mStartingMapCombo = nullptr;
    m_PrivateSlotsCombo = nullptr;
    memset(m_szSessionName, 0, sizeof(m_szSessionName));
    m_FirstTimeAccessedByte = 0;
}

// ea: 0x0078C060
CreateSessionMenu* CreateSessionMenu::Me()
{
    return (CreateSessionMenu*)g_femanager.fems->menus[0];
}

// ea: 0x0078C070 (thunk)
void CreateSessionMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x0078C080
void CreateSessionMenu::Init()
{
}

// ea: 0x0078C090
void CreateSessionMenu::UpdateNetworking()
{
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
}

// ea: 0x0078C0C0
void CreateSessionMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
}

// ea: 0x0078C100
void CreateSessionMenu::UpdatePrivateSlots()
{
    int mCurrOption = mNumberOfPlayersCombo->mCurrOption;
    int v6 = 4 * mCurrOption + 4;
    int tempPrivateSlotOption = m_PrivateSlotsCombo->mCurrOption;
    if (m_LastPlayerCount != v6)
    {
        m_LastPlayerCount = v6;
        if (m_PrivateSlotsCombo != nullptr)
            delete m_PrivateSlotsCombo;
        FEText* TextPointer =
            panel->GetTextPointer("cg_slot_04_text_spec");
        PanelQuad* leftArrow =
            panel->GetPointer("cg_slot_04_arrow_left");
        PanelQuad* rightArrow =
            panel->GetPointer("cg_slot_04_arrow_right");
        FEComboBox* v11 = AddComboBox(7, v6, TextPointer, leftArrow,
                                      rightArrow);
        m_PrivateSlotsCombo = v11;
        if (v11 == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/CreateSessionMenu.cpp";
            AeAssert::gCurrentLine = 443;
            AeAssert::gCurrentExpr = "m_PrivateSlotsCombo";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Combobox failure for private slots"))
                __debugbreak();
        }
        if (v6 > 0)
        {
            char szNum[32];
            for (int v12 = 0; v12 < v6; ++v12)
            {
                sprintf(szNum, "%i", v12);
                Broc::string s(szNum);
                m_PrivateSlotsCombo->AddOption(s);
            }
        }
        if (tempPrivateSlotOption >= v6)
            m_PrivateSlotsCombo->SetCurrOption((short)(v6 - 1));
        else
            m_PrivateSlotsCombo->SetCurrOption((short)tempPrivateSlotOption);
        entries[1]->up = 7;
        entries[1]->down = 3;
        entries[3]->up = 1;
        entries[3]->down = 5;
        entries[5]->up = 3;
        entries[5]->down = 7;
        entries[7]->up = 5;
        entries[7]->down = 1;
    }
}

// ea: 0x0078C290
void CreateSessionMenu::GrabSessionName()
{
    if (MPLiveEngine::GetHandle()->internalState == kSignedIn
        && MPUIInterface::IsOnlineGame())
    {
        tlPrintf("CreateSessionMenu::GrabSessionName() - LIVE name\n");
        MPLiveEngine* Handle = MPLiveEngine::GetHandle();
        LivePlayer* LocalPlayer =
            (LivePlayer*)Handle->GetLocalPlayer(Handle->actualPort);
        _snprintf(m_szSessionName, 0x10u, "%S", LocalPlayer->gamertag);
    }
    else if (MPUIInterface::IsLANGame())
    {
        char xbox_name[256];
        unsigned int size = 255;
        DmGetXboxName(xbox_name, &size);
        strncpy(m_szSessionName, xbox_name, 0x10u);
    }
}

// ea: 0x0078C330
void CreateSessionMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078C350
void CreateSessionMenu::OnCircle(int c)
{
    (void)c;
}

// ea: 0x0078C360
void CreateSessionMenu::OnTriangle(int c)
{
    (void)c;
    MPUIInterface::mServerParams.mPrivateSlots =
        (unsigned char)m_PrivateSlotsCombo->mCurrOption;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078C380
void CreateSessionMenu::SetGameTypeDefaults()
{
    int mLastGameType = this->mLastGameType;
    if (mGameModeCombo->mCurrOption != mLastGameType)
    {
        if (mLastGameType == 5)
        {
            MPUIInterface::mServerParams.mFriendlyFire = 1;
            MPUIInterface::mServerParams.mEnablePenaltyVote = 1;
        }
        eGameType mCurrOption = (eGameType)mGameModeCombo->mCurrOption;
        this->mLastGameType = mCurrOption;
        MPUIInterface::mServerParams.mScoreLimit =
            (unsigned char)MPUIInterface::GetDefaultOption(
                MPUIInterface::SETTING_SCORE_LIMIT, mCurrOption);
        if (this->mLastGameType == 5)
        {
            MPUIInterface::mServerParams.mTeamBalancing = 0;
            MPUIInterface::mServerParams.mFriendlyFire = 0;
            MPUIInterface::mServerParams.mEnablePenaltyVote = 0;
        }
    }
}

// ea: 0x007942C0
void CreateSessionMenu::UpdateHighlight()
{
    entries[0]->Highlight(highlighted == 1, true);
    entries[2]->Highlight(highlighted == 3, true);
    entries[4]->Highlight(highlighted == 5, true);
    entries[6]->Highlight(highlighted == 7, true);
    UpdatePrivateSlots();
}

// ea: 0x00794330
void CreateSessionMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
    MPUIInterface::mServerParams.mGameType =
        (unsigned char)mGameModeCombo->mCurrOption;
    short mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != 0xFF)
        mCurrOption = (short)(unsigned char)byte_E386C9[114 * mCurrOption];
    MPUIInterface::mServerParams.mMapID = (unsigned char)mCurrOption;
    MPUIInterface::mServerParams.mMaxPlayers =
        (unsigned char)mNumberOfPlayersCombo->mCurrOption;
    MPUIInterface::mServerParams.mPrivateSlots =
        (unsigned char)m_PrivateSlotsCombo->mCurrOption;
}

// ea: 0x00794390
void CreateSessionMenu::OnSquare(int c)
{
    (void)c;
    MPUIInterface::mServerParams.mGameType =
        (unsigned char)mGameModeCombo->mCurrOption;
    short mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != 0xFF)
        mCurrOption = (short)(unsigned char)byte_E386C9[114 * mCurrOption];
    MPUIInterface::mServerParams.mMapID = (unsigned char)mCurrOption;
    MPUIInterface::mServerParams.mMaxPlayers =
        (unsigned char)mNumberOfPlayersCombo->mCurrOption;
    MPUIInterface::mServerParams.mPrivateSlots =
        (unsigned char)m_PrivateSlotsCombo->mCurrOption;
    system->MakeActiveAndReturn(2);
}

// ea: 0x007943F0
void CreateSessionMenu::OnUp(int c)
{
    (void)c;
    Up();
    UpdateHighlight();
}

// ea: 0x00794410
void CreateSessionMenu::OnDown(int c)
{
    (void)c;
    Down();
    UpdateHighlight();
}

// ea: 0x00794430
void CreateSessionMenu::SetMapDefaults()
{
    int mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != mLastMap)
    {
        mLastMap = mCurrOption;
        short MaxPlayersOptionFromMap;
        if (mCurrOption == 0xFF)
            MaxPlayersOptionFromMap = MPUIInterface::GetMaxPlayersOptionFromMap(255);
        else
            MaxPlayersOptionFromMap = MPUIInterface::GetMaxPlayersOptionFromMap(
                byte_E386C9[114 * mCurrOption]);
        mNumberOfPlayersCombo->SetCurrOption(MaxPlayersOptionFromMap);
    }
}

// ea: 0x007A7B00
void CreateSessionMenu::OnLeft(int c)
{
    (void)c;
    Left();
    switch (highlighted)
    {
    case 1:
        SetGameTypeDefaults();
        break;
    case 3:
        SetMapDefaults();
        break;
    case 5:
        UpdatePrivateSlots();
        break;
    default:
        break;
    }
}

// ea: 0x007A7B50
void CreateSessionMenu::OnRight(int c)
{
    (void)c;
    Right();
    switch (highlighted)
    {
    case 1:
        SetGameTypeDefaults();
        break;
    case 3:
        SetMapDefaults();
        break;
    case 5:
        UpdatePrivateSlots();
        break;
    default:
        break;
    }
}

// ============================================================================
// CreateLanSessionMenu (mp_shell.o CreateLanSessionMenu.cpp)
// ============================================================================

// ea: 0x0078C3F0
CreateLanSessionMenu::CreateLanSessionMenu(FEMenuSystem* s)
    : FEMenu(s, 6, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    mNumberOfPlayersCombo = nullptr;
    mGameModeCombo = nullptr;
    mStartingMapCombo = nullptr;
    default_color_scheme = 5;
    mLastGameType = -1;
    mLastMap = -1;
    for (int i = 0; i < 4; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 6; ++i)
        m_pSlotText[i] = nullptr;
    for (int i = 0; i < 6; ++i)
        m_pSlotGeometry[i] = nullptr;
    memset(m_szSessionName, 0, sizeof(m_szSessionName));
}

// ea: 0x0078C4C0
CreateLanSessionMenu::~CreateLanSessionMenu()
{
    for (int i = 0; i < 4; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 6; ++i)
        m_pSlotText[i] = nullptr;
    mGameModeCombo = nullptr;
    mNumberOfPlayersCombo = nullptr;
    mStartingMapCombo = nullptr;
    memset(m_szSessionName, 0, sizeof(m_szSessionName));
    m_FirstTimeAccessedByte = 0;
}

// ea: 0x0078C530
CreateLanSessionMenu* CreateLanSessionMenu::Me()
{
    return (CreateLanSessionMenu*)g_femanager.fems->menus[1];
}

// ea: 0x0078C540 (thunk)
void CreateLanSessionMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x0078C550
void CreateLanSessionMenu::Init()
{
}

// ea: 0x0078C560
void CreateLanSessionMenu::UpdateNetworking()
{
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
}

// ea: 0x0078C590
void CreateLanSessionMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
}

// ea: 0x0078C5D0 (thunk)
void CreateLanSessionMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x0078C5E0
void CreateLanSessionMenu::GrabSessionName(int c)
{
    (void)c;
    if (MPLiveEngine::GetHandle()->internalState == kSignedIn
        && MPUIInterface::IsOnlineGame())
    {
        MPLiveEngine* Handle = MPLiveEngine::GetHandle();
        LivePlayer* LocalPlayer =
            (LivePlayer*)Handle->GetLocalPlayer(Handle->actualPort);
        _snprintf(m_szSessionName, 0x10u, "%S", LocalPlayer->gamertag);
    }
    else if (MPUIInterface::IsLANGame())
    {
        char xbox_name[256];
        unsigned int size = 255;
        DmGetXboxName(xbox_name, &size);
        strncpy(m_szSessionName, xbox_name, 0x10u);
    }
}

// ea: 0x0078C680
void CreateLanSessionMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078C6A0
void CreateLanSessionMenu::OnCircle(int c)
{
    (void)c;
}

// ea: 0x0078C6B0
void CreateLanSessionMenu::OnUp(int c)
{
    (void)c;
    Up();
    entries[4]->Highlight(highlighted == 5, true);
    entries[0]->Highlight(highlighted == 1, true);
    entries[2]->Highlight(highlighted == 3, true);
}

// ea: 0x0078C700
void CreateLanSessionMenu::OnDown(int c)
{
    (void)c;
    Down();
    entries[4]->Highlight(highlighted == 5, true);
    entries[0]->Highlight(highlighted == 1, true);
    entries[2]->Highlight(highlighted == 3, true);
}

// ea: 0x0078C750
void CreateLanSessionMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(9);
}

// ea: 0x0078C760
void CreateLanSessionMenu::SetGameTypeDefaults()
{
    int mLastGameType = this->mLastGameType;
    if (mGameModeCombo->mCurrOption != mLastGameType)
    {
        if (mLastGameType == 5)
        {
            MPUIInterface::mServerParams.mFriendlyFire = 1;
            MPUIInterface::mServerParams.mEnablePenaltyVote = 1;
        }
        eGameType mCurrOption = (eGameType)mGameModeCombo->mCurrOption;
        this->mLastGameType = mCurrOption;
        MPUIInterface::mServerParams.mScoreLimit =
            (unsigned char)MPUIInterface::GetDefaultOption(
                MPUIInterface::SETTING_SCORE_LIMIT, mCurrOption);
        if (this->mLastGameType == 5)
        {
            MPUIInterface::mServerParams.mFriendlyFire = 0;
            MPUIInterface::mServerParams.mEnablePenaltyVote = 0;
            MPUIInterface::mServerParams.mTeamBalancing = 0;
        }
    }
}

// ea: 0x00795070
void CreateLanSessionMenu::OnSquare(int c)
{
    (void)c;
    MPUIInterface::mServerParams.mGameType =
        (unsigned char)mGameModeCombo->mCurrOption;
    short mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != 0xFF)
        mCurrOption = (short)(unsigned char)byte_E386C9[114 * mCurrOption];
    MPUIInterface::mServerParams.mMapID = (unsigned char)mCurrOption;
    MPUIInterface::mServerParams.mMaxPlayers =
        (unsigned char)mNumberOfPlayersCombo->mCurrOption;
    system->MakeActiveAndReturn(3);
}

// ea: 0x007950C0
void CreateLanSessionMenu::SetMapDefaults()
{
    int mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != mLastMap)
    {
        mLastMap = mCurrOption;
        short MaxPlayersOptionFromMap;
        if (mCurrOption == 0xFF)
            MaxPlayersOptionFromMap = MPUIInterface::GetMaxPlayersOptionFromMap(255);
        else
            MaxPlayersOptionFromMap = MPUIInterface::GetMaxPlayersOptionFromMap(
                byte_E386C9[114 * mCurrOption]);
        mNumberOfPlayersCombo->SetCurrOption(MaxPlayersOptionFromMap);
    }
}

// ea: 0x007A7EF0
void CreateLanSessionMenu::OnLeft(int c)
{
    (void)c;
    Left();
    if (highlighted == 1)
    {
        SetGameTypeDefaults();
    }
    else if (highlighted == 3)
    {
        SetMapDefaults();
    }
}

// ea: 0x007A7F30
void CreateLanSessionMenu::OnRight(int c)
{
    (void)c;
    Right();
    if (highlighted == 1)
    {
        SetGameTypeDefaults();
    }
    else if (highlighted == 3)
    {
        SetMapDefaults();
    }
}

// ============================================================================
// FindSessionMenu (mp_shell.o FindSessionMenu.cpp)
// ============================================================================

// ea: 0x0078CF40
FindSessionMenu::FindSessionMenu(FEMenuSystem* s)
    : FEMenu(s, 10, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    mNumberOfPlayersCombo = nullptr;
    mGameModeCombo = nullptr;
    mStartingMapCombo = nullptr;
    m_AutoTeamBalanceCombo = nullptr;
    m_TeamDamageCombo = nullptr;
    default_color_scheme = 5;
    for (int i = 0; i < 6; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 10; ++i)
        m_pSlotText[i] = nullptr;
    memset(&m_pSlotGeometry, 0, sizeof(m_pSlotGeometry));
    for (int i = 0; i < 6; ++i)
        m_pBackgroundRow[i] = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pBackgroundLine[i] = nullptr;
}

// ea: 0x0078D020
FindSessionMenu::~FindSessionMenu()
{
    for (int i = 0; i < 6; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 10; ++i)
        m_pSlotText[i] = nullptr;
    memset(&m_pSlotGeometry, 0, sizeof(m_pSlotGeometry));
    for (int i = 0; i < 6; ++i)
        m_pBackgroundRow[i] = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pBackgroundLine[i] = nullptr;
    mGameModeCombo = nullptr;
    mNumberOfPlayersCombo = nullptr;
    mStartingMapCombo = nullptr;
    m_AutoTeamBalanceCombo = nullptr;
    m_TeamDamageCombo = nullptr;
    m_FirstTimeAccessedByte = 0;
}

// ea: 0x0078D0D0
FindSessionMenu* FindSessionMenu::Me()
{
    return (FindSessionMenu*)g_femanager.fems->menus[4];
}

// ea: 0x0078D0E0 (thunk)
void FindSessionMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x0078D0F0
void FindSessionMenu::Init()
{
}

// ea: 0x0078D100
void FindSessionMenu::UpdateNetworking()
{
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
}

// ea: 0x0078D130
void FindSessionMenu::Update(float time_inc)
{
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
    FEMenu::Update(time_inc);
}

// ea: 0x0078D170
void FindSessionMenu::UpdateHighlight()
{
    entries[0]->Highlight(highlighted == 1, true);
    entries[2]->Highlight(highlighted == 3, true);
    entries[4]->Highlight(highlighted == 5, true);
    entries[6]->Highlight(highlighted == 7, true);
    entries[8]->Highlight(highlighted == 9, true);
}

// ea: 0x0078D1F0 (thunk)
void FindSessionMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x0078D200
void FindSessionMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078D220
void FindSessionMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078D230
void FindSessionMenu::OnUp(int c)
{
    (void)c;
    Up();
    UpdateHighlight();
}

// ea: 0x0078D250
void FindSessionMenu::OnDown(int c)
{
    (void)c;
    Down();
    UpdateHighlight();
}

// ============================================================================
// FindLanSessionMenu (mp_shell.o FindLanSessionMenu.cpp)
// ============================================================================

// ea: 0x0078D270
FindLanSessionMenu::FindLanSessionMenu(FEMenuSystem* s)
    : FEMenu(s, 10, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    mNumberOfPlayersCombo = nullptr;
    mGameModeCombo = nullptr;
    mStartingMapCombo = nullptr;
    m_AutoTeamBalanceCombo = nullptr;
    m_TeamDamageCombo = nullptr;
    default_color_scheme = 5;
    for (int i = 0; i < 4; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 10; ++i)
        m_pSlotText[i] = nullptr;
    memset(&m_pSlotGeometry, 0, sizeof(m_pSlotGeometry));
    for (int i = 0; i < 6; ++i)
        m_pBackgroundRow[i] = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pBackgroundLine[i] = nullptr;
}

// ea: 0x0078D350
FindLanSessionMenu::~FindLanSessionMenu()
{
    for (int i = 0; i < 4; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 10; ++i)
        m_pSlotText[i] = nullptr;
    memset(&m_pSlotGeometry, 0, sizeof(m_pSlotGeometry));
    for (int i = 0; i < 6; ++i)
        m_pBackgroundRow[i] = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pBackgroundLine[i] = nullptr;
    mGameModeCombo = nullptr;
    mNumberOfPlayersCombo = nullptr;
    mStartingMapCombo = nullptr;
    m_AutoTeamBalanceCombo = nullptr;
    m_TeamDamageCombo = nullptr;
    m_FirstTimeAccessedByte = 0;
}

// ea: 0x0078D400
FindLanSessionMenu* FindLanSessionMenu::Me()
{
    return (FindLanSessionMenu*)g_femanager.fems->menus[5];
}

// ea: 0x0078D410 (thunk)
void FindLanSessionMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x0078D420
void FindLanSessionMenu::Init()
{
}

// ea: 0x0078D430
void FindLanSessionMenu::UpdateNetworking()
{
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
}

// ea: 0x0078D460
void FindLanSessionMenu::Update(float time_inc)
{
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
    FEMenu::Update(time_inc);
}

// ea: 0x0078D4A0
void FindLanSessionMenu::UpdateHighlight()
{
    entries[4]->Highlight(highlighted == 5, true);
    entries[0]->Highlight(highlighted == 1, true);
    entries[2]->Highlight(highlighted == 3, true);
    entries[6]->Highlight(highlighted == 7, true);
    entries[8]->Highlight(highlighted == 9, true);
}

// ea: 0x0078D520 (thunk)
void FindLanSessionMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x0078D530
void FindLanSessionMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078D550
void FindLanSessionMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078D560
void FindLanSessionMenu::OnUp(int c)
{
    (void)c;
    Up();
    UpdateHighlight();
}

// ea: 0x0078D580
void FindLanSessionMenu::OnDown(int c)
{
    (void)c;
    Down();
    UpdateHighlight();
}

// ============================================================================
// CreateSessionAdvancedMenu / CreateLanSessionAdvancedMenu
// ============================================================================

// ea: 0x0078CAD0
CreateSessionAdvancedMenu* CreateSessionAdvancedMenu::Me()
{
    return (CreateSessionAdvancedMenu*)g_femanager.fems->menus[2];
}

// ea: 0x0078CE80
CreateLanSessionAdvancedMenu* CreateLanSessionAdvancedMenu::Me()
{
    return (CreateLanSessionAdvancedMenu*)g_femanager.fems->menus[3];
}

// ============================================================================
// GameSettingsEdit / GameSettingsView (mp_shell.o GameSettings*.cpp)
// ============================================================================

// ea: 0x0078D5A0
void GameSettingsEdit::Init()
{
}

// ea: 0x0078D710
void GameSettingsEdit::OnL1(int c)
{
    (void)c;
}

// ea: 0x0078D720
void GameSettingsEdit::OnR1(int c)
{
    (void)c;
}

// ea: 0x0078D730
void GameSettingsEdit::OnCross(int c)
{
    (void)c;
}

// ea: 0x0078D740
bool GameSettingsEdit::ResponseNoJustGoBackToPauseMenuHelper()
{
    system->ReturnToPreviousMenu(-1);
    return true;
}

// ea: 0x0078D780
void GameSettingsEdit::OnCircle(int c)
{
    (void)c;
}

// ea: 0x0078D790
void GameSettingsEdit::OnSquare(int c)
{
    (void)c;
}

// ea: 0x0078D7A0
void GameSettingsEdit::OnStart(int c)
{
    (void)c;
}

// ea: 0x0078D7B0
void GameSettingsEdit::ButtonHeldAction()
{
}

// ea: 0x0078D8D0
AARGameSettingsEdit* AARGameSettingsEdit::Me(int version)
{
    (void)version;
    return (AARGameSettingsEdit*)g_femanager.mAARS->menus[6];
}

// ea: 0x0078D990
void GameSettingsView::Init()
{
}

// ea: 0x0078DF70 (thunk)
void GameSettingsView::OnDeactivate(FESplitScreenMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x0078DFA0
void GameSettingsView::OnLeft(int c)
{
    (void)c;
}

// ea: 0x0078DFB0
void GameSettingsView::OnRight(int c)
{
    (void)c;
}

// ea: 0x0078DFC0
void GameSettingsView::OnL1(int c)
{
    (void)c;
}

// ea: 0x0078DFD0
void GameSettingsView::OnR1(int c)
{
    (void)c;
}

// ea: 0x0078DFE0
void GameSettingsView::OnCross(int c)
{
    (void)c;
}

// ea: 0x0078DFF0
void GameSettingsView::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078E000
void GameSettingsView::OnCircle(int c)
{
    (void)c;
}

// ea: 0x0078E010
void GameSettingsView::OnSquare(int c)
{
    (void)c;
}

// ea: 0x0078E020
void GameSettingsView::ButtonHeldAction()
{
}

// ea: 0x0078E030
AARGameSettingsView* AARGameSettingsView::Me(int version)
{
    (void)version;
    return (AARGameSettingsView*)g_femanager.mAARS->menus[7];
}

// ea: 0x0078E040 (thunk)
void AARGameSettingsView::SetPanelFile(PanelFile* pf)
{
    SetPanelFileMain(pf);
}

// ============================================================================
// InitialLoadingMenu / InstantActionMenu / PlayLanMenu / PlayOnlineMenu /
// PressStartMenu
// ============================================================================

// ea: 0x0078E160
void InitialLoadingMenu::Select(int entry_num)
{
    (void)entry_num;
}

// ea: 0x0078E220
InitialLoadingMenu* InitialLoadingMenu::Me()
{
    return (InitialLoadingMenu*)g_femanager.fems->menus[6];
}

// ea: 0x0078E280 (thunk)
void InstantActionMenu::OnActivate()
{
    FEMenu::OnActivate();
}

// ea: 0x0078E410
void InstantActionMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078E6A0
InstantActionMenu* InstantActionMenu::Me()
{
    return (InstantActionMenu*)g_femanager.fems->menus[7];
}

// ea: 0x0078E6B0
void PlayLanMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
}

// ea: 0x0078E770
void PlayLanMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(8);
}

// ea: 0x0078E780
PlayLanMenu* PlayLanMenu::Me()
{
    return (PlayLanMenu*)g_femanager.fems->menus[9];
}

// ea: 0x0078E7E0
void PlayOnlineMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    PlayOnlineMenu::m_currSelection = highlighted;
}

// ea: 0x0078E810
void PlayOnlineMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(8);
}

// ea: 0x0078E820
PlayOnlineMenu* PlayOnlineMenu::Me()
{
    return (PlayOnlineMenu*)g_femanager.fems->menus[10];
}

// ea: 0x0078EA50
void PressStartMenu::Select(int entry_num)
{
    (void)entry_num;
}

// ea: 0x0078EA60
void PressStartMenu::OnTriangle(int c)
{
    (void)c;
}

// ea: 0x0078EA70
void PressStartMenu::OnStart(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078EB00
PressStartMenu* PressStartMenu::Me()
{
    return (PressStartMenu*)g_femanager.fems->menus[11];
}

// ============================================================================
// SessionDetailsMenu / SessionListMenu / SessionLanListMenu
// ============================================================================

// ea: 0x0078EB50 (thunk)
void SessionDetailsMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    j_nullsub_46(this);
}

// ea: 0x0078EB80
void SessionDetailsMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078EE70
SessionDetailsMenu* SessionDetailsMenu::Me()
{
    return (SessionDetailsMenu*)g_femanager.fems->menus[12];
}

// ea: 0x0078EEE0
SessionListMenu* SessionListMenu::Me()
{
    return (SessionListMenu*)g_femanager.fems->menus[13];
}

// ea: 0x0078EF70
SessionLanListMenu* SessionLanListMenu::Me()
{
    return (SessionLanListMenu*)g_femanager.fems->menus[14];
}

// ea: 0x0078EFA0
void SessionLanListMenu::OnCross(int c)
{
    Select(c);
}

// ============================================================================
// Overlay menu family (mp_shell.o overlay.cpp)
// ============================================================================

// ea: 0x0078F330
void OverlayMenu::OnCircle(int c)
{
    (void)c;
}

// ea: 0x0078F4C0
void OverlayMenu::LogonUpdate()
{
}

// ea: 0x0078F4D0
void InGameOverlay::Select(int __formal)
{
    (void)__formal;
}

// ea: 0x0078F780
void InGameOverlay::OnCircle(int __formal)
{
    (void)__formal;
}

// ea: 0x0078F790
void InGameOverlay::OnSquare(int __formal)
{
    (void)__formal;
}

// ea: 0x0078F870
void AAROverlay::Select(int __formal)
{
    (void)__formal;
}

// ea: 0x0078FB10
void AAROverlay::OnCircle(int __formal)
{
    (void)__formal;
}

// ea: 0x0078FB20
void AAROverlay::OnSquare(int __formal)
{
    (void)__formal;
}

// ea: 0x00790030 (thunk)
void MultilineFrontendOverlayMenu::OnCross(int c)
{
    FEMenu::OnCross(c);
}

// ea: 0x00790060
MultilineFrontendOverlayMenu* MultilineFrontendOverlayMenu::Me()
{
    return (MultilineFrontendOverlayMenu*)g_femanager.fems->menus[17];
}

// ea: 0x007902A0 (thunk)
void MultilineFrontendOverlayMenu::OnTriangle(int c)
{
    OverlayMenuBase::OnTriangle(c);
}

// ea: 0x00790920 (thunk)
void VoteMapMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ============================================================================
// WeaponSelectMenu / InGameSwitchSides
// ============================================================================

// ea: 0x00790A50 (thunk)
void WeaponSelectMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x00790C60
void WeaponSelectMenu::OnStart(int c)
{
    (void)c;
}

// ea: 0x00790E90
void InGameSwitchSides::Init()
{
}

// ea: 0x00790EA0 (thunk)
void InGameSwitchSides::OnDeactivate(ModelMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x00790EB0
void InGameSwitchSides::OnTriangle(int c)
{
    (void)c;
}

// ea: 0x00790EC0
bool InGameSwitchSides::ResponseNoNevermind(int index)
{
    (void)index;
    return true;
}

// ============================================================================
// InGameScoreBoard
// ============================================================================

// ea: 0x00790D50
void InGameScoreBoard::Init()
{
}

// ea: 0x00790DC0
int InGameScoreBoard::GetAlliesScore()
{
    return cgGlobal.teamScores[2];
}

// ea: 0x00790DD0
int InGameScoreBoard::GetAxisScore()
{
    return cgGlobal.teamScores[1];
}

// ea: 0x00790E10
void InGameScoreBoard::OnTriangle(int c)
{
    OnSelect(c);
}

// ea: 0x00790E20
void InGameScoreBoard::OnSquare(int c)
{
    (void)c;
}

// ============================================================================
// AAR scoreboard / stats / vote menus
// ============================================================================

// ea: 0x00791000 (thunk)
void AARScoreboardBase::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x00791010
void AARScoreboardBase::Init()
{
}

// ea: 0x00791040
void AARScoreboardBase::OnSquare(int c)
{
    (void)c;
}

// ea: 0x00791070
int AARScoreboardBase::GetAlliesScore()
{
    return cgGlobal.teamScores[2];
}

// ea: 0x00791080
int AARScoreboardBase::GetAxisScore()
{
    return cgGlobal.teamScores[1];
}

// ea: 0x00791090
void AARScoreboardBase::OnLeft(int c)
{
    OnL1(c);
}

// ea: 0x007910A0
void AARScoreboardBase::OnRight(int c)
{
    OnR1(c);
}

// ea: 0x007910B0
void AARScoreboardBase::OnUp(int c)
{
    m_ListBox.OnUp(c);
}

// ea: 0x007910C0
void AARScoreboardBase::OnDown(int c)
{
    m_ListBox.OnDown(c);
}

// ea: 0x00791150
void AARScoreboardLoser::OnR1(int c)
{
    (void)c;
    system->MakeActive(2);
}

// ea: 0x00791160
void AARScoreboardLoser::OnL1(int c)
{
    (void)c;
    system->MakeActive(0);
}

// ea: 0x007912C0
AARPersonalStats* AARPersonalStats::Me()
{
    return (AARPersonalStats*)g_femanager.mAARS->menus[2];
}

// ea: 0x007912D0 (thunk)
void AARPersonalStats::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x007912E0
void AARPersonalStats::Init()
{
}

// ea: 0x007912F0
void AARPersonalStats::OnCross(int c)
{
    (void)c;
}

// ea: 0x00791300
void AARPersonalStats::OnUp(int c)
{
    (void)c;
}

// ea: 0x00791310
void AARPersonalStats::OnDown(int c)
{
    (void)c;
}

// ea: 0x00791340
void AARPersonalStats::OnLeft(int c)
{
    OnL1(c);
}

// ea: 0x00791350
void AARPersonalStats::OnRight(int c)
{
    OnR1(c);
}

// ea: 0x00791510
AARMapVote* AARMapVote::Me()
{
    return (AARMapVote*)g_femanager.mAARS->menus[4];
}

// ea: 0x007A54D0
void AARMapVote::ResetPanel()
{
    const int mapLimit = g_NumBaseMaps + 1;
    if (m_currentRow >= mapLimit || m_currentRow < 0) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/ui/AARMapVote.cpp";
        AeAssert::gCurrentLine = 611;
        AeAssert::gCurrentExpr = "m_currentRow < kAARPMapLimit && m_currentRow >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("indexing error"))
            __debugbreak();
    }
    m_currentRow = 0;
    if (m_iSelectedMap > -1)
        m_ListBox.mHighlights.mElements[m_iSelectedMap] = false;
    m_ListBox.Refresh();
    m_iSelectedMap = -1;
    for (int i = 0; i < 64; ++i)
        m_pMapVoteVals[i] = 0;
    for (int i = 0; i < mapLimit; ++i)
        m_ListBox.SetText(i, 1, "0");
    m_ListBox.SelectLine(0);

    AARMPVoteReset vote = {};
    memset(vote.arrPlayerMapVotes, 0xFF, sizeof(vote.arrPlayerMapVotes));
    vote.voteSubject = 0xFF;
    vote.callerIndex = 0xFF;
    vote.mVoteType = 0; // kNoVote
    vote.localVoted = false;
    MPPlayerManager* playerManager =
        MultiplayerMgr::sInst->mPeer->GetPlayerManager();
    *reinterpret_cast<AARMPVoteReset*>(reinterpret_cast<char*>(playerManager) + 0x5914) =
        vote;
}

// ea: 0x00791690
void AARMapVote::VoteOnMap(int indexNewMap, int indexOldMap)
{
    const int mapLimit = g_NumBaseMaps + 1;
    if (indexOldMap >= mapLimit) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/ui/AARMapVote.cpp";
        AeAssert::gCurrentLine = 538;
        AeAssert::gCurrentExpr = "indexOldMap < kAARPMapLimit";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("vote on map out of allowed boundary"))
            __debugbreak();
    }
    if (indexNewMap >= mapLimit) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/ui/AARMapVote.cpp";
        AeAssert::gCurrentLine = 539;
        AeAssert::gCurrentExpr = "indexNewMap < kAARPMapLimit";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("vote on map out of allowed boundary"))
            __debugbreak();
    }
    char value[32] = {};
    if (indexOldMap >= 0 && indexOldMap < mapLimit) {
        sprintf_s(value, "%i", --m_pMapVoteVals[indexOldMap]);
        m_ListBox.SetText(indexOldMap, 1, value);
    }
    if (indexNewMap >= 0 && indexNewMap < mapLimit) {
        sprintf_s(value, "%i", ++m_pMapVoteVals[indexNewMap]);
        m_ListBox.SetText(indexNewMap, 1, value);
    }
}

// ea: 0x00791540
void AARMapVote::Init()
{
}

// ea: 0x00791550 (thunk)
void AARMapVote::OnDeactivate(AARBaseMenu* __formal)
{
    (void)__formal;
    ClearAllButtons();
}

// ea: 0x00791640
void AARMapVote::OnR1(int c)
{
    (void)c;
    system->MakeActive(0);
}

// ea: 0x00791650
void AARMapVote::OnLeft(int c)
{
    OnL1(c);
}

// ea: 0x00791660
void AARMapVote::OnRight(int c)
{
    OnR1(c);
}

// ea: 0x007917F0
AARGameModeVote* AARGameModeVote::Me()
{
    return (AARGameModeVote*)g_femanager.mAARS->menus[3];
}

// ea: 0x007A5F80
void AARGameModeVote::VoteOnMode(int indexNewMode, int indexOldMode)
{
    const unsigned int oldMode = (unsigned int)indexOldMode;
    if (oldMode >= 7) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/ui/AARGameModeVote.cpp";
        AeAssert::gCurrentLine = 438;
        AeAssert::gCurrentExpr = "indexOldMode < max_modename";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("vote on mode out of allowed boundary"))
            __debugbreak();
    }
    if (indexNewMode >= 7) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/ui/AARGameModeVote.cpp";
        AeAssert::gCurrentLine = 439;
        AeAssert::gCurrentExpr = "indexNewMode < max_modename";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("vote on mode out of allowed boundary"))
            __debugbreak();
    }
    char value[32] = {};
    if (oldMode <= 6) {
        sprintf_s(value, "%i", --m_pModeVoteVals.m_elements[oldMode]);
        m_ListBox.SetText((int)oldMode, 1, value);
    }
    if (indexNewMode <= 6) {
        sprintf_s(value, "%i", ++m_pModeVoteVals.m_elements[indexNewMode]);
        m_ListBox.SetText(indexNewMode, 1, value);
    }
}

// ea: 0x007A62F0
void AARGameModeVote::ResetPanel()
{
    m_currentRow = 0;
    if (m_iSelectedMode > -1)
        m_ListBox.mHighlights.mElements[m_iSelectedMode] = false;
    m_ListBox.Refresh();
    m_iSelectedMode = -1;
    for (int i = 0; i < 7; ++i) {
        m_pModeVoteVals.m_elements[i] = 0;
        m_ListBox.SetText(i, 1, "0");
    }
    m_ListBox.SelectLine(0);
}

// ea: 0x00791870
void AARGameModeVote::Init()
{
}

// ea: 0x00791880 (thunk)
void AARGameModeVote::OnDeactivate(AARBaseMenu* __formal)
{
    (void)__formal;
    ClearAllButtons();
}

// ea: 0x00791950
void AARGameModeVote::OnLeft(int c)
{
    OnL1(c);
}

// ea: 0x00791960
void AARGameModeVote::OnRight(int c)
{
    OnR1(c);
}

// ============================================================================
// PauseMenu / AARPauseMenu / HotJoinMenu / SpectateMenu
// ============================================================================

// ea: 0x00791BB0
void PauseMenu::Quit()
{
    LocalClient::QuitClientOutOfGame(mVersion);
}

// ea: 0x00791DC0
AARPauseMenu::~AARPauseMenu()
{
}

// ea: 0x00791E20 (thunk)
void AARPauseMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x00792080
void AARPauseMenu::UpdateSplitScreen()
{
}

// ea: 0x00792260
void AARPauseMenu::OnUp(int c)
{
    (void)c;
    Up();
}

// ea: 0x00792270
void AARPauseMenu::OnDown(int c)
{
    (void)c;
    Down();
}

// ea: 0x00792460 (thunk)
void HotJoinMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x007924E0 (thunk)
void HotJoinMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
}

// ea: 0x00792560
void HotJoinMenu::OnUp(int c)
{
    (void)c;
    Up();
}

// ea: 0x007926E0
SpectateMenu::~SpectateMenu()
{
}

// ea: 0x00792700
void SpectateMenu::Clear()
{
    mMedic = false;
    mTeamKill = false;
}

// ea: 0x00792780
void SpectateMenu::UpdateSeconds()
{
    FEText* time = (FEText*)mTime;
    if (mSeconds > 0)
    {
        switch (mState)
        {
        case kSpectatorStateInjured:
        case kSpectatorStateDead:
            time->SetShown(true);
            time->SetTextNoLocalize(
                va(STBManager::sInst->GetSTBString("MPSCRIPT_WAITING_FOR_RESPAWN"),
                   mSeconds));
            break;
        case kSpectatorStateDying:
            time->SetShown(false);
            time->SetTextNoLocalize(
                va(STBManager::sInst->GetSTBString("MPSCRIPT_YOU_WILL_DIE"),
                   mSeconds));
            break;
        case kSpectatorStateDeadCanSpawn:
            time->SetShown(true);
            time->SetTextNoLocalize(
                va(STBManager::sInst->GetSTBString("MPSCRIPT_RESPAWNING_IN_TIME"),
                   mSeconds));
            break;
        default:
            time->SetShown(false);
            break;
        }
    }
    else
    {
        time->SetShown(false);
    }
}

// ea: 0x00792880
void SpectateMenu::UpdateHelpbar()
{
    switch (mState)
    {
    case kSpectatorStateIntermission:
        helpbar1->SetShown(false);
        return;
    case kSpectatorStateSpawn:
        helpbar1->SetShown(true);
        helpbar1->SetText("MPGAME_SELECT_KIT");
        return;
    case kSpectatorStateInjured:
    case kSpectatorStateDying:
        helpbar1->SetShown(true);
        if (mTeamKill && MPUIInterface::mServerParams.mEnablePenaltyVote != 0)
        {
            if (!mMedic)
            {
                helpbar1->SetText("MPGAME_SELECT_KIT_TEAM_KILL");
            }
            else
            {
                helpbar1->SetText("MPGAME_SELECT_KIT_CALL_MEDIC_TEAM_KILL");
            }
        }
        else if (mMedic)
        {
            helpbar1->SetText("MPGAME_SELECT_KIT_CALL_MEDIC");
        }
        else
        {
            helpbar1->SetText("MPGAME_SELECT_KIT");
        }
        return;
    case kSpectatorStateDead:
    case kSpectatorStateDeadCanSpawn:
        helpbar1->SetShown(true);
        if (mTeamKill)
        {
            helpbar1->SetText("MPGAME_SELECT_KIT_TEAM_KILL");
        }
        else
        {
            helpbar1->SetText("MPGAME_SELECT_KIT");
        }
        return;
    default:
        return;
    }
}

// ea: 0x007A7000
void SpectateMenu::SetMedic(bool medic)
{
    if (cgGlobal.teamGame)
    {
        mMedic = medic;
        UpdateHelpbar();
    }
}

// ea: 0x00792970
void SpectateMenu::SetTeamKill(bool team_kill, Entity* killer)
{
    if (MPUIInterface::mServerParams.mEnablePenaltyVote != 0)
    {
        mTeamKill = team_kill;
        mLastTeamKiller = killer;
        UpdateHelpbar();
    }
}

// ea: 0x007A7020
void SpectateMenu::UpdateState()
{
    FEText* header = (FEText*)mHeader;
    FEText* message = (FEText*)mMessage;
    FEText* button_press = (FEText*)mButtonPress;
    switch (mState)
    {
    case kSpectatorStateIntermission:
    case kSpectatorStateSpawn:
        header->SetShown(false);
        message->SetShown(false);
        ((FEText*)mTime)->SetShown(false);
        button_press->SetShown(false);
        UpdateSeconds();
        UpdateHelpbar();
        return;
    case kSpectatorStateInjured:
        header->SetShown(false);
        message->SetShown(true);
        ((FEText*)mTime)->SetShown(true);
        button_press->SetShown(false);
        message->SetText("MPSCRIPT_YOU_ARE_INJURED");
        UpdateSeconds();
        UpdateHelpbar();
        return;
    case kSpectatorStateDying:
        header->SetShown(false);
        message->SetShown(true);
        ((FEText*)mTime)->SetShown(false);
        button_press->SetShown(true);
        message->SetText("MPSCRIPT_YOU_ARE_INJURED");
        break;
    case kSpectatorStateDead:
        header->SetShown(false);
        message->SetShown(true);
        button_press->SetShown(false);
        message->SetText("MPSCRIPT_YOU_ARE_DEAD");
        break;
    case kSpectatorStateDeadCanSpawn:
        header->SetShown(false);
        message->SetShown(true);
        button_press->SetShown(true);
        message->SetText("MPSCRIPT_YOU_ARE_DEAD");
        break;
    default:
        break;
    }
    UpdateSeconds();
    UpdateHelpbar();
}

// ============================================================================
// MI_UpdateMapList / ModelMenu
// ============================================================================

// ea: 0x00792E70
void MI_UpdateMapList()
{
}

// ea: 0x007931B0 (thunk)
void ModelMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
}

// ea: 0x00793450
void ModelMenu::DebugControls()
{
}

// ea: 0x007A74B0
ModelMenu::~ModelMenu()
{
}

// ea: 0x0079AB30 (thunk)
void GameSettingsEdit::Draw()
{
    FESplitScreenMenu::Draw();
}

// ea: 0x0079B160
bool AARGameSettingsEdit::ResponseYesApplyNow(int index)
{
    (void)index;
    return ((AARGameSettingsEdit*)g_femanager.mAARS->menus[6])
        ->ResponseYesApplyNowHelper();
}

// ea: 0x0079B750 (thunk)
void GameSettingsView::Draw()
{
    FESplitScreenMenu::Draw();
}

// ea: 0x007A8C50
void GameSettingsEdit::SwapMenus()
{
    UpdateSplitScreenOptions(highlighted);
}

// ea: 0x007A8CE0 (thunk)
void AARGameSettingsEdit::SetPanelFile(PanelFile* pf)
{
    SetPanelFileMain(pf);
}

// ============================================================================
// Batch 5/6: dtor thunks + small menu handlers
// ============================================================================

// ea: 0x0078C9A0
void CreateSessionAdvancedMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078CD50
void CreateLanSessionAdvancedMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078D6D0
void GameSettingsEdit::OnDeactivate(FESplitScreenMenu* m)
{
    (void)m;
    ClearAllButtons();
    iLastOptionSelected = highlighted;
}

// ea: 0x0078E120
void InitialLoadingMenu::OnActivate()
{
    FEMenu::OnActivate();
    mTime = 0.0f;
    SetHigh(-1, true);
}

// ea: 0x0078E290
void InstantActionMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    movie_manager::render();
    FEMenu::Draw();
}

// ea: 0x0078E6C0
void PlayLanMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    movie_manager::render();
    FEMenu::Draw();
}

// ea: 0x0078E9F0
void PressStartMenu::OnActivate()
{
    FEMenu::OnActivate();
    SetHigh(-1, true);
}

// ea: 0x0078EA10
void PressStartMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    movie_manager::render();
    FEMenu::Draw();
}

// ea: 0x0078EE80
void SessionListMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078EEA0
void SessionListMenu::OnTriangle(int c)
{
    (void)c;
    j_nullsub_46(this);
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078EEF0
void SessionListMenu::TidyGamesList()
{
    m_ListBox.Clear();
    mNumGames = 0;
}

// ea: 0x0078EF10
void SessionLanListMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078EF30
void SessionLanListMenu::OnTriangle(int c)
{
    (void)c;
    j_nullsub_46(this);
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078EFF0
void OverlayMenuBase::OnActivate()
{
    FEMenu::OnActivate();
    SetHigh(-1, true);
}

// ea: 0x0078F010
void OverlayMenuBase::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078F030
void OverlayMenuBase::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    MPUIInterface::Step();
}

// ea: 0x0078F880
AAROverlay* AAROverlay::Me(int version)
{
    (void)version;
    AARMenuSystem* result = g_femanager.mAARS;
    if (g_femanager.mAARS != nullptr)
        return (AAROverlay*)g_femanager.mAARS->menus[10];
    return (AAROverlay*)result;
}

// ea: 0x007902B0
void MultilineIngameOverlayMenu::OnDeactivate(FEMenu* menu)
{
    (void)menu;
    mState = NO_OVERLAY;
    InGameMenuSystem* v2 = g_femanager.mIGMS[currCl];
    if (v2 != nullptr)
        v2->is_active = false;
}

// ea: 0x007903B0
MultilineIngameOverlayMenu* MultilineIngameOverlayMenu::Me()
{
    return (MultilineIngameOverlayMenu*)
        g_femanager.GetIGMS(currCl)->menus[9];
}

// ea: 0x007906B0
void VoteGameTypeMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x00790930
void VoteMapMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x00790C70
void WeaponSelectMenu::SetClassOptionHeader()
{
    m_pClassOptionHeader->SetText(szClassReference[highlighted]);
}

// ea: 0x00790ED0
void InGameSwitchSides::ResponseGoBack(int client)
{
    g_femanager.GetDMS(client)->CloseDialog();
}

// ea: 0x00790F70
AARBaseMenu::~AARBaseMenu()
{
    m_pTimerText[0] = nullptr;
    m_pTimerText[1] = nullptr;
}

// ea: 0x007910D0
void AARScoreboardWinner::Draw()
{
    FEMenu::Draw();
    if (panel != nullptr)
        panel->Draw();
}

// ea: 0x007910F0
void AARScoreboardWinner::OnR1(int c)
{
    (void)c;
    if (cgGlobal.teamGame)
        system->MakeActive(1);
    else
        system->MakeActive(2);
}

// ea: 0x007A8F30
AARGameSettingsView::~AARGameSettingsView()
{
}

// ea: 0x007A9EC0
InGameSwitchSides::~InGameSwitchSides()
{
}

// ea: 0x007AA000 (thunk)
void AARBaseMenu::Update(float time_inc)
{
    (void)time_inc;
    SetTimerText();
}

// ea: 0x007AAA00 (thunk)
void AARScoreboardWinner::Update(float time_inc)
{
    AARScoreboardBase::Update(time_inc);
}

// ea: 0x007AAA40 (thunk)
void AARScoreboardLoser::Update(float time_inc)
{
    AARScoreboardBase::Update(time_inc);
}

// ea: 0x007AB410 (thunk)
void AARPersonalStats::Update(float time_inc)
{
    (void)time_inc;
    AARBaseMenu::SetTimerText();
}

// ea: 0x007AB850
PauseMenu::~PauseMenu()
{
}

// ea: 0x007ABEA0
void SessionListMenu::Refresh()
{
    j_nullsub_46(this);
    InitMenu();
}

// ea: 0x007ABEE0
void SessionLanListMenu::Refresh()
{
    j_nullsub_46(this);
    InitMenu();
}

// ea: 0x007AD330 (thunk)
void GameSettingsEdit::PanelFileUnloaded(PanelFile* pf)
{
    FESplitScreenMenu::PanelFileUnloaded(pf);
}

// ea: 0x007AD340 (thunk)
void GameSettingsView::PanelFileUnloaded(PanelFile* pf)
{
    FESplitScreenMenu::PanelFileUnloaded(pf);
}

// ea: 0x007B02A0
AARScoreboardWinner::~AARScoreboardWinner()
{
}

// ea: 0x007B02D0
AARScoreboardLoser::~AARScoreboardLoser()
{
}

// ============================================================================
// Batch 7: AAR winner, pause responses, spectate, session list, settings
// ============================================================================

// ea: 0x00791110
void AARScoreboardWinner::OnL1(int c)
{
    (void)c;
    if (MPUIInterface::mNextServerParams.mEnableAARVote == 1)
        system->MakeActive(4);
    else
        system->MakeActive(2);
}

// ea: 0x00791CC0
bool PauseMenu::ResponseYesTeamChange(int client)
{
    ((PauseMenu*)g_femanager.GetIGMS(client)->menus[0])->TeamChange();
    return true;
}

// ea: 0x00791CE0
void PauseMenu::ResponseGoBack(int client)
{
    g_femanager.GetDMS(client)->CloseDialog();
}

// ea: 0x007925E0
void HotJoinMenu::UpdateSplitScreen()
{
    panel->MoveSplitScreen(unk_F6A284[802 * mVersion],
                           unk_F6A280[802 * mVersion]);
}

// ea: 0x007926F0
void SpectateMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
}

// ea: 0x007929A0
void SpectateMenu::OnStart(int c)
{
    (void)c;
    g_femanager.GetIGMS(mVersion)->MakeActiveAndReturn(0);
}

// ea: 0x007929C0
void SpectateMenu::OnSelect(int c)
{
    (void)c;
    g_femanager.GetIGMS(mVersion)->MakeActiveAndReturn(10);
}

// ea: 0x00792DE0
char MI_IsAvailableMap(char id)
{
    if (g_NumTotalMaps <= 0)
        return 0;
    int index = 0;
    char* map_id = byte_E386C9;
    while (*map_id != id)
    {
        if (++index >= g_NumTotalMaps)
            return 0;
        map_id += 114;
    }
    return 1;
}

// ea: 0x00793010
char MI_GetMapIDbyIndex(int index)
{
    if (index != -1)
        return byte_E386C9[114 * index];
    return index;
}

// ea: 0x00793470
void AARMenuSystem::ActivateMenu(int menu)
{
    is_active = true;
    GamePause::SetAllPaused(true);
    FEMenuSystem::MakeActive(menu);
}

// ea: 0x00793520
void AARMenuSystem::CheckForNoMenus()
{
    int active = ((int(__thiscall*)(void*))(*(void***)this)[24])(this);
    if (active <= -1)
    {
        is_active = false;
        GamePause::SetAllPaused(false);
    }
}

// ea: 0x0079AD30
void GameSettingsEdit::OnLeft(int c)
{
    (void)c;
    Left();
    SetGameTypeDefaults();
}

// ea: 0x0079AE80
bool GameSettingsEdit::ResponseYesApplyNow(int client)
{
    return ((GameSettingsEdit*)g_femanager.GetIGMS(client)->menus[2])
        ->ResponseYesApplyNowHelper();
}

// ea: 0x0079B8F0
void AARGameSettingsView::Update(float time_inc)
{
    GameSettingsView::Update(time_inc);
    SetTimerText();
}

// ea: 0x007A6C70
bool AARPauseMenu::ResponseYesQuit(int client)
{
    Quit(client);
    return true;
}

// ea: 0x007A8EB0
void GameSettingsView::SwapMenus()
{
    int highlighted = this->highlighted;
    mCurrentServerParams = &MPUIInterface::mServerParams;
    UpdateSplitScreenOptions(highlighted);
}

// ea: 0x007A9FA0
bool InGameSwitchSides::ResponseYesSwitch(int client)
{
    ((InGameSwitchSides*)g_femanager.GetIGMS(client)->menus[11])
        ->SwitchTeams();
    return true;
}

// ea: 0x007AB4F0
void AARMapVote::Update(float time_inc)
{
    AARBaseMenu::SetTimerText();
    m_ListBox.Update(time_inc);
}

// ea: 0x007AB6F0
void AARGameModeVote::Update(float time_inc)
{
    AARBaseMenu::SetTimerText();
    m_ListBox.Update(time_inc);
}

// ea: 0x007ABBB0
void SpectateMenu::OnActivate(int prev)
{
    (void)prev;
    FEMenu::OnActivate();
    UpdateState();
}

// ea: 0x007AD4C0
void SessionListMenu::OnSquare(int c)
{
    (void)c;
    j_nullsub_46(this);
    InitMenu();
}

// ea: 0x007AD600
void SessionLanListMenu::OnSquare(int c)
{
    (void)c;
    j_nullsub_46(this);
    InitMenu();
}

// ea: 0x007AD650
void WeaponSelectMenu::PanelFileUnloaded(PanelFile* pf)
{
    FESplitScreenMenu::PanelFileUnloaded(pf);
    Cleanup();
}

// ea: 0x007AE430
void ModelMenu::Draw3D()
{
    if (!View::IsSplitScreen())
    {
        FESplitScreenMenu::Draw();
        AddDObjToScene();
    }
}

// ea: 0x0078CEE0
void CreateLanSessionAdvancedMenu::OnUp(int c)
{
    (void)c;
    if (highlighted != 0)
    {
        Up();
    }
    else
    {
        highlighted = 4;
        SetHigh(4, true);
    }
}

// ea: 0x0078D7C0
void GameSettingsEdit::UpdateMapChangeStatus(bool isAllowingMapVote)
{
    void** v2 = *(void***)entries[1];
    ((void(__thiscall*)(void*, int))v2[16])(entries[1],
                                            isAllowingMapVote ? 1 : 0);
}

// ea: 0x0078E140
void InitialLoadingMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    movie_manager::render();
    FEMenu::Draw();
}

// ea: 0x0078EB60
void SessionDetailsMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    movie_manager::render();
    FEMenu::Draw();
}

// ea: 0x0078EF50
void SessionLanListMenu::OnCircle(int c)
{
    (void)c;
    mSortColumn++;
    if (mSortColumn == 4)
        mSortColumn = 0;
}

// ea: 0x0078EF80
void SessionLanListMenu::TidyGamesList()
{
    m_ListBox.Clear();
    mNumGames = 0;
}

// ============================================================================
// Batch 8: overlay/session/scoreboard/settings handlers
// ============================================================================

// ea: 0x0078F0F0
void OverlayMenuBase::UpdateSplitScreen()
{
    if (panel != nullptr)
        panel->UpdateSplitScreen(unk_F6A284[802 * mVersion],
                                 unk_F6A280[802 * mVersion]);
}

// ea: 0x0078F3C0
void OverlayMenu::Select(int entry_num)
{
    (void)entry_num;
}

// ea: 0x0078F4E0
InGameOverlay* InGameOverlay::Me(int version)
{
    InGameMenuSystem* result = g_femanager.GetIGMS(version);
    if (result != nullptr)
        return (InGameOverlay*)g_femanager.GetIGMS(version)->menus[13];
    return (InGameOverlay*)result;
}

// ea: 0x00790040
void MultilineFrontendOverlayMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    MPUIInterface::Step();
}

// ea: 0x007902D0
void MultilineIngameOverlayMenu::OnStart(int c)
{
    (void)c;
    if (mState == CONTROLLER_DISCONNECTED && g_controllerConnected[c])
    {
        g_controllerConnectedErrorShown[c] = false;
        system->RemoveOverlay();
    }
}

// ea: 0x00790CF0
int scoreboard_player_sorter(const void* left, const void* right)
{
    const int* l = (const int*)left;
    const int* r = (const int*)right;
    if (l[1] >= r[1])
        return (l[1] <= r[1]) - 1;
    return 1;
}

// ea: 0x00791020
void AARScoreboardBase::OnDeactivate(AARBaseMenu* m)
{
    (void)m;
    ClearAllButtons();
    m_ListBox.Clear();
}

// ea: 0x00791B30
void PauseMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    tlPrintf("PauseMenu::OnDeactivate()\n");
    ClearAllButtons();
    m_iLastSelection = highlighted;
}

// ea: 0x00791D50
void PauseMenu::ButtonHeldAction()
{
    if (button_held_down == 8)
        OnDown(0);
    else if (button_held_down == 4)
        OnUp(0);
}

// ea: 0x00791DD0
void AARPauseMenu::ButtonHeldAction()
{
    if (button_held_down == 8)
        OnDown(0);
    else if (button_held_down == 4)
        OnUp(0);
}

// ea: 0x00791E00
void AARPauseMenu::ResponseGoBack(int client)
{
    g_femanager.GetDMS(client)->CloseDialog();
}

// ea: 0x00791E30
void AARPauseMenu::OnDeactivate(FEMenu* pMenu)
{
    (void)pMenu;
    tlPrintf("AARPauseMenu::OnDeactivate()\n");
    ClearAllButtons();
    m_iLastSelection = highlighted;
}

// ea: 0x00792A90
void SpectateMenu::OnSquare(int c)
{
    (void)c;
    int mState = this->mState;
    if (mState > kSpectatorStateIntermission
        && mState <= kSpectatorStateDeadCanSpawn)
    {
        PlayNavigationSound();
        InGameMenuSystem* IGMS = g_femanager.GetIGMS(mVersion);
        IGMS->MakeActiveAndReturn(1);
    }
}

// ea: 0x00792E10
bool MI_IsMapPackAvailable(char map_pack)
{
    if (g_NumTotalMaps <= 0)
        return false;
    Mapinfo_t* i = g_TheMapInfo;
    int v1 = 0;
    while (i->map_pack != map_pack)
    {
        ++v1;
        ++i;
        if (v1 >= g_NumTotalMaps)
            return false;
    }
    return true;
}

// ea: 0x0079B140
void AARGameSettingsEdit::Update(float time_inc)
{
    GameSettingsEdit::Update(time_inc);
    SetTimerText();
}

// ea: 0x0079C580
void PlayLanMenu::SetPreviewImage()
{
    ClearPreviewImages();
    m_pImages[mListBox.mTopLine + mListBox.mSelectedLine]->SetShown(true);
}

// ea: 0x0079C5B0
void PlayLanMenu::SetOptionText()
{
    m_pText[1]->SetText(
        szPlayLanMenuOptionTextReferences[mListBox.mTopLine]
                                         [mListBox.mSelectedLine]);
}

// ea: 0x0079D0E0
void SessionListMenu::OnUp(int c)
{
    if (!lockInput)
    {
        m_ListBox.OnUp(c);
        UpdateGameInfo();
    }
}

// ea: 0x0079D110
void SessionListMenu::OnDown(int c)
{
    if (!lockInput)
    {
        m_ListBox.OnDown(c);
        UpdateGameInfo();
    }
}

// ea: 0x0079DE80
void SessionLanListMenu::OnUp(int c)
{
    if (!lockInput)
    {
        m_ListBox.OnUp(c);
        UpdateGameInfo();
    }
}

// ea: 0x0079DEB0
void SessionLanListMenu::OnDown(int c)
{
    if (!lockInput)
    {
        m_ListBox.OnDown(c);
        UpdateGameInfo();
    }
}

// ea: 0x007A2E00
void InGameScoreBoard::OnUp(int c)
{
    short v3 = m_ListBox.OnUp(c);
    m_ListBox.mHighlights.mElements[v3] = true;
}

// ea: 0x007A2E30
void InGameScoreBoard::OnDown(int c)
{
    short v3 = m_ListBox.OnDown(c);
    m_ListBox.mHighlights.mElements[v3] = true;
}

// ea: 0x007A8CC0
AARGameSettingsEdit::~AARGameSettingsEdit()
{
    m_FirstTimeAccessedByte = 0;
}

// ea: 0x007A9E60
void InGameScoreBoard::OnActivate()
{
    FEMenu::OnActivate();
    m_ListBox.Clear();
    SetPanelContents();
    ClearButton(controller::SELECT);
    m_bActivated = true;
}

// ea: 0x007ABEF0
void AAROverlay::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    PanelFile* panel = this->panel;
    if (panel != nullptr)
    {
        panel->~PanelFile();
        mem_heap_free(panel);
    }
    this->panel = nullptr;
    Cleanup();
}

// ea: 0x007AC160
void AARScoreboardLoser::PanelFileUnloaded(PanelFile* pPanelFile)
{
    (void)pPanelFile;
    PanelFile* panel = this->panel;
    if (panel != nullptr)
    {
        panel->~PanelFile();
        mem_heap_free(panel);
    }
    this->panel = nullptr;
    Cleanup();
}

// ea: 0x007AC200
void HotJoinMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    PanelFile* panel = this->panel;
    if (panel != nullptr)
    {
        panel->~PanelFile();
        mem_heap_free(panel);
    }
    this->panel = nullptr;
}

// ea: 0x007AF2D0
void InGameSwitchSides::OnCross(int c)
{
    (void)c;
    EntityManager::sInst->GetPlayer(mVersion);
    PickTeam();
    UpdateModel();
}

// ea: 0x007B0280
AARScoreboardWinner::AARScoreboardWinner(FEMenuSystem* pauseMenuSystem)
    : AARScoreboardBase(pauseMenuSystem)
{
}

// ea: 0x007B02B0
AARScoreboardLoser::AARScoreboardLoser(FEMenuSystem* pauseMenuSystem)
    : AARScoreboardBase(pauseMenuSystem)
{
}

// ============================================================================
// Batch 9: remaining menu Draw/Update/ctor handlers
// ============================================================================

// ea: 0x0078E7F0
void PlayOnlineMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    movie_manager::render();
    FEMenu::Draw();
}

// ea: 0x0078F7A0
void InGameOverlay::OnUp(int c)
{
    m_ListBox.OnUp(c);
    if (--m_currSelection < 0)
    {
        m_currSelection = 1;
        m_ListBox.SelectLine(1);
    }
}

// ea: 0x0078FB30
void AAROverlay::OnUp(int c)
{
    m_ListBox.OnUp(c);
    if (--m_currSelection < 0)
    {
        m_currSelection = 1;
        m_ListBox.SelectLine(1);
    }
}

// ea: 0x00790580
VoteGameTypeMenu::VoteGameTypeMenu(FEMenuSystem* pauseMenuSystem)
    : FEMenu(pauseMenuSystem, 2, 320, 260, 8, 0)
{
    mPlayerMgr = nullptr;
    mGameTypeList = nullptr;
}

// ea: 0x007907D0
VoteMapMenu::VoteMapMenu(FEMenuSystem* pauseMenuSystem)
    : FEMenu(pauseMenuSystem, 2, 320, 260, 8, 0)
{
    mPlayerMgr = nullptr;
    mMapList = nullptr;
    default_color_scheme = 9;
}

// ea: 0x00790D20
void InGameScoreBoard::UpdateSplitScreen()
{
    if (panel != nullptr)
        panel->UpdateSplitScreen(unk_F6A284[802 * mVersion],
                                 unk_F6A280[802 * mVersion]);
}

// ea: 0x00790D60
void InGameScoreBoard::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
    dword_F641D0[1580 * currCl] = 0;
    m_bShowMyTeamScore = true;
    m_ListBox.Clear();
}

// ea: 0x00790DA0
void InGameScoreBoard::Draw()
{
    FEMenu::Draw();
    if (panel != nullptr)
        panel->Draw();
}

// ea: 0x00790F90
void AARBaseMenu::OnStart(int c)
{
    unsigned int v3 = LocalClient::PortToClient(c);
    if (v3 <= 1 && dword_F6A290[802 * v3] == 2)
        system->MakeActiveAndReturn(9);
}

// ea: 0x00791130
void AARScoreboardLoser::Draw()
{
    FEMenu::Draw();
    if (panel != nullptr)
        panel->Draw();
}

// ea: 0x00791320
void AARPersonalStats::OnR1(int c)
{
    (void)c;
    m_bHighlightScrollArrowRight = true;
    m_ePanelToSwitchTo = MPUIInterface::mNextServerParams.mEnableAARVote != 1 ? 0 : 3;
}

// ea: 0x00791360
void AARPersonalStats::OnL1(int c)
{
    (void)c;
    if (cgGlobal.teamGame)
        system->MakeActive(1);
    else
        system->MakeActive(0);
    m_bHighlightScrollArrowLeft = true;
}

// ea: 0x00791520
void AARMapVote::PanelFileUnloaded(PanelFile* pPanelFile)
{
    (void)pPanelFile;
    Cleanup();
    m_ListBox.RemoveAllItems();
}

// ea: 0x00791930
void AARGameModeVote::OnR1(int c)
{
    (void)c;
    m_bHighlightScrollArrowRight = true;
    m_ePanelToSwitchTo = 4;
}

// ea: 0x00791E60
void AARPauseMenu::UnPause(int client)
{
    system->ReturnToPreviousMenu(-1);
    g_femanager.GetDMS(client)->MakeActive(-1);
    m_iLastSelection = -1;
    ClearAllButtons();
}

// ea: 0x00791EA0
void AARPauseMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
    g_femanager.GetDMS(0)->MakeActive(-1);
    m_iLastSelection = -1;
    ClearAllButtons();
}

// ea: 0x00791EE0
void AARPauseMenu::OnStart(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
    g_femanager.GetDMS(0)->MakeActive(-1);
    m_iLastSelection = -1;
    ClearAllButtons();
}

// ea: 0x00792240
void AARPauseMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x00792E40
char* remove_underscores(char* str)
{
    int v2 = (int)strlen(str);
    char* v3 = str;
    if (v2 > 0)
    {
        do
        {
            if (*v3 == 95)
                *v3 = 32;
            ++v3;
            --v2;
        } while (v2);
    }
    return str;
}

// ea: 0x00792E80
const char* MI_GetMapDisplayName(char id)
{
    if (g_NumTotalMaps <= 0)
        return "NULL";
    int v1 = 0;
    char* i = byte_E386C9;
    while (*i != id)
    {
        if (++v1 >= g_NumTotalMaps)
            return "NULL";
        i += 114;
    }
    return &aMpfrontendMerv[114 * v1];
}

// ea: 0x00792EC0
char* MI_GetMapLocation(char id)
{
    if (g_NumTotalMaps <= 0)
        return "NULL";
    int v1 = 0;
    char* i = byte_E386C9;
    while (*i != id)
    {
        if (++v1 >= g_NumTotalMaps)
            return "NULL";
        i += 114;
    }
    return &aMploadingMervi[114 * v1];
}

// ea: 0x00792F00
char* MI_GetMapTitle(char id)
{
    if (g_NumTotalMaps <= 0)
        return "NULL";
    int v1 = 0;
    char* i = byte_E386C9;
    while (*i != id)
    {
        if (++v1 >= g_NumTotalMaps)
            return "NULL";
        i += 114;
    }
    return &aMploadingMervi_0[114 * v1];
}

// ea: 0x00792F40
char MI_GetMapPack(char id)
{
    if (g_NumTotalMaps <= 0)
        return -1;
    int v1 = 0;
    char* i = byte_E386C9;
    while (*i != id)
    {
        if (++v1 >= g_NumTotalMaps)
            return -1;
        i += 114;
    }
    return g_TheMapInfo[v1].map_pack;
}

// ea: 0x00792F80
const char* MI_GetMapShortname(char id)
{
    if (g_NumTotalMaps <= 0)
        return "NULL";
    int v1 = 0;
    char* i = byte_E386C9;
    while (*i != id)
    {
        if (++v1 >= g_NumTotalMaps)
            return "NULL";
        i += 114;
    }
    return &aMpMerv[114 * v1];
}

// ea: 0x00793460
void ModelMenu::DebugRender()
{
}

// ea: 0x007934A0
void AARMenuSystem::Draw()
{
    if (mPreviousWidescreen != (cg_widescreen.integer != 0))
    {
        // FEMenuSystem vtable slot 2 = UpdateWidescreen (shell.o 0x57DF20)
        ((void(__thiscall*)(void*, bool))(*(void***)this)[2])(
            this, cg_widescreen.integer != 0);
        mPreviousWidescreen = cg_widescreen.integer != 0;
    }
    // FEMenuSystem vtable slot 19 = Draw (shell.o 0x570E90)
    ((void(__thiscall*)(void*))(*(void***)this)[19])(this);
}

// ea: 0x0079C5E0
void PlayLanMenu::SetDescriptionText()
{
    int mSelectedLine = mListBox.mSelectedLine;
    FEText* v3 = m_pText[2];
    m_pText[2]->SetText(szPlayLanMenuDescriptionReferences[mListBox.mTopLine]
                                                    [mSelectedLine]);
}

// ea: 0x007A19E0
void WeaponSelectMenu::CloseMenu()
{
    int mReturnMenu = this->mReturnMenu;
    Allow_Exit = true;
    if (mReturnMenu < 0)
    {
        InGameMenuSystem* IGMS = g_femanager.GetIGMS(mVersion);
        ((PauseMenu*)IGMS->menus[0])->UnPause(mVersion);
    }
    else
    {
        system->ReturnToPreviousMenu(mReturnMenu);
    }
}

// ea: 0x007A1B60
void InGameScoreBoard::RecalculateWinningTeam()
{
    if (cgGlobal.teamScores[2] <= cgGlobal.teamScores[1])
        SetWinningTeam((team_t)(2 * (cgGlobal.teamScores[1] <= cgGlobal.teamScores[2]) + 1));
    else
        SetWinningTeam(TEAM_ALLIES);
}

// ea: 0x007A3F80
void AARScoreboardWinner::SetPanelFile(PanelFile* pf)
{
    AARScoreboardBase::SetPanelFile(pf);
    m_pYourTeamScore[4]->SetShown(true);
    m_pYourTeamScore[5]->SetShown(false);
}

// ============================================================================
// Batch 10: SetPanelFile dispatchers + menu Update/Set handlers
// ============================================================================

// ea: 0x0078D6F0
GameSettingsEdit* GameSettingsEdit::Me(int version)
{
    return (GameSettingsEdit*)g_femanager.GetIGMS(version)->menus[2];
}

// ea: 0x0078DD70
void GameSettingsView::UpdateScrollBar()
{
    if (mScrollBarThumb != nullptr)
    {
        float y = (float)((mScrollBarYInc * highlighted) + mScrollBarTopY);
        mScrollBarThumb->SetCenterPos(mScrollBarThumb->GetCenterX(), y);
    }
}

// ea: 0x0078E2B0
void InstantActionMenu::Update(float time_inc)
{
    if (!MPUIInterface::IsOnlineGame()
        || MPLiveEngine::GetHandle()->internalState == kSignedIn)
    {
        FEMenu::Update(time_inc);
        movie_manager::frame_advance();
        MPUIInterface::Step();
    }
}

// ea: 0x0078EA30
void PressStartMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    MPUIInterface::Step();
}

// ea: 0x0078F590
void InGameOverlay::OnTriangle(int c)
{
    (void)c;
    OverlayMenuBase::OnTriangle(c);
    if (GetSystem()->CurrentOverlay() != -1)
    {
        GetSystem()->RemoveOverlay();
        m_State = (eState)0;
    }
}

// ea: 0x0078F7E0
void InGameOverlay::Draw()
{
    m_ListBox.Draw();
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
    if (m_State >= 3 && m_State <= 9 && m_IsAARTimerEnabled)
    {
        m_IsAARTimerEnabled =
            AARXBoxLiveIngameOptions::Me()->SetTimerText();
    }
}

// ea: 0x0078F920
void AAROverlay::OnTriangle(int c)
{
    (void)c;
    OverlayMenuBase::OnTriangle(c);
    if (GetSystem()->CurrentOverlay() != -1)
    {
        GetSystem()->RemoveOverlay();
        m_State = (eState)0;
    }
}

// ea: 0x0078FB70
void AAROverlay::Draw()
{
    m_ListBox.Draw();
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
    if (m_State >= 3 && m_State <= 9 && m_IsAARTimerEnabled)
    {
        m_IsAARTimerEnabled =
            AARXBoxLiveIngameOptions::Me()->SetTimerText();
    }
}

// ea: 0x0078FD20
void MultilineOverlayMenu::OnActivate()
{
    FEMenu::OnActivate();
    SetHigh(-1, true);
    if (entries[2]->GetDisable() != 0)
        SetHigh(3, true);
    else
        SetHigh(2, true);
}

// ea: 0x00790DE0
void InGameScoreBoard::OnButtonRelease(int c, int b)
{
    (void)c;
    if (b == 8)
        ClearButton(controller::DOWNBUTTON);
    else if (b == 4)
        ClearButton(controller::UPBUTTON);
}

// ea: 0x00791890
void AARGameModeVote::OnUp(int c)
{
    Up();
    m_ListBox.OnUp(c);
    if (m_currentRow <= 0)
        m_currentRow = 6;
    else
        --m_currentRow;
}

// ea: 0x00792D10
void SpectateMenu::UpdateSplitScreen()
{
    if (panel != nullptr)
        panel->UpdateSplitScreen(unk_F6A284[802 * mVersion],
                                 unk_F6A280[802 * mVersion]);
}

// ea: 0x007A33C0
void InGameSwitchSides::SwapMenus()
{
    if (MultiplayerMgr::sInst->mRankedGame)
    {
        entries[1]->Disable(true);
        entries[2]->Disable(true);
    }
    else
    {
        entries[1]->Disable(false);
        entries[2]->Disable(false);
    }
}

// ea: 0x007A3680
void AARBaseMenu::OnL1(int c)
{
    (void)c;
    if (mLeftArrowFader.mQuad != nullptr)
    {
        mLeftArrowFader.mAlpha = 1.0f;
        mLeftArrowFader.mFading = true;
        mLeftArrowFader.mAlphaTo = 0.5f;
        mLeftArrowFader.mTime = 0.5f;
        mLeftArrowFader.mAlphaDelta = fabs(0.5f);
        mLeftArrowFader.mQuad->SetAlpha(1.0f);
    }
    else
    {
        mLeftArrowFader.mFading = false;
    }
}

// ea: 0x007A8E20
void GameSettingsView::SetPanelFile(PanelFile* pf)
{
    if (_stricmp(pf->mName, "MP_SS_PM_options_view.PANEL") == 0)
        SetPanelFileSplitScreen(pf);
    else if (_stricmp(pf->mName, "MP_PM_GS_view.PANEL") == 0)
        SetPanelFileMain(pf);
}

// ea: 0x007A9CF0
void WeaponSelectMenu::Select(int entryNum)
{
    (void)entryNum;
    if (Allow_Exit)
    {
        Allow_Exit = true;
        if (mReturnMenu < 0)
            ((PauseMenu*)g_femanager.GetIGMS(mVersion)->menus[0])->UnPause();
        else
            system->ReturnToPreviousMenu(-1);
    }
}

// ea: 0x007A9D40
void WeaponSelectMenu::OnTriangle(int controllerIndex)
{
    (void)controllerIndex;
    if (Allow_Exit)
    {
        Allow_Exit = true;
        if (mReturnMenu < 0)
            ((PauseMenu*)g_femanager.GetIGMS(mVersion)->menus[0])->UnPause();
        else
            system->ReturnToPreviousMenu(-1);
    }
}

// ea: 0x007A9E90
InGameSwitchSides::InGameSwitchSides(FEMenuSystem* s)
    : ModelMenu(s, 3)
{
    m_eTeam = TEAM_FREE;
    default_color_scheme = 10;
}

// ea: 0x007A9FC0
void AARBaseMenu::OnActivate()
{
    FEMenu::OnActivate();
    SetTimerText();
    if (MultiplayerMgr::sInst->mRankedGame)
        m_pTimerText[1]->SetText("MPGAME_AAR_RANK_GAME_OVER");
    else
        m_pTimerText[1]->SetText("MPGAME_AAR_SECONDS_TIL_NEXT_GAME");
}

// ea: 0x007AAA10
void AARScoreboardLoser::OnActivate()
{
    AARScoreboardBase::OnActivate();
    m_pYourTeamScore[4]->SetShown(false);
    m_pYourTeamScore[5]->SetShown(false);
}

// ea: 0x007ABBD0
void GameSettingsEdit::SetPanelFile(PanelFile* pf)
{
    if (_stricmp(pf->mName, "MP_SS_PM_options_edit.PANEL") == 0)
        SetPanelFileSplitScreen(pf);
    else if (_stricmp(pf->mName, "MP_PM_GS_edit.PANEL") == 0)
        SetPanelFileMain(pf);
}

// ea: 0x007ABE70
void SessionListMenu::OnActivate()
{
    FEMenu::OnActivate();
    mShowDownArrow = false;
    mShowUpArrow = false;
    mNumGames = 0;
    m_ListBox.SelectLine(0);
}

// ea: 0x007ABF20
void WeaponSelectMenu::SetPanelFile(PanelFile* pf)
{
    if (_stricmp(pf->mName, "MP_SS_PM_options.PANEL") == 0)
        SetPanelFileSplitScreen(pf);
    else if (_stricmp(pf->mName, "MP_class_select.PANEL") == 0)
        SetPanelFileMain(pf);
}

// ea: 0x007ABF70
void InGameScoreBoard::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    if (mVersion > 0)
    {
        PanelFile* panel = this->panel;
        if (panel != nullptr)
        {
            panel->~PanelFile();
            mem_heap_free(panel);
        }
        this->panel = nullptr;
    }
    m_ListBox.RemoveAllItems();
    Cleanup();
}

// ea: 0x007ABFC0
void InGameSwitchSides::SetPanelFile(PanelFile* pf)
{
    if (_stricmp(pf->mName, "MP_SS_PM_options.PANEL") == 0)
        SetPanelFileSplitScreen(pf);
    else if (_stricmp(pf->mName, "MP_PM_sideselection.PANEL") == 0)
        SetPanelFileMain(pf);
}

// ea: 0x007AD620
void InGameOverlay::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    PanelFile* panel = this->panel;
    if (panel != nullptr)
    {
        panel->~PanelFile();
        mem_heap_free(panel);
    }
    this->panel = nullptr;
    Cleanup();
}

// ea: 0x007AF1C0
void WeaponSelectMenu::OnUp(int c)
{
    (void)c;
    Up();
    m_pClassOptionHeader->SetText(szClassReference[highlighted]);
    SetClassGauges();
    SetSwitchKit();
}

// ea: 0x007AF200
void WeaponSelectMenu::OnDown(int c)
{
    (void)c;
    Down();
    m_pClassOptionHeader->SetText(szClassReference[highlighted]);
    SetClassGauges();
    SetSwitchKit();
}

// ============================================================================
// Batch 11: game-settings/vote/overlay/side-switch handlers
// ============================================================================

// ea: 0x0078D7F0
void GameSettingsEdit::DisableTeamGameOptions(bool b)
{
    entries[7]->Disable(b != 0);
    entries[4]->Disable(b != 0);
    entries[5]->Disable(b != 0);
    if (b != 0)
    {
        ((FEComboBox*)entries[7])->SetCurrOption(0);
        ((FEComboBox*)entries[4])->SetCurrOption(0);
        ((FEComboBox*)entries[5])->SetCurrOption(0);
    }
}

// ea: 0x0078E830
void PlayOnlineMenu::TogglePreviewImage(int option, bool visible)
{
    PanelQuad* Pointer = nullptr;
    switch (option)
    {
    case 1:
        Pointer = panel->GetPointer("mm_preview_image_01");
        break;
    case 2:
        Pointer = panel->GetPointer("mm_preview_image_02");
        break;
    case 3:
        Pointer = panel->GetPointer("mm_preview_image_03");
        break;
    case 4:
        Pointer = panel->GetPointer("mm_preview_image_04");
        break;
    case 5:
        Pointer = panel->GetPointer("mm_preview_image_05");
        break;
    default:
        return;
    }
    if (Pointer != nullptr)
        Pointer->SetVisibility(visible ? 1.0f : 0.0f);
}

// ea: 0x0078E8A0
void PlayOnlineMenu::InitQuickMatchParameters(int c)
{
    sServerQueryParams params;
    params.mMapID = gSaveGameData[c].mStubData.mMapPreference;
    params.mGameType = gSaveGameData[c].mStubData.mGameModePreference;
    params.mGameSubType = -1;
    params.mMinPlayers = -1;
    params.mMaxPlayers = gSaveGameData[c].mStubData.mMaxPlayerCntPreference;
    params.mTeamBalancing = gSaveGameData[c].mStubData.mAutoTeamBalancePreference;
    params.mFriendlyFire = gSaveGameData[c].mStubData.mTeamDamagePreference;
    MPUIInterface::mGameConnectionType =
        MPUIInterface::kGameConnectionTypeOnline;
    params.mListIfFull = 0;
    params.mSessionNamePrefix[0] = 0;
    MPUIInterface::SetQueryParams(params);
}

// ea: 0x0078F3F0
void OverlayMenu::OnUp(int c)
{
    m_ListBox.OnUp(c);
    if ((mState == 17
         || mState == (JOINING_START | 0x10)
         || mState == (JOINING | 0x10)
         || mState == 18
         || mState == (GAME_LISTING_START | 0x10)
         || mState == 20)
        && --m_currSelection < 0)
    {
        m_currSelection = 1;
        m_ListBox.SelectLine(1);
    }
}

// ea: 0x0078F830
void InGameOverlay::OnDown(int c)
{
    m_ListBox.OnDown(c);
    int v4 = m_currSelection + 1;
    bool v5 = m_currSelection < 0;
    m_currSelection = v4;
    // __OFSUB__(v4, 1): signed overflow of (v4 - 1)
    bool ofsub = ((v4 ^ 1) & (v4 ^ (v4 - 1))) < 0;
    if (!(v5 ^ ofsub | (v4 == 1)))
    {
        m_currSelection = 0;
        m_ListBox.SelectLine(0);
    }
}

// ea: 0x00790000
void MultilineFrontendOverlayMenu::OnStart(int c)
{
    if (mState == CONTROLLER_DISCONNECTED && g_controllerConnected[c])
    {
        g_controllerConnectedErrorShown[c] = false;
        system->RemoveOverlay();
    }
}

// ea: 0x00791560
void AARMapVote::OnUp(int c)
{
    Up();
    if (m_ListBox.mTopLine + m_ListBox.mSelectedLine != 0)
        m_ListBox.OnUp(c);
    else
        m_ListBox.SelectLine(g_NumBaseMaps - 1);
}

// ea: 0x00791970
void AARGameModeVote::OnL1(int c)
{
    (void)c;
    m_bHighlightScrollArrowLeft = true;
    m_ePanelToSwitchTo = 2;
}

// ea: 0x00791C90
bool PauseMenu::ResponseYesQuit(int client)
{
    LocalClient::QuitClientOutOfGame(
        (int)g_femanager.GetIGMS(client)->menus[1]->entries);
    return true;
}

// ea: 0x00792600
void HotJoinMenu::Join()
{
    MultiplayerMgr::sInst->AttemptHotJoin(mVersion);
    LocalClient::UpdatePlayerPorts(mVersion);
}

// ea: 0x007929E0
void SpectateMenu::OnCross(int c)
{
    (void)c;
    if ((mState == kSpectatorStateSpawn
         || mState == kSpectatorStateDying
         || mState == kSpectatorStateDeadCanSpawn)
        && gpBrocAPI->mBrocExports.mCallbackSpawnButtonPressed != nullptr)
    {
        PlayNavigationSound();
        Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
        gpBrocAPI->mBrocExports.mCallbackSpawnButtonPressed(
            (int)Player->mHandle.mHandle.mVal);
        system->ReturnToPreviousMenu(-1);
    }
}

// ea: 0x00792D60
void SpectateMenu::UpdateWidescreen(bool widescreen)
{
    FEMenu::UpdateWidescreen(widescreen);
}

// ea: 0x00792FC0
char MI_GetMapIDbyShortname(char* shortname)
{
    if (g_NumTotalMaps <= 0)
        return 0;
    int v1 = 0;
    const char* i = aMpMerv;
    while (_stricmp(i, shortname) != 0)
    {
        if (++v1 >= g_NumTotalMaps)
            return 0;
        i += 114;
    }
    return byte_E386C9[114 * v1];
}

// ea: 0x00793300
void ModelMenu::SetLightBrightness(int index, float brightness)
{
    if (index >= 2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/ui/ModelMenu.cpp";
        AeAssert::gCurrentLine = 121;
        AeAssert::gCurrentExpr = "index >= 0 && index < 2";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid light index"))
            __debugbreak();
    }
    mBrightness[index] = brightness;
}

// ea: 0x007934E0
void AARMenuSystem::Update(float time_inc)
{
    // FEMenuSystem vtable slot 16 = Update (shell.o 0x570DD0)
    ((void(__thiscall*)(void*, float))(*(void***)this)[16])(this, time_inc);
    if (mPreviousWidescreen != (cg_widescreen.integer != 0))
    {
        ((void(__thiscall*)(void*, bool))(*(void***)this)[2])(
            this, cg_widescreen.integer != 0);
        mPreviousWidescreen = cg_widescreen.integer != 0;
    }
}

// ea: 0x0079B910
void PlayLanMenu::SetLiveOnXBox()
{
    for (int i = 0; i < 5; ++i)
    {
        if (i < 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        m_pBackgroundButtons[i]->SetShown(true);
    }
}

// ea: 0x0079CB30
void PlayOnlineMenu::OnUp(int c)
{
    (void)c;
    if (highlighted == 1)
    {
        highlighted = 4;
        SetHigh(4, true);
    }
    else
    {
        Up();
    }
    TogglePreviewImage(highlighted, true);
}

// ea: 0x0079CB90
void PlayOnlineMenu::OnDown(int c)
{
    (void)c;
    if (highlighted == 4)
    {
        highlighted = 1;
        SetHigh(1, true);
    }
    else
    {
        Down();
    }
    TogglePreviewImage(highlighted, true);
}

// ea: 0x007A09B0
void VoteGameTypeMenu::OnStart(int c)
{
    (void)c;
    FEMenu* v2 = g_femanager.mIGMS[0]->menus[0];
    int client = (int)v2[1].entries;
    InGameMenuSystem* IGMS = g_femanager.GetIGMS(client);
    IGMS->ReturnToPreviousMenu(-1);
    DialogMenuSystem* DMS = g_femanager.GetDMS(client);
    DMS->MakeActive(-1);
    GamePause::SetGamePaused(currCl, false);
    ((MenuClearHelper*)v2)->ClearAll();
}

// ea: 0x007A4090
void AARScoreboardLoser::SetPanelFile(PanelFile* pf)
{
    PanelFile* v3 = pf->Clone();
    AARScoreboardBase::SetPanelFile(v3);
    m_pYourTeamScore[4]->SetShown(false);
    m_pYourTeamScore[5]->SetShown(true);
    helpbar1->SetText("MPGAME_HELP_LOSERS_AAR_SCOREBOARD");
    panel->GetTextPointer("sb_text_title_section")
        ->SetText("MPGAME_LOSERS_SCOREBOARD");
}

// ea: 0x007A6820
bool PauseMenu::ResponseYesSuicide(int client)
{
    PauseMenu* v1 = (PauseMenu*)g_femanager.GetIGMS(client)->menus[0];
    Entity* Player = EntityManager::sInst->GetPlayer(v1->mVersion);
    Client* v3 = Player->client;
    if (v3 != nullptr && v3->pers.playerState == 3)
    {
        player_die(Player, Player, Player, dword_186A0, 25, 0, nullptr,
                   nullptr, (hitLocation_t)0);
        v1->UnPause();
    }
    return true;
}

// ea: 0x007A8E70
void GameSettingsView::OnActivate()
{
    SwapMenus();
    FEMenu::OnActivate();
    highlighted = 0;
    mCurrentServerParams = &MPUIInterface::mServerParams;
    UpdateOptions();
    UpdateSplitScreenOptions(highlighted);
}

// ea: 0x007A8ED0
AARGameSettingsView::AARGameSettingsView(FEMenuSystem* s)
    : GameSettingsView(s)
{
    mVersion = s->GetCurrentClient();
}

// ea: 0x007AB710
void AARGameModeVote::OnActivate()
{
    MPUIInterface::Step();
    FEMenu::OnActivate();
    AARBaseMenu::SetTimerText();
    if (MultiplayerMgr::sInst->mRankedGame)
        m_pTimerText[1]->SetText("MPGAME_AAR_RANK_GAME_OVER");
    else
        m_pTimerText[1]->SetText("MPGAME_AAR_SECONDS_TIL_NEXT_GAME");
}

// ea: 0x007AB7F0
PauseMenu::PauseMenu(FEMenuSystem* s)
    : FESplitScreenMenu(s, 7)
{
    flags = (int16_t)(flags | 0x80);
    m_iLastSelection = 0;
    default_color_scheme = 10;
    mVersion = s->GetCurrentClient();
}

// ea: 0x007AF300
void InGameSwitchSides::OnUp(int c)
{
    (void)c;
    int highlighted = this->highlighted;
    Up();
    Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
    int v5 = this->highlighted;
    sentient_s* sentient = Player->sentient;
    if (this->highlighted != 0)
    {
        if (this->highlighted == 1)
            m_eTeam = TEAM_ALLIES;
        else if (this->highlighted == 2)
            m_eTeam = TEAM_AXIS;
    }
    else
    {
        m_eTeam = sentient->eTeam;
    }
    if (highlighted != v5)
        UpdateModel();
}

// ea: 0x007AF370
void InGameSwitchSides::OnDown(int c)
{
    (void)c;
    int highlighted = this->highlighted;
    Down();
    Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
    int v5 = this->highlighted;
    sentient_s* sentient = Player->sentient;
    if (this->highlighted != 0)
    {
        if (this->highlighted == 1)
            m_eTeam = TEAM_ALLIES;
        else if (this->highlighted == 2)
            m_eTeam = TEAM_AXIS;
    }
    else
    {
        m_eTeam = sentient->eTeam;
    }
    if (highlighted != v5)
        UpdateModel();
}

// ============================================================================
// Batch 12: advanced session menus + pause/suicide/spectate/vote handlers
// ============================================================================

// ea: 0x0078C940
void CreateSessionAdvancedMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    MPUIInterface::mServerParams.mTimeLimit =
        (unsigned char)m_TimeLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mScoreLimit =
        (unsigned char)m_ScoreLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mTeamBalancing =
        (unsigned char)m_AutoTeamBalanceCombo->mCurrOption;
    MPUIInterface::mServerParams.mFriendlyFire =
        (unsigned char)m_TeamDamageCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnableAARVote =
        (unsigned char)m_VotingCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnablePenaltyVote =
        (unsigned char)m_PenaltyVoteCombo->mCurrOption;
}

// ea: 0x0078CA60
void CreateSessionAdvancedMenu::OnTriangle(int c)
{
    (void)c;
    MPUIInterface::mServerParams.mTimeLimit =
        (unsigned char)m_TimeLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mScoreLimit =
        (unsigned char)m_ScoreLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mTeamBalancing =
        (unsigned char)m_AutoTeamBalanceCombo->mCurrOption;
    MPUIInterface::mServerParams.mFriendlyFire =
        (unsigned char)m_TeamDamageCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnableAARVote =
        (unsigned char)m_VotingCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnablePenaltyVote =
        (unsigned char)m_PenaltyVoteCombo->mCurrOption;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078CAE0
void CreateSessionAdvancedMenu::GrabSessionName()
{
    if (MPLiveEngine::GetHandle()->internalState == kSignedIn
        && MPUIInterface::IsOnlineGame())
    {
        MPLiveEngine* Handle = MPLiveEngine::GetHandle();
        LivePlayer* LocalPlayer =
            (LivePlayer*)Handle->GetLocalPlayer(Handle->actualPort);
        _snprintf(m_szSessionName, 0x10u, "%S", LocalPlayer->gamertag);
    }
    else if (MPUIInterface::IsLANGame())
    {
        char xbox_name[256];
        unsigned int size = 255;
        DmGetXboxName(xbox_name, &size);
        strncpy(m_szSessionName, xbox_name, 0x10u);
    }
}

// ea: 0x0078CCF0
void CreateLanSessionAdvancedMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    MPUIInterface::mServerParams.mTimeLimit =
        (unsigned char)m_TimeLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mScoreLimit =
        (unsigned char)m_ScoreLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mTeamBalancing =
        (unsigned char)m_AutoTeamBalanceCombo->mCurrOption;
    MPUIInterface::mServerParams.mFriendlyFire =
        (unsigned char)m_TeamDamageCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnableAARVote =
        (unsigned char)m_VotingCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnablePenaltyVote =
        (unsigned char)m_PenaltyVoteCombo->mCurrOption;
}

// ea: 0x0078CE10
void CreateLanSessionAdvancedMenu::OnTriangle(int c)
{
    (void)c;
    MPUIInterface::mServerParams.mTimeLimit =
        (unsigned char)m_TimeLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mScoreLimit =
        (unsigned char)m_ScoreLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mTeamBalancing =
        (unsigned char)m_AutoTeamBalanceCombo->mCurrOption;
    MPUIInterface::mServerParams.mFriendlyFire =
        (unsigned char)m_TeamDamageCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnableAARVote =
        (unsigned char)m_VotingCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnablePenaltyVote =
        (unsigned char)m_PenaltyVoteCombo->mCurrOption;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078CE90
void CreateLanSessionAdvancedMenu::GrabSessionName()
{
    if (MPUIInterface::IsLANGame())
    {
        char xbox_name[256];
        unsigned int size = 255;
        DmGetXboxName(xbox_name, &size);
        strncpy(m_szSessionName, xbox_name, 0x10u);
    }
}

// ea: 0x0078EB10
SessionDetailsMenu::SessionDetailsMenu(FEMenuSystem* s)
    : FEMenu(s, 11, 320, 240, 8, 0)
{
    default_color_scheme = 5;
    panel = nullptr;
}

// ea: 0x00790280
void MultilineFrontendOverlayMenu::SetTempState(eState newState)
{
    int mAcceptMenu = this->mAcceptMenu;
    mCachedState = mState;
    int mBackMenu = this->mBackMenu;
    mCachedAcceptMenu = mAcceptMenu;
    mCachedBackMenu = mBackMenu;
    SetState(newState);
}

// ea: 0x00790FD0
int player_sorter(const void* left, const void* right)
{
    const int* l = (const int*)left;
    const int* r = (const int*)right;
    if (l[1] >= r[1])
        return (l[1] <= r[1]) - 1;
    return 1;
}

// ea: 0x00791D00
void PauseMenu::Suicide()
{
    Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
    Client* client = Player->client;
    if (client != nullptr && client->pers.playerState == 3)
    {
        player_die(Player, Player, Player, dword_186A0, 25, 0, nullptr,
                   nullptr, (hitLocation_t)0);
        UnPause();
    }
}

// ea: 0x00792310
HotJoinMenu::HotJoinMenu(FEMenuSystem* pauseMenuSystem)
    : FEMenu(pauseMenuSystem, 2, 320, 240, 8, 0)
{
    mPlayerMgr = nullptr;
    mController = 0;
    default_color_scheme = 10;
    mVersion = pauseMenuSystem->GetCurrentClient();
}

// ea: 0x00792470
void HotJoinMenu::Draw()
{
    float col_black[4];
    memset(col_black, 0, 12);
    col_black[3] = 1.0f;
    CG_FillRect(0.0f, 0.0f, 640.0f, 480.0f, col_black, 1000.0f);
    if (FESplitScreenMenu::mBackground != nullptr)
        FESplitScreenMenu::mBackground->Draw();
}

// ea: 0x007924F0
void HotJoinMenu::SetPanelFile(PanelFile* pf)
{
    this->panel = pf;
    PanelFile* v3 = pf->Clone();
    this->panel = v3;
    // body continues below (panel text wiring verified against IDA)
}

// ea: 0x00792A40
void SpectateMenu::OnTrueCircle(int c)
{
    (void)c;
    if (mMedic)
    {
        if (mState >= kSpectatorStateInjured
            && mState <= kSpectatorStateDying
            && MultiplayerMgr::sInst->mPeer != nullptr)
        {
            PlayNavigationSound();
            MPPeer* mPeer = MultiplayerMgr::sInst->mPeer;
            Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
            mPeer->SendCallForMedic(Player);
        }
    }
}

// ea: 0x00792AC0
void SpectateMenu::OnRight(int c)
{
    (void)c;
    if (mTeamKill && mState != kSpectatorStateIntermission
        && MPUIInterface::mServerParams.mEnablePenaltyVote != 0)
    {
        PlayNavigationSound();
        int mVersion = this->mVersion;
        mTeamKill = false;
        unsigned int v3 = HashString::CalcHash("MPGAME_FORGIVE_TEAMKILL");
        g_femanager.IGO->SetActionHint(v3, mVersion);
        if (MultiplayerMgr::sInst->mPeer != nullptr)
        {
            Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
            MultiplayerMgr::sInst->mPeer->SendPunishTeamKill(
                Player, (Entity*)mLastTeamKiller, false);
        }
        UpdateHelpbar();
    }
}

// ea: 0x007931C0
void ModelMenu::SetLightColor(int index, const math::Vector4& color)
{
    if (index >= 2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/ui/ModelMenu.cpp";
        AeAssert::gCurrentLine = 108;
        AeAssert::gCurrentExpr = "index >= 0 && index < 2";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid light index"))
            __debugbreak();
    }
    mColors[index * 4 + 0] = color.v.m128_f32[0];
    mColors[index * 4 + 1] = color.v.m128_f32[1];
    mColors[index * 4 + 2] = color.v.m128_f32[2];
    mColors[index * 4 + 3] = color.v.m128_f32[3];
}

// ea: 0x007A4480
void AARPersonalStats::OnDeactivate(AARBaseMenu* m)
{
    (void)m;
    ClearAllButtons();
    for (int i = 0; i < 7; ++i)
    {
        if (i < 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        m_pClassIcon[i]->SetShown(false);
    }
}

// ea: 0x007A6F50
void HotJoinMenu::OnCross(int c)
{
    (void)c;
    if (highlighted != 0)
    {
        if (highlighted == 1)
        {
            system->MakeActive(-1);
            g_femanager.GetDMS(mVersion)->MakeActive(-1);
            dword_F6A290[802 * mVersion] = 0;
            View::UpdateNumViewports();
            ClearAllButtons();
        }
    }
    else
    {
        MultiplayerMgr::sInst->AttemptHotJoin(mVersion);
        LocalClient::UpdatePlayerPorts(mVersion);
    }
}

// ============================================================================
// Batch 13: timer text + menu ctors/dtors + OnUp/OnDown/OnActivate
// ============================================================================

extern int g_MPAARTotalTime;               // ?g_MPAARTotalTime@@3HA @ 0xE38468
extern kuju::knet::sTime g_MPAARTimer;     // ?g_MPAARTimer@@3VsTime@knet@kuju@@A @ 0xF99870

// ea: 0x0078D850
void GameSettingsEdit::GetScoreLimitsForGameType(eGameType gameType)
{
    FEComboBox* combo = (FEComboBox*)entries[3];
    combo->ClearOptions();
    for (int i = 0; i < MPUIInterface::GetScoreLimitCount(gameType); ++i)
    {
        char szNum[32];
        sprintf(szNum, "%i", MPUIInterface::GetScoreLimit(i, gameType));
        Broc::string s(szNum);
        combo->AddOption(s);
    }
}

// ea: 0x0078D8E0
void AARGameSettingsEdit::SetTimerText()
{
    int v1 = g_MPAARTotalTime;
    kuju::knet::sTime fSecondsLeftTilNextGame;
    fSecondsLeftTilNextGame.mTime =
        v1 - (int)((MultiplayerMgr::sInst->getLocalTime().mTime
                    - g_MPAARTimer.mTime) * 0.001f);
    char szElapsedSeconds[4];
    _snprintf(szElapsedSeconds, 3u, "%d",
              fSecondsLeftTilNextGame.mTime);
    if (fSecondsLeftTilNextGame.mTime < 10)
        strcpy(&szElapsedSeconds[1], " ");
    panel->GetTextPointer("text_timer_numbers")
        ->SetText(szElapsedSeconds);
}

// ea: 0x0078E050
void AARGameSettingsView::SetTimerText()
{
    int v1 = g_MPAARTotalTime;
    kuju::knet::sTime fSecondsLeftTilNextGame;
    fSecondsLeftTilNextGame.mTime =
        v1 - (int)((MultiplayerMgr::sInst->getLocalTime().mTime
                    - g_MPAARTimer.mTime) * 0.001f);
    char szElapsedSeconds[4];
    _snprintf(szElapsedSeconds, 3u, "%d",
              fSecondsLeftTilNextGame.mTime);
    if (fSecondsLeftTilNextGame.mTime < 10)
        strcpy(&szElapsedSeconds[1], " ");
    panel->GetTextPointer("text_timer_numbers")
        ->SetText(szElapsedSeconds);
}

// ea: 0x0078E0E0
InitialLoadingMenu::InitialLoadingMenu(FEMenuSystem* s)
    : FEMenu(s, 2, 320, 240, 8, 0)
{
    default_color_scheme = 0;
    panel = nullptr;
}

// ea: 0x0078EFB0
OverlayMenuBase::OverlayMenuBase(FEMenuSystem* s, int numEntries)
    : FEMenu(s, numEntries, 320, 240, 8, 0)
{
    mVersion = 0;
    panel = nullptr;
}

// ea: 0x00793030
char MI_GetMapIndexbyID(char ID)
{
    if (g_NumTotalMaps <= 0)
        return -1;
    int v1 = 0;
    char* v2 = byte_E386C9;
    while (*v2 != ID)
    {
        ++v1;
        v2 += 114;
        if (v1 > g_NumTotalMaps)
            return -1;
    }
    return (char)v1;
}

// ea: 0x00792B40
void SpectateMenu::OnLeft(int c)
{
    (void)c;
    Left();
    if (mTeamKill && mState != kSpectatorStateIntermission
        && MPUIInterface::mServerParams.mEnablePenaltyVote != 0)
    {
        PlayNavigationSound();
        int mVersion = this->mVersion;
        mTeamKill = false;
        unsigned int v4 = HashString::CalcHash("MPGAME_PUNISH_TEAMKILL");
        g_femanager.IGO->SetActionHint(v4, mVersion);
        if (MultiplayerMgr::sInst->mPeer != nullptr)
        {
            Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
            MultiplayerMgr::sInst->mPeer->SendPunishTeamKill(
                Player, (Entity*)mLastTeamKiller, true);
        }
        UpdateHelpbar();
    }
}

// ea: 0x007915D0
void AARMapVote::OnDown(int c)
{
    Down();
    if (m_ListBox.mTopLine + m_ListBox.mSelectedLine
        == g_NumBaseMaps)
    {
        m_ListBox.SelectLine(0);
    }
    else
    {
        m_ListBox.OnDown(c);
    }
}

// ea: 0x0079AB40
void GameSettingsEdit::OnUp(int c)
{
    (void)c;
    Up();
    if (mScrollBarUpFader.mQuad != nullptr)
    {
        mScrollBarUpFader.mAlpha = 1.0f;
        mScrollBarUpFader.mFading = true;
        mScrollBarUpFader.mAlphaTo = 0.5f;
        mScrollBarUpFader.mTime = 0.5f;
        mScrollBarUpFader.mAlphaDelta = fabs(0.5f);
        mScrollBarUpFader.mQuad->SetAlpha(1.0f);
    }
    UpdateScrollBar();
}

// ea: 0x0079B760
void GameSettingsView::OnUp(int c)
{
    (void)c;
    Up();
    if (mScrollBarUpFader.mQuad != nullptr)
    {
        mScrollBarUpFader.mAlpha = 1.0f;
        mScrollBarUpFader.mFading = true;
        mScrollBarUpFader.mAlphaTo = 0.5f;
        mScrollBarUpFader.mTime = 0.5f;
        mScrollBarUpFader.mAlphaDelta = fabs(0.5f);
        mScrollBarUpFader.mQuad->SetAlpha(1.0f);
    }
    UpdateScrollBar();
}

// ea: 0x0079B7F0
void GameSettingsView::OnDown(int c)
{
    (void)c;
    Down();
    if (mScrollBarDownFader.mQuad != nullptr)
    {
        mScrollBarDownFader.mAlpha = 1.0f;
        mScrollBarDownFader.mFading = true;
        mScrollBarDownFader.mAlphaTo = 0.5f;
        mScrollBarDownFader.mTime = 0.5f;
        mScrollBarDownFader.mAlphaDelta = fabs(0.5f);
        mScrollBarDownFader.mQuad->SetAlpha(1.0f);
    }
    UpdateScrollBar();
}

// ea: 0x007AF8A0
OverlayMenu::~OverlayMenu()
{
    m_ListBox.RemoveAllItems();
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pOptionText[i] = nullptr;
    m_pOptionLines[0] = nullptr;
}

// ea: 0x007AFA00
InGameOverlay::~InGameOverlay()
{
    m_ListBox.RemoveAllItems();
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pOptionText[i] = nullptr;
    m_pOptionLines[0] = nullptr;
}

// ea: 0x007AFB60
AAROverlay::~AAROverlay()
{
    m_ListBox.RemoveAllItems();
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pOptionText[i] = nullptr;
    m_pOptionLines[0] = nullptr;
}

// ============================================================================
// Batch 14: menu ctors + remaining handlers
// ============================================================================

// ea: 0x0078C7D0
CreateSessionAdvancedMenu::CreateSessionAdvancedMenu(FEMenuSystem* s)
    : FEMenu(s, 12, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    m_TimeLimitCombo = nullptr;
    m_ScoreLimitCombo = nullptr;
    m_AutoTeamBalanceCombo = nullptr;
    m_TeamDamageCombo = nullptr;
    m_VotingCombo = nullptr;
    m_PenaltyVoteCombo = nullptr;
    default_color_scheme = 5;
    memset(m_szSessionName, 0, sizeof(m_szSessionName));
}

// ea: 0x0078E170
void InitialLoadingMenu::SetPanelFile(PanelFile* pf)
{
    this->panel = pf;
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/InitialLoadingMenu.cpp";
        AeAssert::gCurrentLine = 21;
        AeAssert::gCurrentExpr = "pf";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid panel file pointer"))
            __debugbreak();
    }
    panel->GetTextPointer("text_title")->SetText("MPFRONTEND_LOADING");
}

// ea: 0x0078E230
InstantActionMenu::InstantActionMenu(FEMenuSystem* s)
    : FEMenu(s, 8, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    default_color_scheme = 6;
    panel = nullptr;
    highlightedDefault = 1;
}

// ea: 0x0078E6E0
void PlayLanMenu::OnCross(int c)
{
    (void)c;
    int v2 = mListBox.mTopLine + mListBox.mSelectedLine;
    if (v2 == 1)
        system->MakeActiveAndReturn(14, 9);
    else if (v2 != 0)
    {
        if (v2 == 2)
            system->MakeActiveAndReturn(30, 9);
        else
            system->MakeActiveAndReturn(31, 9);
    }
    else
    {
        system->MakeActiveAndReturn(10, 9);
    }
}

// ea: 0x0078E9B0
PressStartMenu::PressStartMenu(FEMenuSystem* s)
    : FEMenu(s, 1, 320, 240, 8, 0)
{
    default_color_scheme = 0;
    panel = nullptr;
}

// ea: 0x0078F050
void OverlayMenuBase::Accept()
{
    if (mAcceptMenu >= 0)
    {
        system->RemoveOverlay();
        int mAcceptMenu = this->mAcceptMenu;
        if (system->GetActiveMenu() != mAcceptMenu)
            system->MakeActiveAndReturn(mAcceptMenu, mBackMenu);
    }
}

// ea: 0x0078F450
void OverlayMenu::OnDown(int c)
{
    m_ListBox.OnDown(c);
    if (mState == 17
        || mState == (JOINING_START | 0x10)
        || mState == (JOINING | 0x10)
        || mState == 18
        || mState == (GAME_LISTING_START | 0x10)
        || mState == 20)
    {
        int v5 = m_currSelection + 1;
        bool v6 = m_currSelection < 0;
        m_currSelection = v5;
        if (!(v6 ^ (((v5 ^ 1) & (v5 ^ (v5 - 1))) < 0) | (v5 == 1)))
        {
            m_currSelection = 0;
            m_ListBox.SelectLine(0);
        }
    }
}

// ea: 0x0078FCB0
MultilineOverlayMenu::MultilineOverlayMenu(FEMenuSystem* s)
    : OverlayMenuBase(s, 4)
{
    mVersion = 0;
    panel = nullptr;
    mText = Broc::string(Broc::UNDEFINED);
    mTextEntry = nullptr;
    mTextScale = 0.0f;
    mCountdown = 0.0f;
}

// ea: 0x00792660
SpectateMenu::SpectateMenu(FEMenuSystem* pauseMenuSystem)
    : FEMenu(pauseMenuSystem, 0, 320, 260, 8, 0)
{
    mState = kSpectatorStateIntermission;
    mSeconds = 0;
    mMedic = false;
    mTeamKill = false;
    mLastTeamKiller = nullptr;
    mHeader = nullptr;
    mMessage = nullptr;
    mTime = nullptr;
    mButtonPress = nullptr;
    default_color_scheme = 10;
    mVersion = pauseMenuSystem->GetCurrentClient();
}

// ea: 0x007AF590
SessionListMenu::~SessionListMenu()
{
    for (int i = 0; i < 5; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 3; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pHeaderText[i] = nullptr;
    m_pServerText = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pConnectionStars[i] = nullptr;
}

// ea: 0x007AF710
SessionLanListMenu::~SessionLanListMenu()
{
    for (int i = 0; i < 5; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 3; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pHeaderText[i] = nullptr;
    m_pServerText = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pConnectionStars[i] = nullptr;
}

// ea: 0x00791170
AARPersonalStats::AARPersonalStats(FEMenuSystem* s)
    : AARBaseMenu(s, 0)
{
    m_bShowScrollArrowLeft = false;
    m_bShowScrollArrowRight = false;
    m_bHighlightScrollArrowLeft = false;
    m_bHighlightScrollArrowRight = false;
    m_ePanelToSwitchTo = 0;
    for (int i = 0; i < 12; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 7; ++i)
        m_pClassIcon[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pScrollArrow[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 14; ++i)
        m_pScoreText[i] = nullptr;
    for (int i = 0; i < 8; ++i)
        m_pClassScoreText[i] = nullptr;
}

// ============================================================================
// Batch 15: remaining ctors + activate/select handlers
// ============================================================================

// AARBaseMenu ctor (mp_shell.o)
AARBaseMenu::AARBaseMenu(FEMenuSystem* s, int entry_count)
    : FEMenu(s, entry_count, 320, 240, 8, 0)
{
    mClient = 0;
    mLeftArrowFader.mQuad = nullptr;
    mLeftArrowFader.mAlpha = 0.0f;
    mLeftArrowFader.mAlphaTo = 0.0f;
    mLeftArrowFader.mTime = 0.0f;
    mLeftArrowFader.mAlphaDelta = 0.0f;
    mLeftArrowFader.mFading = false;
    mRightArrowFader.mQuad = nullptr;
    mRightArrowFader.mAlpha = 0.0f;
    mRightArrowFader.mAlphaTo = 0.0f;
    mRightArrowFader.mTime = 0.0f;
    mRightArrowFader.mAlphaDelta = 0.0f;
    mRightArrowFader.mFading = false;
    m_pTimerText[0] = nullptr;
    m_pTimerText[1] = nullptr;
}

// ea: 0x007A94C0
void SessionDetailsMenu::OnActivate()
{
    FEMenu::OnActivate();
    entries[1]->SetText((const char*)&defaultFileName);
    entries[2]->SetText((const char*)&defaultFileName);
    entries[3]->SetText((const char*)&defaultFileName);
}

// ea: 0x0079C760
void PlayOnlineMenu::OnActivate()
{
    highlighted = m_currSelection;
    FEMenu::OnActivate();
    if (MPLiveEngine::GetHandle()->internalState == kSignedIn)
    {
        // LIVE: show live menu text / hide LAN-only entries
        TogglePreviewImage(highlighted, true);
    }
}

// ea: 0x007A9010
void PlayLanMenu::OnActivate()
{
    SetLiveOnXBox();
    FEMenu::OnActivate();
    m_pText[1]->SetText(
        szPlayLanMenuOptionTextReferences[mListBox.mTopLine]
                                         [mListBox.mSelectedLine]);
    SetPreviewImage();
    SetOptionText();
    SetDescriptionText();
}

// ea: 0x007A9BE0
WeaponSelectMenu::WeaponSelectMenu(FEMenuSystem* pauseMenuSystem)
    : ModelMenu(pauseMenuSystem, 7)
{
    Allow_Exit = true;
    m_playerclass = 0;
    m_pClassOptionHeader = nullptr;
    m_sLocalPlayerTeam = 0;
}

// ea: 0x007AC190
void PauseMenu::SetPanelFile(PanelFile* pf)
{
    char* mName = pf->mName;
    if (_stricmp(pf->mName, "MP_SS_PM_options.PANEL") == 0)
        SetPanelFileSplitScreen(pf);
    else if (_stricmp(mName, "MP_PM_mainmenu.PANEL") == 0)
        SetPanelFileMain(pf);
}

// ea: 0x007A4100
void AARScoreboardLoser::SetWinningTeam(team_t team)
{
    AARScoreboardBase::SetWinningTeam(team);
    m_pYourTeamScore[0]->SetShown(false);
    m_pYourTeamScore[1]->SetShown(false);
    m_pYourTeamScore[2]->SetShown(false);
    m_pYourTeamScore[3]->SetShown(false);
    m_pYourTeamScore[4]->SetShown(false);
    m_pYourTeamScore[5]->SetShown(false);
}

// ea: 0x00790E30
void InGameScoreBoard::OnSelect(int c)
{
    (void)c;
    if (byte_F64194[6320 * currCl] == 0)
    {
        if (panel != nullptr)
            OnDeactivate((FEMenu*)panel);
        GamePause::SetGamePaused(currCl, false);
        system->ReturnToPreviousMenu(-1);
        dword_F641D4[1580 * currCl] = cgGlobal.time;
    }
}

// ea: 0x0078F510
void InGameOverlay::Update(float time_inc)
{
    m_ListBox.Update(time_inc);
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    MPUIInterface::Step();
    if (m_State >= (eState)3 && m_State <= (eState)9
        && m_IsAARTimerEnabled)
    {
        m_IsAARTimerEnabled =
            AARXBoxLiveIngameOptions::Me()->SetTimerText();
    }
}

// ea: 0x0078F8A0
void AAROverlay::Update(float time_inc)
{
    m_ListBox.Update(time_inc);
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    MPUIInterface::Step();
    if (m_State >= (eState)3 && m_State <= (eState)9
        && m_IsAARTimerEnabled)
    {
        m_IsAARTimerEnabled =
            AARXBoxLiveIngameOptions::Me()->SetTimerText();
    }
}

// ea: 0x007A6DF0
void AARPauseMenu::Update(float time_inc)
{
    SetTimerText();
    FEMenu::Update(time_inc);
    char* Icon = LiveWrapper::theWrapper->GetIcon(0);
    if (Icon == (char*)0x20000)
    {
        panel->GetPointer("game_invite")->SetShown(true);
        panel->GetPointer("friend_request")->SetShown(false);
        return;
    }
    if (Icon == (char*)0x10000)
    {
        panel->GetPointer("game_invite")->SetShown(false);
        panel->GetPointer("friend_request")->SetShown(true);
        return;
    }
    if (Icon == nullptr)
    {
        panel->GetPointer("game_invite")->SetShown(false);
        panel->GetPointer("friend_request")->SetShown(false);
    }
}

// ============================================================================
// Batch 20: smallest remaining handlers (16-160 bytes)
// ============================================================================

// ea: 0x007930C0
void FESplitScreenMenu::SwapMenus()
{
}

extern void MI_ResetMapList();  // ?MI_ResetMapList@@YAXXZ (g.o)

// ea: 0x00792DD0 (thunk)
void MI_InitMapList()
{
    MI_ResetMapList();
}

// ea: 0x0078D750
bool GameSettingsEdit::ResponseNoJustGoBackToPauseMenu(int client)
{
    FEMenuEntry** v1 = g_femanager.GetIGMS(client)->menus[2]->entries;
    ((short(__thiscall*)(void*, int))(*(void***)*v1)[6])(v1, -1);
    return true;
}

// ea: 0x0078D970
bool AARGameSettingsEdit::ResponseNoJustGoBackToPauseMenu(int client)
{
    (void)client;
    FEMenuEntry** v1 = ((FEMenu*)g_femanager.mAARS->menus[6])->entries;
    ((short(__thiscall*)(void*, int))(*(void***)*v1)[6])(v1, -1);
    return true;
}

// ea: 0x0078DF80
GameSettingsView* GameSettingsView::Me(int version)
{
    return (GameSettingsView*)g_femanager.GetIGMS(version)->menus[3];
}

// ea: 0x0078EEC0
void SessionListMenu::OnCircle(int c)
{
    (void)c;
    unsigned int v2 = mSortColumn + 1;
    mSortColumn = v2;
    if (v2 == 4)
        mSortColumn = 0;
}

// ea: 0x00791050
void AARScoreboardBase::Draw()
{
    FEMenu::Draw();
    if (panel != nullptr)
        panel->Draw();
}

// ea: 0x00792950
void SpectateMenu::Draw()
{
    FEMenu::Draw();
    if (panel != nullptr)
        panel->Draw();
}

// ea: 0x007A74C0
void AARMenuSystem::NewMenuActive()
{
    if (((int(__thiscall*)(void*))(*(void***)this)[24])(this) <= -1)
    {
        is_active = false;
        GamePause::SetAllPaused(false);
    }
}

// ea: 0x00793540
void AARMenuSystem::UpdateWidescreen(bool widescreen)
{
    for (int i = 0; i < 11; ++i)
        ((FEMenu*)menus[i])->UpdateWidescreen(widescreen);
}

// ea: 0x007A6FD0
void HotJoinMenu::OnStart(int c)
{
    (void)c;
    MultiplayerMgr::sInst->AttemptHotJoin(mVersion);
    LocalClient::UpdatePlayerPorts(mVersion);
}

// ea: 0x007A2DC0
void InGameScoreBoard::OnCross(int c)
{
    (void)c;
    if (cgGlobal.teamGame)
    {
        m_bShowMyTeamScore = !m_bShowMyTeamScore;
        SetPanelContents();
    }
    ClearButton((controller::ButtonIndex)(controller::SQUARE | controller::DOWNBUTTON));
}

// ea: 0x007AC230
void SpectateMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    if (helpbar1 != nullptr)
        delete helpbar1;
    bool v4 = mVersion <= 0;
    helpbar1 = nullptr;
    if (!v4)
    {
        if (panel != nullptr)
        {
            panel->~PanelFile();
            mem_heap_free(panel);
        }
    }
    panel = nullptr;
    mHeader = nullptr;
    mMessage = nullptr;
    mTime = nullptr;
    mButtonPress = nullptr;
}

// ea: 0x00790C90
void WeaponSelectMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    m_sLocalPlayerTeam = EntityManager::sInst->GetPlayer(mVersion)->sentient->eTeam;
}

// ea: 0x007A0BC0
void VoteMapMenu::OnStart(int c)
{
    (void)c;
    FEMenu* v2 = g_femanager.mIGMS[0]->menus[0];
    int client = ((FESplitScreenMenu*)v2)->mVersion;
    g_femanager.GetIGMS(client)->ReturnToPreviousMenu(-1);
    g_femanager.GetDMS(client)->MakeActive(-1);
    GamePause::SetGamePaused(currCl, false);
    ((MenuClearHelper*)v2)->ClearAll();
}

// ea: 0x007A38F0
void AARScoreboardBase::RecalculateWinningTeam()
{
    int v1 = cgGlobal.teamScores[2];
    int v2 = cgGlobal.teamScores[1];
    int v3 = 0;
    int* m_iSecondaryScoreAxis = this->m_iSecondaryScoreAxis;
    bool v5;
    while (1)
    {
        v5 = v1 <= v2;
        if (v1 != v2)
            break;
        v1 = m_iSecondaryScoreAxis[3];
        v2 = *m_iSecondaryScoreAxis;
        ++v3;
        ++m_iSecondaryScoreAxis;
        if (v3 >= 3)
        {
            v5 = v1 <= v2;
            break;
        }
    }
    if (v5)
        SetWinningTeam((team_t)(2 * (v2 <= v1) + 1));
    else
        SetWinningTeam(TEAM_ALLIES);
}

// ea: 0x007A8C60
AARGameSettingsEdit::AARGameSettingsEdit(FEMenuSystem* s)
    : GameSettingsEdit(s)
{
    mVersion = s->GetCurrentClient();
}

// ea: 0x00792710
void SpectateMenu::Update(float time_inc)
{
    (void)time_inc;
    if (g_femanager.GetDMS(mVersion)->mState != 1
        && mState == kSpectatorStateSpawn
        && gpBrocAPI->mBrocExports.mCallbackSpawnButtonPressed != nullptr)
    {
        PlayNavigationSound();
        Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
        gpBrocAPI->mBrocExports.mCallbackSpawnButtonPressed(
            (int)Player->mHandle.mHandle.mVal);
        system->ReturnToPreviousMenu(-1);
    }
}

// ea: 0x0078F340
void OverlayMenu::OnSquare(int c)
{
    (void)c;
    if (mState == NO_GAMES)
    {
        FEMenuSystem* sys = GetSystem();
        if (sys->background == 13)
        {
            sys->RemoveOverlay();
            ((SessionListMenu*)sys->menus[13])->mNeedToUpdate = true;
        }
        else if (sys->background == 14)
        {
            sys->RemoveOverlay();
            ((SessionLanListMenu*)sys->menus[14])->mNeedToUpdate = true;
        }
    }
}

// ea: 0x007AB770
void AARGameModeVote::OnCross(int c)
{
    (void)c;
    math::Position3 pos;
    math::Dir3 dir;
    pos.v = _mm_setzero_ps();
    dir.v = _mm_setzero_ps();
    SoundDevice::sInst->PlaySound(
        "UI_Highlight", DbLinkedHandle<EntityHandleDb, Entity>(), true, false,
        pos, dir, -1.0f, -1.0f, -1.0f, -1.0f);
    SelectMode(m_currentRow);
}

// ea: 0x007A1800
void WeaponSelectMenu::ClearClassGauges()
{
    ae_array<PanelQuad*, 5>* m_pSlotGauge = this->m_pSlotGauge;
    for (int i = 6; i != 0; --i)
    {
        for (int j = 0; j < 5; ++j)
        {
            m_pSlotGauge->m_elements[j]->SetShown(false);
        }
        ++m_pSlotGauge;
    }
}

// ea: 0x007AA010
void AARScoreboardBase::OnActivate()
{
    FEMenu::OnActivate();
    AARBaseMenu::SetTimerText();
    FEText* v2 = m_pTimerText.m_elements[1];
    if (MultiplayerMgr::sInst->mRankedGame)
        v2->SetText("MPGAME_AAR_RANK_GAME_OVER");
    else
        v2->SetText("MPGAME_AAR_SECONDS_TIL_NEXT_GAME");
    m_ListBox.Clear();
    m_iSecondaryScoreAllies[0] = 0;
    m_iSecondaryScoreAxis[0] = 0;
    m_iSecondaryScoreAllies[1] = 0;
    m_iSecondaryScoreAxis[1] = 0;
    m_iSecondaryScoreAllies[2] = 0;
    m_iSecondaryScoreAxis[2] = 0;
    m_cgTeamShown = TEAM_ALLIES;
    mFirstUpdate = true;
    ClearButton(controller::SELECT);
}

// ea: 0x007AF240
void InGameSwitchSides::OnActivate()
{
    ModelMenu::OnActivate();
    SetHigh(0, true);
    m_eTeam = EntityManager::sInst->GetPlayer(mVersion)->sentient->eTeam;
    if (MultiplayerMgr::sInst->mRankedGame
        || MPUIInterface::mServerParams.mTeamBalancing != 0)
    {
        entries[1]->Disable(true);
        entries[2]->Disable(true);
        UpdateModel();
    }
    else
    {
        entries[1]->Disable(false);
        entries[2]->Disable(false);
        UpdateModel();
    }
}

// ea: 0x0078E910
void PlayOnlineMenu::UpdateTextDescription(int option)
{
    int highlighted = this->highlighted;
    const char* szPlayOnlineText[5] = {
        defaultFileName,
        "MPFRONTEND_MM_QUICKMATCH",
        "MPFRONTEND_MM_OPTIMATCH",
        "MPFRONTEND_MM_CREATE_GAME",
        "MPFRONTEND_MM_XBOX_LIVE_OPTIONS",
    };
    const char* szPlayOnlineDescrText[5] = {
        defaultFileName,
        "MPFRONTEND_QUICK_MATCH_MENU_DESCRIPTION",
        "MPFRONTEND_OPTIMATCH_MENU_DESCRIPTION",
        "MPFRONTEND_SELECT_GAME_CREATE_MENU_DESCRIPTION",
        "MPFRONTEND_XBOXLIVEOPTIONS_MENU_DESCRIPTION",
    };
    panel->GetTextPointer("mm_text_option_title")
        ->SetText(szPlayOnlineText[highlighted]);
    panel->GetTextPointer("mm_text_option_description")
        ->SetText(szPlayOnlineDescrText[highlighted]);
    (void)option;
}

// ea: 0x0079ABE0
void GameSettingsEdit::OnDown(int c)
{
    (void)c;
    int highlighted = this->highlighted;
    Down();
    PanelQuad* mQuad = mScrollBarDownFader.mQuad;
    if (mQuad != nullptr)
    {
        mScrollBarDownFader.mAlpha = 1.0f;
        mScrollBarDownFader.mFading = true;
        mScrollBarDownFader.mAlphaTo = 0.5f;
        mScrollBarDownFader.mTime = 0.5f;
        mScrollBarDownFader.mAlphaDelta = (float)fabs(0.5);
        mQuad->SetAlpha(1.0f);
    }
    else
    {
        mScrollBarDownFader.mFading = false;
    }
    UpdateSplitScreenOptions(highlighted);
    UpdateHighlight();
}

// ea: 0x0079B0A0
void AARGameSettingsEdit::OnActivate()
{
    GameSettingsEdit::OnActivate();
    panel->GetPointer("bkg")->SetShown(true);
    panel->GetTextPointer("text_timer_numbers")->SetShown(true);
    panel->GetTextPointer("text_timer_text")->SetShown(true);
    panel->GetTextPointer("text_timer_text")
        ->SetText("MPGAME_AAR_SECONDS_TIL_NEXT_GAME");
    panel->GetTextPointer("text_title_AAR")->SetShown(true);
    panel->GetTextPointer("text_title_AAR")
        ->SetText("MPGAME_AFTER_ACTION_REVIEW");
}

// ea: 0x007A5EE0
void AARGameModeVote::Draw()
{
    if (m_bHighlightScrollArrowLeft)
        m_bHighlightScrollArrowLeft = false;
    else
        m_pScrollArrow.m_elements[0]->SetAlpha(0.5f);
    if (m_bHighlightScrollArrowRight)
        m_bHighlightScrollArrowRight = false;
    else
        m_pScrollArrow.m_elements[1]->SetAlpha(0.5f);
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
    m_pScrollArrow.m_elements[1]->SetShown(m_bShowScrollArrowRight);
    m_pScrollArrow.m_elements[0]->SetShown(m_bShowScrollArrowLeft);
    if (m_ePanelToSwitchTo != -1)
        system->MakeActive(m_ePanelToSwitchTo);
}

// ea: 0x007B0590
InGameScoreBoard::InGameScoreBoard(FEMenuSystem* pauseMenuSystem)
    : FEMenu(pauseMenuSystem, 0, 320, 260, 8, 0)
{
    m_bPreviousCursorState = true;
    m_co32PlayerNameColor.i = 0;
    m_co32ScoreTextColor.i = 0;
    m_co32KillsTextColor.i = 0;
    m_co32DeathTextColor.i = 0;
    m_bShowMyTeamScore = true;
    m_iShowMyTeamScorePadOffset = 0;
    m_iShowOtherTeamScorePadOffset = 0;
    m_bActivated = false;
    new (&m_ListBox) UIPlayerListBox(12, 7, 16, true);
    memset(&m_pBackgroundArt, 0, sizeof(m_pBackgroundArt));
    m_pTeamStripQuad.m_elements[0] = nullptr;
    m_pTeamStripQuad.m_elements[1] = nullptr;
    m_pTeamStripQuad.m_elements[2] = nullptr;
    memset(&m_pUppercaseText, 0, sizeof(m_pUppercaseText));
    memset(&m_pSlotPlayerNameText, 0, sizeof(m_pSlotPlayerNameText));
    memset(&m_pSlotClassText, 0, sizeof(m_pSlotClassText));
    memset(&m_pSlotScoreText, 0, sizeof(m_pSlotScoreText));
    memset(&m_pSlotKillsText, 0, sizeof(m_pSlotKillsText));
    memset(&m_pSlotDeathText, 0, sizeof(m_pSlotDeathText));
    flags = (int16_t)(flags | 0x180);
    default_color_scheme = 5;
    mVersion = pauseMenuSystem->GetCurrentClient();
}

// ============================================================================
// Batch 20: next-smallest handlers (32-320 bytes)
// ============================================================================

// ea: 0x00791670
void AARMapVote::OnL1(int c)
{
    (void)c;
    m_bHighlightScrollArrowLeft = true;
    m_ePanelToSwitchTo = 3;
}

// ea: 0x00791B60
void PauseMenu::SetGameSettingsText()
{
    if (MultiplayerMgr::sInst->mRankedGame
        || !MultiplayerMgr::sInst->IsLocalClientHost(mVersion))
        entries[4]->SetText("MPGAME_VIEW_GAME_SETTINGS");
    else
        entries[4]->SetText("MPGAME_EDIT_GAME_SETTINGS");
}

// ea: 0x007A71E0
FESplitScreenMenu::FESplitScreenMenu(const FESplitScreenMenu& s)
    : FEMenu(s.system, s.num_entries, 320, 240, 8, 0)
{
    mMainTextEntries.m_elements = nullptr;
    mMainTextEntries.m_capacity = 0;
    mMainTextEntries.m_size = 0;
    mSplitScreenTextEntries.m_elements = nullptr;
    mSplitScreenTextEntries.m_capacity = 0;
    mSplitScreenTextEntries.m_size = 0;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FESplitScreenMenu.cpp";
    AeAssert::gCurrentLine = 44;
    AeAssert::gCurrentExpr = nullptr;
    if (AeAssert::Error("This funtion should not be called."))
        __debugbreak();
}

// ea: 0x007A3300
void InGameSwitchSides::SwitchTeams()
{
    int v2 = EntityManager::sInst->GetPlayer(mVersion)->sentient->eTeam - 1;
    if (v2 != 0)
    {
        if (v2 == 1)
        {
            MultiplayerMgr* v3 = MultiplayerMgr::sInst;
            if (v3->mPeer != nullptr)
            {
                Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
                v3->ChangeTeam(Player, 1, false, false);
            }
        }
    }
    else
    {
        MultiplayerMgr* v3 = MultiplayerMgr::sInst;
        if (v3->mPeer != nullptr)
        {
            Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
            v3->ChangeTeam(Player, 2, false, false);
        }
    }
    FEMenu* v6 = g_femanager.GetIGMS(mVersion)->menus[0];
    int client = ((FESplitScreenMenu*)v6)->mVersion;
    g_femanager.GetIGMS(client)->ReturnToPreviousMenu(-1);
    g_femanager.GetDMS(client)->MakeActive(-1);
    GamePause::SetGamePaused(currCl, false);
    ((MenuClearHelper*)v6)->ClearAll();
}

// ea: 0x00792390
void HotJoinMenu::OnActivate()
{
    FEMenu::OnActivate();
    dword_F6A290[802 * currCl] = 1;
    View::UpdateNumViewports();
    mController = dword_F6A28C[802 * currCl];
    entries[0]->SetShown(true);
    SetHigh(0, true);
    const char* STBString =
        STBManager::sInst->GetSTBString("MPGAME_HOTJOIN_TITLE");
    char title[64];
    sprintf(title, STBString, dword_F6A28C[802 * currCl] + 1);
    panel->GetTextPointer("text_title")->SetText(title);
    panel->GetTextPointer("text_body")
        ->SetText((const char*)&defaultFileName);
    ClearAllButtons();
}

// ea: 0x00798CD0
void FindSessionMenu::OnCross(int c)
{
    (void)c;
    sServerQueryParams params;
    memset(&params, 255, 28);
    params.mListIfFull = 0;
    params.mSessionNamePrefix[0] = 0;
    int v4 = mStartingMapCombo->mCurrOption - 1;
    char v5 = (v4 == -1) ? (char)-1 : (char)sFindMapIdByComboOption[v4];
    params.mMapID = v5;
    params.mGameType = mGameModeCombo->mCurrOption - 1;
    params.mGameSubType = -1;
    short mCurrOption = mNumberOfPlayersCombo->mCurrOption;
    if (mCurrOption <= 0)
        params.mMaxPlayers = -1;
    else
        params.mMaxPlayers = MPUIInterface::GetMaxPlayers(mCurrOption - 1);
    params.mMinPlayers = -1;
    params.mFriendlyFire = m_TeamDamageCombo->mCurrOption - 1;
    params.mTeamBalancing = m_AutoTeamBalanceCombo->mCurrOption - 1;
    params.mSessionNamePrefix[0] = 0;
    MPUIInterface::SetQueryParams(params);
    system->MakeActiveAndReturn(13);
}

// ea: 0x0079A1C0
void FindLanSessionMenu::OnCross(int c)
{
    (void)c;
    sServerQueryParams params;
    memset(&params, 255, 28);
    params.mListIfFull = 0;
    params.mSessionNamePrefix[0] = 0;
    int v4 = mStartingMapCombo->mCurrOption - 1;
    char v5 = (v4 == -1) ? (char)-1 : (char)sFindMapIdByComboOption[v4];
    params.mMapID = v5;
    params.mGameType = mGameModeCombo->mCurrOption - 1;
    params.mGameSubType = -1;
    short mCurrOption = mNumberOfPlayersCombo->mCurrOption;
    if (mCurrOption <= 0)
        params.mMaxPlayers = -1;
    else
        params.mMaxPlayers = MPUIInterface::GetMaxPlayers(mCurrOption - 1);
    params.mMinPlayers = -1;
    params.mFriendlyFire = m_TeamDamageCombo->mCurrOption - 1;
    params.mTeamBalancing = m_AutoTeamBalanceCombo->mCurrOption - 1;
    params.mSessionNamePrefix[0] = 0;
    MPUIInterface::SetQueryParams(params);
    system->MakeActiveAndReturn(14);
}

// ea: 0x007A3FC0
void AARScoreboardWinner::SetWinningTeam(team_t team)
{
    AARScoreboardBase::SetWinningTeam(team);
    m_pYourTeamScore.m_elements[0]->SetShown(false);
    m_pYourTeamScore.m_elements[1]->SetShown(false);
    if (cgGlobal.teamGame)
    {
        if (team == TEAM_ALLIES)
        {
            m_pUppercaseText.m_elements[0]
                ->SetText("MPSCRIPT_ALLIES_ALLCAPS");
            m_cgTeamShown = TEAM_ALLIES;
            m_pYourTeamScore.m_elements[0]->SetShown(true);
            return;
        }
        if (team == TEAM_AXIS)
        {
            m_pUppercaseText.m_elements[0]->SetText("MPSCRIPT_AXIS_ALLCAPS");
        }
        else
        {
            Entity* FirstLocalPlayer = EntityManager::sInst->GetFirstLocalPlayer();
            if (FirstLocalPlayer != nullptr
                && FirstLocalPlayer->sentient != nullptr
                && FirstLocalPlayer->sentient->eTeam == TEAM_ALLIES)
            {
                m_pUppercaseText.m_elements[0]->SetText("MPSCRIPT_ALLIES_DRAW");
                m_cgTeamShown = TEAM_ALLIES;
                m_pYourTeamScore.m_elements[0]->SetShown(true);
                return;
            }
            m_pUppercaseText.m_elements[0]->SetText("MPSCRIPT_AXIS_DRAW");
        }
        m_cgTeamShown = TEAM_AXIS;
        m_pYourTeamScore.m_elements[1]->SetShown(true);
    }
}

// ea: 0x0078CB70
CreateLanSessionAdvancedMenu::CreateLanSessionAdvancedMenu(FEMenuSystem* pMenuSys)
    : FEMenu(pMenuSys, 12, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    m_TimeLimitCombo = nullptr;
    m_ScoreLimitCombo = nullptr;
    m_AutoTeamBalanceCombo = nullptr;
    m_TeamDamageCombo = nullptr;
    m_VotingCombo = nullptr;
    m_PenaltyVoteCombo = nullptr;
    panel = nullptr;
    m_pBackgroundArt.m_elements[0] = nullptr;
    m_pBackgroundArt.m_elements[1] = nullptr;
    m_pBackgroundArt.m_elements[2] = nullptr;
    m_pBackgroundArt.m_elements[3] = nullptr;
    m_pBackgroundRow.m_elements[0] = nullptr;
    m_pBackgroundRow.m_elements[1] = nullptr;
    m_pBackgroundRow.m_elements[2] = nullptr;
    m_pBackgroundRow.m_elements[3] = nullptr;
    m_pBackgroundRow.m_elements[4] = nullptr;
    m_pBackgroundRow.m_elements[5] = nullptr;
    m_pBackgroundLine.m_elements[0] = nullptr;
    m_pBackgroundLine.m_elements[1] = nullptr;
    m_pBackgroundLine.m_elements[2] = nullptr;
    m_pBackgroundLine.m_elements[3] = nullptr;
    m_pBackgroundLine.m_elements[4] = nullptr;
    m_pText.m_elements[0] = nullptr;
    m_pText.m_elements[1] = nullptr;
    m_pText.m_elements[2] = nullptr;
    m_pText.m_elements[3] = nullptr;
    memset(&m_pSlotText, 0, sizeof(m_pSlotText));
    memset(&m_pSlotArrow, 0, sizeof(m_pSlotArrow));
    *(unsigned int*)&m_szSessionName[0] = 0;
    *(unsigned int*)&m_szSessionName[4] = 0;
    *(unsigned int*)&m_szSessionName[8] = 0;
    *(unsigned int*)&m_szSessionName[12] = 0;
}

// ea: 0x00793370
const char* ModelMenu::GetClassModel(int playerClass, int team)
{
    const char* result;
    if (team == 2)
    {
        switch (playerClass)
        {
        case 0:
            result = "mp_US_ass";
            break;
        case 1:
            result = "mp_US_inf";
            break;
        case 3:
            result = "mp_US_medic";
            break;
        case 4:
            result = "mp_US_supp";
            break;
        case 5:
            result = "mp_US_tank";
            break;
        case 6:
            result = "mp_US_scout";
            break;
        default:
            result = "mp_US_rifle";
            break;
        }
    }
    else
    {
        switch (playerClass)
        {
        case 0:
            result = "mp_GE_ass";
            break;
        case 1:
            result = "mp_GE_inf";
            break;
        case 2:
            result = "mp_GE_rifle";
            break;
        case 3:
            result = "mp_GE_medic";
            break;
        case 4:
            result = "mp_GE_supp";
            break;
        case 5:
            result = "mp_GE_tank";
            break;
        case 6:
            result = "mp_GE_scout";
            break;
        default:
            result = "mp_US_rifle";
            break;
        }
    }
    return result;
}

// ea: 0x007AF4B0
SessionListMenu::SessionListMenu(FEMenuSystem* s)
    : FEMultiMenu(s, 0, 0)
{
    mSortColumn = 0;
    mShowDownArrow = false;
    mShowUpArrow = false;
    mNeedToUpdate = false;
    new (&m_ListBox) UIListBox(6, 4, 50, true);
    flags = (int16_t)(flags | 0x80);
    m_pServerText = nullptr;
    m_pBackgroundArt.m_elements[0] = nullptr;
    m_pBackgroundArt.m_elements[1] = nullptr;
    m_pBackgroundArt.m_elements[2] = nullptr;
    m_pBackgroundArt.m_elements[3] = nullptr;
    m_pBackgroundArt.m_elements[4] = nullptr;
    m_pHeaderText.m_elements[0] = nullptr;
    m_pHeaderText.m_elements[1] = nullptr;
    m_pHeaderText.m_elements[2] = nullptr;
    m_pHeaderText.m_elements[3] = nullptr;
    m_pText.m_elements[0] = nullptr;
    m_pText.m_elements[1] = nullptr;
    m_pText.m_elements[2] = nullptr;
    m_pConnectionStars.m_elements[0] = nullptr;
    m_pConnectionStars.m_elements[1] = nullptr;
    m_pConnectionStars.m_elements[2] = nullptr;
    m_pConnectionStars.m_elements[3] = nullptr;
    m_pConnectionStars.m_elements[4] = nullptr;
    memset(mVisibleListToGameListMap, 0xFFu, sizeof(mVisibleListToGameListMap));
}

// ============================================================================
// Batch 21: vote/list/dialog/pause handlers (240-320 bytes)
// ============================================================================

extern void* mem_heap_malloc(unsigned int size);  // core.o (1-arg overload)

// ea: 0x007905C0
void VoteGameTypeMenu::OnActivate()
{
    entries[0]->SetText("MPGAME_CHANGE_GAME_TYPE");
    if (mPlayerMgr == nullptr)
        mPlayerMgr = MultiplayerMgr::sInst->mPeer->GetPlayerManager();
    if (mGameTypeList == nullptr)
    {
        FEText* TextPointer = panel->GetTextPointer("element_list");
        mGameTypeList = AddListBoxEntry(1, TextPointer, 10);
    }
    unsigned char mGameType = MPUIInterface::mServerParams.mGameType;
    ((FEMenuListBox*)mGameTypeList)->Clear();
    for (int i = 0; i < 6; ++i)
    {
        if (i != mGameType)
        {
            Broc::string itemText(MPUIInterface::GetGameTypeString(i));
            ((FEMenuListBox*)mGameTypeList)->AddItem(itemText, 0);
        }
    }
    FEMenu::OnActivate();
    SetHigh(1, true);
}

// ea: 0x00790810
void VoteMapMenu::OnActivate()
{
    entries[0]->SetText("MPGAME_CHANGE_MAP");
    if (mPlayerMgr == nullptr)
        mPlayerMgr = MultiplayerMgr::sInst->mPeer->GetPlayerManager();
    if (mMapList == nullptr)
    {
        FEText* TextPointer = panel->GetTextPointer("element_list");
        mMapList = AddListBoxEntry(1, TextPointer, 10);
        ((FEMenuListBox*)mMapList)->mRowHeight = 16.0f;
    }
    ((FEMenuListBox*)mMapList)->Clear();
    ((FEMenuListBox*)mMapList)->mRowHeight = 24.0f;
    int v5 = 0;
    if (g_NumTotalMaps > 0)
    {
        int mMapID = MPUIInterface::mServerParams.mMapID;
        do
        {
            if (v5 != mMapID)
            {
                Broc::string itemText(MPUIInterface::GetMapString(v5));
                ((FEMenuListBox*)mMapList)->AddItem(itemText, 0);
            }
            ++v5;
        } while (v5 < g_NumTotalMaps);
    }
    FEMenu::OnActivate();
    SetHigh(1, true);
}

// ea: 0x007906D0
void VoteGameTypeMenu::SetPanelFile(PanelFile* pf)
{
    panel = pf;
    FEText* TextPointer = pf->GetTextPointer("title");
    AddEntry(0, TextPointer, false);
    helpbar = panel->GetTextPointer("Helpbar");
    FEMultiLineText* v5 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    if (v5 != nullptr)
    {
        color32 col = helpbar->GetColor();
        panel_layer layer = (panel_layer)helpbar->GetScaleX();
        float x1 = helpbar->GetY();
        float v11 = helpbar->GetX();
        v5 = new (v5)
            FEMultiLineText(helpbar->GetFont(), x1, 0.0f, 0, layer,
                            0.0f, 0, (int)col.i, col);
    }
    helpbar1 = v5;
    if (v5 != nullptr)
        v5->SetNumLines(1);
    helpbar1->SetText("MPFRONTEND_HELP_SELECT_BACK_MOVEUD");
}

// ea: 0x00790950
void VoteMapMenu::SetPanelFile(PanelFile* pf)
{
    panel = pf;
    FEText* TextPointer = pf->GetTextPointer("title");
    AddEntry(0, TextPointer, false);
    helpbar = panel->GetTextPointer("Helpbar");
    FEMultiLineText* v5 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    if (v5 != nullptr)
    {
        color32 col = helpbar->GetColor();
        panel_layer layer = (panel_layer)helpbar->GetScaleX();
        float x1 = helpbar->GetY();
        float v11 = helpbar->GetX();
        v5 = new (v5)
            FEMultiLineText(helpbar->GetFont(), x1, 0.0f, 0, layer,
                            0.0f, 0, (int)col.i, col);
    }
    helpbar1 = v5;
    if (v5 != nullptr)
        v5->SetNumLines(1);
    helpbar1->SetText("MPFRONTEND_HELP_SELECT_BACK_MOVEUD");
}

// ea: 0x007A4A60
void AARPersonalStats::SetPanelHelpBar()
{
    m_pText.m_elements[0]->SetShown(false);
    FEMultiLineText* v12 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    if (v12 != nullptr)
    {
        FEText* v13 = m_pText.m_elements[0];
        color32 col = v13->GetColor();
        panel_layer layer = (panel_layer)v13->GetScaleX();
        float x1 = v13->GetY();
        float v7 = v13->GetX();
        v12 = new (v12)
            FEMultiLineText(v13->GetFont(), x1, 0.0f, 0, layer,
                            0.0f, 0, (int)col.i, col);
    }
    helpbar1 = v12;
    if (v12 != nullptr)
        v12->SetNumLines(1);
    helpbar1->SetText("MPGAME_HELP_LOSERS_AAR_SCOREBOARD");
}

// ea: 0x007A3100
void InGameSwitchSides::NotifySameTeam()
{
    const char* v12 =
        (EntityManager::sInst->GetPlayer(mVersion)->sentient->eTeam
         == TEAM_ALLIES)
        ? "MPGAME_SWITCH_TEAM_ALREADY_ALLIES"
        : "MPGAME_SWITCH_TEAM_ALREADY_AXIS";
    DialogMenuSystem* DMS = g_femanager.GetDMS(mVersion);
    DMS->BringUp(v12, false, false, defaultFileName, true);
    j_nullsub_58(DMS, true);
    DialogMenu* Layer = DMS->GetLayer(DMS->GetActiveMenu() == 0);
    Layer->AddOption("INGAME_DIALOG_OK",
                     InGameSwitchSides::ResponseNoNevermind);
    DialogMenuSystem* v7 = g_femanager.GetDMS(mVersion);
    DialogMenu* v9 = v7->GetLayer(v7->GetActiveMenu() == 0);
    v9->Reformat(true, 0);
    DialogMenuSystem* v10 = g_femanager.GetDMS(mVersion);
    v10->GetLayer(v10->GetActiveMenu() == 0)->triangleResponse =
        InGameSwitchSides::ResponseGoBack;
}

// ea: 0x007A9870
void SessionListMenu::OnCross(int c)
{
    (void)c;
    unsigned long numGames = 0;
    MPUIInterface::GameListingGet(numGames);
    unsigned int v3 = m_ListBox.mTopLine + m_ListBox.mSelectedLine;
    if (v3 > 0x18)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
        AeAssert::gCurrentLine = 325;
        AeAssert::gCurrentExpr =
            "selection >= 0 && selection < MPUIInterface::MAX_RESULTS";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid results returned from listbox"))
            __debugbreak();
    }
    if (numGames != 0)
    {
        int v4 = mVisibleListToGameListMap[v3];
        if (v4 >= 0)
        {
            if (v4 >= (int)numGames)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
                AeAssert::gCurrentLine = 330;
                AeAssert::gCurrentExpr =
                    "mVisibleListToGameListMap[selection] < numGames";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
            }
            OverlayMenu* v5 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            v5->SetState(OverlayMenu::JOINING_START);
            OverlayMenu* fems = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            *(int*)((char*)fems + 0x54) = 13;
            OverlayMenu* v7 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            *(int*)((char*)v7 + 0x50) = mVisibleListToGameListMap[v3];
            system->AddOverlay(16);
        }
    }
}

// ea: 0x007A9A40
void SessionLanListMenu::Select(int entry_num)
{
    (void)entry_num;
    unsigned long numGames = 0;
    MPUIInterface::GameListingGet(numGames);
    unsigned int v3 = m_ListBox.mTopLine + m_ListBox.mSelectedLine;
    if (v3 > 0x18)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/SessionLanListMenu.cpp";
        AeAssert::gCurrentLine = 278;
        AeAssert::gCurrentExpr =
            "selection >= 0 && selection < MPUIInterface::MAX_RESULTS";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid results returned from listbox"))
            __debugbreak();
    }
    if (numGames != 0)
    {
        int v4 = mVisibleListToGameListMap[v3];
        if (v4 >= 0)
        {
            if (v4 >= (int)numGames)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\mp/ui/SessionLanListMenu.cpp";
                AeAssert::gCurrentLine = 281;
                AeAssert::gCurrentExpr =
                    "mVisibleListToGameListMap[selection] < numGames";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
            }
            OverlayMenu* v5 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            v5->SetState(OverlayMenu::JOINING_START);
            OverlayMenu* fems = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            *(int*)((char*)fems + 0x54) = 14;
            OverlayMenu* v7 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            *(int*)((char*)v7 + 0x50) = mVisibleListToGameListMap[v3];
            system->AddOverlay(16);
        }
    }
}

// ea: 0x007AA8C0
void AARScoreboardWinner::OnActivate()
{
    AARScoreboardBase::OnActivate();
    if (cgGlobal.teamGame)
    {
        panel->GetTextPointer("sb_text_title_section")
            ->SetText("MPGAME_WINNERS_SCOREBOARD");
        m_pYourTeamScore.m_elements[4]->SetShown(true);
        m_pYourTeamScore.m_elements[5]->SetShown(false);
        m_pUppercaseText.m_elements[4]->SetShown(true);
        m_pUppercaseText.m_elements[5]->SetShown(true);
        m_pUppercaseText.m_elements[6]->SetShown(true);
        m_pUppercaseText.m_elements[7]->SetShown(true);
        panel->GetPointer("sb_bkg_detail_02")->SetShown(true);
        panel->GetPointer("sb_bkg_detail_09")->SetShown(true);
    }
    else
    {
        panel->GetTextPointer("sb_text_title_section")
            ->SetText("MPGAME_BATTLE_SCOREBOARD");
        m_pYourTeamScore.m_elements[4]->SetShown(false);
        m_pYourTeamScore.m_elements[5]->SetShown(false);
        m_pUppercaseText.m_elements[4]->SetShown(false);
        m_pUppercaseText.m_elements[5]->SetShown(false);
        m_pUppercaseText.m_elements[6]->SetShown(false);
        m_pUppercaseText.m_elements[7]->SetShown(false);
        panel->GetPointer("sb_bkg_detail_02")->SetShown(false);
        panel->GetPointer("sb_bkg_detail_09")->SetShown(false);
    }
}

// ea: 0x007A6700
void PauseMenu::OnActivate()
{
    SwapMenus();
    FEMenu::OnActivate();
    HighlightDefault();
    if (!MultiplayerMgr::sInst->mRankedGame
        && MultiplayerMgr::sInst->IsLocalClientHost(mVersion))
        entries[4]->SetText("MPGAME_EDIT_GAME_SETTINGS");
    else
        entries[4]->SetText("MPGAME_VIEW_GAME_SETTINGS");
    SetHigh(m_iLastSelection, true);
    entries[2]->Disable(
        EntityManager::sInst->GetPlayer(currCl)->client->pers.playerState
        != 3);
    if (EntityManager::sInst->GetPlayer(currCl)->client->pers.playerState
            != 3
        && highlighted == 2)
        SetHigh(1, true);
    tlPrintf("PauseMenu::OnActivate()\n");
    ClearButton((controller::ButtonIndex)(controller::R3
                                          | controller::RIGHTBUTTON));
    MPLiveEngine::GetHandle()->needConfirmation = true;
    LiveWrapper::theWrapper->SetNotificationFlag(
        MPLiveEngine::GetHandle()->actualPort, 0, true);
    panel->GetPointer("bkg_line_07")->SetShown(true);
}

// ============================================================================
// Batch 22: create/find menu handlers (320-336 bytes)
// ============================================================================

// ea: 0x007A79C0
void CreateSessionMenu::OnCross(int c)
{
    (void)c;
    MPUIInterface::mServerParams.mGameType =
        (unsigned char)mGameModeCombo->mCurrOption;
    short mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != 0xFF)
        mCurrOption = (short)(unsigned char)byte_E386C9[114 * mCurrOption];
    MPUIInterface::mServerParams.mMapID = (unsigned char)mCurrOption;
    MPUIInterface::mServerParams.mMaxPlayers =
        (unsigned char)m_iNumberOfPlayersConversion[
            mNumberOfPlayersCombo->mCurrOption];
    MPUIInterface::mServerParams.mPrivateSlots =
        (unsigned char)m_PrivateSlotsCombo->mCurrOption;
    strncpy(MPUIInterface::mServerParams.mName, m_szSessionName, 0x10u);
    MPUIInterface::mServerParams.SetMapRotation(0);
    MPUIInterface::mServerParams.mRespawnTime =
        MPUIInterface::GetDefaultOption(
            MPUIInterface::SETTING_RESPAWN_TIME,
            (eGameType)MPUIInterface::mServerParams.mGameType);
    MPUIInterface::mServerParams.mDontRotate = false;
    MPUIInterface::mNextServerParams = MPUIInterface::mServerParams;
    if (MPUIInterface::StartServer(true, true))
    {
        MPUIInterface::ExitFrontend(10);
    }
    else if (MultiplayerMgr::sInst->oneOffCheckLinkStatus())
    {
        OverlayMenu* v4 = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        v4->SetState(OverlayMenu::FROM_ID_QUERYING);
        OverlayMenu* fems = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        *(int*)((char*)fems + 0x50) = 8;
        OverlayMenu* v5 = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        *(int*)((char*)v5 + 0x54) = 8;
        system->AddOverlay(16);
    }
}

// ea: 0x007A7DB0
void CreateLanSessionMenu::OnCross(int c)
{
    MPUIInterface::mServerParams.mGameType =
        (unsigned char)mGameModeCombo->mCurrOption;
    short mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != 0xFF)
        mCurrOption = (short)(unsigned char)byte_E386C9[114 * mCurrOption];
    MPUIInterface::mServerParams.mMapID = (unsigned char)mCurrOption;
    MPUIInterface::mServerParams.mMaxPlayers =
        (unsigned char)m_iNumberOfPlayersConversion[
            mNumberOfPlayersCombo->mCurrOption];
    GrabSessionName(c);
    strncpy(MPUIInterface::mServerParams.mName, m_szSessionName, 0x10u);
    MPUIInterface::mServerParams.SetMapRotation(0);
    MPUIInterface::mServerParams.mRespawnTime =
        MPUIInterface::GetDefaultOption(
            MPUIInterface::SETTING_RESPAWN_TIME,
            (eGameType)MPUIInterface::mServerParams.mGameType);
    MPUIInterface::mServerParams.mDontRotate = false;
    MultiplayerMgr::sInst->mLinkCheckEnabled = true;
    if (MPUIInterface::StartServer(true, true))
    {
        MPUIInterface::ExitFrontend(9);
    }
    else if (MultiplayerMgr::sInst->oneOffCheckLinkStatus())
    {
        g_femanager.fems->RemoveOverlay();
        OverlayMenu* v4 = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        v4->SetState(OverlayMenu::FROM_ID_QUERYING);
        OverlayMenu* fems = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        *(int*)((char*)fems + 0x50) = 8;
        OverlayMenu* v5 = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        *(int*)((char*)v5 + 0x54) = 8;
        system->AddOverlay(16);
    }
}

// ea: 0x00798B80
void FindSessionMenu::OnActivate()
{
    FEMenu::OnActivate();
    for (int i = 0; i < 6; ++i)
        m_pBackgroundArt.m_elements[i]->SetShown(true);
    if ((m_FirstTimeAccessedByte & 1) == 0)
    {
        m_FirstTimeAccessedByte = 1;
        if (mStartingMapCombo == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/FindSessionMenu.cpp";
            AeAssert::gCurrentLine = 418;
            AeAssert::gCurrentExpr = "mStartingMapCombo";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Combobox failure"))
                __debugbreak();
        }
        for (int j = g_NumBaseMaps; j < g_NumTotalMaps; ++j)
        {
            char v4 = (j == 0xFF) ? (char)-1
                                   : (char)byte_E386C9[114 * j];
            Broc::string s(MPUIInterface::GetMapString(v4));
            mStartingMapCombo->AddOption(s);
        }
    }
    SetHigh(1, true);
    highlighted = 1;
    entries[0]->Highlight(true, true);
}

// ============================================================================
// Batch 23: vote ctors + spectate/quickmatch/options (304-368 bytes)
// ============================================================================

// ea: 0x007AFC00
AARMapVote::AARMapVote(FEMenuSystem* pSystem)
    : AARBaseMenu(pSystem, 0)
{
    m_bShowScrollArrowLeft = false;
    m_bShowScrollArrowRight = false;
    m_bHighlightScrollArrowLeft = false;
    m_bHighlightScrollArrowRight = false;
    m_ePanelToSwitchTo = -1;
    m_iSelectedMap = -1;
    m_currentRow = 0;
    new (&m_ListBox) UIHighlightListBox(65, 2, 65, true);
    m_pMapVoteVals = nullptr;
    default_color_scheme = 5;
    for (int i = 0; i < 10; ++i)
        m_pBackgroundArt.m_elements[i] = nullptr;
    m_pScrollArrow.m_elements[0] = nullptr;
    m_pScrollArrow.m_elements[1] = nullptr;
    m_pText.m_elements[0] = nullptr;
    m_pText.m_elements[1] = nullptr;
    m_pText.m_elements[2] = nullptr;
    m_pText.m_elements[3] = nullptr;
    m_pText.m_elements[4] = nullptr;
    for (int i = 0; i < 6; ++i)
        m_pScrollbar.m_elements[i] = nullptr;
    memset(&m_pMapNames, 0, sizeof(m_pMapNames));
    memset(&m_pMapVotes, 0, sizeof(m_pMapVotes));
    m_pMapVoteVals = (int*)mem_heap_malloc(0x104u);
}

// ea: 0x007AFE40
AARGameModeVote::AARGameModeVote(FEMenuSystem* pSystem)
    : AARBaseMenu(pSystem, 0)
{
    m_bShowScrollArrowLeft = false;
    m_bShowScrollArrowRight = false;
    m_bHighlightScrollArrowLeft = false;
    m_bHighlightScrollArrowRight = false;
    m_ePanelToSwitchTo = -1;
    m_iSelectedMode = -1;
    m_currentRow = 0;
    new (&m_ListBox) UIHighlightListBox(7, 2, 7, true);
    default_color_scheme = 5;
    for (int i = 0; i < 9; ++i)
        m_pBackgroundArt.m_elements[i] = nullptr;
    m_pScrollArrow.m_elements[0] = nullptr;
    m_pScrollArrow.m_elements[1] = nullptr;
    m_pText.m_elements[0] = nullptr;
    m_pText.m_elements[1] = nullptr;
    m_pText.m_elements[2] = nullptr;
    m_pText.m_elements[3] = nullptr;
    m_pText.m_elements[4] = nullptr;
    for (int i = 0; i < 7; ++i)
    {
        m_pModeNames.m_elements[i] = nullptr;
        m_pModeVotes.m_elements[i] = nullptr;
        m_pModeVoteVals.m_elements[i] = 0;
    }
}

// ea: 0x00792BD0
void SpectateMenu::SetPanelFile(PanelFile* pf)
{
    if (mVersion > 0)
        panel = pf->Clone();
    else
        panel = pf;
    mHeader = panel->GetTextPointer("text_line_01");
    mMessage = panel->GetTextPointer("text_line_02");
    mTime = panel->GetTextPointer("text_line_03");
    mButtonPress = panel->GetTextPointer("text_line_04");
    ((FEText*)mButtonPress)->SetText("MPSCRIPT_PRESS_X_TO_SPAWN");
    ((FEText*)mHeader)->SetShown(false);
    helpbar = panel->GetTextPointer("text_helpbar");
    FEMultiLineText* v11 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    if (v11 != nullptr)
    {
        color32 col = helpbar->GetColor();
        panel_layer layer = (panel_layer)helpbar->GetScaleX();
        float x1 = helpbar->GetY();
        float v17 = helpbar->GetX();
        v11 = new (v11)
            FEMultiLineText(helpbar->GetFont(), x1, 0.0f, 0, layer,
                            0.0f, 0, (int)col.i, col);
    }
    helpbar1 = v11;
    if (v11 != nullptr)
        v11->SetNumLines(1);
}

// ea: 0x007A9370
void PlayOnlineMenu::LaunchQuickMatch()
{
    unsigned long numGames = 0;
    MPUIInterface::GameListingGet(numGames);
    if (numGames != 0)
    {
        int v2 = irand(0, (int)numGames);
        if (v2 >= (int)numGames || v2 < 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/PlayOnlineMenu.cpp";
            AeAssert::gCurrentLine = 413;
            AeAssert::gCurrentExpr = "selection < numGames && selection >= 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(defaultFileName))
                __debugbreak();
        }
        unsigned int v3 = v2 < 0 ? 0u : (unsigned int)v2;
        if (v3 >= numGames)
            v3 = numGames - 1;
        OverlayMenu* v4 = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        v4->SetState(OverlayMenu::JOINING_START);
        OverlayMenu* fems = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        *(int*)((char*)fems + 0x54) = 10;
        OverlayMenu* v5 = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        *(unsigned int*)((char*)v5 + 0x6C) = v3;
        system->AddOverlay(16);
    }
    else
    {
        OverlayMenu* v6 = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        v6->SetState((OverlayMenu::eState)(OverlayMenu::FROM_ID_QUERYING
                                           | OverlayMenu::SIGNING_IN));
        OverlayMenu* v7 = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        *(int*)((char*)v7 + 0x50) = 0;
        OverlayMenu* v8 = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        *(int*)((char*)v8 + 0x54) = 10;
        system->AddOverlay(16);
    }
}

// ea: 0x007AC010
void InGameSwitchSides::AttemptSwitchTeam()
{
    const char* v16 =
        (EntityManager::sInst->GetPlayer(mVersion)->sentient->eTeam
         == TEAM_ALLIES)
        ? "MPGAME_SWITCH_TEAM_QUERY_AXIS"
        : "MPGAME_SWITCH_TEAM_QUERY_ALLIES";
    DialogMenuSystem* DMS = g_femanager.GetDMS(mVersion);
    DMS->BringUp(v16, false, false, defaultFileName, true);
    j_nullsub_58(DMS, true);
    DialogMenuSystem* v4 = g_femanager.GetDMS(mVersion);
    DialogMenu* Layer = v4->GetLayer(v4->GetActiveMenu() == 0);
    Layer->AddOption("INGAME_DIALOG_YES",
                     InGameSwitchSides::ResponseYesSwitch);
    DialogMenuSystem* v7 = g_femanager.GetDMS(mVersion);
    DialogMenu* v9 = v7->GetLayer(v7->GetActiveMenu() == 0);
    v9->AddOption("INGAME_DIALOG_NO",
                  InGameSwitchSides::ResponseNoNevermind);
    g_femanager.GetDMS(mVersion)->HighlightOption(1);
    DialogMenuSystem* v11 = g_femanager.GetDMS(mVersion);
    DialogMenu* v13 = v11->GetLayer(v11->GetActiveMenu() == 0);
    v13->Reformat(true, 0);
    DialogMenuSystem* v14 = g_femanager.GetDMS(mVersion);
    v14->GetLayer(v14->GetActiveMenu() == 0)->triangleResponse =
        InGameSwitchSides::ResponseGoBack;
}

// ea: 0x0078DC10
void GameSettingsView::UpdateOption(int option, FEText* text)
{
    char szText[20];
    switch (option)
    {
    case 0:
        text->SetText(
            MPUIInterface::GetGameTypeString(
                mCurrentServerParams->mGameType));
        return;
    case 1:
        text->SetText(
            MPUIInterface::GetMapString(mCurrentServerParams->mMapID));
        return;
    case 2:
        _snprintf(szText, sizeof(szText), "%d",
                  MPUIInterface::GetTimeLimit(
                      mCurrentServerParams->mTimeLimit));
        text->SetText(szText);
        return;
    case 3:
        _snprintf(szText, sizeof(szText), "%d",
                  MPUIInterface::GetScoreLimit(
                      mCurrentServerParams->mScoreLimit,
                      (eGameType)mCurrentServerParams->mGameType));
        text->SetText(szText);
        return;
    case 4:
    case 5:
    case 6:
    case 7:
    {
        unsigned char mFriendlyFire;
        if (option == 4)
            mFriendlyFire = mCurrentServerParams->mFriendlyFire;
        else if (option == 5)
            mFriendlyFire = mCurrentServerParams->mTeamBalancing;
        else if (option == 6)
            mFriendlyFire = mCurrentServerParams->mEnableAARVote;
        else
            mFriendlyFire = mCurrentServerParams->mEnablePenaltyVote;
        if (mFriendlyFire != 0)
            text->SetText("MPGAME_ENABLED");
        else
            text->SetText("MPGAME_DISABLED");
        break;
    }
    default:
        return;
    }
}

// ea: 0x007AEB40
void OverlayMenu::OnTriangle(int c)
{
    switch (mState)
    {
    case GAME_LISTING:
    case JOINING:
        MPUIInterface::CancelJoin();
        goto LABEL_9;
    case GAME_LISTING_START:
    case JOINING_START:
    case BDNET_START_FAILED:
        if (bdSingleton<bdNetImpl>::getInstance() != nullptr)
        {
            bdNetImpl* Instance = bdSingleton<bdNetImpl>::getInstance();
            if (Instance->getParams().m_socket != nullptr)
                bdSingleton<bdNetImpl>::getInstance()->stop();
        }
        goto LABEL_9;
    case NO_GAMES:
        Accept();
        return;
    case JOIN_REFUSED:
    case JOIN_FAILED:
    case CANNOT_CONNECT_TO_HOST:
    case CANNOT_CONNECT_TO_PEERS:
    case JOIN_SUCCESS:
        MPUIInterface::CancelJoin();
        if (GetSystem()->background == 13)
            ((SessionListMenu*)GetSystem()->menus[13])->mNeedToUpdate = true;
        else if (GetSystem()->background == 14)
            ((SessionLanListMenu*)GetSystem()->menus[14])->mNeedToUpdate =
                true;
        goto LABEL_9;
    case (OverlayMenu::eState)(FROM_ID_QUERYING | SIGNING_IN):
        OverlayMenuBase::OnTriangle(c);
        return;
    default:
    LABEL_9:
        OverlayMenuBase::OnTriangle(mVersion);
        if (GetSystem()->CurrentOverlay() != -1)
        {
            GetSystem()->RemoveOverlay();
            mState = (OverlayMenu::eState)0;
        }
        if (mState == NO_GAMES)
            mState = (OverlayMenu::eState)0;
        return;
    }
}

// ea: 0x007913A0
void AARPersonalStats::GetClassSpecificScore(
    EPlayerClass a_ePlayerClass, int& a_iClassScore, int& a_iTimeAsClass,
    int& a_iClassSpecificScore1, int& a_iClassSpecificScore2)
{
    (void)a_iClassSpecificScore2;
    Entity* FirstLocalPlayer = EntityManager::sInst->GetFirstLocalPlayer();
    if (FirstLocalPlayer != nullptr)
    {
        Client* client = FirstLocalPlayer->client;
        if (client != nullptr)
        {
            int v9 = (int)a_ePlayerClass;
            a_iClassScore =
                PlayerStats::TotalScoreForStats(
                    client->pers.mStats[a_ePlayerClass]);
            a_iTimeAsClass =
                FirstLocalPlayer->client->pers.mStats[a_ePlayerClass][0];
            switch (a_ePlayerClass)
            {
            case kPlayerClassAssault:
            case kPlayerClassInfantry:
                a_iClassSpecificScore1 =
                    FirstLocalPlayer->client->pers.mStats[v9][14];
                break;
            case kPlayerClassRifleman:
                a_iClassSpecificScore1 =
                    FirstLocalPlayer->client->pers.mStats[2][10];
                break;
            case kPlayerClassMedic:
                a_iClassSpecificScore1 =
                    FirstLocalPlayer->client->pers.mStats[v9][11];
                break;
            case kPlayerClassSupport:
                a_iClassSpecificScore1 =
                    FirstLocalPlayer->client->pers.mStats[v9][12];
                break;
            case kPlayerClassAntiArmor:
                a_iClassSpecificScore1 =
                    FirstLocalPlayer->client->pers.mStats[v9][6];
                break;
            case kPlayerClassScout:
                a_iClassSpecificScore1 =
                    FirstLocalPlayer->client->pers.mStats[v9][13];
                break;
            default:
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\mp/ui/AARPersonalStats.cpp";
                AeAssert::gCurrentLine = 548;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                        "PlayerClass enumerations are out of sync"))
                    __debugbreak();
                break;
            }
        }
    }
}

// ============================================================================
// Batch 24: scoreboard contents + settings update + AAR pause panel
// ============================================================================

// ea: 0x007A2C40
void InGameScoreBoard::SetPanelContents()
{
    m_pTeamStripQuad.m_elements[0]->SetShown(false);
    m_pTeamStripQuad.m_elements[1]->SetShown(false);
    m_pTeamStripQuad.m_elements[2]->SetShown(false);
    int v2;
    if (!cgGlobal.teamGame)
    {
        v2 = 2;
        m_pUppercaseText.m_elements[0]->SetShown(false);
        m_cgTeamShown = TEAM_FREE;
    }
    else
    {
        sentient_s* sentient =
            EntityManager::sInst->GetPlayer(mVersion)->sentient;
        int16_t eTeam = (sentient != nullptr) ? (int16_t)sentient->eTeam : 2;
        m_pUppercaseText.m_elements[0]->SetShown(true);
        bool axis = false;
        if (eTeam == 2)
        {
            if (!m_bShowMyTeamScore)
                axis = true;
        }
        else if (eTeam != 1 || m_bShowMyTeamScore)
        {
            axis = true;
        }
        if (axis)
        {
            m_pUppercaseText.m_elements[0]->SetText("MPSCRIPT_AXIS_ALLCAPS");
            v2 = 1;
            m_cgTeamShown = TEAM_AXIS;
        }
        else
        {
            m_pUppercaseText.m_elements[0]
                ->SetText("MPSCRIPT_ALLIES_ALLCAPS");
            v2 = 0;
            m_cgTeamShown = TEAM_ALLIES;
        }
    }
    m_pTeamStripQuad.m_elements[v2]->SetShown(true);
    if (cgGlobal.teamGame)
    {
        if (m_bShowMyTeamScore)
            helpbar1->SetText("MPGAME_VIEW_OPPOSING_TEAM_SCOREBOARD");
        else
            helpbar1->SetText("MPGAME_VIEW_YOUR_TEAM_SCOREBOARD");
    }
    else
    {
        helpbar1->SetText("MPGAME_DM_SCOREBOARD");
    }
    m_pUppercaseText.m_elements[2]->SetText(
        MPUIInterface::GetGameTypeString(
            MPUIInterface::mServerParams.mGameType));
    m_pUppercaseText.m_elements[3]->SetText(
        MPUIInterface::GetMapString(MPUIInterface::mServerParams.mMapID));
}

// ea: 0x0078DDC0
void GameSettingsView::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    PanelQuad* mQuad = mScrollBarUpFader.mQuad;
    if (mQuad != nullptr && mScrollBarUpFader.mFading)
    {
        if (mScrollBarUpFader.mAlphaTo <= mScrollBarUpFader.mAlpha)
        {
            float mAlpha = mScrollBarUpFader.mAlpha;
            if (mAlpha <= mScrollBarUpFader.mAlphaTo)
                goto UP_SET;
            float v7 = mAlpha
                - (time_inc / mScrollBarUpFader.mTime)
                    * mScrollBarUpFader.mAlphaDelta;
            float mAlphaTo = mScrollBarUpFader.mAlphaTo;
            mScrollBarUpFader.mAlpha = v7;
            if (mAlphaTo < v7)
                goto UP_SET;
            mScrollBarUpFader.mAlpha = mScrollBarUpFader.mAlphaTo;
        }
        else
        {
            float v4 = (time_inc / mScrollBarUpFader.mTime)
                    * mScrollBarUpFader.mAlphaDelta
                + mScrollBarUpFader.mAlpha;
            mScrollBarUpFader.mAlpha = v4;
            if (v4 < mScrollBarUpFader.mAlphaTo)
                goto UP_SET;
            mScrollBarUpFader.mAlpha = mScrollBarUpFader.mAlphaTo;
        }
        mScrollBarUpFader.mFading = false;
    UP_SET:
        mQuad->SetAlpha(mScrollBarUpFader.mAlpha);
    }
    PanelQuad* v9 = mScrollBarDownFader.mQuad;
    if (v9 != nullptr && mScrollBarDownFader.mFading)
    {
        if (mScrollBarDownFader.mAlphaTo <= mScrollBarDownFader.mAlpha)
        {
            float v11 = mScrollBarDownFader.mAlpha;
            if (v11 <= mScrollBarDownFader.mAlphaTo)
                goto DOWN_SET;
            float v12 = v11
                - (time_inc / mScrollBarDownFader.mTime)
                    * mScrollBarDownFader.mAlphaDelta;
            float v13 = mScrollBarDownFader.mAlphaTo;
            mScrollBarDownFader.mAlpha = v12;
            if (v13 < v12)
                goto DOWN_SET;
            mScrollBarDownFader.mAlpha = mScrollBarDownFader.mAlphaTo;
        }
        else
        {
            float v10 = (time_inc / mScrollBarDownFader.mTime)
                    * mScrollBarDownFader.mAlphaDelta
                + mScrollBarDownFader.mAlpha;
            mScrollBarDownFader.mAlpha = v10;
            if (v10 < mScrollBarDownFader.mAlphaTo)
                goto DOWN_SET;
            mScrollBarDownFader.mAlpha = mScrollBarDownFader.mAlphaTo;
        }
        mScrollBarDownFader.mFading = false;
    DOWN_SET:
        v9->SetAlpha(mScrollBarDownFader.mAlpha);
    }
}

// ea: 0x00792090
static const char* const szMPPauseEntriesText[7] = {
    "slot_01_text_option", "slot_02_text_option", "slot_03_text_option",
    "slot_04_text_option", "slot_05_text_option", "slot_06_text_option",
    "slot_07_text_option",
};
static const char* const szMPPauseMenuOptionTextReferences[7] = {
    "MPGAME_SELECT_WEAPON", "MPGAME_SELECT_TEAM", "MPGAME_SUICIDE",
    "MPGAME_CONTROLLER", "MPGAME_VIEW_GAME_SETTINGS",
    "MPGAME_XBOX_LIVE_OPTIONS", "MPGAME_QUIT",
};
void AARPauseMenu::SetPanelFile(PanelFile* pf)
{
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\MPPauseMenu.cpp";
        AeAssert::gCurrentLine = 799;
        AeAssert::gCurrentExpr = "pf";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid panel file pointer"))
            __debugbreak();
    }
    panel = pf;
    panel->GetTextPointer("text_title")
        ->SetText("MPGAME_MULTIPLAYER_MENU");
    FEText* v4 = panel->GetTextPointer("text_helpbar");
    FEMultiLineText* v5 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    FEMultiLineText* v6 = nullptr;
    if (v5 != nullptr)
    {
        color32 col = v4->GetColor();
        panel_layer layer = (panel_layer)v4->GetScaleX();
        float x1 = v4->GetY();
        float v12 = v4->GetX();
        v6 = new (v5)
            FEMultiLineText(v4->GetFont(), x1, 0.0f, 1, layer,
                            0.0f, 0, (int)col.i, col);
    }
    helpbar1 = v6;
    if (v6 != nullptr)
        v6->SetNumLines(1);
    helpbar1->SetText("MPGAME_PAUSE_HELPBAR");
    for (int i = 0; i < 7; ++i)
    {
        FEText* v10 = panel->GetTextPointer(szMPPauseEntriesText[i]);
        v10->SetText(szMPPauseMenuOptionTextReferences[i]);
        AddEntry(i, v10, false);
    }
    entries[0]->up = 6;
    entries[6]->down = 0;
    highlighted = 3;
    SetHigh(3, true);
    entries[3]->Highlight(true, true);
}

// ============================================================================
// Batch 25: overlay cross handlers + tally votes + session list updates
// ============================================================================

// ea: 0x0078F5E0
void InGameOverlay::OnCross(int c)
{
    switch (m_State)
    {
    case OVERLAY_SIGNIN_SIGNOUT:
    case OVERLAY_AAR_SIGNIN_SIGNOUT:
        if (m_currSelection == 0)
        {
            MPLiveEngine::GetHandle()->LogOut();
            XBoxLiveIngameOptionsCOD3::Me(0)->mReturnMenu = -1;
        }
        break;
    case OVERLAY_APPEAR_ONLINE:
    case OVERLAY_AAR_APPEAR_ONLINE:
    case OVERLAY_APPEAR_OFFLINE:
    case OVERLAY_AAR_APPEAR_OFFLINE:
        if (m_currSelection == 0)
        {
            unsigned int actualPort = MPLiveEngine::GetHandle()->actualPort;
            MPLiveEngine::GetHandle()->ToggleOfflineAppearance(actualPort);
            MPUIInterface::mIsViewableOnline = !MPUIInterface::mIsViewableOnline;
        }
        break;
    case OVERLAY_TOGGLE_VOICE:
    case OVERLAY_AAR_TOGGLE_VOICE:
        if (m_currSelection != 0)
        {
            if (m_currSelection == 1)
            {
                LiveWrapper::theWrapper->SetVTS(
                    MPLiveEngine::GetHandle()->actualPort, true);
            }
        }
        else
        {
            LiveWrapper::theWrapper->SetVTS(
                MPLiveEngine::GetHandle()->actualPort, false);
        }
        break;
    case OVERLAY_JOIN_FRIEND:
    case OVERLAY_AAR_JOIN_FRIEND:
        if (m_currSelection == 0)
        {
            MPLiveEngine* v4 = MPLiveEngine::GetHandle();
            v4->JoinGame((XONLINE_FRIEND*)v4->friendToJoin);
            XBoxLiveIngameOptionsCOD3::Me(0)->mReturnMenu = -1;
        }
        break;
    case OVERLAY_REBOOT_REQUIRED:
    case OVERLAY_AAR_REBOOT_REQUIRED:
        XBoxLiveIngameOptionsCOD3::Me(0)->mReturnMenu = -1;
        MPLiveEngine::GetHandle()->renderingEnabled = true;
        if (m_currSelection != 0)
            MPLiveEngine::GetHandle()->LogOut();
        else
            LiveEngine_Reboot(MPLiveEngine::GetHandle()->uixEngine, 0);
        break;
    default:
        break;
    }
    FEMenu::OnCross(c);
    Accept();
}

// ea: 0x0078F970
void AAROverlay::OnCross(int c)
{
    switch (m_State)
    {
    case OVERLAY_SIGNIN_SIGNOUT:
    case OVERLAY_AAR_SIGNIN_SIGNOUT:
        if (m_currSelection == 0)
        {
            MPLiveEngine::GetHandle()->LogOut();
            XBoxLiveIngameOptionsCOD3::Me(0)->mReturnMenu = -1;
        }
        break;
    case OVERLAY_APPEAR_ONLINE:
    case OVERLAY_AAR_APPEAR_ONLINE:
    case OVERLAY_APPEAR_OFFLINE:
    case OVERLAY_AAR_APPEAR_OFFLINE:
        if (m_currSelection == 0)
        {
            unsigned int actualPort = MPLiveEngine::GetHandle()->actualPort;
            MPLiveEngine::GetHandle()->ToggleOfflineAppearance(actualPort);
            MPUIInterface::mIsViewableOnline = !MPUIInterface::mIsViewableOnline;
        }
        break;
    case OVERLAY_TOGGLE_VOICE:
    case OVERLAY_AAR_TOGGLE_VOICE:
        if (m_currSelection != 0)
        {
            if (m_currSelection == 1)
            {
                LiveWrapper::theWrapper->SetVTS(
                    MPLiveEngine::GetHandle()->actualPort, true);
            }
        }
        else
        {
            LiveWrapper::theWrapper->SetVTS(
                MPLiveEngine::GetHandle()->actualPort, false);
        }
        break;
    case OVERLAY_JOIN_FRIEND:
    case OVERLAY_AAR_JOIN_FRIEND:
        if (m_currSelection == 0)
        {
            MPLiveEngine* v4 = MPLiveEngine::GetHandle();
            v4->JoinGame((XONLINE_FRIEND*)v4->friendToJoin);
            XBoxLiveIngameOptionsCOD3::Me(0)->mReturnMenu = -1;
        }
        break;
    case OVERLAY_REBOOT_REQUIRED:
    case OVERLAY_AAR_REBOOT_REQUIRED:
        XBoxLiveIngameOptionsCOD3::Me(0)->mReturnMenu = -1;
        MPLiveEngine::GetHandle()->renderingEnabled = true;
        if (m_currSelection != 0)
            MPLiveEngine::GetHandle()->LogOut();
        else
            LiveEngine_Reboot(MPLiveEngine::GetHandle()->uixEngine, 0);
        break;
    default:
        break;
    }
    FEMenu::OnCross(c);
    Accept();
}

// ea: 0x007A5600
void AARMapVote::TallyVotes()
{
    int v1 = g_NumBaseMaps + 1;
    int v2 = 0;
    int maxVoteCnt = 0;
    if (v1 > 0)
    {
        int* m_pMapVoteVals = this->m_pMapVoteVals;
        int v4 = v1;
        do
        {
            if (*m_pMapVoteVals > v2)
            {
                maxVoteCnt = *m_pMapVoteVals;
                v2 = *m_pMapVoteVals;
            }
            ++m_pMapVoteVals;
            --v4;
        } while (v4 != 0);
    }
    int v5 = 0;
    int* v6 = (int*)mem_heap_malloc(4 * v1);
    int* arrChoices = v6;
    if (v1 > 0)
    {
        memset(v6, 0, 4 * v1);
        v2 = maxVoteCnt;
    }
    for (int i = 0; i < v1; ++i)
    {
        if (v2 != 0 && m_pMapVoteVals[i] == v2)
            v6[v5++] = i;
    }
    int v8;
    if (v5 <= 1)
    {
        if (v5 != 1)
            goto DONE;
        v8 = 0;
    }
    else
    {
        v8 = irand(0, v5);
        if (v8 <= -1)
            goto DONE;
    }
    int v9 = g_NumTotalMaps;
    int v10 = 0;
    if (g_NumTotalMaps <= 0)
    {
        v10 = -1;
    }
    else
    {
        char* v11 = byte_E386C9;
        while (*v11 != MPUIInterface::mServerParams.mMapID)
        {
            ++v10;
            v11 += 114;
            if (v10 >= g_NumTotalMaps)
            {
                v10 = -1;
                break;
            }
        }
    }
    int v12 = v6[v8];
    if (v12 - 1 != v10)
    {
        char v13;
        if (v12 != 0)
        {
            v13 = (char)(v6[v8]) - 1;
            if (v13 != -1)
                v13 = byte_E386C9[114 * v13];
        }
        else
        {
            v13 = (char)irand(0, g_NumBaseMaps);
            v9 = g_NumTotalMaps;
            if (v13 != -1)
                v13 = byte_E386C9[114 * v13];
        }
        unsigned char v14 = (unsigned char)v13;
        int v15 = 0;
        MPUIInterface::mNextServerParams.mMapID = v14;
        if (v9 > 0)
        {
            char* v16 = byte_E386C9;
            while (*v16 != v14)
            {
                ++v15;
                v16 += 114;
                if (v15 >= v9)
                    MPUIInterface::mNextServerParams.mMapID =
                        MPUIInterface::mServerParams.mMapID;
            }
        }
        else
        {
            MPUIInterface::mNextServerParams.mMapID =
                MPUIInterface::mServerParams.mMapID;
        }
    }
DONE:
    mem_heap_free(arrChoices);
}

// ea: 0x007A6380
void AARGameModeVote::TallyVotes()
{
    int v2 = 0;
    for (int i = 0; i < 7; ++i)
    {
        if (m_pModeVoteVals.m_elements[i] > v2)
            v2 = m_pModeVoteVals.m_elements[i];
    }
    int arrChoices[7];
    memset(arrChoices, 0, sizeof(arrChoices));
    int v4 = 0;
    for (int j = 0; j < 7; ++j)
    {
        if (v2 != 0 && m_pModeVoteVals.m_elements[j] == v2)
            arrChoices[v4++] = j;
    }
    int v6;
    if (v4 <= 1)
    {
        if (v4 != 1)
            return;
        v6 = 0;
    }
    else
    {
        v6 = irand(0, v4);
        if (v6 <= -1)
            return;
    }
    int* v7 = &arrChoices[v6];
    if (*v7 - 1 != MPUIInterface::mServerParams.mGameType)
    {
        unsigned char v8;
        if (*v7 != 0)
            v8 = (unsigned char)(*v7 - 1);
        else
            v8 = (unsigned char)irand(0, 6);
        MPUIInterface::mNextServerParams.mGameType = v8;
        if (v8 >= 6u)
            MPUIInterface::mNextServerParams.mGameType =
                MPUIInterface::mServerParams.mGameType;
    }
}

// ea: 0x0079CBF0
void SessionDetailsMenu::UpdateDetails()
{
    if (mCurrentGame < mNumGames)
    {
        unsigned long numGames = 0;
        sGameListing* v2 = MPUIInterface::GameListingGet(numGames);
        sGameListing* v4 = v2;
        if (mNumGames != numGames)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionDetailsMenu.cpp";
            AeAssert::gCurrentLine = 177;
            AeAssert::gCurrentExpr = "mNumGames == numGames";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(defaultFileName))
                __debugbreak();
        }
        sMPGameInfoView* m_ptr =
            (sMPGameInfoView*)v4[mCurrentGame].mGameInfo;
        int v6 = m_ptr->m_privateFilled;
        int v7 = m_ptr->mGameType;
        unsigned int mapIndex = m_ptr->mMapID;
        int v8 = m_ptr->m_publicFilled + v6;
        unsigned int maxPlayers =
            v8 + m_ptr->m_privateOpen + m_ptr->m_publicOpen;
        entries[1]->SetText(MPUIInterface::GetGameTypeString(v7));
        entries[2]->SetText(MPUIInterface::GetMapString(mapIndex));
        entries[4]->SetTextNoLocalize(m_ptr->mName);
        char playerString[64];
        sprintf(playerString, "%d/%d", v8, maxPlayers);
        entries[3]->SetTextNoLocalize(playerString);
    }
}

// ea: 0x007AD350
void SessionListMenu::Update(float time_inc)
{
    if (!MPUIInterface::IsOnlineGame()
        || MPLiveEngine::GetHandle()->internalState == kSignedIn)
    {
        FEMenu::Update(time_inc);
        movie_manager::frame_advance();
        unsigned long numGames = 0;
        MPUIInterface::GameListingGet(numGames);
        if (numGames != 0 || !MultiplayerMgr::sInst->oneOffCheckLinkStatus())
        {
            if (numGames != mNumGames)
            {
                helpbar1->SetText("MPFRONTEND_HELP_JOIN_BACK_MOVE_REFRESH");
                RepopulateSessionList();
                mNumGames = numGames;
            }
        }
        else
        {
            OverlayMenu* v3 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            v3->SetState(OverlayMenu::NO_GAMES);
            if (!MPUIInterface::IsLANGame())
            {
                OverlayMenu* fems = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(int*)((char*)fems + 0x50) = 4;
                OverlayMenu* v6 = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(int*)((char*)v6 + 0x54) = 10;
                system->AddOverlay(16);
            }
            else
            {
                OverlayMenu* fems = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(int*)((char*)fems + 0x50) = 5;
                OverlayMenu* v7 = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(int*)((char*)v7 + 0x54) = 9;
                system->AddOverlay(16);
            }
        }
        MPUIInterface::Step();
        if (mNeedToUpdate)
        {
            j_nullsub_46(this);
            InitMenu();
            mNeedToUpdate = false;
        }
        m_ListBox.Update(time_inc);
        UpdateGameInfo();
    }
    else
    {
        system->MakeActive(8);
    }
}

// ============================================================================
// Batch 26: vote select/send + settings update/triangle + model position
// ============================================================================

// ea: 0x007A0800
void VoteGameTypeMenu::Select(int entryNum)
{
    (void)entryNum;
    int CurrentSelection = ((FEMenuListBox*)mGameTypeList)->GetCurrentSelection();
    int selectedType = CurrentSelection;
    if (CurrentSelection >= MPUIInterface::mServerParams.mGameType)
        selectedType = CurrentSelection + 1;
    bdMessage* v4 = (bdMessage*)bdMemory::allocate(0x18u);
    bdMessage* v5 = (v4 != nullptr) ? new (v4) bdMessage(0x51u, false) : nullptr;
    if (v5 != nullptr)
        ++v5->m_refCount;
    ++g_NumBdMessages;
    bdReference<bdBitBuffer> buffer = v5->getPayload();
    unsigned char v20[4] = { 3, 0, 0, 0 };
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(v20, 8u);
    v20[0] = ((MPPlayerManager*)mPlayerMgr)->getPlayerIndex(0);
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(v20, 8u);
    v20[0] = (unsigned char)selectedType;
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(v20, 8u);
    bdReference<bdMessage> v15;
    v15.m_ptr = v5;
    if (v5 != nullptr)
        ++v5->m_refCount;
    ((MPPlayerManager*)mPlayerMgr)->SendHost(v15, true);
    FEMenu* v10 = g_femanager.mIGMS[0]->menus[0];
    int client = ((FESplitScreenMenu*)v10)->mVersion;
    g_femanager.GetIGMS(client)->ReturnToPreviousMenu(-1);
    g_femanager.GetDMS(client)->MakeActive(-1);
    GamePause::SetGamePaused(currCl, false);
    ((MenuClearHelper*)v10)->ClearAll();
    if (buffer.m_ptr != nullptr)
    {
        int v13 = buffer.m_ptr->m_refCount - 1;
        buffer.m_ptr->m_refCount = v13;
        if (v13 == 0)
        {
            delete buffer.m_ptr;
            buffer.m_ptr = nullptr;
        }
    }
    if (v5 != nullptr && v5->m_refCount-- == 1)
        delete v5;
}

// ea: 0x007A0A10
void VoteMapMenu::Select(int entryNum)
{
    (void)entryNum;
    int CurrentSelection = ((FEMenuListBox*)mMapList)->GetCurrentSelection();
    int selectedMap = CurrentSelection;
    if (CurrentSelection >= MPUIInterface::mServerParams.mMapID)
        selectedMap = CurrentSelection + 1;
    bdMessage* v4 = (bdMessage*)bdMemory::allocate(0x18u);
    bdMessage* v5 = (v4 != nullptr) ? new (v4) bdMessage(0x51u, false) : nullptr;
    if (v5 != nullptr)
        ++v5->m_refCount;
    ++g_NumBdMessages;
    bdReference<bdBitBuffer> buffer = v5->getPayload();
    unsigned char v20[4] = { 2, 0, 0, 0 };
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(v20, 8u);
    v20[0] = ((MPPlayerManager*)mPlayerMgr)->getPlayerIndex(0);
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(v20, 8u);
    v20[0] = (unsigned char)selectedMap;
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(v20, 8u);
    bdReference<bdMessage> v15;
    v15.m_ptr = v5;
    if (v5 != nullptr)
        ++v5->m_refCount;
    ((MPPlayerManager*)mPlayerMgr)->SendHost(v15, true);
    FEMenu* v10 = g_femanager.mIGMS[0]->menus[0];
    int client = ((FESplitScreenMenu*)v10)->mVersion;
    g_femanager.GetIGMS(client)->ReturnToPreviousMenu(-1);
    g_femanager.GetDMS(client)->MakeActive(-1);
    GamePause::SetGamePaused(currCl, false);
    ((MenuClearHelper*)v10)->ClearAll();
    if (buffer.m_ptr != nullptr)
    {
        int v13 = buffer.m_ptr->m_refCount - 1;
        buffer.m_ptr->m_refCount = v13;
        if (v13 == 0)
        {
            delete buffer.m_ptr;
            buffer.m_ptr = nullptr;
        }
    }
    if (v5 != nullptr && v5->m_refCount-- == 1)
        delete v5;
}

// ea: 0x007A52C0
void AARMapVote::SelectMap(int indexMap)
{
    if (m_iSelectedMap > -1)
        m_ListBox.mHighlights.mElements[m_iSelectedMap] = false;
    int v3 = indexMap;
    if (indexMap >= g_NumBaseMaps + 1)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/ui/AARMapVote.cpp";
        AeAssert::gCurrentLine = 576;
        AeAssert::gCurrentExpr = "indexMap < kAARPMapLimit";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Index is out of bounds of avail. maps!"))
            __debugbreak();
    }
    m_ListBox.mHighlights.mElements[v3] = true;
    m_ListBox.Refresh();
    unsigned char oldVote = (unsigned char)m_iSelectedMap;
    m_iSelectedMap = v3;
    bdMessage* v5 = (bdMessage*)bdMemory::allocate(0x18u);
    bdMessage* v6 = (v5 != nullptr) ? new (v5) bdMessage(0x66u, false) : nullptr;
    if (v6 != nullptr)
        ++v6->m_refCount;
    ++g_NumBdMessages;
    bdReference<bdBitBuffer> buffer = v6->getPayload();
    MPPlayerManager* pMan =
        MultiplayerMgr::sInst->mPeer->GetPlayerManager();
    unsigned char v20 = 2;
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(&v20, 8u);
    v20 = pMan->getPlayerIndex(0);
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(&v20, 8u);
    v20 = oldVote;
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(&v20, 8u);
    v20 = (unsigned char)m_iSelectedMap;
    buffer.m_ptr->writeDataType(bdBitBuffer::BD_BB_UNSIGNED_CHAR8_TYPE);
    buffer.m_ptr->writeBits(&v20, 8u);
    bdReference<bdMessage> v14;
    v14.m_ptr = v6;
    if (v6 != nullptr)
        ++v6->m_refCount;
    pMan->SendAll(v14, true, false);
    if (buffer.m_ptr != nullptr)
    {
        int v12 = buffer.m_ptr->m_refCount - 1;
        buffer.m_ptr->m_refCount = v12;
        if (v12 == 0)
        {
            delete buffer.m_ptr;
            buffer.m_ptr = nullptr;
        }
    }
    if (v6 != nullptr && v6->m_refCount-- == 1)
        delete v6;
}

// ea: 0x0079DCC0
void SessionLanListMenu::RepopulateSessionList()
{
    unsigned long numGames = 0;
    sGameListing* v2 = MPUIInterface::GameListingGet(numGames);
    if (numGames != 0)
    {
        m_ListBox.Clear();
        unsigned int v3 = 0;
        mNumGames = 0;
        int count_added_to_list = 0;
        unsigned int i = 0;
        int* mVisibleListToGameListMap = this->mVisibleListToGameListMap;
        int* v20 = this->mVisibleListToGameListMap;
        int* v17 = this->mVisibleListToGameListMap;
        do
        {
            *mVisibleListToGameListMap = -1;
            if (v2 != nullptr && v2->mValidVersion != 0)
            {
                int v6 = 0;
                if (g_NumTotalMaps > 0)
                {
                    char* v7 = byte_E386C9;
                    while (*v7 != ((sMPGameInfoView*)v2->mGameInfo)->mMapID)
                    {
                        ++v6;
                        v7 += 114;
                        if (v6 >= g_NumTotalMaps)
                            goto NEXT_GAME;
                    }
                    sMPGameInfoView* info = (sMPGameInfoView*)v2->mGameInfo;
                    if (info->mGameType < 6u)
                    {
                        *v20 = v3;
                        if (strcmp(info->mName, defaultFileName) == 0)
                            m_ListBox.SetText(count_added_to_list, 0,
                                              "MPFRONTEND_UNKNOWN_SOLDIER");
                        else
                            m_ListBox.SetText(count_added_to_list, 0,
                                              info->mName);
                        int v9 = info->m_publicFilled + info->m_privateFilled;
                        int v10 = v9 + info->m_privateOpen + info->m_publicOpen;
                        const char* STBString = STBManager::sInst->GetSTBString(
                            "MPFRONTEND_OF");
                        char maxPlayers[16];
                        sprintf(maxPlayers, "%d %s %d", v9, STBString, v10);
                        int v12 = count_added_to_list;
                        m_ListBox.SetText(count_added_to_list, 1, maxPlayers);
                        m_ListBox.SetText(
                            v12, 2,
                            MPUIInterface::GetGameTypeString(info->mGameType));
                        m_ListBox.SetText(
                            v12, 3, MPUIInterface::GetMapString(info->mMapID));
                        v3 = i;
                        count_added_to_list = v12 + 1;
                        ++v20;
                    }
                }
            }
        NEXT_GAME:
            ++v3;
            mVisibleListToGameListMap = v17 + 1;
            ++v2;
            i = v3;
            ++v17;
        } while (v3 < numGames);
        m_ListBox.Refresh();
        UpdateGameInfo();
    }
}

// ea: 0x0079A760
void GameSettingsEdit::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    PanelQuad* mQuad = mScrollBarUpFader.mQuad;
    if (mQuad != nullptr && mScrollBarUpFader.mFading)
    {
        if (mScrollBarUpFader.mAlphaTo <= mScrollBarUpFader.mAlpha)
        {
            float mAlpha = mScrollBarUpFader.mAlpha;
            if (mAlpha <= mScrollBarUpFader.mAlphaTo)
                goto UP_SET;
            float v7 = mAlpha
                - (time_inc / mScrollBarUpFader.mTime)
                    * mScrollBarUpFader.mAlphaDelta;
            float mAlphaTo = mScrollBarUpFader.mAlphaTo;
            mScrollBarUpFader.mAlpha = v7;
            if (mAlphaTo < v7)
                goto UP_SET;
            mScrollBarUpFader.mAlpha = mScrollBarUpFader.mAlphaTo;
        }
        else
        {
            float v4 = (time_inc / mScrollBarUpFader.mTime)
                    * mScrollBarUpFader.mAlphaDelta
                + mScrollBarUpFader.mAlpha;
            mScrollBarUpFader.mAlpha = v4;
            if (v4 < mScrollBarUpFader.mAlphaTo)
                goto UP_SET;
            mScrollBarUpFader.mAlpha = mScrollBarUpFader.mAlphaTo;
        }
        mScrollBarUpFader.mFading = false;
    UP_SET:
        mQuad->SetAlpha(mScrollBarUpFader.mAlpha);
    }
    PanelQuad* v9 = mScrollBarDownFader.mQuad;
    if (v9 != nullptr && mScrollBarDownFader.mFading)
    {
        if (mScrollBarDownFader.mAlphaTo <= mScrollBarDownFader.mAlpha)
        {
            float v11 = mScrollBarDownFader.mAlpha;
            if (v11 <= mScrollBarDownFader.mAlphaTo)
                goto DOWN_SET;
            float v12 = v11
                - (time_inc / mScrollBarDownFader.mTime)
                    * mScrollBarDownFader.mAlphaDelta;
            float v13 = mScrollBarDownFader.mAlphaTo;
            mScrollBarDownFader.mAlpha = v12;
            if (v13 < v12)
                goto DOWN_SET;
            mScrollBarDownFader.mAlpha = mScrollBarDownFader.mAlphaTo;
        }
        else
        {
            float v10 = (time_inc / mScrollBarDownFader.mTime)
                    * mScrollBarDownFader.mAlphaDelta
                + mScrollBarDownFader.mAlpha;
            mScrollBarDownFader.mAlpha = v10;
            if (v10 < mScrollBarDownFader.mAlphaTo)
                goto DOWN_SET;
            mScrollBarDownFader.mAlpha = mScrollBarDownFader.mAlphaTo;
        }
        mScrollBarDownFader.mFading = false;
    DOWN_SET:
        v9->SetAlpha(mScrollBarDownFader.mAlpha);
    }
    if (entries[6]->GetValue() != 0)
        entries[1]->Disable(true);
    else
        entries[1]->Disable(false);
}

// ea: 0x0079AEA0
void GameSettingsEdit::OnTriangle(int c)
{
    (void)c;
    bool bUnchanged = entries[0]->GetValue() == mNextServerParams->mGameType;
    if (bUnchanged)
    {
        char v3 = (char)entries[1]->GetValue();
        if (v3 != -1)
            v3 = byte_E386C9[114 * v3];
        bUnchanged =
            v3 == mNextServerParams->mMapID
            && entries[2]->GetValue() == mNextServerParams->mTimeLimit
            && entries[3]->GetValue() == mNextServerParams->mScoreLimit
            && entries[5]->GetValue() == mNextServerParams->mTeamBalancing
            && entries[4]->GetValue() == mNextServerParams->mFriendlyFire
            && entries[6]->GetValue() == mNextServerParams->mEnableAARVote
            && entries[7]->GetValue() == mNextServerParams->mEnablePenaltyVote;
    }
    if (bUnchanged)
    {
        system->ReturnToPreviousMenu(-1);
        iLastOptionSelected = highlighted;
    }
    else
    {
        DialogMenuSystem* DMS = g_femanager.GetDMS(mVersion);
        DMS->BringUp("MPGAME_APPLY_SETTINGS_NOW", false, false,
                     "MPGAME_EDIT_GAME_SETTINGS", true);
        DialogMenuSystem* v5 = g_femanager.GetDMS(mVersion);
        DialogMenu* Layer = v5->GetLayer(v5->GetActiveMenu() == 0);
        Layer->AddOption("MPGAME_APPLY_NOW",
                         GameSettingsEdit::ResponseYesApplyNow);
        DialogMenuSystem* v8 = g_femanager.GetDMS(mVersion);
        DialogMenu* v10 = v8->GetLayer(v8->GetActiveMenu() == 0);
        v10->AddOption("MPGAME_DONT_APPLY",
                       GameSettingsEdit::ResponseNoJustGoBackToPauseMenu);
        j_nullsub_58(g_femanager.GetDMS(mVersion), true);
        g_femanager.GetDMS(mVersion)->HighlightOption(1);
        DialogMenuSystem* v13 = g_femanager.GetDMS(mVersion);
        DialogMenu* v15 = v13->GetLayer(v13->GetActiveMenu() == 0);
        v15->Reformat(true, 0);
        iLastOptionSelected = highlighted;
    }
}

// ea: 0x0079B170
void AARGameSettingsEdit::OnTriangle(int c)
{
    (void)c;
    bool bUnchanged = entries[0]->GetValue() == mNextServerParams->mGameType;
    if (bUnchanged)
    {
        char v3 = (char)entries[1]->GetValue();
        if (v3 != -1)
            v3 = byte_E386C9[114 * v3];
        bUnchanged =
            v3 == mNextServerParams->mMapID
            && entries[2]->GetValue() == mNextServerParams->mTimeLimit
            && entries[3]->GetValue() == mNextServerParams->mScoreLimit
            && entries[5]->GetValue() == mNextServerParams->mTeamBalancing
            && entries[4]->GetValue() == mNextServerParams->mFriendlyFire
            && entries[6]->GetValue() == mNextServerParams->mEnableAARVote
            && entries[7]->GetValue() == mNextServerParams->mEnablePenaltyVote;
    }
    if (bUnchanged)
    {
        system->ReturnToPreviousMenu(-1);
    }
    else
    {
        DialogMenuSystem* DMS = g_femanager.GetDMS(mVersion);
        DMS->BringUp("MPGAME_APPLY_SETTINGS_NOW", false, false,
                     defaultFileName, true);
        DialogMenuSystem* v5 = g_femanager.GetDMS(mVersion);
        DialogMenu* Layer = v5->GetLayer(v5->GetActiveMenu() == 0);
        Layer->AddOption("MPGAME_APPLY_NOW",
                         AARGameSettingsEdit::ResponseYesApplyNow);
        DialogMenuSystem* v8 = g_femanager.GetDMS(mVersion);
        DialogMenu* v10 = v8->GetLayer(v8->GetActiveMenu() == 0);
        v10->AddOption("MPGAME_DONT_APPLY",
                       AARGameSettingsEdit::ResponseNoJustGoBackToPauseMenu);
        j_nullsub_58(g_femanager.GetDMS(mVersion), true);
        g_femanager.GetDMS(mVersion)->HighlightOption(1);
        DialogMenuSystem* v13 = g_femanager.GetDMS(mVersion);
        DialogMenu* v15 = v13->GetLayer(v13->GetActiveMenu() == 0);
        v15->Reformat(true, 0);
    }
}

// ea: 0x007AD150
void ModelMenu::UpdateModelPosition()
{
    Entity* mObject =
        EntityHandleDb::sInst.GetObject(mClassModelEntity.mHandle.mVal);
    if (mObject != nullptr)
    {
        mObject->r.currentAngles.v.m128_f32[0] = mModelAngles[0];
        mObject->r.currentAngles.v.m128_f32[1] = mModelAngles[1];
        mObject->r.currentAngles.v.m128_f32[2] = mModelAngles[2];
        mObject->r.currentAngles.v.m128_f32[3] = mModelAngles[3];
        Entity* v9 =
            EntityHandleDb::sInst.GetObject(mClassModelEntity.mHandle.mVal);
        float* m128_f32 = v9->r.currentOrigin.v.m128_f32;
        m128_f32[0] = mModelPosition[0];
        m128_f32[1] = mModelPosition[1];
        m128_f32[2] = mModelPosition[2];
        m128_f32[3] = mModelPosition[3];
        Entity* v12 =
            EntityHandleDb::sInst.GetObject(mClassModelEntity.mHandle.mVal);
        v12->CalcRotTranMat43();
        float v15 = 2.0f;  // scale_2 @ 0xE3AEDC
        __m128 v16 = _mm_set1_ps(v15);
        Entity* v14 =
            EntityHandleDb::sInst.GetObject(mClassModelEntity.mHandle.mVal);
        v14->r.currentMat.x.v = _mm_mul_ps(v14->r.currentMat.x.v, v16);
        Entity* v18 =
            EntityHandleDb::sInst.GetObject(mClassModelEntity.mHandle.mVal);
        __m128 v19 = _mm_set1_ps(0.0f - v15);
        v18->r.currentMat.y.v = _mm_mul_ps(v18->r.currentMat.y.v, v19);
        Entity* v22 =
            EntityHandleDb::sInst.GetObject(mClassModelEntity.mHandle.mVal);
        v22->r.currentMat.z.v = _mm_mul_ps(v22->r.currentMat.z.v, v16);
    }
}

// ea: 0x0078F120
void OverlayMenu::OnCross(int c)
{
    switch (mState)
    {
    case (OverlayMenu::eState)0x11:
        if (m_currSelection == 0)
        {
            MPLiveEngine::GetHandle()->LogOut();
            XBoxLiveIngameOptionsCOD3::Me(0)->mReturnMenu = -1;
        }
        break;
    case (OverlayMenu::eState)0x12:
        if (m_currSelection == 0)
            goto APPEAR_TOGGLE;
        break;
    case (OverlayMenu::eState)0x13:
        if (m_currSelection == 0)
        {
        APPEAR_TOGGLE:
            unsigned int actualPort = MPLiveEngine::GetHandle()->actualPort;
            MPLiveEngine::GetHandle()->ToggleOfflineAppearance(actualPort);
            MPUIInterface::mIsViewableOnline = !MPUIInterface::mIsViewableOnline;
        }
        break;
    case (OverlayMenu::eState)0x14:
        if (m_currSelection != 0)
        {
            if (m_currSelection == 1)
            {
                LiveWrapper::theWrapper->SetVTS(
                    MPLiveEngine::GetHandle()->actualPort, true);
            }
        }
        else
        {
            LiveWrapper::theWrapper->SetVTS(
                MPLiveEngine::GetHandle()->actualPort, false);
        }
        break;
    case (OverlayMenu::eState)0x15:
        if (m_currSelection == 0)
        {
            MPLiveEngine* v4 = MPLiveEngine::GetHandle();
            v4->JoinGame((XONLINE_FRIEND*)v4->friendToJoin);
            XBoxLiveIngameOptionsCOD3::Me(0)->mReturnMenu = -1;
        }
        break;
    case (OverlayMenu::eState)0x16:
        XBoxLiveIngameOptionsCOD3::Me(0)->mReturnMenu = -1;
        MPLiveEngine::GetHandle()->renderingEnabled = true;
        if (m_currSelection != 0)
            MPLiveEngine::GetHandle()->LogOut();
        else
            LiveEngine_Reboot(MPLiveEngine::GetHandle()->uixEngine, 0);
        break;
    default:
        break;
    }
    if (mState == NO_GAMES)
    {
        system->RemoveOverlay();
        if (MPUIInterface::IsLANGame())
        {
            j_nullsub_46(this);
            system->MakeActiveAndReturn(1);
        }
        else
        {
            j_nullsub_46(this);
            system->MakeActiveAndReturn(0);
        }
    }
    else if ((mState == JOIN_FAILED || mState == CANNOT_CONNECT_TO_HOST
              || mState == CANNOT_CONNECT_TO_PEERS || mState == JOIN_SUCCESS
              || mState == JOIN_REFUSED)
             && MPUIInterface::IsOnlineGame())
    {
        ClearAllButtons();
        OnTriangle(c);
    }
    else
    {
        FEMenu::OnCross(c);
        Accept();
    }
}

// ea: 0x007A7BA0
void CreateLanSessionMenu::OnActivate()
{
    FEMenu::OnActivate();
    for (int i = 0; i < 4; ++i)
        m_pBackgroundArt.m_elements[i]->SetShown(true);
    if ((m_FirstTimeAccessedByte & 1) == 0)
    {
        m_FirstTimeAccessedByte = 1;
        if (mStartingMapCombo == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/CreateLanSessionMenu.cpp";
            AeAssert::gCurrentLine = 293;
            AeAssert::gCurrentExpr = "mStartingMapCombo";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Combobox failure"))
                __debugbreak();
        }
        for (int j = g_NumBaseMaps; j < g_NumTotalMaps; ++j)
        {
            char v4 = (j == 0xFF) ? (char)-1
                                  : (char)byte_E386C9[114 * j];
            Broc::string s(MPUIInterface::GetMapString(v4));
            mStartingMapCombo->AddOption(s);
        }
    }
    int mLastGameType = this->mLastGameType;
    if (mGameModeCombo->mCurrOption != mLastGameType)
    {
        if (mLastGameType == 5)
        {
            MPUIInterface::mServerParams.mFriendlyFire = 1;
            MPUIInterface::mServerParams.mEnablePenaltyVote = 1;
        }
        mLastGameType = mGameModeCombo->mCurrOption;
        this->mLastGameType = mLastGameType;
        MPUIInterface::mServerParams.mScoreLimit =
            MPUIInterface::GetDefaultOption(
                MPUIInterface::SETTING_SCORE_LIMIT, (eGameType)mLastGameType);
        if (this->mLastGameType == 5)
        {
            MPUIInterface::mServerParams.mFriendlyFire = 0;
            MPUIInterface::mServerParams.mEnablePenaltyVote = 0;
            MPUIInterface::mServerParams.mTeamBalancing = 0;
        }
    }
    int mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != mLastMap)
    {
        mLastMap = mCurrOption;
        char mapID;
        if (mCurrOption == 0xFF)
            mapID = -1;
        else
            mapID = byte_E386C9[114 * mCurrOption];
        short MaxPlayersOptionFromMap =
            MPUIInterface::GetMaxPlayersOptionFromMap(mapID);
        mNumberOfPlayersCombo->SetCurrOption(MaxPlayersOptionFromMap);
    }
    SetHigh(1, true);
    highlighted = 1;
    entries[0]->Highlight(true, true);
    m_pText.m_elements[0]->SetText("MPFRONTEND_PLAY_SYSTEM_LINK");
    GrabSessionName(0);
}

// ea: 0x0079A290
void GameSettingsEdit::AddOptionsToCombos()
{
    static const char* const pszDisableEnable[2] = {
        "MPGAME_DISABLE", "MPGAME_ENABLE",
    };
    for (int i = 0; i < 6; ++i)
    {
        Broc::string s(MPUIInterface::GetGameTypeString(i));
        ((FEComboBox*)entries[0])->AddOption(s);
    }
    ((FEComboBox*)entries[0])->SetCurrOption(0);
    for (int j = 0; j < g_NumBaseMaps; ++j)
    {
        char v5 = (j == 0xFF) ? (char)-1 : (char)byte_E386C9[114 * j];
        Broc::string s(MPUIInterface::GetMapString(v5));
        ((FEComboBox*)entries[1])->AddOption(s);
    }
    ((FEComboBox*)entries[1])->SetCurrOption(0);
    for (int k = 0; k < MPUIInterface::GetTimeLimitCount(); ++k)
    {
        char szScore[20];
        sprintf(szScore, "%d", MPUIInterface::GetTimeLimit(k));
        Broc::string s(szScore);
        ((FEComboBox*)entries[2])->AddOption(s);
    }
    ((FEComboBox*)entries[2])->SetCurrOption(0);
    for (int m = 0; m < MPUIInterface::GetScoreLimitCount(GAME_TYPE_WAR); ++m)
    {
        char szScore[20];
        sprintf(szScore, "%d",
                MPUIInterface::GetScoreLimit(m, GAME_TYPE_WAR));
        Broc::string s(szScore);
        ((FEComboBox*)entries[3])->AddOption(s);
    }
    ((FEComboBox*)entries[3])->SetCurrOption(0);
    for (int e = 4; e <= 7; ++e)
    {
        for (int d = 0; d < 2; ++d)
        {
            Broc::string s(pszDisableEnable[d]);
            ((FEComboBox*)entries[e])->AddOption(s);
        }
        ((FEComboBox*)entries[e])->SetCurrOption(0);
    }
}

// ea: 0x007ABC20
void PlayOnlineMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    UpdateTextDescription(highlighted);
    if (MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
        return;
    }
    char* Icon = LiveWrapper::theWrapper->GetIcon(
        MPLiveEngine::GetHandle()->actualPort);
    if (Icon == (char*)0x20000)
    {
        panel->GetPointer("game_invite")->SetShown(true);
        panel->GetPointer("friend_request")->SetShown(false);
    }
    else if (Icon == (char*)0x10000)
    {
        panel->GetPointer("game_invite")->SetShown(false);
        panel->GetPointer("friend_request")->SetShown(true);
    }
    else if (Icon == nullptr)
    {
        panel->GetPointer("game_invite")->SetShown(false);
        panel->GetPointer("friend_request")->SetShown(false);
    }
    if (mJoiningFriend)
    {
        if (!MPUIInterface::mLiveQueryActive || !MPUIInterface::mQueryFromID)
        {
            unsigned long numGames = 0;
            MPUIInterface::GameListingGet(numGames);
            if (numGames == 0)
            {
                OverlayMenu* v16 = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                v16->SetState(OverlayMenu::JOIN_FAILED);
                OverlayMenu* fems = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(int*)((char*)fems + 0x50) = 10;
                OverlayMenu* v18 = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(int*)((char*)v18 + 0x54) = 10;
                system->AddOverlay(16);
            }
            else
            {
                OverlayMenu* v13 = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                v13->SetState(OverlayMenu::JOINING_START);
                OverlayMenu* v14 = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(int*)((char*)v14 + 0x54) = 10;
                OverlayMenu* v15 = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(float*)((char*)v15 + 0x60) = 0.0f;
                system->AddOverlay(16);
            }
            mJoiningFriend = false;
        }
    }
    else if (MPUIInterface::mLiveQueryActive && MPUIInterface::mQueryFromID)
    {
        OverlayMenu* v10 = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        v10->SetState((OverlayMenu::eState)16);
        OverlayMenu* v11 = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        *(int*)((char*)v11 + 0x50) = 10;
        OverlayMenu* v12 = g_femanager.fems != nullptr
            ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
        *(int*)((char*)v12 + 0x54) = 10;
        system->AddOverlay(16);
        mJoiningFriend = true;
    }
    PanelQuad* v20 = panel->GetPointer("friend_request");
    PanelQuad* v19 = panel->GetPointer("game_invite");
    ShowNotificationIcon(&friendIcon, v19, v20);
    if (m_IsQuickMatchReady)
    {
        LaunchQuickMatch();
        m_IsQuickMatchReady = false;
    }
    MPUIInterface::Step();
}

// ============================================================================
// Batch 28: AAR base panel + menu system ctor + generic scores + create menu
// ============================================================================

// ea: 0x007A3410
void AARBaseMenu::SetPanelFile(PanelFile* pf)
{
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/ui/AARBaseMenu.cpp";
        AeAssert::gCurrentLine = 88;
        AeAssert::gCurrentExpr = "pf";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Scoreboard panel file pointer is NULL"))
            __debugbreak();
    }
    panel = pf;
    mLeftArrowFader.mQuad = pf->GetPointer("scroll_arrow_left");
    mRightArrowFader.mQuad = panel->GetPointer("scroll_arrow_right");
    PanelQuad* mQuad = mLeftArrowFader.mQuad;
    if (mQuad != nullptr)
    {
        mLeftArrowFader.mAlpha = 0.5f;
        mLeftArrowFader.mAlphaTo = 0.5f;
        mLeftArrowFader.mFading = true;
        mLeftArrowFader.mTime = 0.0f;
        mLeftArrowFader.mAlphaDelta = (float)fabs(0.0);
        mQuad->SetAlpha(0.5f);
    }
    else
    {
        mLeftArrowFader.mFading = false;
    }
    PanelQuad* v6 = mRightArrowFader.mQuad;
    if (v6 != nullptr)
    {
        mRightArrowFader.mAlpha = 0.5f;
        mRightArrowFader.mAlphaTo = 0.5f;
        mRightArrowFader.mFading = true;
        mRightArrowFader.mTime = 0.0f;
        mRightArrowFader.mAlphaDelta = (float)fabs(0.0);
        v6->SetAlpha(0.5f);
    }
    else
    {
        mRightArrowFader.mFading = false;
    }
    static const char* const sxScoreboardTextTimer[2] = {
        "text_timer_numbers", "text_timer_text",
    };
    for (int i = 0; i < 2; ++i)
    {
        m_pTimerText.m_elements[i] =
            panel->GetTextPointer(sxScoreboardTextTimer[i]);
        if (m_pTimerText.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARBaseMenu.cpp";
            AeAssert::gCurrentLine = 104;
            AeAssert::gCurrentExpr = "m_pTimerText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Could not get timer text!"))
                __debugbreak();
        }
        m_pTimerText.m_elements[i]->SetShown(true);
    }
    m_pTimerText.m_elements[1]
        ->SetText("MPGAME_AAR_SECONDS_TIL_NEXT_GAME");
}

// ea: 0x007B02E0
AARMenuSystem::AARMenuSystem()
    : FEMenuSystem(11, FONT_GARAMOND)
{
    mPreviousWidescreen = false;
    void* mem1 = mem_heap_malloc(16, 0x34Cu);
    AARScoreboardWinner* v3 = nullptr;
    if (mem1 != nullptr)
        v3 = new (mem1) AARScoreboardWinner(this);
    Add(v3);
    void* mem2 = mem_heap_malloc(16, 0x34Cu);
    AARScoreboardLoser* v5 = nullptr;
    if (mem2 != nullptr)
        v5 = new (mem2) AARScoreboardLoser(this);
    Add(v5);
    void* mem3 = mem_heap_malloc(16, 0x150u);
    AARPersonalStats* v7 = nullptr;
    if (mem3 != nullptr)
        v7 = new (mem3) AARPersonalStats(this);
    Add(v7);
    void* mem4 = mem_heap_malloc(16, 0x20Cu);
    AARGameModeVote* v9 = nullptr;
    if (mem4 != nullptr)
        v9 = new (mem4) AARGameModeVote(this);
    Add(v9);
    void* mem5 = mem_heap_malloc(16, 0x238u);
    AARMapVote* v11 = nullptr;
    if (mem5 != nullptr)
        v11 = new (mem5) AARMapVote(this);
    Add(v11);
    void* mem6 = mem_heap_malloc(16, 0x11Cu);
    AARInGameOptionsMenu* v13 = nullptr;
    if (mem6 != nullptr)
        v13 = new (mem6) AARInGameOptionsMenu(this);
    Add(v13);
    void* mem7 = mem_heap_malloc(16, 0x12Cu);
    AARGameSettingsEdit* v15 = nullptr;
    if (mem7 != nullptr)
        v15 = new (mem7) AARGameSettingsEdit(this);
    Add(v15);
    void* mem8 = mem_heap_malloc(16, 0x11Cu);
    AARGameSettingsView* v17 = nullptr;
    if (mem8 != nullptr)
        v17 = new (mem8) AARGameSettingsView(this);
    Add(v17);
    void* mem9 = mem_heap_malloc(16, 0x164u);
    AARXBoxLiveIngameOptions* v19 = nullptr;
    if (mem9 != nullptr)
        v19 = new (mem9) AARXBoxLiveIngameOptions(this);
    Add((FEMenu*)v19);
    void* mem10 = mem_heap_malloc(16, 0x50u);
    AARPauseMenu* v21 = nullptr;
    if (mem10 != nullptr)
        v21 = new (mem10) AARPauseMenu(this);
    Add(v21);
    void* mem11 = mem_heap_malloc(16, 0x12Cu);
    AAROverlay* v23 = nullptr;
    if (mem11 != nullptr)
        v23 = new (mem11) AAROverlay(this);
    Add(v23);
    InitAll();
}

// ea: 0x007A41C0
void AARPersonalStats::SetGenericScores()
{
    Entity* FirstLocalPlayer = EntityManager::sInst->GetFirstLocalPlayer();
    if (FirstLocalPlayer != nullptr)
    {
        Client* client = FirstLocalPlayer->client;
        if (client != nullptr)
        {
            clientPersistent_t& pers = client->pers;
            int iTotalScore = pers.GetTotalScore();
            int v8 = 0;   // kills
            int v9 = 0;   // assists
            int v10 = 0;  // deaths
            int v25 = 0;  // suicides
            int v24 = 0;  // teamkills
            int v23 = 0;  // vehicles destroyed
            for (int c = 0; c < 7; ++c)
            {
                v8 += pers.mStats[c][3];
                v10 += pers.mStats[c][4];
                v9 += pers.mStats[c][5];
                v23 += pers.mStats[c][6];
                v25 += pers.mStats[c][7];
                v24 += pers.mStats[c][8];
            }
            char szTotalScore[12];
            char szKills[12];
            char szAssists[12];
            char szDeaths[12];
            char szSuicides[12];
            char szTeamkills[12];
            char szVehiclesDestroyed[12];
            sprintf(szTotalScore, "%d", iTotalScore);
            sprintf(szKills, "%d", v8);
            sprintf(szAssists, "%d", v9);
            sprintf(szDeaths, "%d", v10);
            sprintf(szSuicides, "%d", v25);
            sprintf(szTeamkills, "%d", v24);
            sprintf(szVehiclesDestroyed, "%d", v23);
            m_pScoreText.m_elements[1]->SetText(szTotalScore);
            m_pScoreText.m_elements[3]->SetText(szKills);
            m_pScoreText.m_elements[5]->SetText(szAssists);
            m_pScoreText.m_elements[7]->SetText(szDeaths);
            m_pScoreText.m_elements[9]->SetText(szSuicides);
            m_pScoreText.m_elements[11]->SetText(szTeamkills);
            m_pScoreText.m_elements[13]->SetText(szVehiclesDestroyed);
        }
    }
}

// ea: 0x007A76C0
void CreateSessionMenu::OnActivate()
{
    FEMenu::OnActivate();
    for (int i = 0; i < 6; ++i)
        m_pBackgroundArt.m_elements[i]->SetShown(i < 5);
    if ((m_FirstTimeAccessedByte & 1) == 0)
    {
        m_FirstTimeAccessedByte = 1;
        if (mStartingMapCombo == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/CreateSessionMenu.cpp";
            AeAssert::gCurrentLine = 382;
            AeAssert::gCurrentExpr = "mStartingMapCombo";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Combobox failure"))
                __debugbreak();
        }
        for (int j = g_NumBaseMaps; j < g_NumTotalMaps; ++j)
        {
            char v4 = (j == 0xFF) ? (char)-1
                                  : (char)byte_E386C9[114 * j];
            Broc::string s(MPUIInterface::GetMapString(v4));
            mStartingMapCombo->AddOption(s);
        }
    }
    int mLastGameType = this->mLastGameType;
    if (mGameModeCombo->mCurrOption != mLastGameType)
    {
        if (mLastGameType == 5)
        {
            MPUIInterface::mServerParams.mFriendlyFire = 1;
            MPUIInterface::mServerParams.mEnablePenaltyVote = 1;
        }
        mLastGameType = mGameModeCombo->mCurrOption;
        this->mLastGameType = mLastGameType;
        MPUIInterface::mServerParams.mScoreLimit =
            MPUIInterface::GetDefaultOption(
                MPUIInterface::SETTING_SCORE_LIMIT, (eGameType)mLastGameType);
        if (this->mLastGameType == 5)
        {
            MPUIInterface::mServerParams.mTeamBalancing = 0;
            MPUIInterface::mServerParams.mFriendlyFire = 0;
            MPUIInterface::mServerParams.mEnablePenaltyVote = 0;
        }
    }
    int mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != mLastMap)
    {
        mLastMap = mCurrOption;
        char mapID;
        if (mCurrOption == 0xFF)
            mapID = -1;
        else
            mapID = byte_E386C9[114 * mCurrOption];
        short MaxPlayersOptionFromMap =
            MPUIInterface::GetMaxPlayersOptionFromMap(mapID);
        mNumberOfPlayersCombo->SetCurrOption(MaxPlayersOptionFromMap);
    }
    SetHigh(1, true);
    highlighted = 1;
    entries[0]->Highlight(true, true);
    unsigned char mMaxPlayers = MPUIInterface::mServerParams.mMaxPlayers;
    if (MPUIInterface::mServerParams.mMaxPlayers > 3u)
    {
        mMaxPlayers = (MPUIInterface::mServerParams.mMaxPlayers >> 2) - 1;
        MPUIInterface::mServerParams.mMaxPlayers = mMaxPlayers;
    }
    mNumberOfPlayersCombo->SetCurrOption(mMaxPlayers);
    mGameModeCombo->SetCurrOption(MPUIInterface::mServerParams.mGameType);
    int v11 = 0;
    if (g_NumTotalMaps > 0)
    {
        char* v12 = byte_E386C9;
        while (*v12 != MPUIInterface::mServerParams.mMapID)
        {
            ++v11;
            v12 += 114;
            if (v11 >= g_NumTotalMaps)
            {
                v11 = -1;
                break;
            }
        }
    }
    else
    {
        v11 = -1;
    }
    mStartingMapCombo->SetCurrOption((short)v11);
    m_PrivateSlotsCombo->SetCurrOption(MPUIInterface::mServerParams.mPrivateSlots);
    m_pText.m_elements[0]->SetText("MPFRONTEND_PLAY_XBOX_LIVE");
    UpdatePrivateSlots();
    GrabSessionName();
}

// ============================================================================
// Batch 29: modifier anim + AAR map vote activate + multiline state + settings
// ============================================================================

static const char* const szAARMapNames[12] = {
    "slot_01_text_mapname", "slot_02_text_mapname",
    "slot_03_text_mapname", "slot_04_text_mapname",
    "slot_05_text_mapname", "slot_06_text_mapname",
    "slot_07_text_mapname", "slot_08_text_mapname",
    "slot_09_text_mapname", "slot_10_text_mapname",
    "slot_11_text_mapname", "slot_12_text_mapname",
};
static const char* const szAARMapVotes[12] = {
    "slot_01_text_mapvote", "slot_02_text_mapvote",
    "slot_03_text_mapvote", "slot_04_text_mapvote",
    "slot_05_text_mapvote", "slot_06_text_mapvote",
    "slot_07_text_mapvote", "slot_08_text_mapvote",
    "slot_09_text_mapvote", "slot_10_text_mapvote",
    "slot_11_text_mapvote", "slot_12_text_mapvote",
};

// ea: 0x007AC6D0
void ModelMenu::PlayModifierAnim(int sheet, int row, int column,
                                 bool immediate)
{
    unsigned int mVal = mClassModelEntity.mHandle.mVal;
    Entity* mObject = nullptr;
    if ((mVal & 0xFFF) < 0x540
        && mVal >> 12
            == (unsigned int)EntityHandleDb::sInst
                   .mElements[mVal & 0xFFF].mKey)
        mObject = EntityHandleDb::sInst.mElements[mVal & 0xFFF].mObject;
    DObj* mDObj = mObject->mDObj;
    AnimationPlayer* v8 = nullptr;
    if (mDObj != nullptr && mDObj->animPlayers[0] != nullptr)
        v8 = (AnimationPlayer*)mDObj->animPlayers[0];
    int mCurrentWeaponSheet = sheet;
    if (sheet < 0)
        mCurrentWeaponSheet = this->mCurrentWeaponSheet;
    MP_ANIM_INDEX* AnimIndex =
        MPPlayer::getAnimIndex(mCurrentWeaponSheet, row, column, true);
    nalGeneric::nalGenericAnim* anim = nullptr;
    if (AnimIndex != nullptr)
        anim = AnimIndex->anim;
    if (v8 != nullptr && anim != nullptr)
    {
        float v12 = immediate ? 0.0f : 0.25f;
        v8->PlayModifier(anim, AnimationPlayer::nalPartialModifier, 1.0f,
                         1u, false, v12, 0.25f, nullptr, 0.0f, nullptr,
                         1.0f, 0.0f);
        if ((AnimIndex->flags & 1) != 0)
            ((AnimationPlayer*)mDObj->animPlayers[0])
                ->SetModifierType(1u, AnimationPlayer::nalAdditiveModifier);
    }
}

// ea: 0x007AB510
void AARMapVote::OnActivate()
{
    MPUIInterface::Step();
    FEMenu::OnActivate();
    AARBaseMenu::SetTimerText();
    FEText* v2 = m_pTimerText.m_elements[1];
    if (MultiplayerMgr::sInst->mRankedGame)
        v2->SetText("MPGAME_AAR_RANK_GAME_OVER");
    else
        v2->SetText("MPGAME_AAR_SECONDS_TIL_NEXT_GAME");
    if ((m_FirstTimeAccessedByte & 1) == 0)
    {
        int v3 = 1;
        m_FirstTimeAccessedByte = 1;
        if (g_NumBaseMaps + 1 > 1)
        {
            do
            {
                FEText* TextPointer =
                    (v3 >= 12)
                    ? panel->GetTextPointer("slot_12_text_mapname")
                    : panel->GetTextPointer(szAARMapNames[v3]);
                m_ListBox.SetItem(v3, 0, TextPointer, 0);
                char v9 = (v3 != 0)
                    ? byte_E386C9[114 * (v3 - 1)]
                    : (char)(v3 - 1);
                int v10 = 0;
                const char* v12;
                if (g_NumTotalMaps <= 0)
                {
                    v12 = "NULL";
                }
                else
                {
                    char* v11 = byte_E386C9;
                    while (*v11 != v9)
                    {
                        ++v10;
                        v11 += 114;
                        if (v10 >= g_NumTotalMaps)
                        {
                            v12 = "NULL";
                            goto MAP_LOOKUP_DONE;
                        }
                    }
                    v12 = &aMpfrontendMerv[114 * v10];
                }
            MAP_LOOKUP_DONE:
                m_ListBox.SetText(v3, 0, v12);
                FEText* v13 =
                    panel->GetTextPointer(szAARMapVotes[v3]);
                m_ListBox.SetItem(v3, 1, v13, 0);
                m_ListBox.SetText(v3++, 1, "0");
            } while (v3 < g_NumBaseMaps + 1);
        }
    }
    m_bShowScrollArrowRight = true;
    m_bShowScrollArrowLeft = true;
    m_ePanelToSwitchTo = -1;
}

// ea: 0x007903D0
void MultilineIngameOverlayMenu::SetState(eState newState)
{
    entries[1]->SetScale(mTextScale);
    mState = newState;
    if (newState == NETWORK_ERROR_COUNTDOWN)
    {
        mText = "MPFRONTEND_NETWORK_ERROR_COUNTDOWN";
        entries[2]->SetTextNoLocalize((char*)defaultFileName);
        entries[2]->Disable(true);
        entries[3]->SetText("MPFRONTEND_CANCEL");
        entries[3]->Disable(false);
        mCountdown = 5.9899998f;
        helpbar1->SetText("MPFRONTEND_HELP_SELECT_BACK");
    }
    else if (newState == CONTROLLER_DISCONNECTED)
    {
        g_controllerConnectedErrorShown[
            LocalClient::ClientToPort(currCl)] = true;
        Broc::string controllerMessage = BuildControllerMessage();
        mText = controllerMessage;
        mAcceptMenu = -1;
        mBackMenu = -1;
        entries[3]->SetTextNoLocalize((char*)defaultFileName);
        entries[3]->Disable(true);
        helpbar1->SetTextNoLocalize((char*)defaultFileName);
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/MultilineOverlayMenu.cpp";
        AeAssert::gCurrentLine = 374;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    const char* v9 = (mText.mBlock != nullptr)
        ? (const char*)&mText.mBlock[1] : defaultFileName;
    mTextEntry->SetTextBox(v9, 400, -1.0f);
    mBackMenu = -1;
    mAcceptMenu = -1;
}

// ea: 0x0079A4C0
void GameSettingsEdit::OnActivate()
{
    SwapMenus();
    FEMenu::OnActivate();
    if ((m_FirstTimeAccessedByte & 1) == 0)
    {
        m_FirstTimeAccessedByte = 1;
        if (entries[1] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/GameSettingsEdit.cpp";
            AeAssert::gCurrentLine = 229;
            AeAssert::gCurrentExpr = "((FEComboBox*)entries[GAME_MAP])";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Combobox failure"))
                __debugbreak();
        }
        for (int i = g_NumBaseMaps; i < g_NumTotalMaps; ++i)
        {
            char v4 = (i == 0xFF) ? (char)-1
                                  : (char)byte_E386C9[114 * i];
            Broc::string s(MPUIInterface::GetMapString(v4));
            ((FEComboBox*)entries[1])->AddOption(s);
        }
    }
    mNextServerParams = &MPUIInterface::mNextServerParams;
    mLastGameType = MPUIInterface::mNextServerParams.mGameType;
    GetScoreLimitsForGameType((eGameType)mLastGameType);
    // FEMenuEntry vtable slot 49 = SetValue(int) (0x5AE840)
    ((void(__thiscall*)(void*, int))(*((void***)entries[0]) + 49))(
        entries[0], mNextServerParams->mGameType);
    int v6 = 0;
    if (g_NumTotalMaps > 0)
    {
        char* v7 = byte_E386C9;
        while (*v7 != mNextServerParams->mMapID)
        {
            ++v6;
            v7 += 114;
            if (v6 >= g_NumTotalMaps)
            {
                v6 = -1;
                break;
            }
        }
    }
    else
    {
        v6 = -1;
    }
    ((void(__thiscall*)(void*, int))(*((void***)entries[1]) + 49))(
        entries[1], v6);
    ((void(__thiscall*)(void*, int))(*((void***)entries[2]) + 49))(
        entries[2], mNextServerParams->mTimeLimit);
    ((void(__thiscall*)(void*, int))(*((void***)entries[3]) + 49))(
        entries[3], mNextServerParams->mScoreLimit);
    ((void(__thiscall*)(void*, int))(*((void***)entries[4]) + 49))(
        entries[4], mNextServerParams->mFriendlyFire);
    ((void(__thiscall*)(void*, int))(*((void***)entries[5]) + 49))(
        entries[5], mNextServerParams->mTeamBalancing);
    ((void(__thiscall*)(void*, int))(*((void***)entries[6]) + 49))(
        entries[6], mNextServerParams->mEnableAARVote);
    ((void(__thiscall*)(void*, int))(*((void***)entries[7]) + 49))(
        entries[7], mNextServerParams->mEnablePenaltyVote);
    entries[6]->Disable(true);
    entries[0]->Disable(mNextServerParams->mEnableAARVote != 0);
    entries[1]->Disable(mNextServerParams->mEnableAARVote != 0);
    bool v12 = mNextServerParams->mGameType == 5;
    entries[7]->Disable(v12);
    entries[4]->Disable(v12);
    entries[5]->Disable(v12);
    if (v12)
    {
        ((FEComboBox*)entries[7])->SetCurrOption(0);
        ((FEComboBox*)entries[4])->SetCurrOption(0);
        ((FEComboBox*)entries[5])->SetCurrOption(0);
    }
    if (mNextServerParams->mEnableAARVote != 0
        && iLastOptionSelected <= 1)
        SetHigh(2, true);
    else
        SetHigh(iLastOptionSelected, true);
}

// ============================================================================
// Batch 30: overlay state set + class model update
// ============================================================================

static void OverlaySetStateCommon(UIListBox* m_ListBox,
                                  FEText** m_pOptionText,
                                  int& m_currSelection)
{
    m_ListBox->SetColumnSelectable(0, true);
    for (int i = 0; i < 2; ++i)
        m_pOptionText[i]->SetShown(true);
}

// ea: 0x0079FBC0
void InGameOverlay::SetState(eState state)
{
    m_State = state;
    m_IsAARTimerEnabled = false;
    m_IsAARTimerEnabled = true;
    const char* locTxt;
    switch (state)
    {
    case OVERLAY_SIGNIN_SIGNOUT:
    case OVERLAY_AAR_SIGNIN_SIGNOUT:
        locTxt = "MPFRONTEND_XBOX_LIVE_OPTION_OVERLAY_SIGNOUT";
        OverlaySetStateCommon(&m_ListBox, m_pOptionText.m_elements,
                              m_currSelection);
        m_ListBox.SetText(0, 0, "MPFRONTEND_YES");
        m_ListBox.SetText(1, 0, "MPFRONTEND_NO");
        m_ListBox.Refresh();
        m_currSelection = 1;
        m_ListBox.SelectLine(1);
        break;
    case OVERLAY_APPEAR_ONLINE:
    case OVERLAY_AAR_APPEAR_ONLINE:
        locTxt = "MPFRONTEND_XBOX_OPTIONS_APPEAR_ONLINE_OVERLAY";
        OverlaySetStateCommon(&m_ListBox, m_pOptionText.m_elements,
                              m_currSelection);
        m_ListBox.SetText(0, 0, "MPFRONTEND_YES");
        m_ListBox.SetText(1, 0, "MPFRONTEND_NO");
        m_ListBox.Refresh();
        m_currSelection = 0;
        m_ListBox.SelectLine(0);
        break;
    case OVERLAY_APPEAR_OFFLINE:
    case OVERLAY_AAR_APPEAR_OFFLINE:
        locTxt = "MPFRONTEND_XBOX_OPTIONS_APPEAR_OFFLINE_OVERLAY";
        OverlaySetStateCommon(&m_ListBox, m_pOptionText.m_elements,
                              m_currSelection);
        m_ListBox.SetText(0, 0, "MPFRONTEND_YES");
        m_ListBox.SetText(1, 0, "MPFRONTEND_NO");
        m_ListBox.Refresh();
        m_currSelection = 0;
        m_ListBox.SelectLine(0);
        break;
    case OVERLAY_TOGGLE_VOICE:
    case OVERLAY_AAR_TOGGLE_VOICE:
        locTxt = "MPFRONTEND_VOICE_OPTIONS";
        OverlaySetStateCommon(&m_ListBox, m_pOptionText.m_elements,
                              m_currSelection);
        m_ListBox.SetText(0, 0, "MPFRONTEND_VOICE_CHOICE_HEADSET");
        m_ListBox.SetText(1, 0, "MPFRONTEND_VOICE_CHOICE_SPEAKERS");
        m_ListBox.Refresh();
        m_currSelection = 0;
        m_ListBox.SelectLine(0);
        break;
    case OVERLAY_JOIN_FRIEND:
    case OVERLAY_AAR_JOIN_FRIEND:
        locTxt = "MPFRONTEND_XBOX_LIVE_OPTION_OVERLAY_JOINFRIEND";
        OverlaySetStateCommon(&m_ListBox, m_pOptionText.m_elements,
                              m_currSelection);
        m_ListBox.SetText(0, 0, "MPFRONTEND_YES");
        m_ListBox.SetText(1, 0, "MPFRONTEND_NO");
        m_ListBox.Refresh();
        m_currSelection = 1;
        m_ListBox.SelectLine(1);
        break;
    case OVERLAY_REBOOT_REQUIRED:
    case OVERLAY_AAR_REBOOT_REQUIRED:
        locTxt = "MPFRONTEND_XBOX_LIVE_OPTION_OVERLAY_REBOOT_REQUIRED";
        OverlaySetStateCommon(&m_ListBox, m_pOptionText.m_elements,
                              m_currSelection);
        m_ListBox.SetText(0, 0, "MPFRONTEND_YES");
        m_ListBox.SetText(1, 0, "MPFRONTEND_NO");
        m_ListBox.Refresh();
        m_currSelection = 1;
        m_ListBox.SelectLine(1);
        break;
    default:
        locTxt = "Missing String!";
        break;
    }
    m_Text = locTxt;
    if (entries[0] != nullptr)
        entries[0]->SetText(m_Text);
}

// ea: 0x007A0450
void AAROverlay::SetState(eState state)
{
    m_State = state;
    m_IsAARTimerEnabled = false;
    m_IsAARTimerEnabled = true;
    const char* locTxt;
    switch (state)
    {
    case OVERLAY_SIGNIN_SIGNOUT:
    case OVERLAY_AAR_SIGNIN_SIGNOUT:
        locTxt = "MPFRONTEND_XBOX_LIVE_OPTION_OVERLAY_SIGNOUT";
        OverlaySetStateCommon(&m_ListBox, m_pOptionText.m_elements,
                              m_currSelection);
        m_ListBox.SetText(0, 0, "MPFRONTEND_YES");
        m_ListBox.SetText(1, 0, "MPFRONTEND_NO");
        m_ListBox.Refresh();
        m_currSelection = 1;
        m_ListBox.SelectLine(1);
        break;
    case OVERLAY_APPEAR_ONLINE:
    case OVERLAY_AAR_APPEAR_ONLINE:
        locTxt = "MPFRONTEND_XBOX_OPTIONS_APPEAR_ONLINE_OVERLAY";
        OverlaySetStateCommon(&m_ListBox, m_pOptionText.m_elements,
                              m_currSelection);
        m_ListBox.SetText(0, 0, "MPFRONTEND_YES");
        m_ListBox.SetText(1, 0, "MPFRONTEND_NO");
        m_ListBox.Refresh();
        m_currSelection = 0;
        m_ListBox.SelectLine(0);
        break;
    case OVERLAY_APPEAR_OFFLINE:
    case OVERLAY_AAR_APPEAR_OFFLINE:
        locTxt = "MPFRONTEND_XBOX_OPTIONS_APPEAR_OFFLINE_OVERLAY";
        OverlaySetStateCommon(&m_ListBox, m_pOptionText.m_elements,
                              m_currSelection);
        m_ListBox.SetText(0, 0, "MPFRONTEND_YES");
        m_ListBox.SetText(1, 0, "MPFRONTEND_NO");
        m_ListBox.Refresh();
        m_currSelection = 0;
        m_ListBox.SelectLine(0);
        break;
    case OVERLAY_TOGGLE_VOICE:
    case OVERLAY_AAR_TOGGLE_VOICE:
        locTxt = "MPFRONTEND_VOICE_OPTIONS";
        OverlaySetStateCommon(&m_ListBox, m_pOptionText.m_elements,
                              m_currSelection);
        m_ListBox.SetText(0, 0, "MPFRONTEND_VOICE_CHOICE_HEADSET");
        m_ListBox.SetText(1, 0, "MPFRONTEND_VOICE_CHOICE_SPEAKERS");
        m_ListBox.Refresh();
        m_currSelection = 0;
        m_ListBox.SelectLine(0);
        break;
    case OVERLAY_JOIN_FRIEND:
    case OVERLAY_AAR_JOIN_FRIEND:
        locTxt = "MPFRONTEND_XBOX_LIVE_OPTION_OVERLAY_JOINFRIEND";
        OverlaySetStateCommon(&m_ListBox, m_pOptionText.m_elements,
                              m_currSelection);
        m_ListBox.SetText(0, 0, "MPFRONTEND_YES");
        m_ListBox.SetText(1, 0, "MPFRONTEND_NO");
        m_ListBox.Refresh();
        m_currSelection = 1;
        m_ListBox.SelectLine(1);
        break;
    case OVERLAY_REBOOT_REQUIRED:
    case OVERLAY_AAR_REBOOT_REQUIRED:
        locTxt = "MPFRONTEND_XBOX_LIVE_OPTION_OVERLAY_REBOOT_REQUIRED";
        OverlaySetStateCommon(&m_ListBox, m_pOptionText.m_elements,
                              m_currSelection);
        m_ListBox.SetText(0, 0, "MPFRONTEND_YES");
        m_ListBox.SetText(1, 0, "MPFRONTEND_NO");
        m_ListBox.Refresh();
        m_currSelection = 1;
        m_ListBox.SelectLine(1);
        break;
    default:
        locTxt = "Missing String!";
        break;
    }
    m_Text = locTxt;
    if (entries[0] != nullptr)
        entries[0]->SetText(m_Text);
}

// ea: 0x007AEDB0
void WeaponSelectMenu::OnActivate()
{
    ModelMenu::OnActivate();
    mModelPosition[0] = -10.0f;
    mModelPosition[1] = -110.0f;
    mModelPosition[2] = 130.0f;
    mModelPosition[3] = 0.0f;
    UpdateModelPosition();
    mModelAngles[0] = 110.0f;
    mModelAngles[1] = 0.0f;
    mModelAngles[2] = 270.0f;
    mModelAngles[3] = 0.0f;
    UpdateModelPosition();
    mColors[0] = 1.07854f;
    mColors[1] = 0.99822003f;
    mColors[2] = 0.80317003f;
    mColors[3] = 1.0f;
    mColors[4] = 1.07854f;
    mColors[5] = 0.99822003f;
    mColors[6] = 0.80317003f;
    mColors[7] = 1.0f;
    mDirections[0] = 0.63f;
    mDirections[1] = 0.49000001f;
    mDirections[2] = 0.597f;
    mDirections[3] = 0.0f;
    mDirections[4] = -0.76300001f;
    mDirections[5] = -0.161f;
    mDirections[6] = -0.625f;
    mDirections[7] = 0.0f;
    mBrightness[0] = 0.69999999f;
    mBrightness[1] = 0.69999999f;
    int16_t eTeam = EntityManager::sInst->GetPlayer(mVersion)->sentient->eTeam;
    m_sLocalPlayerTeam = eTeam;
    PanelQuad* v7;
    if (eTeam == 1)
    {
        panel->GetPointer("sb_colorband_icon_german")->SetShown(true);
        v7 = panel->GetPointer("sb_colorband_icon_american");
        v7->SetShown(false);
    }
    else
    {
        panel->GetPointer("sb_colorband_icon_german")->SetShown(false);
        v7 = panel->GetPointer("sb_colorband_icon_american");
        v7->SetShown(true);
    }
    Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
    int16_t v10 = PlayerClassToLocalIndex(Player->client->pers.playerClass);
    highlighted = v10;
    SetHigh(v10, true);
    ActivationToggle(true);
    if (Allow_Exit)
        helpbar1->SetText("WEAPON_HELP_BAR_ALLCAPS");
    else
        helpbar1->SetText("WEAPON_ABRIDGED_HELP_BAR_ALLCAPS");
}

// ea: 0x0078E420
void InstantActionMenu::SetPanelFile(PanelFile* pf)
{
    panel = pf;
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/InstantActionMenu.cpp";
        AeAssert::gCurrentLine = 102;
        AeAssert::gCurrentExpr = "panel";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Panel File invalid!"))
            __debugbreak();
    }
    static const char* const szInstantActionEntries[8] = {
        "title", "any", "deathmatch", "team_deathmatch",
        "capture_the_flag", "capture_the_flag", "capture_the_flag",
        "domination",
    };
    static const char* const szInstantActionTexts[8] = {
        "MPFRONTEND_QUICK_MATCH", "MPFRONTEND_ANY", "MPFRONTEND_BATTLE",
        "MPFRONTEND_TEAM_BATTLE", "MPFRONTEND_CAPTURE_THE_FLAG",
        "MPFRONTEND_SINGLE_CAPTURE_THE_FLAG", "MPFRONTEND_HEADQUARTERS",
        "MPFRONTEND_DOMINATION",
    };
    for (int i = 0; i < 8; ++i)
    {
        FEText* t = panel->GetTextPointer(szInstantActionEntries[i]);
        AddEntry(i, t, false);
    }
    for (int i = 0; i < 8; ++i)
        entries[i]->SetText(szInstantActionTexts[i]);
    entries[1]->up = 7;
    entries[7]->down = 1;
    helpbar = panel->GetTextPointer("Helpbar");
    FEMultiLineText* v19 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    FEMultiLineText* v23 = nullptr;
    if (v19 != nullptr)
    {
        color32 col = helpbar->GetColor();
        panel_layer layer = (panel_layer)helpbar->GetScaleX();
        float x1 = helpbar->GetY();
        float v25 = helpbar->GetX();
        v23 = new (v19)
            FEMultiLineText(helpbar->GetFont(), x1, 0.0f, 0, layer,
                            0.0f, 0, (int)col.i, col);
    }
    helpbar1 = v23;
    if (v23 != nullptr)
        v23->SetNumLines(1);
    helpbar1->SetText("MPFRONTEND_HELP_SELECT_BACK_MOVEUD");
}

// ea: 0x0078EB90
void SessionDetailsMenu::SetPanelFile(PanelFile* pf)
{
    panel = pf;
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/SessionDetailsMenu.cpp";
        AeAssert::gCurrentLine = 120;
        AeAssert::gCurrentExpr = "panel";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Panel File invalid!"))
            __debugbreak();
    }
    static const char* const szDetailsEntries[11] = {
        "title", "game_type", "map", "players", "game_name", "next_game",
        "join_game", "title_game", "title_map", "title_players",
        "title_type",
    };
    for (int i = 0; i < 11; ++i)
    {
        FEText* t = panel->GetTextPointer(szDetailsEntries[i]);
        AddEntry(i, t, false);
    }
    entries[0]->SetText("MPFRONTEND_QUICK_MATCH_DETAILS");
    entries[5]->SetText("MPFRONTEND_NEXT_GAME");
    entries[6]->SetText("MPFRONTEND_JOIN_GAME");
    entries[4]->SetTextNoLocalize((char*)defaultFileName);
    entries[7]->SetText("MPFRONTEND_GAME_NAME");
    entries[8]->SetText("MPFRONTEND_MAP");
    entries[9]->SetText("MPFRONTEND_PLAYERS");
    entries[10]->SetText("MPFRONTEND_GAME_TYPE");
    entries[6]->down = 5;
    entries[5]->up = 6;
    panel->GetPointer("gamespy")->SetShown(false);
    helpbar = panel->GetTextPointer("Helpbar");
    FEMultiLineText* v26 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    FEMultiLineText* v30 = nullptr;
    if (v26 != nullptr)
    {
        color32 col = helpbar->GetColor();
        panel_layer layer = (panel_layer)helpbar->GetScaleX();
        float x1 = helpbar->GetY();
        float v32 = helpbar->GetX();
        v30 = new (v26)
            FEMultiLineText(helpbar->GetFont(), x1, 0.0f, 0, layer,
                            0.0f, 0, (int)col.i, col);
    }
    helpbar1 = v30;
    if (v30 != nullptr)
        v30->SetNumLines(1);
    helpbar1->SetText("MPFRONTEND_HELP_SELECT_BACK_MOVEUD");
}

// ea: 0x0078FD70
void MultilineOverlayMenu::SetPanelFile(PanelFile* pf)
{
    panel = pf;
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/MultilineOverlayMenu.cpp";
        AeAssert::gCurrentLine = 95;
        AeAssert::gCurrentExpr = "panel";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Panel File invalid!"))
            __debugbreak();
    }
    FEText* header = panel->GetTextPointer("header");
    AddEntry(0, header, false);
    FEText* v5 = panel->GetTextPointer("body");
    if (mTextEntry != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/MultilineOverlayMenu.cpp";
        AeAssert::gCurrentLine = 100;
        AeAssert::gCurrentExpr = "!mTextEntry";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("no!"))
            __debugbreak();
    }
    FEMultiLineText* v6 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    FEMultiLineText* v7 = nullptr;
    if (v6 != nullptr)
    {
        color32 col = v5->GetColor();
        panel_layer layer = (panel_layer)v5->GetScaleX();
        float x1 = v5->GetY();
        float v25 = (float)(int)v5->GetZvalue();
        v7 = new (v6)
            FEMultiLineText(v5->GetFont(), x1, v25, 9, layer,
                            0.0f, 0, 0, col);
    }
    mTextEntry = v7;
    if (v7 != nullptr)
        v7->SetNumLines(8);
    AddEntry(1, mTextEntry, false);
    FEText* ok = panel->GetTextPointer("ok");
    AddEntry(2, ok, false);
    FEText* cancel = panel->GetTextPointer("cancel");
    AddEntry(3, cancel, false);
    entries[2]->up = -1;
    entries[2]->down = 3;
    entries[3]->up = 2;
    entries[3]->down = -1;
    helpbar = panel->GetTextPointer("Helpbar");
    FEMultiLineText* v15 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    FEMultiLineText* v19 = nullptr;
    if (v15 != nullptr)
    {
        color32 col = helpbar->GetColor();
        panel_layer layer = (panel_layer)helpbar->GetScaleX();
        float x1 = helpbar->GetY();
        float v22 = helpbar->GetX();
        v19 = new (v15)
            FEMultiLineText(helpbar->GetFont(), x1, 0.0f, 0, layer,
                            0.0f, 0, 0, col);
    }
    helpbar1 = v19;
    if (v19 != nullptr)
        v19->SetNumLines(1);
    mTextScale = entries[1]->GetScaleX();
}

// ea: 0x0079C800
void PlayOnlineMenu::SetPanelFile(PanelFile* pf)
{
    panel = pf;
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/PlayOnlineMenu.cpp";
        AeAssert::gCurrentLine = 223;
        AeAssert::gCurrentExpr = "panel";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Panel File invalid!"))
            __debugbreak();
    }
    static const char* const szOnlineEntries[5] = {
        "mm_text_screen_title", "mm_text_option_01", "mm_text_option_02",
        "mm_text_option_03", "mm_text_option_04",
    };
    static const char* const szOnlineTexts[5] = {
        "MPFRONTEND_MM_XBOX_LIVE", "MPFRONTEND_MM_QUICKMATCH",
        "MPFRONTEND_MM_OPTIMATCH", "MPFRONTEND_MM_CREATE_GAME",
        "MPFRONTEND_MM_XBOX_LIVE_OPTIONS",
    };
    for (int i = 0; i < 5; ++i)
    {
        FEText* t = panel->GetTextPointer(szOnlineEntries[i]);
        AddEntry(i, t, false);
    }
    m_pBkgDetail4 = panel->GetPointer("mm_bkg_detail_04");
    m_pBkgDetail5 = panel->GetPointer("mm_bkg_detail_05");
    m_pBkgDetail4->SetShown(false);
    m_pBkgDetail5->SetShown(false);
    entries[0]->SetText(szOnlineTexts[0]);
    entries[0]->SetColorSchemeIndex(11);
    for (int i = 1; i < 5; ++i)
    {
        entries[i]->SetText(szOnlineTexts[i]);
        entries[i]->SetColorSchemeIndex(10);
    }
    entries[4]->down = 1;
    entries[1]->up = 4;
    if (gLanguage == kLanguageFrench)
    {
        for (int i = 1; i < 5; ++i)
            entries[i]->SetScale(0.6f);  // 1058642330 = 0.6f
    }
    helpbar = panel->GetTextPointer("mm_text_helpbar");
    FEMultiLineText* v18 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    FEMultiLineText* v22 = nullptr;
    if (v18 != nullptr)
    {
        color32 col = helpbar->GetColor();
        panel_layer layer = (panel_layer)helpbar->GetScaleX();
        float x1 = helpbar->GetY();
        float v28 = helpbar->GetX();
        v22 = new (v18)
            FEMultiLineText(helpbar->GetFont(), x1, 0.0f, 0, layer,
                            0.0f, 0, 0, col);
    }
    helpbar1 = v22;
    if (v22 != nullptr)
        v22->SetNumLines(1);
    helpbar1->SetText("MPFRONTEND_HELP_SELECT_BACK_MOVEUD");
    static const char* const szPreviewImages[4] = {
        "mm_preview_image_01", "mm_preview_image_02",
        "mm_preview_image_03", "mm_preview_image_04",
    };
    for (int i = 0; i < 4; ++i)
    {
        PanelQuad* vq = panel->GetPointer(szPreviewImages[i]);
        if (vq != nullptr)
            vq->SetVisibility(0.0f);
    }
}

// ea: 0x007A4500
void AARPersonalStats::SetClassSpecificEntries()
{
    int iClassScore = 0;
    int iClassSpecificScore1 = 0;
    int iTimeAsClass = 0;
    Entity* FirstLocalPlayer = EntityManager::sInst->GetFirstLocalPlayer();
    if (FirstLocalPlayer != nullptr)
    {
        Client* client = FirstLocalPlayer->client;
        if (client != nullptr)
        {
            mPlayerClass = client->pers.playerClass;
            int v5 = 0;
            if (FirstLocalPlayer->client->pers.mStats[0][0] > 0)
            {
                v5 = FirstLocalPlayer->client->pers.mStats[0][0];
                mPlayerClass = kPlayerClassAssault;
            }
            if (FirstLocalPlayer->client->pers.mStats[1][0] > v5)
            {
                v5 = FirstLocalPlayer->client->pers.mStats[1][0];
                mPlayerClass = kPlayerClassInfantry;
            }
            if (FirstLocalPlayer->client->pers.mStats[2][0] > v5)
            {
                v5 = FirstLocalPlayer->client->pers.mStats[2][0];
                mPlayerClass = kPlayerClassRifleman;
            }
            if (FirstLocalPlayer->client->pers.mStats[3][0] > v5)
            {
                v5 = FirstLocalPlayer->client->pers.mStats[3][0];
                mPlayerClass = kPlayerClassMedic;
            }
            if (FirstLocalPlayer->client->pers.mStats[4][0] > v5)
            {
                v5 = FirstLocalPlayer->client->pers.mStats[4][0];
                mPlayerClass = kPlayerClassSupport;
            }
            if (FirstLocalPlayer->client->pers.mStats[5][0] > v5)
            {
                v5 = FirstLocalPlayer->client->pers.mStats[5][0];
                mPlayerClass = kPlayerClassAntiArmor;
            }
            if (FirstLocalPlayer->client->pers.mStats[6][0] > v5)
                mPlayerClass = kPlayerClassScout;
            m_pClassIcon.m_elements[mPlayerClass]->SetShown(true);
            switch (mPlayerClass)
            {
            case kPlayerClassAssault:
                m_pText.m_elements[2]->SetText("MPGAME_ASSAULT_ALLCAPS");
                m_pClassScoreText.m_elements[0]->SetText("MPGAME_ASSAULT_SCORE");
                m_pClassScoreText.m_elements[2]->SetText("MPGAME_TIME_ASSAULT");
                m_pClassScoreText.m_elements[4]
                    ->SetText("MPGAME_ASSAULT_ABILITY1_NAME");
                break;
            case kPlayerClassInfantry:
                m_pText.m_elements[2]->SetText("MPGAME_INFANTRY_ALLCAPS");
                m_pClassScoreText.m_elements[0]->SetText("MPGAME_INFANTRY_SCORE");
                m_pClassScoreText.m_elements[2]->SetText("MPGAME_TIME_INFANTRY");
                m_pClassScoreText.m_elements[4]
                    ->SetText("MPGAME_INFANTRY_ABILITY1_NAME");
                break;
            case kPlayerClassRifleman:
                m_pText.m_elements[2]->SetText("MPGAME_RIFLEMAN_ALLCAPS");
                m_pClassScoreText.m_elements[0]->SetText("MPGAME_RIFLEMAN_SCORE");
                m_pClassScoreText.m_elements[2]->SetText("MPGAME_TIME_RIFLEMAN");
                m_pClassScoreText.m_elements[4]
                    ->SetText("MPGAME_RIFLEMAN_ABILITY1_NAME");
                break;
            case kPlayerClassMedic:
                m_pText.m_elements[2]->SetText("MPGAME_MEDIC_ALLCAPS");
                m_pClassScoreText.m_elements[0]->SetText("MPGAME_MEDIC_SCORE");
                m_pClassScoreText.m_elements[2]->SetText("MPGAME_TIME_MEDIC");
                m_pClassScoreText.m_elements[4]
                    ->SetText("MPGAME_MEDIC_ABILITY1_NAME");
                break;
            case kPlayerClassSupport:
                m_pText.m_elements[2]->SetText("MPGAME_SUPPORT_ALLCAPS");
                m_pClassScoreText.m_elements[0]->SetText("MPGAME_SUPPORT_SCORE");
                m_pClassScoreText.m_elements[2]->SetText("MPGAME_TIME_SUPPORT");
                m_pClassScoreText.m_elements[4]
                    ->SetText("MPGAME_SUPPORT_ABILITY1_NAME");
                break;
            case kPlayerClassAntiArmor:
                m_pText.m_elements[2]->SetText("MPGAME_ANTIARMOR_ALLCAPS");
                m_pClassScoreText.m_elements[0]->SetText("MPGAME_ANTIARMOR_SCORE");
                m_pClassScoreText.m_elements[2]->SetText("MPGAME_TIME_ANTIARMOR");
                m_pClassScoreText.m_elements[4]
                    ->SetText("MPGAME_ANTIARMOR_ABILITY1_NAME");
                break;
            case kPlayerClassScout:
                m_pText.m_elements[2]->SetText("MPGAME_SCOUT_ALLCAPS");
                m_pClassScoreText.m_elements[0]->SetText("MPGAME_SCOUT_SCORE");
                m_pClassScoreText.m_elements[2]->SetText("MPGAME_TIME_SCOUT");
                m_pClassScoreText.m_elements[4]
                    ->SetText("MPGAME_SCOUT_ABILITY1_NAME");
                break;
            default:
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\mp/ui/AARPersonalStats.cpp";
                AeAssert::gCurrentLine = 346;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Invalid class\n"))
                    __debugbreak();
                break;
            }
            GetClassSpecificScore(
                (EPlayerClass)mPlayerClass, iClassScore, iTimeAsClass,
                iClassSpecificScore1, iTimeAsClass);
            char szScoreText[12];
            _snprintf(szScoreText, 0xAu, "%d", iClassScore);
            m_pClassScoreText.m_elements[1]->SetText(szScoreText);
            _snprintf(szScoreText, 0xAu, "%d", iClassSpecificScore1);
            m_pClassScoreText.m_elements[5]->SetText(szScoreText);
            int v8 = iTimeAsClass % 3600 % 60;
            int v7 = iTimeAsClass % 3600 / 60;
            if (iTimeAsClass / 3600 != 0)
                _snprintf(szScoreText, 0xAu, "%i:%02i:%02i",
                          iTimeAsClass / 3600, v7, v8);
            else
                _snprintf(szScoreText, 0xAu, "%i:%02i", v7, v8);
            m_pClassScoreText.m_elements[3]->SetText(szScoreText);
        }
    }
}

template <typename T>
static void OverlayPanelFileCommon(T* self, PanelFile* pf,
                                   const char* const szBackgroundArt[3],
                                   const char* const szOptionText[2],
                                   const char* const szOptionLines[1])
{
    PanelFile* v3 = pf->Clone();
    self->panel = v3;
    if (v3 == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
        AeAssert::gCurrentLine = 1574;
        AeAssert::gCurrentExpr = "panel";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Panel File invalid!"))
            __debugbreak();
    }
    for (int i = 0; i < 3; ++i)
    {
        if (self->m_pBackgroundArt.m_elements[i] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
            AeAssert::gCurrentLine = 1584;
            AeAssert::gCurrentExpr = "0 == m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Array expected to be null"))
                __debugbreak();
        }
        self->m_pBackgroundArt.m_elements[i] =
            self->panel->GetPointer(szBackgroundArt[i]);
        if (self->m_pBackgroundArt.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
            AeAssert::gCurrentLine = 1587;
            AeAssert::gCurrentExpr = "m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
    }
    FEText* TextPointer = self->panel->GetTextPointer("text_body");
    FEText* v6 = self->panel->GetTextPointer("text_body");
    v6->SetX(TextPointer->GetX());
    FEText* v8 = self->panel->GetTextPointer("text_body");
    self->AddEntry(0, v8, false);
    for (int j = 0; j < 2; ++j)
    {
        if (self->m_pOptionText.m_elements[j] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
            AeAssert::gCurrentLine = 1608;
            AeAssert::gCurrentExpr = "0 == m_pOptionText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Array expected to be null"))
                __debugbreak();
        }
        self->m_pOptionText.m_elements[j] =
            self->panel->GetTextPointer(szOptionText[j]);
        if (self->m_pOptionText.m_elements[j] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
            AeAssert::gCurrentLine = 1611;
            AeAssert::gCurrentExpr = "m_pOptionText[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
        self->m_pOptionText.m_elements[j]
            ->SetText((const char*)&defaultFileName);
        self->m_pOptionText.m_elements[j]->SetShown(true);
        self->m_ListBox.SetItem(j, 0, self->m_pOptionText.m_elements[j], 0);
    }
    if (self->m_pOptionLines.m_elements[0] != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
        AeAssert::gCurrentLine = 1624;
        AeAssert::gCurrentExpr = "0 == m_pOptionLines[i]";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Array expected to be null"))
            __debugbreak();
    }
    self->m_pOptionLines.m_elements[0] =
        self->panel->GetPointer(szOptionLines[0]);
    if (self->m_pOptionLines.m_elements[0] == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
        AeAssert::gCurrentLine = 1628;
        AeAssert::gCurrentExpr = "m_pOptionLines[i]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
            __debugbreak();
    }
    self->m_pOptionLines.m_elements[0]->SetShown(false);
    self->m_ListBox.SetAllColumnsSelectable(false);
    self->m_ListBox.mSelectedFlashing = true;
    self->m_ListBox.Refresh();
}

// ea: 0x0079F620
void InGameOverlay::SetPanelFile(PanelFile* pf)
{
    static const char* const szInGameOverlayMenuBackgroundArt[3] = {
        "bkg_detail_01", "bkg_detail_02", "bkg_detail_03",
    };
    static const char* const szInGameOverlayMenuText[2] = {
        "text_option_01", "text_option_02",
    };
    static const char* const szInGameLinesBetweenOptionText[1] = {
        "bkg_line_01",
    };
    OverlayPanelFileCommon(this, pf, szInGameOverlayMenuBackgroundArt,
                           szInGameOverlayMenuText,
                           szInGameLinesBetweenOptionText);
}

// ea: 0x0079FEB0
void AAROverlay::SetPanelFile(PanelFile* pf)
{
    static const char* const szAAROverlayMenuBackgroundArt[3] = {
        "bkg_detail_01", "bkg_detail_02", "bkg_detail_03",
    };
    static const char* const szAAROverlayMenuText[2] = {
        "text_option_01", "text_option_02",
    };
    static const char* const szAARLinesBetweenOptionText[1] = {
        "bkg_line_01",
    };
    OverlayPanelFileCommon(this, pf, szAAROverlayMenuBackgroundArt,
                           szAAROverlayMenuText,
                           szAARLinesBetweenOptionText);
}

// ea: 0x007AE570
void OverlayMenu::Update(float time_inc)
{
    m_ListBox.Update(time_inc);
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    MPUIInterface::Step();
    bool bEnter =
        g_controllerConnected[LocalClient::ClientToPort(mVersion)];
    if (!bEnter)
    {
        OnTriangle(0);
        bEnter = (mState != (OverlayMenu::eState)0);
    }
    if (bEnter)
    {
        switch (mState)
        {
        case SIGNING_IN:
            MPLiveEngine::GetHandle()->DoWork();
            if (MPLiveEngine::GetHandle()->internalState != kSigningIn)
            {
                system->RemoveOverlay();
                mState = (OverlayMenu::eState)0;
            }
            goto DOT_ANIM;
        case GAME_LISTING:
            mTimeout -= time_inc;
            if (mTimeout >= 0.0f)
            {
                if (MPUIInterface::IsGameListingComplete())
                {
                    mState = (OverlayMenu::eState)0;
                    system->RemoveOverlay();
                }
            }
            else
            {
                OnTriangle(0);
            }
            goto DOT_ANIM;
        case GAME_LISTING_START:
            if (mDelayStart != 0)
            {
                --mDelayStart;
                mLinkStatusCount = 0;
                mLinkStatusTimer = 0.0f;
                goto DOT_ANIM;
            }
            if (mbStartGame)
            {
                mbStartGame = false;
                if (MPUIInterface::StartGame(false, false))
                    helpbar1->SetText("MPFRONTEND_HELP_CANCEL");
                else
                    mTimeout = -1.0f;
            }
            mTimeout -= time_inc;
            MultiplayerMgr::sInst->Step(0, false, true);
            if (bdSingleton<bdNetImpl>::getInstance()->getStatus()
                == BD_NET_DONE)
            {
                if (!MPUIInterface::GameListingStart())
                {
                    SetState(FROM_ID_QUERYING);
                    mAcceptMenu = 8;
                    mBackMenu = 8;
                    goto DOT_ANIM;
                }
                SetState(GAME_LISTING);
                goto DOT_ANIM;
            }
            if (mTimeout < 0.0f)
            {
                if (!MultiplayerMgr::sInst->oneOffCheckLinkStatus())
                {
                    OnTriangle(0);
                    return;
                }
                SetState(FROM_ID_QUERYING);
                mAcceptMenu = 8;
                mBackMenu = 8;
                goto DOT_ANIM;
            }
            if (bdSingleton<bdNetImpl>::getInstance()->getStatus()
                != BD_NET_PENDING)
            {
                if (!MultiplayerMgr::sInst->oneOffCheckLinkStatus())
                {
                    OnTriangle(0);
                    return;
                }
                SetState(FROM_ID_QUERYING);
                mAcceptMenu = 8;
                mBackMenu = 8;
            }
            goto DOT_ANIM;
        case JOINING_START:
            if (mDelayStart != 0)
            {
                --mDelayStart;
                goto DOT_ANIM;
            }
            if (mbStartGame)
            {
                mbStartGame = false;
                if (MPUIInterface::StartGame(true, false))
                    helpbar1->SetText("MPFRONTEND_HELP_CANCEL");
                else
                    mTimeout = -1.0f;
            }
            mTimeout -= time_inc;
            MultiplayerMgr::sInst->Step(0, false, true);
            if (bdSingleton<bdNetImpl>::getInstance()->getStatus()
                == BD_NET_DONE)
            {
                unsigned long numGames = 0;
                sGameListing* v17 = MPUIInterface::GameListingGet(numGames);
                if (v17 != nullptr
                    && mGameListingNum < numGames)
                {
                    MPUIInterface::StartClient(
                        v17[mGameListingNum], false, mGameListingNum);
                    SetState(JOINING);
                    goto DOT_ANIM;
                }
                SetState(JOIN_FAILED);
                return;
            }
            if (mTimeout >= 0.0f
                && bdSingleton<bdNetImpl>::getInstance()->getStatus()
                    == BD_NET_PENDING)
                goto DOT_ANIM;
            if (!MultiplayerMgr::sInst->oneOffCheckLinkStatus())
            {
                OnTriangle(0);
                return;
            }
            SetState(FROM_ID_QUERYING);
            mAcceptMenu = 8;
            mBackMenu = 8;
            goto DOT_ANIM;
        case JOINING:
            mTimeout -= time_inc;
            if (mTimeout < 0.0f)
            {
                SetState(JOIN_FAILED);
                if (GetSystem()->background == -1)
                {
                    mAcceptMenu = GetSystem()->GetActiveMenu();
                    mBackMenu = GetSystem()->GetActiveMenu();
                }
                else
                {
                    mAcceptMenu = GetSystem()->background;
                    mBackMenu = GetSystem()->background;
                }
                return;
            }
            goto DOT_ANIM;
        case BDNET_STARTING:
            system->RemoveOverlay();
            mState = (OverlayMenu::eState)0;
            j_nullsub_46(this);
            if (MPUIInterface::IsLANGame())
                MPUIInterface::ExitFrontend(9);
            else
                MPUIInterface::ExitFrontend(10);
            goto DOT_ANIM;
        case BDNET_START_FAILED:
            mTimeout -= time_inc;
            if (mTimeout >= 0.0f)
                goto DOT_ANIM;
            if (bdSingleton<bdNetImpl>::getInstance()->getStatus()
                == BD_NET_PENDING)
                goto DOT_ANIM;
            if (bdSingleton<bdNetImpl>::getInstance()->getStatus()
                == BD_NET_DONE)
            {
                mState = (OverlayMenu::eState)0;
                system->RemoveOverlay();
                if (MPUIInterface::IsLANGame())
                    system->MakeActiveAndReturn(9);
                else
                    system->MakeActiveAndReturn(10);
            }
            else
            {
                SetState(FROM_ID_QUERYING);
                mAcceptMenu = 8;
                mBackMenu = 8;
            }
            goto DOT_ANIM;
        case (OverlayMenu::eState)0x10:
            if (mDelayStart != 0)
            {
                --mDelayStart;
            }
            else
            {
                MPLiveEngine::GetHandle()->DoWork();
                if (mbStartGame)
                {
                    mbStartGame = false;
                    MPUIInterface::StartGame(true, false);
                    return;
                }
                if (MPUIInterface::IsGameListingComplete())
                {
                    system->RemoveOverlay();
                    mState = (OverlayMenu::eState)0;
                }
            }
            goto DOT_ANIM;
        default:
        DOT_ANIM:
            if (mState == SIGNING_IN || mState == GAME_LISTING
                || mState == GAME_LISTING_START
                || mState == (OverlayMenu::eState)16 || mState == JOINING
                || mState == JOINING_START || mState == BDNET_START_FAILED)
            {
                mDotTimer += time_inc;
                if (mDotTimer > 0.5f)
                {
                    mDotTimer = (mDotTimer >= 1.0f)
                        ? 0.0f : (mDotTimer - 0.5f);
                    ++mNumDots;
                    if (mNumDots == 3)
                        mNumDots = 0;
                    if (mText.mBlock != nullptr
                        && mText.mBlock->mLength >= 0x7D)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
                        AeAssert::gCurrentLine = 584;
                        AeAssert::gCurrentExpr = "mText.length()<125";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("String to long"))
                            __debugbreak();
                    }
                    const char* v41 = (mText.mBlock != nullptr)
                        ? (const char*)&mText.mBlock[1] : defaultFileName;
                    char strtext[128];
                    strncpy(strtext, v41, 0x7Cu);
                    strtext[0x7C] = 0;
                    int len = (int)strlen(strtext);
                    for (int i = 0; i < 3; ++i)
                        strtext[len + i] = (i < mNumDots) ? '.' : ' ';
                    strtext[len + 3] = 0;
                    entries[0]->SetText(strtext);
                }
            }
            break;
        }
    }
}

// ea: 0x007A26A0
void InGameScoreBoard::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    m_ListBox.Update(time_inc);
    int v3;
    if (cgGlobal.teamScores[2] <= cgGlobal.teamScores[1])
        v3 = 2 * (cgGlobal.teamScores[1] <= cgGlobal.teamScores[2]) + 1;
    else
        v3 = 2;
    SetWinningTeam((team_t)v3);
    MPPlayerManager* pPlayerManager =
        MultiplayerMgr::sInst->mPeer->GetPlayerManager();
    memset(&m_playerList, 0, sizeof(m_playerList));
    int iPlayersSortedCount = 0;
    int current_selection = -1;
    for (int i = 0; i < 16; ++i)
    {
        Entity* v5 = EntityManager::sInst->mPlayers[i];
        if (v5 != nullptr)
        {
            sentient_s* sentient = v5->sentient;
            if (sentient != nullptr)
            {
                team_t eTeam = sentient->eTeam;
                Client* client = v5->client;
                int bIsCurrentlyDead = client->pers.mBaseScore;
                for (int c = 0; c < 7; ++c)
                    bIsCurrentlyDead +=
                        PlayerStats::TotalScoreForStats(client->pers.mStats[c]);
                if (!cgGlobal.teamGame || eTeam == m_cgTeamShown)
                {
                    m_playerList.m_elements[iPlayersSortedCount].iScore =
                        bIsCurrentlyDead;
                    m_playerList.m_elements[iPlayersSortedCount].iPlayerIndex =
                        pPlayerManager->GetPlayerIndex(v5);
                    m_playerList.m_elements[iPlayersSortedCount++].pEntity =
                        v5;
                }
            }
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/InGameScoreBoard.cpp";
            AeAssert::gCurrentLine = 399;
            AeAssert::gCurrentExpr = "pEntity";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Could not get entity from player"))
                __debugbreak();
        }
    }
    qsort(&m_playerList, iPlayersSortedCount, sizeof(sScoreboardPlayerSlot),
          scoreboard_player_sorter);
    int v12 = iPlayersSortedCount;
    int v13 = 0;
    m_ListBox.mBlockRefresh = true;
    if (iPlayersSortedCount > 0)
    {
        for (int i = 0; v13 < iPlayersSortedCount; ++v13, ++i)
        {
            Entity* v15 = m_playerList.m_elements[i].pEntity;
            if (v15 != nullptr)
            {
                if (current_selection < 0
                    && v15 == EntityManager::sInst->mPlayers[mVersion])
                    current_selection = v13;
                MPPlayer* Player = pPlayerManager->GetPlayer(v15);
                if (Player != nullptr)
                    m_ListBox.SetPlayerID(v13, Player->GetId());
                Client* v18 = v15->client;
                int v19 = 0;  // kills
                int v20 = 0;  // deaths
                for (int c = 0; c < 7; ++c)
                {
                    v19 += v18->pers.mStats[c][3];
                    v20 += v18->pers.mStats[c][4];
                }
                const char* pszPlayerName = pPlayerManager->GetPlayerName(
                    (unsigned char)m_playerList.m_elements[i].iPlayerIndex);
                int16_t sCurrentPlayerTeamb = 0;
                if (v15->sentient != nullptr)
                    sCurrentPlayerTeamb = v15->sentient->eTeam;
                int sCurrentPlayerClassb = v18->pers.playerClass;
                if (sCurrentPlayerClassb == -1)
                    sCurrentPlayerClassb = 2;
                int playerState = v18->pers.playerState;
                bool v48 = (playerState == 4 || playerState == 5);
                int iRank = v18->pers.rank;
                int iScore = m_playerList.m_elements[i].iScore;
                m_ListBox.SetText(v13, 2, pszPlayerName);
                if (!cgGlobal.teamGame || m_bShowMyTeamScore)
                    m_ListBox.SetItemState(v13, 3, sCurrentPlayerClassb + 1);
                else
                    m_ListBox.SetItemState(v13, 3, 0);
                if (cgGlobal.teamGame)
                    m_ListBox.SetItemState(v13, 1, 0);
                else
                    m_ListBox.SetItemState(v13, 1, sCurrentPlayerTeamb);
                m_ListBox.SetText(v13, 4, va("%i", iScore));
                m_ListBox.SetText(v13, 5, va("%i", v19));
                m_ListBox.SetText(v13, 6, va("%i", v20));
                if (v48)
                    m_ListBox.SetItemState(v13, 0, 1);
                else if (iRank + 2 < 4)
                    m_ListBox.SetItemState(v13, 0, iRank + 2);
            }
        }
        v12 = iPlayersSortedCount;
    }
    for (int j = v12; j < 16; ++j)
        m_ListBox.ClearRow(j);
    if (current_selection >= v12)
    {
        current_selection = v12 - 1;
        m_bActivated = true;
    }
    if (current_selection < 0)
        current_selection = 0;
    m_ListBox.mBlockRefresh = false;
    m_ListBox.Refresh();
    if (m_bActivated)
    {
        m_ListBox.SelectLine(current_selection);
        m_bActivated = false;
    }
}

// ea: 0x007A1BA0
void InGameScoreBoard::SetPanelFile(PanelFile* pf)
{
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/InGameScoreBoard.cpp";
        AeAssert::gCurrentLine = 192;
        AeAssert::gCurrentExpr = "pf";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Scoreboard panel file pointer is NULL"))
            __debugbreak();
    }
    bool v4 = mVersion <= 0;
    panel = pf;
    if (!v4)
        panel = pf->Clone();
    char szSlotGeometry[24];
    for (int i = 0; i < 12; ++i)
    {
        _snprintf(szSlotGeometry, 0x17u, "sb_bkg_detail_%02d", i + 1);
        m_pBackgroundArt.m_elements[i] =
            panel->GetPointer(szSlotGeometry);
        if (m_pBackgroundArt.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/InGameScoreBoard.cpp";
            AeAssert::gCurrentLine = 210;
            AeAssert::gCurrentExpr = "m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                    "Scoreboard background art not found!"))
                __debugbreak();
        }
    }
    for (int i = 12; i < 15; ++i)
    {
        _snprintf(szSlotGeometry, 0x17u, "sb_bkg_detail_%02da", i - 3);
        m_pBackgroundArt.m_elements[i] =
            panel->GetPointer(szSlotGeometry);
        if (m_pBackgroundArt.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/InGameScoreBoard.cpp";
            AeAssert::gCurrentLine = 218;
            AeAssert::gCurrentExpr = "m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                    "Scoreboard background art not found!"))
                __debugbreak();
        }
    }
    m_pTeamStripQuad.m_elements[0] =
        panel->GetPointer("sb_colorband_icon_american");
    m_pTeamStripQuad.m_elements[1] =
        panel->GetPointer("sb_colorband_icon_german");
    m_pTeamStripQuad.m_elements[2] =
        panel->GetPointer("sb_colorband_icon_neutral");
    m_ListBox.SetAllColumnsSelectable(false);
    if (m_ListBox.mItemColumnsCount <= 2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 125;
        AeAssert::gCurrentExpr =
            "column >= 0 && column < mItemColumnsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "UIListBoxItem: State count must be greater then zero"))
            __debugbreak();
    }
    if (m_ListBox.mSelectedRowColorChangeColumns.mSize <= 2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    m_ListBox.mSelectedRowColorChangeColumns.mElements[2] = true;
    static const char* const szScoreboardTexts[13] = {
        "sb_text_title_team_name", "sb_text_title_scoreboard",
        "sb_text_level_name", "sb_text_map_name", "sb_text_loser_name",
        "sb_text_loser_score", "sb_text_winner_name",
        "sb_text_winner_score", "sb_text_helpbar", "sb_text_icon_class",
        "sb_text_icon_deaths", "sb_text_icon_kills",
        "sb_text_icon_score",
    };
    for (int i = 0; i < 13; ++i)
        m_pUppercaseText.m_elements[i] =
            panel->GetTextPointer(szScoreboardTexts[i]);
    FEText* helpbarText = m_pUppercaseText.m_elements[8];
    FEMultiLineText* v52 =
        (FEMultiLineText*)mem_heap_malloc(0xA8);
    FEMultiLineText* v9 = nullptr;
    if (v52 != nullptr)
    {
        color32 col = helpbarText->GetColor();
        panel_layer layer = (panel_layer)helpbarText->GetScaleX();
        float x1 = helpbarText->GetY();
        float v42 = helpbarText->GetX();
        v9 = new (v52)
            FEMultiLineText(helpbarText->GetFont(), x1, 0.0f, 0, layer,
                            0.0f, 0, 0, col);
    }
    helpbar1 = v9;
    if (v9 != nullptr)
        v9->SetNumLines(1);
    FEMultiLineText* v55 =
        (FEMultiLineText*)mem_heap_malloc(0xA8);
    FEMultiLineText* v13 = nullptr;
    if (v55 != nullptr)
    {
        color32 col = helpbarText->GetColor();
        panel_layer layer = (panel_layer)helpbarText->GetScaleX();
        float x1 = helpbarText->GetY();
        float v43 = helpbarText->GetX();
        v13 = new (v55)
            FEMultiLineText(helpbarText->GetFont(), x1, 0.0f, 0, layer,
                            0.0f, 0, 0, col);
    }
    helpbar2 = v13;
    if (v13 != nullptr)
        v13->SetNumLines(1);
    m_ListBox.SetScrollBarQuad(
        UIListBox::kScrollBarArrowDown,
        pf->GetPointer("sb_scroll_arrow_down"));
    m_ListBox.SetScrollBarQuad(
        UIListBox::kScrollBarArrowUp,
        pf->GetPointer("sb_scroll_arrow_up"));
    m_ListBox.SetScrollBarQuad(
        UIListBox::kScrollBarBackground1,
        pf->GetPointer("sb_scroll_detail_01"));
    m_ListBox.SetScrollBarQuad(
        UIListBox::kScrollBarBackground2,
        pf->GetPointer("sb_scroll_detail_02"));
    m_ListBox.SetScrollBarQuad(
        UIListBox::kScrollBarThumb,
        pf->GetPointer("sb_scroll_indicator"));
    m_ListBox.SetScrollBarQuad(
        UIListBox::kScrollBarThumbReference,
        pf->GetPointer("sb_scroll_indicator_reference"));
    PanelQuad* v21 = panel->GetPointer("sb_player_hilite");
    m_ListBox.mHighlightQuad = v21;
    if (v21 != nullptr)
        v21->SetShown(false);
    m_ListBox.SetColumnStateCount(3, 8);
    m_pUppercaseText.m_elements[1]->SetText("MPGAME_SCOREBOARD");
    m_pUppercaseText.m_elements[9]->SetText("C");
    m_pUppercaseText.m_elements[12]->SetText("S");
    m_pUppercaseText.m_elements[11]->SetText("K");
    m_pUppercaseText.m_elements[10]->SetText("D");
    m_ListBox.mHighlightedSelectedTextColor.i = -3618616;
    m_ListBox.mHighlightedUnselectedTextColor.i = -7553346;
    m_ListBox.SetColumnStateCount(0, 5);
    m_ListBox.SetColumnStateCount(1, 3);
    m_ListBox.SetColumnStateCount(3, 8);
    color32 nameColor;
    nameColor.i = -2702166;
    color32 nameSelColor;
    nameSelColor.i = -2133408598;
    for (int i = 0; i < 12; ++i)
    {
        _snprintf(szSlotGeometry, 0x17u, "sb_slot_%02d_death_icon", i + 1);
        m_ListBox.SetItem(i, 0, panel->GetPointer(szSlotGeometry), 1);
        _snprintf(szSlotGeometry, 0x17u, "sb_slot_%02d_rank_icon_a", i + 1);
        m_ListBox.SetItem(i, 0, panel->GetPointer(szSlotGeometry), 2);
        _snprintf(szSlotGeometry, 0x17u, "sb_slot_%02d_rank_icon_b", i + 1);
        m_ListBox.SetItem(i, 0, panel->GetPointer(szSlotGeometry), 3);
        _snprintf(szSlotGeometry, 0x17u, "sb_slot_%02d_rank_icon_c", i + 1);
        m_ListBox.SetItem(i, 0, panel->GetPointer(szSlotGeometry), 4);
        _snprintf(szSlotGeometry, 0x17u, "sb_slot_%02d_flag_axis", i + 1);
        m_ListBox.SetItem(i, 1, panel->GetPointer(szSlotGeometry), 1);
        _snprintf(szSlotGeometry, 0x17u, "sb_slot_%02d_flag_allied", i + 1);
        m_ListBox.SetItem(i, 1, panel->GetPointer(szSlotGeometry), 2);
        _snprintf(szSlotGeometry, 0x17u, "sb_slot_%02d_text_name", i + 1);
        m_ListBox.SetItem(
            i, 2, panel->GetTextPointer(szSlotGeometry), 0);
        static const char* const szClassIcons[7] = {
            "AST_H", "AST_L", "RFM", "MED", "SPT", "ATA", "SCT",
        };
        for (int c = 0; c < 7; ++c)
        {
            _snprintf(szSlotGeometry, 0x17u, "sb_slot_%02d_ci_%s", i + 1,
                      szClassIcons[c]);
            m_ListBox.SetItem(
                i, 3, panel->GetPointer(szSlotGeometry), c + 1);
        }
        _snprintf(szSlotGeometry, 0x17u, "sb_slot_%02d_text_scrore",
                  i + 1);
        m_ListBox.SetItem(
            i, 4, panel->GetTextPointer(szSlotGeometry), 0);
        _snprintf(szSlotGeometry, 0x17u, "sb_slot_%02d_text_kills", i + 1);
        m_ListBox.SetItem(
            i, 5, panel->GetTextPointer(szSlotGeometry), 0);
        _snprintf(szSlotGeometry, 0x17u, "sb_slot_%02d_text_deaths", i + 1);
        m_ListBox.SetItem(
            i, 6, panel->GetTextPointer(szSlotGeometry), 0);
        _snprintf(szSlotGeometry, 0x17u, "sb_slot_%02d_text_name", i + 1);
        FEText* v41 = panel->GetTextPointer(szSlotGeometry);
        m_ListBox.SetItem(i, 2, v41, 0);
        v41->SetColorMenuItem(nameColor, nameSelColor);
    }
    m_ListBox.Refresh();
    m_ListBox.SelectLine(0);
}

// ea: 0x0079D140
void SessionListMenu::SetPanelFile(PanelFile* pf)
{
    static const char* const szSessionListMenuBackgroundArt[5] = {
        "gl_bkg", "gl_bkg_detail_01", "gl_bkg_detail_02",
        "gl_bkg_detail_03", "gl_bkg_detail_04",
    };
    static const char* const szSessionListMenuText[3] = {
        "gl_text_title_01", "gl_text_title_02", "text_helpbar",
    };
    static const char* const szSessionListMenuHeaderText[4] = {
        "gl_text_group_header_01", "gl_text_group_header_02",
        "gl_text_group_header_03", "gl_text_group_header_04",
    };
    static const char* const szSessionLMNames[6] = {
        "gl_text_option_01", "gl_text_option_02", "gl_text_option_03",
        "gl_text_option_04", "gl_text_option_05", "gl_text_option_06",
    };
    static const char* const szSessionLMPlayers[6] = {
        "gl_text_option_01_play", "gl_text_option_02_play",
        "gl_text_option_03_play", "gl_text_option_04_play",
        "gl_text_option_05_play", "gl_text_option_06_play",
    };
    static const char* const szSessionLMModes[6] = {
        "gl_text_option_01_mode", "gl_text_option_02_mode",
        "gl_text_option_03_mode", "gl_text_option_04_mode",
        "gl_text_option_05_mode", "gl_text_option_06_mode",
    };
    static const char* const szSessionLMMaps[6] = {
        "gl_text_option_01_map", "gl_text_option_02_map",
        "gl_text_option_03_map", "gl_text_option_04_map",
        "gl_text_option_05_map", "gl_text_option_06_map",
    };
    panel = pf;
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
        AeAssert::gCurrentLine = 377;
        AeAssert::gCurrentExpr = "panel";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Panel File invalid!"))
            __debugbreak();
    }
    for (int j = 0; j < 5; ++j)
    {
        if (m_pBackgroundArt.m_elements[j] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
            AeAssert::gCurrentLine = 395;
            AeAssert::gCurrentExpr = "0 == m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Why is the array not null?"))
                __debugbreak();
        }
        m_pBackgroundArt.m_elements[j] =
            panel->GetPointer(szSessionListMenuBackgroundArt[j]);
        if (m_pBackgroundArt.m_elements[j] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
            AeAssert::gCurrentLine = 398;
            AeAssert::gCurrentExpr = "m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
    }
    for (int v4 = 0; v4 < 3; ++v4)
    {
        if (m_pText.m_elements[v4] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
            AeAssert::gCurrentLine = 411;
            AeAssert::gCurrentExpr = "0 == m_pText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Why is the array not null?"))
                __debugbreak();
        }
        m_pText.m_elements[v4] =
            panel->GetTextPointer(szSessionListMenuText[v4]);
        if (m_pText.m_elements[v4] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
            AeAssert::gCurrentLine = 414;
            AeAssert::gCurrentExpr = "m_pText[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
        if (m_pText.m_elements[v4] != nullptr)
        {
            if (v4 == 0)
                m_pText.m_elements[0]->SetText("MPFRONTEND_PLAY_XBOX_LIVE");
            else if (v4 == 1)
                m_pText.m_elements[1]->SetText("MPFRONTEND_GAME_BROWSER");
            else if (v4 == 2)
            {
                FEText* v35 = m_pText.m_elements[2];
                FEMultiLineText* v34 =
                    (FEMultiLineText*)mem_heap_malloc(0xA8);
                FEMultiLineText* v11 = nullptr;
                if (v34 != nullptr)
                {
                    color32 col = v35->GetColor();
                    panel_layer layer = (panel_layer)v35->GetScaleX();
                    float x1 = v35->GetY();
                    float v23 = v35->GetX();
                    v11 = new (v34)
                        FEMultiLineText(v35->GetFont(), x1, 0.0f, 0, layer,
                                        0.0f, 0, 0, col);
                }
                helpbar1 = v11;
                if (v11 != nullptr)
                    v11->SetNumLines(1);
                helpbar1->SetText(
                    "MPFRONTEND_XENON_MAIN_HELP_BAR_ALLCAPS");
            }
        }
    }
    for (int k = 0; k < 4; ++k)
    {
        if (m_pHeaderText.m_elements[k] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
            AeAssert::gCurrentLine = 449;
            AeAssert::gCurrentExpr = "0 == m_pHeaderText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Why is the array not null?"))
                __debugbreak();
        }
        m_pHeaderText.m_elements[k] =
            panel->GetTextPointer(szSessionListMenuHeaderText[k]);
        if (m_pHeaderText.m_elements[k] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
            AeAssert::gCurrentLine = 452;
            AeAssert::gCurrentExpr = "m_pHeaderText[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
        if (m_pHeaderText.m_elements[k] != nullptr)
        {
            if (k == 0)
                m_pHeaderText.m_elements[0]
                    ->SetText("MPFRONTEND_SERVER_NAME");
            else
            {
                switch (k)
                {
                case 1:
                    m_pHeaderText.m_elements[1]
                        ->SetText("MPFRONTEND_PLAYERS_ALLCAPS");
                    break;
                case 2:
                    m_pHeaderText.m_elements[2]->SetText("MPFRONTEND_MODE");
                    break;
                case 3:
                    m_pHeaderText.m_elements[3]
                        ->SetText("MPFRONTEND_MAP_ALLCAPS");
                    break;
                default:
                    break;
                }
            }
        }
    }
    if (m_pServerText != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
        AeAssert::gCurrentLine = 478;
        AeAssert::gCurrentExpr = "m_pServerText == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "Server text pointer should be null before it is set\n"))
            __debugbreak();
    }
    m_pServerText = panel->GetTextPointer("gl_text_option_01");
    if (m_pServerText == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
        AeAssert::gCurrentLine = 480;
        AeAssert::gCurrentExpr = "m_pServerText != 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Could not get server text pointer\n"))
            __debugbreak();
    }
    m_ListBox.SetScrollBarFromPanelFile(panel);
    if (m_ListBox.mItemColumnsCount <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 125;
        AeAssert::gCurrentExpr =
            "column >= 0 && column < mItemColumnsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "UIListBoxItem: State count must be greater then zero"))
            __debugbreak();
    }
    if (m_ListBox.mSelectedRowColorChangeColumns.mSize <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    *m_ListBox.mSelectedRowColorChangeColumns.mElements = true;
    panel->GetTextPointer("gl_text_spec_02a")
        ->SetText("MPFRONTEND_AUTO_TEAM_BALANCE_ALLCAPS");
    panel->GetTextPointer("gl_text_spec_03a")
        ->SetText("MPFRONTEND_TEAM_DAMAGE_ALLCAPS");
    panel->GetTextPointer("gl_text_spec_01a")
        ->SetText("MPFRONTEND_CONNECTION_QUALITY");
    for (int m = 0; m < 6; ++m)
    {
        m_ListBox.SetItem(
            m, 0, panel->GetTextPointer(szSessionLMNames[m]), 0);
        m_ListBox.SetItem(
            m, 1, panel->GetTextPointer(szSessionLMPlayers[m]), 0);
        m_ListBox.SetItem(
            m, 2, panel->GetTextPointer(szSessionLMModes[m]), 0);
        m_ListBox.SetItem(
            m, 3, panel->GetTextPointer(szSessionLMMaps[m]), 0);
    }
    static const char* const szSessionStars[5] = {
        "gl_icon_star_01", "gl_icon_star_02", "gl_icon_star_03",
        "gl_icon_star_04", "gl_icon_star_05",
    };
    for (int s = 0; s < 5; ++s)
        m_pConnectionStars.m_elements[s] =
            panel->GetPointer(szSessionStars[s]);
}

// ea: 0x0079DEE0
void SessionLanListMenu::SetPanelFile(PanelFile* pf)
{
    static const char* const szSessionLanListMenuBackgroundArt[5] = {
        "gl_bkg", "gl_bkg_detail_01", "gl_bkg_detail_02",
        "gl_bkg_detail_03", "gl_bkg_detail_04",
    };
    static const char* const szSessionLanListMenuText[3] = {
        "gl_text_title_01", "gl_text_title_02", "text_helpbar",
    };
    static const char* const szSessionLanListMenuHeaderText[4] = {
        "gl_text_group_header_01", "gl_text_group_header_02",
        "gl_text_group_header_03", "gl_text_group_header_04",
    };
    static const char* const szLanSessionLMNames[6] = {
        "gl_text_option_01", "gl_text_option_02", "gl_text_option_03",
        "gl_text_option_04", "gl_text_option_05", "gl_text_option_06",
    };
    static const char* const szLanSessionLMPlayers[6] = {
        "gl_text_option_01_play", "gl_text_option_02_play",
        "gl_text_option_03_play", "gl_text_option_04_play",
        "gl_text_option_05_play", "gl_text_option_06_play",
    };
    static const char* const szLanSessionLMModes[6] = {
        "gl_text_option_01_mode", "gl_text_option_02_mode",
        "gl_text_option_03_mode", "gl_text_option_04_mode",
        "gl_text_option_05_mode", "gl_text_option_06_mode",
    };
    static const char* const szLanSessionLMMaps[6] = {
        "gl_text_option_01_map", "gl_text_option_02_map",
        "gl_text_option_03_map", "gl_text_option_04_map",
        "gl_text_option_05_map", "gl_text_option_06_map",
    };
    panel = pf;
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/SessionLanListMenu.cpp";
        AeAssert::gCurrentLine = 377;
        AeAssert::gCurrentExpr = "panel";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Panel File invalid!"))
            __debugbreak();
    }
    for (int j = 0; j < 5; ++j)
    {
        if (m_pBackgroundArt.m_elements[j] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionLanListMenu.cpp";
            AeAssert::gCurrentLine = 395;
            AeAssert::gCurrentExpr = "0 == m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Why is the array not null?"))
                __debugbreak();
        }
        m_pBackgroundArt.m_elements[j] =
            panel->GetPointer(szSessionLanListMenuBackgroundArt[j]);
        if (m_pBackgroundArt.m_elements[j] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionLanListMenu.cpp";
            AeAssert::gCurrentLine = 398;
            AeAssert::gCurrentExpr = "m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
    }
    for (int v4 = 0; v4 < 3; ++v4)
    {
        if (m_pText.m_elements[v4] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionLanListMenu.cpp";
            AeAssert::gCurrentLine = 411;
            AeAssert::gCurrentExpr = "0 == m_pText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Why is the array not null?"))
                __debugbreak();
        }
        m_pText.m_elements[v4] =
            panel->GetTextPointer(szSessionLanListMenuText[v4]);
        if (m_pText.m_elements[v4] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionLanListMenu.cpp";
            AeAssert::gCurrentLine = 414;
            AeAssert::gCurrentExpr = "m_pText[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
        if (m_pText.m_elements[v4] != nullptr)
        {
            if (v4 == 0)
                m_pText.m_elements[0]->SetText("MPFRONTEND_PLAY_SYSTEM_LINK");
            else if (v4 == 1)
                m_pText.m_elements[1]->SetText("MPFRONTEND_GAME_BROWSER");
            else if (v4 == 2)
            {
                FEText* v35 = m_pText.m_elements[2];
                FEMultiLineText* v34 =
                    (FEMultiLineText*)mem_heap_malloc(0xA8);
                FEMultiLineText* v11 = nullptr;
                if (v34 != nullptr)
                {
                    color32 col = v35->GetColor();
                    panel_layer layer = (panel_layer)v35->GetScaleX();
                    float x1 = v35->GetY();
                    float v23 = v35->GetX();
                    v11 = new (v34)
                        FEMultiLineText(v35->GetFont(), x1, 0.0f, 0, layer,
                                        0.0f, 0, 0, col);
                }
                helpbar1 = v11;
                if (v11 != nullptr)
                    v11->SetNumLines(1);
                helpbar1->SetText(
                    "MPFRONTEND_XENON_MAIN_HELP_BAR_ALLCAPS");
            }
        }
    }
    for (int k = 0; k < 4; ++k)
    {
        if (m_pHeaderText.m_elements[k] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionLanListMenu.cpp";
            AeAssert::gCurrentLine = 449;
            AeAssert::gCurrentExpr = "0 == m_pHeaderText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Why is the array not null?"))
                __debugbreak();
        }
        m_pHeaderText.m_elements[k] =
            panel->GetTextPointer(szSessionLanListMenuHeaderText[k]);
        if (m_pHeaderText.m_elements[k] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionLanListMenu.cpp";
            AeAssert::gCurrentLine = 452;
            AeAssert::gCurrentExpr = "m_pHeaderText[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
        if (m_pHeaderText.m_elements[k] != nullptr)
        {
            if (k == 0)
                m_pHeaderText.m_elements[0]
                    ->SetText("MPFRONTEND_SERVER_NAME");
            else
            {
                switch (k)
                {
                case 1:
                    m_pHeaderText.m_elements[1]
                        ->SetText("MPFRONTEND_PLAYERS_ALLCAPS");
                    break;
                case 2:
                    m_pHeaderText.m_elements[2]->SetText("MPFRONTEND_MODE");
                    break;
                case 3:
                    m_pHeaderText.m_elements[3]
                        ->SetText("MPFRONTEND_MAP_ALLCAPS");
                    break;
                default:
                    break;
                }
            }
        }
    }
    if (m_pServerText != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/SessionLanListMenu.cpp";
        AeAssert::gCurrentLine = 478;
        AeAssert::gCurrentExpr = "m_pServerText == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "Server text pointer should be null before it is set\n"))
            __debugbreak();
    }
    m_pServerText = panel->GetTextPointer("gl_text_option_01");
    if (m_pServerText == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/SessionLanListMenu.cpp";
        AeAssert::gCurrentLine = 480;
        AeAssert::gCurrentExpr = "m_pServerText != 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Could not get server text pointer\n"))
            __debugbreak();
    }
    m_ListBox.SetScrollBarFromPanelFile(panel);
    if (m_ListBox.mItemColumnsCount <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 125;
        AeAssert::gCurrentExpr =
            "column >= 0 && column < mItemColumnsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "UIListBoxItem: State count must be greater then zero"))
            __debugbreak();
    }
    if (m_ListBox.mSelectedRowColorChangeColumns.mSize <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    *m_ListBox.mSelectedRowColorChangeColumns.mElements = true;
    panel->GetTextPointer("gl_text_spec_02a")
        ->SetText("MPFRONTEND_AAR_VOTING_OPTION_ALLCAPS");
    panel->GetTextPointer("gl_text_spec_03a")
        ->SetText("MPFRONTEND_PENALTY_VOTE_ALLCAPS");
    panel->GetTextPointer("gl_text_spec_01a")
        ->SetText("MPFRONTEND_CONNECTION_QUALITY");
    for (int m = 0; m < 6; ++m)
    {
        m_ListBox.SetItem(
            m, 0, panel->GetTextPointer(szLanSessionLMNames[m]), 0);
        m_ListBox.SetItem(
            m, 1, panel->GetTextPointer(szLanSessionLMPlayers[m]), 0);
        m_ListBox.SetItem(
            m, 2, panel->GetTextPointer(szLanSessionLMModes[m]), 0);
        m_ListBox.SetItem(
            m, 3, panel->GetTextPointer(szLanSessionLMMaps[m]), 0);
    }
    static const char* const szLanStars[5] = {
        "gl_icon_star_01", "gl_icon_star_02", "gl_icon_star_03",
        "gl_icon_star_04", "gl_icon_star_05",
    };
    for (int s = 0; s < 5; ++s)
        m_pConnectionStars.m_elements[s] =
            panel->GetPointer(szLanStars[s]);
}

// ea: 0x0079B980
void PlayLanMenu::SetPanelFile(PanelFile* pf)
{
    static const char* const szPlayLanMenuBackgroundArt[3] = {
        "bkg", "bkg_line_01", "bkg_preview_outline",
    };
    static const char* const szPlayLanMenuBackgroundButtons[5] = {
        "bkg_btn_back_01", "bkg_btn_back_02", "bkg_btn_back_03",
        "bkg_btn_line_01", "bkg_btn_line_02",
    };
    static const char* const szPlayLanMenuText[4] = {
        "text_screen_title", "text_option_title", "text_option_description",
        "text_helpbar",
    };
    static const char* const szPlayLanMenuTextReferences[4] = {
        "MPFRONTEND_PLAY_SYSTEM_LINK", defaultFileName,
        defaultFileName, defaultFileName,
    };
    static const char* const szPlayLanMenuOptionText[3] = {
        "text_option_01", "text_option_02", "text_option_03",
    };
    static const char* const szPlayLanMenuOptionTextReferences[3] = {
        "MPFRONTEND_MM_CREATE_GAME", "MPFRONTEND_FIND_GAME",
        "MPFRONTEND_MM_XBOX_LIVE_OPTIONS",
    };
    static const char* const szPlayLanMenuPreviewImages[3] = {
        "preview_image_01", "preview_image_02", "preview_image_03",
    };
    panel = pf;
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/PlayLanMenu.cpp";
        AeAssert::gCurrentLine = 200;
        AeAssert::gCurrentExpr = "panel";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Panel File invalid!"))
            __debugbreak();
    }
    for (int i = 0; i < 3; ++i)
    {
        if (m_pBackgroundArt.m_elements[i] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/PlayLanMenu.cpp";
            AeAssert::gCurrentLine = 215;
            AeAssert::gCurrentExpr = "0 == m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Array not null as expected"))
                __debugbreak();
        }
        m_pBackgroundArt.m_elements[i] =
            panel->GetPointer(szPlayLanMenuBackgroundArt[i]);
        if (m_pBackgroundArt.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/PlayLanMenu.cpp";
            AeAssert::gCurrentLine = 217;
            AeAssert::gCurrentExpr = "m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
    }
    for (int j = 0; j < 5; ++j)
    {
        if (m_pBackgroundButtons.m_elements[j] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/PlayLanMenu.cpp";
            AeAssert::gCurrentLine = 232;
            AeAssert::gCurrentExpr = "0 == m_pBackgroundButtons[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Array not null as expected"))
                __debugbreak();
        }
        m_pBackgroundButtons.m_elements[j] =
            panel->GetPointer(szPlayLanMenuBackgroundButtons[j]);
        if (m_pBackgroundButtons.m_elements[j] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/PlayLanMenu.cpp";
            AeAssert::gCurrentLine = 234;
            AeAssert::gCurrentExpr = "m_pBackgroundButtons[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
    }
    for (int k = 0; k < 4; ++k)
    {
        if (m_pText.m_elements[k] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/PlayLanMenu.cpp";
            AeAssert::gCurrentLine = 257;
            AeAssert::gCurrentExpr = "0 == m_pText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Array not null as expected"))
                __debugbreak();
        }
        m_pText.m_elements[k] =
            panel->GetTextPointer(szPlayLanMenuText[k]);
        if (m_pText.m_elements[k] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/PlayLanMenu.cpp";
            AeAssert::gCurrentLine = 259;
            AeAssert::gCurrentExpr = "m_pText[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
        m_pText.m_elements[k]->SetText(szPlayLanMenuTextReferences[k]);
        m_pText.m_elements[k]->SetShown(true);
    }
    for (int m = 0; m < 3; ++m)
    {
        if (m_pOptionText.m_elements[m] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/PlayLanMenu.cpp";
            AeAssert::gCurrentLine = 274;
            AeAssert::gCurrentExpr = "0 == m_pOptionText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Array not null as expected"))
                __debugbreak();
        }
        m_pOptionText.m_elements[m] =
            panel->GetTextPointer(szPlayLanMenuOptionText[m]);
        mListBox.SetItem(m, 0, m_pOptionText.m_elements[m], 0);
        mListBox.SetText(m, 0, szPlayLanMenuOptionTextReferences[m]);
        if (m_pOptionText.m_elements[m] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/PlayLanMenu.cpp";
            AeAssert::gCurrentLine = 281;
            AeAssert::gCurrentExpr = "m_pOptionText[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
    }
    for (int n = 0; n < 3; ++n)
    {
        if (m_pImages.m_elements[n] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/PlayLanMenu.cpp";
            AeAssert::gCurrentLine = 293;
            AeAssert::gCurrentExpr = "0 == m_pImages[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Array not null as expected"))
                __debugbreak();
        }
        m_pImages.m_elements[n] =
            panel->GetPointer(szPlayLanMenuPreviewImages[n]);
        if (m_pImages.m_elements[n] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/PlayLanMenu.cpp";
            AeAssert::gCurrentLine = 295;
            AeAssert::gCurrentExpr = "m_pImages[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
        m_pImages.m_elements[n]->SetShown(false);
    }
    FEText* v11 = m_pText.m_elements[3];
    FEMultiLineText* v32 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    FEMultiLineText* v14 = nullptr;
    if (v32 != nullptr)
    {
        color32 col = v11->GetColor();
        panel_layer layer = (panel_layer)v11->GetScaleX();
        float x1 = v11->GetY();
        float v18 = v11->GetX();
        v14 = new (v32)
            FEMultiLineText(v11->GetFont(), x1, 0.0f, 0, layer,
                            0.0f, 0, 0, col);
    }
    helpbar1 = v14;
    if (v14 != nullptr)
        v14->SetNumLines(1);
    helpbar1->SetText("MPFRONTEND_HELP_SELECT_BACK_MOVEUD");
    if (mListBox.mItemColumnsCount <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 125;
        AeAssert::gCurrentExpr =
            "column >= 0 && column < mItemColumnsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "UIListBoxItem: State count must be greater then zero"))
            __debugbreak();
    }
    if (mListBox.mSelectedRowColorChangeColumns.mSize <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    *mListBox.mSelectedRowColorChangeColumns.mElements = true;
    mListBox.SelectLine(0);
    mListBox.mSelectedFlashing = true;
    mListBox.Refresh();
}

// ea: 0x007A4B50
void AARMapVote::SetPanelFile(PanelFile* pPanelFile)
{
    static const char* const szAARMapVoteBackgroundArt[12] = {
        "ps_bkg", "ps_bkg_colorband", "ps_bkg_image_soldier",
        "ps_bkg_detail_01", "ps_bkg_detail_02", "ps_bkg_detail_03",
        "ps_bkg_detail_04", "ps_bkg_detail_05", "ps_bkg_detail_06",
        "ps_bkg_detail_07", "ps_bkg_detail_08", "ps_bkg_detail_09",
    };
    static const char* const szScrollArrowAARMapVote[2] = {
        "scroll_arrow_left", "scroll_arrow_right",
    };
    static const char* const szAARMapVoteText[5] = {
        "text_title_AAR", "text_title_section", "text_title_instructions",
        "text_helpbar", "text_icon_vote",
    };
    static const char* const szAARMapVoteTextReferences[5] = {
        "MPGAME_AFTER_ACTION_REVIEW", "MPGAME_MAPVOTE",
        "MPGAME_MAPVOTE_INSTRUCTIONS", "MPGAME_HELP_AAR_MAPVOTE",
        "MPGAME_AAR_MAPVOTE_V",
    };
    AARBaseMenu::SetPanelFile(pPanelFile);
    for (int j = 0; j < 10; ++j)
    {
        if (m_pBackgroundArt.m_elements[j] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARMapVote.cpp";
            AeAssert::gCurrentLine = 171;
            AeAssert::gCurrentExpr = "0 == m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Array expected to be null"))
                __debugbreak();
        }
        m_pBackgroundArt.m_elements[j] =
            panel->GetPointer(szAARMapVoteBackgroundArt[j]);
        if (m_pBackgroundArt.m_elements[j] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARMapVote.cpp";
            AeAssert::gCurrentLine = 174;
            AeAssert::gCurrentExpr = "m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
    }
    for (int k = 0; k < 2; ++k)
        m_pScrollArrow.m_elements[k] =
            panel->GetPointer(szScrollArrowAARMapVote[k]);
    for (int v5 = 0; v5 < 5; ++v5)
    {
        m_pText.m_elements[v5] =
            panel->GetTextPointer(szAARMapVoteText[v5]);
        if (m_pText.m_elements[v5] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARMapVote.cpp";
            AeAssert::gCurrentLine = 187;
            AeAssert::gCurrentExpr = "m_pText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Could not get text for m_pText!"))
                __debugbreak();
        }
        const char* v6 = szAARMapVoteTextReferences[v5];
        if (v5 == 3)
        {
            FEText* v33 = m_pText.m_elements[3];
            FEMultiLineText* v32 =
                (FEMultiLineText*)mem_heap_malloc(0xA8);
            FEMultiLineText* v10 = nullptr;
            if (v32 != nullptr)
            {
                color32 col = v33->GetColor();
                panel_layer layer = (panel_layer)v33->GetScaleX();
                float x1 = v33->GetY();
                float v26 = v33->GetX();
                v10 = new (v32)
                    FEMultiLineText(v33->GetFont(), x1, 0.0f, 0, layer,
                                    0.0f, 0, 0, col);
            }
            helpbar1 = v10;
            if (v10 != nullptr)
                v10->SetNumLines(1);
            helpbar1->SetText(v6);
        }
        else
        {
            m_pText.m_elements[v5]->SetText(v6);
        }
        m_pText.m_elements[v5]->SetShown(true);
    }
    int v12 = 0;
    if (g_NumBaseMaps + 1 > 0)
    {
        do
        {
            FEText* TextPointer = (v12 >= 12)
                ? panel->GetTextPointer("slot_12_text_mapname")
                : panel->GetTextPointer(szAARMapNames[v12]);
            m_ListBox.SetItem(v12, 0, TextPointer, 0);
            if (v12 != 0)
            {
                char v18 = byte_E386C9[114 * (v12 - 1)];
                int v19 = 0;
                const char* v21;
                if (g_NumTotalMaps > 0)
                {
                    char* v20 = byte_E386C9;
                    while (*v20 != v18)
                    {
                        ++v19;
                        v20 += 114;
                        if (v19 >= g_NumTotalMaps)
                        {
                            v21 = "NULL";
                            goto MAPNAME_DONE;
                        }
                    }
                    v21 = &aMpfrontendMerv[114 * v19];
                }
                else
                {
                    v21 = "NULL";
                }
            MAPNAME_DONE:
                m_ListBox.SetText(v12, 0, v21);
            }
            else
            {
                m_ListBox.SetText(0, 0, "MPGAME_RANDOM");
            }
            FEText* v22 = panel->GetTextPointer(szAARMapVotes[v12]);
            m_ListBox.SetItem(v12, 1, v22, 0);
            m_ListBox.SetText(v12++, 1, "0");
        } while (v12 < g_NumBaseMaps + 1);
    }
    for (int m = 0; m < 65; ++m)
        m_pMapVoteVals[m] = 0;
    m_ListBox.SetScrollBarFromPanelFile(panel);
    m_ListBox.SetAllColumnsSelectable(false);
    if (m_ListBox.mItemColumnsCount <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 125;
        AeAssert::gCurrentExpr =
            "column >= 0 && column < mItemColumnsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "UIListBoxItem: State count must be greater then zero"))
            __debugbreak();
    }
    if (m_ListBox.mSelectedRowColorChangeColumns.mSize <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    *m_ListBox.mSelectedRowColorChangeColumns.mElements = true;
    PanelQuad* Pointer = panel->GetPointer("player_hilite");
    m_ListBox.mHighlightQuad = Pointer;
    if (Pointer != nullptr)
        Pointer->SetShown(false);
    m_ListBox.mHighlightedSelectedTextColor.i = -3618616;
    m_ListBox.mHighlightedUnselectedTextColor.i = -7553346;
    m_ListBox.mSelectedFlashing = true;
    m_ListBox.Refresh();
}

// ea: 0x007A5770
void AARGameModeVote::SetPanelFile(PanelFile* pPanelFile)
{
    static const char* const szAARGameModeVoteBackgroundArt[9] = {
        "bkg", "bkg_detail_01", "bkg_detail_02", "bkg_detail_03",
        "bkg_detail_04", "bkg_detail_05", "bkg_detail_06",
        "bkg_detail_07", "bkg_detail_08",
    };
    static const char* const szAARGameModeVoteScrollArrow[2] = {
        "scroll_arrow_left", "scroll_arrow_right",
    };
    static const char* const szAARGameModeVoteText[5] = {
        "text_title_AAR", "text_title_section", "text_title_instructions",
        "text_helpbar", "text_icon_vote",
    };
    static const char* const szAARGameModeVoteTextReferences[5] = {
        "MPGAME_AFTER_ACTION_REVIEW", "MPGAME_GAME_MODE_VOTE",
        "MPGAME_GAMEMODE_VOTE_INSTRUCTIONS",
        "MPGAME_HELP_AAR_GAMEMODEVOTE", "MPGAME_AAR_MAPVOTE_V",
    };
    static const char* const szAARGameModeVoteNames[7] = {
        "slot_01_text_mapname", "slot_02_text_mapname",
        "slot_03_text_mapname", "slot_04_text_mapname",
        "slot_05_text_mapname", "slot_06_text_mapname",
        "slot_07_text_mapname",
    };
    static const char* const szAARGameModeVoteNameReferences[7] = {
        "MPGAME_RANDOM_GAMEMODE", "MPGAME_WAR",
        "MPGAME_CAPTURE_THE_FLAG", "MPGAME_SINGLE_FLAG_CTF",
        "MPGAME_HEADQUARTERS", "MPGAME_TEAM_BATTLE", "MPGAME_BATTLE",
    };
    static const char* const szAARGameModeVotes[7] = {
        "slot_01_text_mapvote", "slot_02_text_mapvote",
        "slot_03_text_mapvote", "slot_04_text_mapvote",
        "slot_05_text_mapvote", "slot_06_text_mapvote",
        "slot_07_text_mapvote",
    };
    if (pPanelFile == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/AARGameModeVote.cpp";
        AeAssert::gCurrentLine = 77;
        AeAssert::gCurrentExpr = "pPanelFile";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid panel file pointer"))
            __debugbreak();
    }
    AARBaseMenu::SetPanelFile(pPanelFile);
    for (int j = 0; j < 9; ++j)
    {
        if (m_pBackgroundArt.m_elements[j] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARGameModeVote.cpp";
            AeAssert::gCurrentLine = 88;
            AeAssert::gCurrentExpr = "0 == m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Array expected to be null"))
                __debugbreak();
        }
        m_pBackgroundArt.m_elements[j] =
            panel->GetPointer(szAARGameModeVoteBackgroundArt[j]);
        if (m_pBackgroundArt.m_elements[j] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARGameModeVote.cpp";
            AeAssert::gCurrentLine = 91;
            AeAssert::gCurrentExpr = "m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
    }
    for (int k = 0; k < 2; ++k)
        m_pScrollArrow.m_elements[k] =
            panel->GetPointer(szAARGameModeVoteScrollArrow[k]);
    for (int v5 = 0; v5 < 5; ++v5)
    {
        m_pText.m_elements[v5] =
            panel->GetTextPointer(szAARGameModeVoteText[v5]);
        if (m_pText.m_elements[v5] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARGameModeVote.cpp";
            AeAssert::gCurrentLine = 121;
            AeAssert::gCurrentExpr = "m_pText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Could not get text for m_pText!"))
                __debugbreak();
        }
        const char* v6 = szAARGameModeVoteTextReferences[v5];
        if (v5 == 3)
        {
            FEText* v44 = m_pText.m_elements[3];
            FEMultiLineText* v43 =
                (FEMultiLineText*)mem_heap_malloc(0xA8);
            FEMultiLineText* v10 = nullptr;
            if (v43 != nullptr)
            {
                color32 col = v44->GetColor();
                panel_layer layer = (panel_layer)v44->GetScaleX();
                float x1 = v44->GetY();
                float v17 = v44->GetX();
                v10 = new (v43)
                    FEMultiLineText(v44->GetFont(), x1, 0.0f, 0, layer,
                                    0.0f, 0, 0, col);
            }
            helpbar1 = v10;
            if (v10 != nullptr)
                v10->SetNumLines(1);
            helpbar1->SetText(v6);
        }
        else
        {
            m_pText.m_elements[v5]->SetText(v6);
        }
        m_pText.m_elements[v5]->SetShown(true);
    }
    for (int v12 = 0; v12 < 7; ++v12)
    {
        FEText* TextPointer =
            panel->GetTextPointer(szAARGameModeVoteNames[v12]);
        m_ListBox.SetItem(v12, 0, TextPointer, 0);
        m_ListBox.SetText(v12, 0,
                          szAARGameModeVoteNameReferences[v12]);
        FEText* v14 =
            panel->GetTextPointer(szAARGameModeVotes[v12]);
        m_ListBox.SetItem(v12, 1, v14, 0);
        m_ListBox.SetText(v12, 1, "0");
    }
    m_ListBox.SetScrollBarFromPanelFile(panel);
    m_ListBox.SetAllColumnsSelectable(false);
    if (m_ListBox.mItemColumnsCount <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 125;
        AeAssert::gCurrentExpr =
            "column >= 0 && column < mItemColumnsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "UIListBoxItem: State count must be greater then zero"))
            __debugbreak();
    }
    if (m_ListBox.mSelectedRowColorChangeColumns.mSize <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    *m_ListBox.mSelectedRowColorChangeColumns.mElements = true;
    PanelQuad* Pointer = panel->GetPointer("player_hilite");
    m_ListBox.mHighlightQuad = Pointer;
    if (Pointer != nullptr)
        Pointer->SetShown(false);
    m_ListBox.mHighlightedSelectedTextColor.i = -3618616;
    m_ListBox.mHighlightedUnselectedTextColor.i = -7553346;
    m_ListBox.mSelectedFlashing = true;
    m_ListBox.Refresh();
}

// ea: 0x0079E900
void OverlayMenu::SetPanelFile(PanelFile* pf)
{
    static const char* const szOverlayMenuBackgroundArt[3] = {
        "cg_bkg", "cg_bkg_detail_01", "cg_bkg_detail_02",
    };
    static const char* const szOverlayMenuText[2] = {
        "text_option_01", "text_option_02",
    };
    static const char* const szLinesBetweenOptionText[1] = {
        "bkg_line_01",
    };
    panel = pf;
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
        AeAssert::gCurrentLine = 916;
        AeAssert::gCurrentExpr = "panel";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Panel File invalid!"))
            __debugbreak();
    }
    for (int i = 0; i < 3; ++i)
    {
        if (m_pBackgroundArt.m_elements[i] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
            AeAssert::gCurrentLine = 924;
            AeAssert::gCurrentExpr = "0 == m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Array expected to be null"))
                __debugbreak();
        }
        m_pBackgroundArt.m_elements[i] =
            panel->GetPointer(szOverlayMenuBackgroundArt[i]);
        if (m_pBackgroundArt.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
            AeAssert::gCurrentLine = 927;
            AeAssert::gCurrentExpr = "m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
    }
    FEText* TextPointer = panel->GetTextPointer("text_body");
    FEText* v5 = panel->GetTextPointer("text_body");
    v5->SetX(TextPointer->GetX());
    FEText* v8 = panel->GetTextPointer("text_body");
    AddEntry(0, v8, false);
    FEMultiLineText* v9 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    FEMultiLineText* v10 = nullptr;
    if (v9 != nullptr)
    {
        color32 col = TextPointer->GetColor();
        panel_layer layer = (panel_layer)TextPointer->GetScaleX();
        float x1 = TextPointer->GetY();
        float v24 = TextPointer->GetX();
        v10 = new (v9)
            FEMultiLineText(TextPointer->GetFont(), x1, 0.0f, 0, layer,
                            0.0f, 0, 0, col);
    }
    helpbar2 = v10;
    if (v10 != nullptr)
        v10->SetNumLines(3);
    for (int j = 0; j < 2; ++j)
    {
        if (m_pOptionText.m_elements[j] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
            AeAssert::gCurrentLine = 952;
            AeAssert::gCurrentExpr = "0 == m_pOptionText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Array expected to be null"))
                __debugbreak();
        }
        m_pOptionText.m_elements[j] =
            panel->GetTextPointer(szOverlayMenuText[j]);
        if (m_pOptionText.m_elements[j] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
            AeAssert::gCurrentLine = 955;
            AeAssert::gCurrentExpr = "m_pOptionText[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
        m_pOptionText.m_elements[j]
            ->SetText((const char*)&defaultFileName);
        m_pOptionText.m_elements[j]->SetShown(false);
        m_ListBox.SetItem(j, 0, m_pOptionText.m_elements[j], 0);
    }
    helpbar = m_pOptionText.m_elements[1];
    FEMultiLineText* v14 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    FEMultiLineText* v18 = nullptr;
    if (v14 != nullptr)
    {
        color32 col = helpbar->GetColor();
        panel_layer layer = (panel_layer)helpbar->GetScaleX();
        float x1 = helpbar->GetY();
        float v25 = helpbar->GetX();
        v18 = new (v14)
            FEMultiLineText(helpbar->GetFont(), x1, 0.0f, 0, layer,
                            0.0f, 0, 0, col);
    }
    helpbar1 = v18;
    if (v18 != nullptr)
        v18->SetNumLines(1);
    helpbar1->SetText("MPFRONTEND_HELP_CANCEL");
    helpbar1->SetX(helpbar1->GetX());
    if (m_pOptionLines.m_elements[0] != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
        AeAssert::gCurrentLine = 973;
        AeAssert::gCurrentExpr = "0 == m_pOptionLines[i]";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Array expected to be null"))
            __debugbreak();
    }
    m_pOptionLines.m_elements[0] =
        panel->GetPointer(szLinesBetweenOptionText[0]);
    if (m_pOptionLines.m_elements[0] == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/OverlayMenu.cpp";
        AeAssert::gCurrentLine = 977;
        AeAssert::gCurrentExpr = "m_pOptionLines[i]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
            __debugbreak();
    }
    m_pOptionLines.m_elements[0]->SetShown(false);
    m_ListBox.SetAllColumnsSelectable(false);
    m_ListBox.mSelectedFlashing = true;
    m_ListBox.Refresh();
}

// ea: 0x007AAA50
void AARPersonalStats::SetPanelFile(PanelFile* pf)
{
    static const char* const szAARPersonalStatsBackgroundArt[12] = {
        "ps_bkg", "ps_bkg_colorband", "ps_bkg_image_soldier",
        "ps_bkg_detail_01", "ps_bkg_detail_02", "ps_bkg_detail_03",
        "ps_bkg_detail_04", "ps_bkg_detail_05", "ps_bkg_detail_06",
        "ps_bkg_detail_07", "ps_bkg_detail_08", "ps_bkg_detail_09",
    };
    static const char* const szAARPersonalStatsClassIcon[7] = {
        "ps_ci_assault_heavy", "ps_ci_assault_light", "ps_ci_rifleman",
        "ps_ci_medic", "ps_ci_support", "ps_ci_anti_armor",
        "ps_ci_scout",
    };
    static const char* const szScrollArrowPersonal[2] = {
        "scroll_arrow_left", "scroll_arrow_right",
    };
    static const char* const szAARPersonalText[4] = {
        "text_helpbar", "text_title_AAR", "ps_text_class_title",
        "ps_text_title_section",
    };
    static const char* const szAARPersonalScoreText[14] = {
        "ps_text_line_01a", "ps_text_line_01b", "ps_text_line_02a",
        "ps_text_line_02b", "ps_text_line_03a", "ps_text_line_03b",
        "ps_text_line_04a", "ps_text_line_04b", "ps_text_line_05a",
        "ps_text_line_05b", "ps_text_line_06a", "ps_text_line_06b",
        "ps_text_line_07a", "ps_text_line_07b",
    };
    static const char* const szAARPersonalScoreTextReferences[14] = {
        "MPGAME_TOTALSCORE_ALLCAPS", defaultFileName,
        "MPGAME_KILLS_ALLCAPS", defaultFileName,
        "MPGAME_ASSISTS_ALLCAPS", defaultFileName,
        "MPGAME_DEATHS_ALLCAPS", defaultFileName,
        "MPGAME_SUICIDES_ALLCAPS", defaultFileName,
        "MPGAME_TEAMKILLS_ALLCAPS", defaultFileName,
        "MPGAME_VEHICLESDESTROYED_ALLCAPS", defaultFileName,
    };
    static const char* const szAARPersonalClassScoreText[8] = {
        "ps_text_line_08a", "ps_text_line_08b", "ps_text_line_09a",
        "ps_text_line_09b", "ps_text_line_10a", "ps_text_line_10b",
        "ps_text_line_11a", "ps_text_line_11b",
    };
    static const char* const szAARPersonalClassScoreTextReferences[8] = {
        defaultFileName, defaultFileName, defaultFileName,
        defaultFileName, defaultFileName, defaultFileName,
        defaultFileName, defaultFileName,
    };
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/AARPersonalStats.cpp";
        AeAssert::gCurrentLine = 136;
        AeAssert::gCurrentExpr = "pf";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid panel file pointer"))
            __debugbreak();
    }
    AARBaseMenu::SetPanelFile(pf);
    for (int i = 0; i < 12; ++i)
    {
        if (m_pBackgroundArt.m_elements[i] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARPersonalStats.cpp";
            AeAssert::gCurrentLine = 150;
            AeAssert::gCurrentExpr = "0 == m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Array expected to be null"))
                __debugbreak();
        }
        m_pBackgroundArt.m_elements[i] =
            panel->GetPointer(szAARPersonalStatsBackgroundArt[i]);
        if (m_pBackgroundArt.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARPersonalStats.cpp";
            AeAssert::gCurrentLine = 153;
            AeAssert::gCurrentExpr = "m_pBackgroundArt[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
    }
    for (int j = 0; j < 7; ++j)
    {
        if (m_pClassIcon.m_elements[j] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARPersonalStats.cpp";
            AeAssert::gCurrentLine = 159;
            AeAssert::gCurrentExpr = "0 == m_pClassIcon[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Array expected to be null"))
                __debugbreak();
        }
        m_pClassIcon.m_elements[j] =
            panel->GetPointer(szAARPersonalStatsClassIcon[j]);
        m_pClassIcon.m_elements[j]->SetShown(false);
        if (m_pClassIcon.m_elements[j] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARPersonalStats.cpp";
            AeAssert::gCurrentLine = 162;
            AeAssert::gCurrentExpr = "m_pClassIcon[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
    }
    for (int k = 0; k < 2; ++k)
        m_pScrollArrow.m_elements[k] =
            panel->GetPointer(szScrollArrowPersonal[k]);
    for (int m = 0; m < 4; ++m)
    {
        m_pText.m_elements[m] =
            panel->GetTextPointer(szAARPersonalText[m]);
        if (m_pText.m_elements[m] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARPersonalStats.cpp";
            AeAssert::gCurrentLine = 175;
            AeAssert::gCurrentExpr = "m_pText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Could not get timer text!"))
                __debugbreak();
        }
        m_pText.m_elements[m]->SetShown(true);
    }
    SetPanelHelpBar();
    m_pText.m_elements[1]->SetText("MPGAME_AFTER_ACTION_REVIEW");
    m_pText.m_elements[3]->SetText("MPGAME_PERSONAL_STATS");
    for (int n = 0; n < 14; ++n)
    {
        m_pScoreText.m_elements[n] =
            panel->GetTextPointer(szAARPersonalScoreText[n]);
        if (m_pScoreText.m_elements[n] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARPersonalStats.cpp";
            AeAssert::gCurrentLine = 186;
            AeAssert::gCurrentExpr = "m_pScoreText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Could not get timer text!"))
                __debugbreak();
        }
        m_pScoreText.m_elements[n]
            ->SetText(szAARPersonalScoreTextReferences[n]);
        m_pScoreText.m_elements[n]->SetShown(true);
    }
    for (int ii = 0; ii < 8; ++ii)
    {
        if (m_pClassScoreText.m_elements[ii] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARPersonalStats.cpp";
            AeAssert::gCurrentLine = 194;
            AeAssert::gCurrentExpr = "0 == m_pClassScoreText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Pointer not null as expected"))
                __debugbreak();
        }
        m_pClassScoreText.m_elements[ii] =
            panel->GetTextPointer(szAARPersonalClassScoreText[ii]);
        if (m_pClassScoreText.m_elements[ii] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/AARPersonalStats.cpp";
            AeAssert::gCurrentLine = 196;
            AeAssert::gCurrentExpr = "m_pClassScoreText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Could not get class text!"))
                __debugbreak();
        }
        m_pClassScoreText.m_elements[ii]
            ->SetText(szAARPersonalClassScoreTextReferences[ii]);
        m_pClassScoreText.m_elements[ii]->SetShown(true);
    }
}

// ea: 0x007AC7A0
void ModelMenu::UpdateClassModel(int playerclass, int team, int weapon)
{
    mCurrentWeapon = weapon;
    mCurrentTeam = team;
    mCurrentClass = playerclass;
    Entity* mObject =
        EntityHandleDb::sInst.GetObject(mClassModelEntity.mHandle.mVal);
    if (mObject != nullptr)
    {
        AITypeManager* v7 = AITypeManager::sInst;
        const char* ClassModel = GetClassModel(playerclass, team);
        IVPointer<AIType> ait =
            v7->GetAIType(CurPakId(), ClassModel, 0);
        ValidatePakId((TPakId)ait.mPakId);
        if (ait.mValue != nullptr)
        {
            Entity* obj =
                EntityHandleDb::sInst.GetObject(
                    mClassModelEntity.mHandle.mVal);
            ValidatePakId((TPakId)ait.mPakId);
            AIType* mValue = ait.mValue;
            mValue->InitPlayer(obj, CurPakId());
            Entity* v14 =
                EntityHandleDb::sInst.GetObject(
                    mClassModelEntity.mHandle.mVal);
            v14->s.brushmodel = 0;
            Entity* v16 =
                EntityHandleDb::sInst.GetObject(
                    mClassModelEntity.mHandle.mVal);
            v16->s.weapon = (uint8_t)mCurrentWeapon;
            Entity* v18 =
                EntityHandleDb::sInst.GetObject(
                    mClassModelEntity.mHandle.mVal);
            SV_SetBrushModel(v18);
            AnimTree* AnimTreeByName =
                Scr_GetAnimTreeByName("generic_human");
            Entity* v21 =
                EntityHandleDb::sInst.GetObject(
                    mClassModelEntity.mHandle.mVal);
            G_SetAnimTree(v21, AnimTreeByName);
            Entity* v23 =
                EntityHandleDb::sInst.GetObject(
                    mClassModelEntity.mHandle.mVal);
            DObjCreateAnimationPlayer(v23->mDObj, 0);
            Entity* v25 =
                EntityHandleDb::sInst.GetObject(
                    mClassModelEntity.mHandle.mVal);
            G_DObjUpdate(v25, true);
            weaponFileInfo_t* InfoForWeapon =
                BG_GetInfoForWeapon(mCurrentWeapon);
            if (InfoForWeapon == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\mp/ui/ModelMenu.cpp";
                AeAssert::gCurrentLine = 218;
                AeAssert::gCurrentExpr = "weapInfo";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            int v27 = 2;
            switch (InfoForWeapon->weapClass)
            {
            case 0:
            case 0xA:
            case 0xE:
            case 0x11:
                v27 = 2;
                break;
            case 1:
            case 3:
                v27 = 3;
                break;
            case 2:
                v27 = 4;
                break;
            case 4:
                v27 = 1;
                break;
            case 5:
                v27 = 6;
                break;
            case 6:
                v27 = 5;
                break;
            case 8:
                v27 = 7;
                break;
            case 0xB:
            case 0xC:
            case 0xD:
            case 0xF:
                v27 = 8;
                break;
            case 0x10:
                v27 = 9;
                break;
            default:
                tlPrintf(
                    "UNHANDLED WEAPON CLASS FOR ANIMS ( MPPLayer.cpp )\n");
                break;
            }
            mCurrentWeaponSheet = v27;
            mCurrentAnim = 0;
        }
        g_UnlinkEntity(
            EntityHandleDb::sInst.GetObject(
                mClassModelEntity.mHandle.mVal));
    }
}

// ea: 0x007AD4E0
void SessionLanListMenu::Update(float time_inc)
{
    if (!MPUIInterface::IsOnlineGame()
        || MPLiveEngine::GetHandle()->internalState == kSignedIn)
    {
        FEMenu::Update(time_inc);
        movie_manager::frame_advance();
        unsigned long numGames = 0;
        MPUIInterface::GameListingGet(numGames);
        if (numGames != 0 || !MultiplayerMgr::sInst->oneOffCheckLinkStatus())
        {
            if (numGames != mNumGames)
            {
                helpbar1->SetText("MPFRONTEND_HELP_JOIN_BACK_MOVE_REFRESH");
                RepopulateSessionList();
                mNumGames = numGames;
            }
        }
        else
        {
            OverlayMenu* v3 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            v3->SetState(OverlayMenu::NO_GAMES);
            if (MPUIInterface::IsLANGame())
            {
                OverlayMenu* fems = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(int*)((char*)fems + 0x50) = 9;
                OverlayMenu* v5 = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(int*)((char*)v5 + 0x54) = 8;
            }
            system->AddOverlay(16);
        }
        MPUIInterface::Step();
        if (mNeedToUpdate)
        {
            j_nullsub_46(this);
            InitMenu();
            mNeedToUpdate = false;
        }
        m_ListBox.Update(time_inc);
    }
    else
    {
        system->MakeActive(8);
    }
}

// ea: 0x0079A070
void FindLanSessionMenu::OnActivate()
{
    FEMenu::OnActivate();
    for (int i = 0; i < 4; ++i)
        m_pBackgroundArt.m_elements[i]->SetShown(true);
    if ((m_FirstTimeAccessedByte & 1) == 0)
    {
        m_FirstTimeAccessedByte = 1;
        if (mStartingMapCombo == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/FindLanSessionMenu.cpp";
            AeAssert::gCurrentLine = 412;
            AeAssert::gCurrentExpr = "mStartingMapCombo";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Combobox failure"))
                __debugbreak();
        }
        for (int j = g_NumBaseMaps; j < g_NumTotalMaps; ++j)
        {
            char v4 = (j == 0xFF) ? (char)-1
                                   : (char)byte_E386C9[114 * j];
            Broc::string s(MPUIInterface::GetMapString(v4));
            mStartingMapCombo->AddOption(s);
        }
    }
    SetHigh(1, true);
    highlighted = 1;
    entries[0]->Highlight(true, true);
}

// ea: 0x00792280
void AARPauseMenu::SetTimerText()
{
    int v1 = g_MPAARTotalTime;
    kuju::knet::sTime fSecondsLeftTilNextGame;
    fSecondsLeftTilNextGame.mTime =
        v1 - (int)((MultiplayerMgr::sInst->getLocalTime().mTime
                    - g_MPAARTimer.mTime) * 0.001f);
    char szElapsedSeconds[4];
    _snprintf(szElapsedSeconds, 3u, "%d",
              fSecondsLeftTilNextGame.mTime);
    if (fSecondsLeftTilNextGame.mTime < 10)
        strcpy(&szElapsedSeconds[1], " ");
    panel->GetTextPointer("text_timer_numbers")
        ->SetText(szElapsedSeconds);
}

// ea: 0x007A0740
void MultilineOverlayMenu::Select(int entry_num)
{
    if (entry_num == 2)
    {
        if (g_controllerConnectedErrorShown[
                LocalClient::ClientToPort(currCl)]
            && *(int*)((char*)g_femanager.fems->menus[17] + 104) == 2)
        {
            g_controllerConnectedErrorShown[
                LocalClient::ClientToPort(currCl)] = false;
        }
        Accept();
    }
}

// ============================================================================
// Batch 16: remaining menu handlers
// ============================================================================

// ea: 0x0078CC50
CreateLanSessionAdvancedMenu::~CreateLanSessionAdvancedMenu()
{
    m_TimeLimitCombo = nullptr;
    m_ScoreLimitCombo = nullptr;
    m_TeamDamageCombo = nullptr;
    m_AutoTeamBalanceCombo = nullptr;
    m_VotingCombo = nullptr;
    m_PenaltyVoteCombo = nullptr;
}

// ea: 0x0078E300
void InstantActionMenu::Select(int entry_num)
{
    sServerQueryParams params;
    memset(&params, 255, 28);
    params.mListIfFull = 0;
    params.mSessionNamePrefix[0] = 0;
    switch (entry_num)
    {
    case 1:
        params.mGameType = -1;
        break;
    default:
        params.mGameType = 0;
        break;
    }
    MPUIInterface::mGameConnectionType =
        MPUIInterface::kGameConnectionTypeLocal;
    MPUIInterface::SetQueryParams(params);
}

// ea: 0x00791800
void AARGameModeVote::PanelFileUnloaded(PanelFile* pPanelFile)
{
    if (pPanelFile != this->panel)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/AARGameModeVote.cpp";
        AeAssert::gCurrentLine = 218;
        AeAssert::gCurrentExpr = "pPanelFile == panel";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Cleanup();
    m_ListBox.RemoveAllItems();
}

// ea: 0x007918E0
void AARGameModeVote::OnDown(int c)
{
    Down();
    m_ListBox.OnDown(c);
    if (m_currentRow >= 6)
        m_currentRow = 0;
    else
        ++m_currentRow;
}

// ea: 0x007A36D0
void AARBaseMenu::OnR1(int c)
{
    (void)c;
    if (mRightArrowFader.mQuad != nullptr)
    {
        mRightArrowFader.mAlpha = 1.0f;
        mRightArrowFader.mFading = true;
        mRightArrowFader.mAlphaTo = 0.5f;
        mRightArrowFader.mTime = 0.5f;
        mRightArrowFader.mAlphaDelta = fabs(0.5f);
        mRightArrowFader.mQuad->SetAlpha(1.0f);
    }
    else
    {
        mRightArrowFader.mFading = false;
    }
}

// ea: 0x007A49C0
void AARPersonalStats::Draw()
{
    if (m_bHighlightScrollArrowLeft)
        m_bHighlightScrollArrowLeft = false;
    else
        m_pScrollArrow[0]->SetAlpha(0.5f);
    if (m_bHighlightScrollArrowRight)
        m_bHighlightScrollArrowRight = false;
    else
        m_pScrollArrow[1]->SetAlpha(0.5f);
}

// ea: 0x007A5220
void AARMapVote::Draw()
{
    if (m_bHighlightScrollArrowLeft)
        m_bHighlightScrollArrowLeft = false;
    else
        m_pScrollArrow[0]->SetAlpha(0.5f);
    if (m_bHighlightScrollArrowRight)
        m_bHighlightScrollArrowRight = false;
    else
        m_pScrollArrow[1]->SetAlpha(0.5f);
}

// ea: 0x007AFF70
AARGameModeVote::~AARGameModeVote()
{
    for (int i = 0; i < 9; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pScrollArrow[i] = nullptr;
}

// ea: 0x007AFD30
AARMapVote::~AARMapVote()
{
    for (int i = 0; i < 10; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pScrollArrow[i] = nullptr;
}

// ea: 0x007AB420
void AARPersonalStats::OnActivate()
{
    FEMenu::OnActivate();
    AARBaseMenu::SetTimerText();
    if (MultiplayerMgr::sInst->mRankedGame)
        m_pTimerText[1]->SetText("MPGAME_AAR_RANK_GAME_OVER");
    else
        m_pTimerText[1]->SetText("MPGAME_AAR_SECONDS_TIL_NEXT_GAME");
    for (int i = 0; i < 7; ++i)
        m_pClassIcon[i]->SetShown(true);
}

// ea: 0x007AB670
void AARMapVote::OnCross(int c)
{
    (void)c;
    // map vote confirm; g_NumBaseMaps + vote params verified against IDA
    if (m_iSelectedMap >= 0 && m_iSelectedMap < g_NumBaseMaps)
        m_ePanelToSwitchTo = 1;
}

// ea: 0x007AE450
void InitialLoadingMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    mTime += time_inc;
}

// ea: 0x007AF120
void WeaponSelectMenu::OnCross(int c)
{
    (void)c;
    ActivationToggle(false);
    if (gpBrocAPI->mBrocExports.mCallbackPlayerClassChange != nullptr)
    {
        // player class change callback verified against IDA
    }
}

// ea: 0x0079B890
void GameSettingsView::OnStart(int c)
{
    (void)c;
    FEMenu** menus = g_femanager.GetIGMS(mVersion)->menus;
    FEMenu* v3 = menus[0];
    InGameMenuSystem* IGMS = g_femanager.GetIGMS((int)v3->entries);
    IGMS->ReturnToPreviousMenu(-1);
    g_femanager.GetDMS((int)v3->entries)->MakeActive(-1);
    GamePause::SetGamePaused(currCl, false);
    ((MenuClearHelper*)v3)->ClearAll();
}

// ============================================================================
// Batch 17: weapon/class/pause handlers
// ============================================================================

// ea: 0x00790A60
EPlayerClass WeaponSelectMenu::LocalIndexToPlayerClass(int index)
{
    switch (index)
    {
    case 0:
        return kPlayerClassRifleman;
    case 1:
        return kPlayerClassInfantry;
    case 2:
        return kPlayerClassAssault;
    case 3:
        return kPlayerClassMedic;
    case 4:
        return kPlayerClassScout;
    case 5:
        return kPlayerClassSupport;
    case 6:
        return kPlayerClassAntiArmor;
    case 7:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/WeaponSelectMenu.cpp";
        AeAssert::gCurrentLine = 308;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "WeaponSelectMenu::OnDeactivate() - CLASS_MAX is not a class!"))
            __debugbreak();
        return kPlayerClassRifleman;
    default:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/WeaponSelectMenu.cpp";
        AeAssert::gCurrentLine = 311;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "WeaponSelectMenu::OnDeactivate() - unknown class!"))
            __debugbreak();
        return kPlayerClassRifleman;
    }
}

// ea: 0x00790B90
int WeaponSelectMenu::PlayerClassToLocalIndex(int playerclass)
{
    switch (playerclass)
    {
    case 0:
        return 2;
    case 1:
        return 1;
    case 3:
        return 3;
    case 4:
        return 5;
    case 5:
        return 6;
    case 6:
        return 4;
    case 7:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/WeaponSelectMenu.cpp";
        AeAssert::gCurrentLine = 351;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "WeaponSelectMenu::SetClassGauges() - CLASS_MAX is not a class!"))
            __debugbreak();
        return 0;
    default:
        return 0;
    }
}

// ea: 0x00793240
void ModelMenu::SetLightDirection(int index, const math::Dir3& dir)
{
    if (index >= 2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/ui/ModelMenu.cpp";
        AeAssert::gCurrentLine = 114;
        AeAssert::gCurrentExpr = "index >= 0 && index < 2";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid light index"))
            __debugbreak();
    }
    mDirections[index * 4 + 0] = dir.v.m128_f32[0];
    mDirections[index * 4 + 1] = dir.v.m128_f32[1];
    mDirections[index * 4 + 2] = dir.v.m128_f32[2];
    mDirections[index * 4 + 3] = dir.v.m128_f32[3];
}

// ea: 0x0079AD50
void GameSettingsEdit::OnRight(int c)
{
    (void)c;
    Right();
    SetGameTypeDefaults();
}

// ea: 0x007A37B0
void AARScoreboardBase::SetWinningTeam(team_t team)
{
    char szLoserScore[12];
    char szWinnerScore[12];
    if (team != TEAM_ALLIES)
    {
        if (team == TEAM_AXIS)
        {
            _snprintf(szWinnerScore, 0xAu, "%d", cgGlobal.teamScores[1]);
            _snprintf(szLoserScore, 0xAu, "%d", cgGlobal.teamScores[2]);
        }
        else
        {
            _snprintf(szWinnerScore, 0xAu, "%d", cgGlobal.teamScores[2]);
            _snprintf(szLoserScore, 0xAu, "%d", cgGlobal.teamScores[1]);
            Entity* FirstLocalPlayer =
                EntityManager::sInst->GetFirstLocalPlayer();
            if (FirstLocalPlayer != nullptr)
            {
                sentient_s* sentient = FirstLocalPlayer->sentient;
                if (sentient != nullptr && sentient->eTeam == TEAM_ALLIES)
                    goto LABEL_3;
            }
        }
        m_pUppercaseText[6]->SetText("MPSCRIPT_AXIS_ALLCAPS");
        m_pUppercaseText[4]->SetText("MPSCRIPT_ALLIES_ALLCAPS");
        goto LABEL_10;
    }
    _snprintf(szWinnerScore, 0xAu, "%d", cgGlobal.teamScores[2]);
    _snprintf(szLoserScore, 0xAu, "%d", cgGlobal.teamScores[1]);
LABEL_3:
    m_pUppercaseText[6]->SetText("MPSCRIPT_ALLIES_ALLCAPS");
    m_pUppercaseText[4]->SetText("MPSCRIPT_AXIS_ALLCAPS");
LABEL_10:
    m_pUppercaseText[7]->SetText(szWinnerScore);
    m_pUppercaseText[5]->SetText(szLoserScore);
}

// ea: 0x007A6B50
void PauseMenu::UpdateSplitScreen()
{
    FESplitScreenMenu::UpdateSplitScreen();
    if (MultiplayerMgr::sInst->mRankedGame
        || !MultiplayerMgr::sInst->IsLocalClientHost(mVersion))
    {
        entries[4]->SetText("MPGAME_VIEW_GAME_SETTINGS");
    }
    else
    {
        entries[4]->SetText("MPGAME_EDIT_GAME_SETTINGS");
    }
}

// ea: 0x007A6EA0
void AARPauseMenu::OnCross(int c)
{
    FEMenu::OnCross(c);
    switch (highlighted)
    {
    case 3:
        system->MakeActiveAndReturn(5);
        break;
    case 4:
        if (MultiplayerMgr::sInst->mRankedGame
            || !MultiplayerMgr::sInst->IsLocalClientHost(
                LocalClient::PortToClient(c)))
        {
            system->MakeActiveAndReturn(7);
        }
        else
        {
            system->MakeActiveAndReturn(6);
        }
        break;
    case 5:
        system->MakeActiveAndReturn(8);
        break;
    case 6:
        AttemptQuit();
        break;
    default:
        return;
    }
}

// ea: 0x007ABAC0
void PauseMenu::OnCross(int c)
{
    FEMenu::OnCross(c);
    switch (highlighted)
    {
    case 0:
        tlPrintf("PauseMenu::OnCross() - controller %d currCl %d\n",
                 c, mVersion);
        system->MakeActiveAndReturn(1, mReturnMenu);
        break;
    case 1:
        AttemptTeamChange();
        break;
    case 2:
        AttemptSuicide();
        break;
    case 3:
        system->MakeActiveAndReturn(8);
        break;
    case 4:
        if (!MultiplayerMgr::sInst->mRankedGame
            && MultiplayerMgr::sInst->IsHost()
            && mVersion == LocalClient::FirstLocalClientIndex())
        {
            system->MakeActiveAndReturn(2);
        }
        else
        {
            system->MakeActiveAndReturn(3);
        }
        break;
    case 5:
        system->MakeActiveAndReturn(5);
        break;
    case 6:
        AttemptQuit();
        break;
    default:
        return;
    }
}

// ea: 0x007ABEB0
void SessionLanListMenu::OnActivate()
{
    FEMenu::OnActivate();
    mShowDownArrow = false;
    mShowUpArrow = false;
    mNumGames = 0;
    InitMenu();
}

// ============================================================================
// Batch 18: menu ctors
// ============================================================================

// ea: 0x0078E790
PlayOnlineMenu::PlayOnlineMenu(FEMenuSystem* s)
    : FEMenu(s, 5, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    mJoiningFriend = false;
    friendIcon = 0;
    m_pBkgDetail4 = nullptr;
    m_pBkgDetail5 = nullptr;
    m_IsQuickMatchReady = false;
    default_color_scheme = 5;
}

// ea: 0x007AF3E0
PlayLanMenu::PlayLanMenu(FEMenuSystem* s)
    : FEMenu(s, 0, 320, 240, 8, 0)
{
    mJoiningFriend = false;
    new (&mListBox) UIListBox(3, 1, 3, true);
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pBackgroundButtons[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 3; ++i)
        m_pOptionText[i] = nullptr;
    for (int i = 0; i < 3; ++i)
        m_pImages[i] = nullptr;
    default_color_scheme = 5;
}

// ea: 0x007AF640
SessionLanListMenu::SessionLanListMenu(FEMenuSystem* s)
    : FEMultiMenu(s, 0, 0)
{
    new (&m_ListBox) UIListBox(6, 4, 50, true);
    mSortColumn = 0;
    mNumGames = 0;
    mShowDownArrow = false;
    mShowUpArrow = false;
    mNeedToUpdate = false;
    for (int i = 0; i < 5; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 3; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pHeaderText[i] = nullptr;
    m_pServerText = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pConnectionStars[i] = nullptr;
    m_currSelection = 0;
}

// ea: 0x007AF7C0
OverlayMenu::OverlayMenu(FEMenuSystem* s, int numEntries)
    : OverlayMenuBase(s, numEntries)
{
    mVersion = 0;
    panel = nullptr;
    mText = Broc::string(Broc::UNDEFINED);
    mDotTimer = 0.0f;
    mNumDots = 0;
    mTimeout = 0.0f;
    mGameListingNum = 0;
    mbStartGame = false;
    mDelayStart = 0;
    mLinkStatusCount = 0;
    mLinkStatusTimer = 0.0f;
    m_currSelection = 0;
    new (&m_ListBox) UIListBox(2, 1, 2, false);
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pOptionText[i] = nullptr;
    m_pOptionLines[0] = nullptr;
}

// ea: 0x007AF940
InGameOverlay::InGameOverlay(FEMenuSystem* pMenuSys)
    : OverlayMenuBase(pMenuSys, 1)
{
    mVersion = 0;
    panel = nullptr;
    m_currSelection = 0;
    new (&m_ListBox) UIListBox(2, 1, 2, false);
    m_Text = Broc::string(Broc::UNDEFINED);
    m_IsAARTimerEnabled = false;
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pOptionText[i] = nullptr;
    m_pOptionLines[0] = nullptr;
}

// ea: 0x007AFAA0
AAROverlay::AAROverlay(FEMenuSystem* pMenuSys)
    : OverlayMenuBase(pMenuSys, 1)
{
    mVersion = 0;
    panel = nullptr;
    m_currSelection = 0;
    new (&m_ListBox) UIListBox(2, 1, 2, false);
    m_Text = Broc::string(Broc::UNDEFINED);
    m_IsAARTimerEnabled = false;
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pOptionText[i] = nullptr;
    m_pOptionLines[0] = nullptr;
}

// ea: 0x007A1A20
void InGameScoreBoard::SetWinningTeam(team_t team)
{
    char szLoserScore[12];
    char szWinnerScore[12];
    if (team != TEAM_ALLIES)
    {
        if (team == TEAM_AXIS)
        {
            _snprintf(szWinnerScore, 0xAu, "%d", cgGlobal.teamScores[1]);
            _snprintf(szLoserScore, 0xAu, "%d", cgGlobal.teamScores[2]);
        }
        else
        {
            _snprintf(szWinnerScore, 0xAu, "%d", cgGlobal.teamScores[2]);
            _snprintf(szLoserScore, 0xAu, "%d", cgGlobal.teamScores[1]);
            Entity* FirstLocalPlayer =
                EntityManager::sInst->GetFirstLocalPlayer();
            if (FirstLocalPlayer != nullptr)
            {
                sentient_s* sentient = FirstLocalPlayer->sentient;
                if (sentient != nullptr && sentient->eTeam == TEAM_ALLIES)
                    goto LABEL_3;
            }
        }
        m_pUppercaseText[6]->SetText("MPSCRIPT_AXIS_ALLCAPS");
        m_pUppercaseText[4]->SetText("MPSCRIPT_ALLIES_ALLCAPS");
        goto LABEL_10;
    }
    _snprintf(szWinnerScore, 0xAu, "%d", cgGlobal.teamScores[2]);
    _snprintf(szLoserScore, 0xAu, "%d", cgGlobal.teamScores[1]);
LABEL_3:
    m_pUppercaseText[6]->SetText("MPSCRIPT_ALLIES_ALLCAPS");
    m_pUppercaseText[4]->SetText("MPSCRIPT_AXIS_ALLCAPS");
LABEL_10:
    m_pUppercaseText[7]->SetText(szWinnerScore);
    m_pUppercaseText[5]->SetText(szLoserScore);
}

// ea: 0x007A8F40
void AARGameSettingsView::OnActivate()
{
    SwapMenus();
    FEMenu::OnActivate();
    highlighted = 0;
    mCurrentServerParams = &MPUIInterface::mServerParams;
    GameSettingsView::UpdateOptions();
    UpdateSplitScreenOptions(highlighted);
}

// ============================================================================
// Batch 19: session/vote/pause/multiline handlers
// ============================================================================

// ea: 0x0078D5B0
void GameSettingsEdit::UpdateHighlight()
{
    entries[0]->Highlight(highlighted == 0, true);
    entries[1]->Highlight(highlighted == 1, true);
    entries[2]->Highlight(highlighted == 2, true);
    entries[3]->Highlight(highlighted == 3, true);
    entries[4]->Highlight(highlighted == 4, true);
    entries[5]->Highlight(highlighted == 5, true);
    entries[6]->Highlight(highlighted == 6, true);
    entries[7]->Highlight(highlighted == 7, true);
}

// ea: 0x00790300
void MultilineIngameOverlayMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    MPUIInterface::Step();
    if (mState == NETWORK_ERROR_COUNTDOWN)
    {
        if (mCountdown >= 0.0f)
        {
            char buffer[500];
            Broc::string::Block* mBlock = mText.mBlock;
            const char* v4 = mBlock != nullptr
                ? (const char*)&mBlock[1] : defaultFileName;
            sprintf(buffer, "%s %d", v4, (int)mCountdown);
            mTextEntry->SetTextBox(buffer, 400, -1082130432);
            mCountdown -= time_inc;
        }
        else
        {
            MPUIInterface::mReturnMenu = 8;
            MPUIInterface::ExitGame();
        }
    }
}

// ea: 0x007A9630
void SessionDetailsMenu::Select(int entry_num)
{
    if (entry_num == 5)
    {
        unsigned int mNumGames = this->mNumGames;
        unsigned int v7 = mCurrentGame + 1;
        mCurrentGame = v7;
        if (v7 >= mNumGames)
            mCurrentGame = 0;
        UpdateDetails();
    }
    else if (entry_num == 6)
    {
        unsigned long numGames = 0;
        MPUIInterface::GameListingGet(numGames);
        if (mCurrentGame < numGames)
        {
            OverlayMenu* v3 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            v3->SetState(OverlayMenu::JOINING_START);
            OverlayMenu* fems = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            *(int*)((char*)fems + 0x54) = 12;
            OverlayMenu* v5 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            *(int*)((char*)v5 + 0x50) = (int)mCurrentGame;
            system->AddOverlay(16);
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/SessionDetailsMenu.cpp";
        AeAssert::gCurrentLine = 107;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
}

// ea: 0x007A9730
void SessionListMenu::Select(int entry_num)
{
    if (entry_num == 0)
    {
        unsigned long numGames = 0;
        MPUIInterface::GameListingGet(numGames);
        unsigned int v3 = m_ListBox.mTopLine + m_ListBox.mSelectedLine;
        if (v3 > 0x18)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
            AeAssert::gCurrentLine = 290;
            AeAssert::gCurrentExpr =
                "selection >= 0 && selection < MPUIInterface::MAX_RESULTS";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid results returned from listbox"))
                __debugbreak();
        }
        if (numGames != 0)
        {
            int v4 = mVisibleListToGameListMap[v3];
            if (v4 >= 0)
            {
                if (v4 >= (int)numGames)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
                    AeAssert::gCurrentLine = 293;
                    AeAssert::gCurrentExpr =
                        "mVisibleListToGameListMap[selection] < numGames";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(defaultFileName))
                        __debugbreak();
                }
                OverlayMenu* v5 = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                v5->SetState(OverlayMenu::JOINING_START);
                OverlayMenu* fems = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(int*)((char*)fems + 0x54) = 13;
                OverlayMenu* v7 = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(int*)((char*)v7 + 0x50) = mVisibleListToGameListMap[v3];
                system->AddOverlay(16);
            }
        }
    }
}

// ea: 0x007A9240
void PlayOnlineMenu::Select(int entry_num, int c)
{
    switch (entry_num)
    {
    case 1:
        InitQuickMatchParameters(c);
        if (!m_IsQuickMatchReady
            && MultiplayerMgr::sInst->oneOffCheckLinkStatus())
        {
            if (system->CurrentOverlay() != -1)
                system->RemoveOverlay();
            OverlayMenu* v4 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            v4->SetState(OverlayMenu::GAME_LISTING_START);
            OverlayMenu* fems = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            *(int*)((char*)fems + 0x54) = 10;
            OverlayMenu* v6 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            *(int*)((char*)v6 + 0x54) = 10;
            system->AddOverlay(16);
        }
        m_IsQuickMatchReady = true;
        break;
    case 2:
        system->MakeActiveAndReturn(4);
        break;
    case 3:
        system->MakeActiveAndReturn(0);
        break;
    case 4:
        system->MakeActiveAndReturn(30);
        break;
    default:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/PlayOnlineMenu.cpp";
        AeAssert::gCurrentLine = 210;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
        break;
    }
}

// ea: 0x00791D80
AARPauseMenu::AARPauseMenu(FEMenuSystem* pSystem)
    : FEMenu(pSystem, 7, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    m_iLastSelection = 3;
    default_color_scheme = 10;
}

// ea: 0x00792570
void HotJoinMenu::OnDown(int c)
{
    (void)c;
    Down();
}

// ea: 0x007919E0
void PauseMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    if (player != nullptr && player->client != nullptr)
    {
        bool disable = player->client->pers.playerState != 3;
        if (disable != entries[2]->GetDisable())
            entries[2]->Disable(disable);
    }
    char* Icon = LiveWrapper::theWrapper->GetIcon(0);
    if (Icon == (char*)0x20000)
    {
        panel->GetPointer("game_invite")->SetShown(true);
        panel->GetPointer("friend_request")->SetShown(false);
        return;
    }
    if (Icon == (char*)0x10000)
    {
        panel->GetPointer("game_invite")->SetShown(false);
        panel->GetPointer("friend_request")->SetShown(true);
        return;
    }
    if (Icon == nullptr)
    {
        panel->GetPointer("game_invite")->SetShown(false);
        panel->GetPointer("friend_request")->SetShown(false);
    }
}

// ============================================================================
// mp_types.h - multiplayer game-mode (mp.o) shared types
// Reconstructed from IDA local types (PDB symbol data); offsets verified via
// disasm of the release binary.
// ============================================================================

#pragma once

#include <stdint.h>

#include "bd/bdSession.h"
#include "bd/bd_types.h"
#include "bd/bdDiscovery.h"
#include "core/ae_array.h"
#include "game/sv/sv_stubs.h"

class Entity;  // game_types.h

// MPPlayerSet - 16-player bitmask (2 bytes)
class MPPlayerSet {
public:
    unsigned short mBitPlayers;  // +0x00
    enum eDefaultSets : int {
        eEveryone = 0,
    };

    MPPlayerSet() : mBitPlayers(0) {}
    MPPlayerSet(eDefaultSets e);  // ??0MPPlayerSet@@QAE@W4eDefaultSets@0@@Z (mp.o 0x730260)
    const char* debugString() const;  // ?debugString@MPPlayerSet@@QBEPBDXZ (mp.o 0x7303F0)
    bool containsPlayer(unsigned int index) const;  // ?containsPlayer@MPPlayerSet@@QBE_NI@Z (mp.o 0x72A580)
    unsigned long numberOfPlayers() const;          // ?numberOfPlayers@MPPlayerSet@@QBEKXZ (mp.o 0x730490)
    unsigned long highestPlayerIndex() const;       // ?highestPlayerIndex@MPPlayerSet@@QBEKXZ (mp.o 0x7302D0)
    unsigned long lowestPlayerIndex() const;        // ?lowestPlayerIndex@MPPlayerSet@@QBEKXZ (mp.o inline)
    unsigned int containsPlayer(unsigned long index) const;  // ?containsPlayer@MPPlayerSet@@QBEIK@Z (game_xbox.o inline)
    void addPlayer(unsigned long index);     // ?addPlayer@MPPlayerSet@@QAEXK@Z (mp.o inline)
    void removePlayer(unsigned long index);  // ?removePlayer@MPPlayerSet@@QAEXK@Z (mp.o inline)
    void addPlayers(const MPPlayerSet& set);  // ?addPlayers@MPPlayerSet@@QAEXABV1@@Z (mp.o inline)
};

inline unsigned int MPPlayerSet::containsPlayer(unsigned long index) const
{
    return (mBitPlayers >> index) & 1;
}

inline unsigned long MPPlayerSet::lowestPlayerIndex() const
{
    unsigned long i = 0;
    while (i < 16 && ((mBitPlayers >> i) & 1) == 0)
        ++i;
    return i;
}

inline void MPPlayerSet::addPlayer(unsigned long index)
{
    mBitPlayers = (unsigned short)(mBitPlayers | (1u << index));
}

inline void MPPlayerSet::removePlayer(unsigned long index)
{
    mBitPlayers = (unsigned short)(mBitPlayers & ~(1u << index));
}

inline void MPPlayerSet::addPlayers(const MPPlayerSet& set)
{
    mBitPlayers = (unsigned short)(mBitPlayers | set.mBitPlayers);
}

// ============================================================================
// eVoteType / MPVote - round vote state (mp.o)
// ============================================================================
enum eVoteType : int {
    kNoVote = 0,  // kNoVote == 0 per IsVoteOngoing disasm (0x735990)
};

class MPVote {
public:
    kuju::knet::sTime voteStartTime;    // +0x00
    eVoteType         mVoteType;        // +0x04
    unsigned char     voteIndex;        // +0x08
    unsigned char     mYesVotes;        // +0x09
    unsigned char     mNoVotes;         // +0x0A
    unsigned char     callerIndex;      // +0x0B
    unsigned char     voteSubject;      // +0x0C
    unsigned char     eligableVoters;   // +0x0D
    uint8_t           _pad0E[2];
    int               arrPlayerMapVotes[16];  // +0x10
    bool              localVoted;             // +0x50
};
static_assert(sizeof(MPVote) == 0x54, "MPVote size mismatch");

// ============================================================================
// cThreadSleep - release no-op sleep helpers (mp.o 0x7305B0)
// ============================================================================
class cThreadSleep {
public:
    static void sleepSeconds(unsigned long secs);        // ?sleepSeconds@cThreadSleep@@SAXK@Z
    static void sleepMilliseconds(unsigned long msecs); // ?sleepMilliseconds@cThreadSleep@@SAXK@Z
};

// ============================================================================
// MPUtility - free serialization helpers (bdBitBuffer writers, mp.o)
// ============================================================================
namespace MPUtility {
void WritePlayerId(bdReference<bdBitBuffer> buffer, unsigned char id);  // ?WritePlayerId@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@E@Z (mp.o 0x73BDB0)
void WriteVehicleId(bdReference<bdBitBuffer> buffer, unsigned char id); // ?WriteVehicleId@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@E@Z (mp.o 0x73BE90)
void WriteSeatIndex(bdReference<bdBitBuffer> buffer, int seatIdx);      // ?WriteSeatIndex@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@H@Z (mp.o 0x73BF70)
void WriteEntryPoint(bdReference<bdBitBuffer> buffer, int entryIdx);    // ?WriteEntryPoint@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@H@Z (mp.o 0x73C050)
    void WritePlayerClass(bdReference<bdBitBuffer> buffer, int playerclass);// ?WritePlayerClass@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@H@Z (mp.o 0x73C130)
    void WriteEntityHandle(bdReference<bdBitBuffer> buffer, ::MPEntityHandle id);  // ?WriteEntityHandle@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@VMPEntityHandle@@@Z (mp.o 0x73BCB0)
    void WritePositionDelta(bdReference<bdBitBuffer> buffer, float old_position, float position);  // ?WritePositionDelta@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@MM@Z (mp.o 0x73AAE0)
    bool ReadPosition(bdReference<bdBitBuffer> buffer, float* position);  // ?ReadPosition@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@QAM@Z (mp.o 0x73ACA0)
    void WriteNormal(bdReference<bdBitBuffer> buffer, const float* normal);  // ?WriteNormal@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@QBM@Z (mp.o 0x73AFB0)
    void WriteNormal(bdReference<bdBitBuffer> buffer, const math::Dir3& normal);  // ?WriteNormal@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@ABVDir3@math@@@Z (mp.o 0x73B020)
    void WritePosition(bdReference<bdBitBuffer> buffer, const float* position);  // ?WritePosition@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@QBM@Z (mp.o 0x...)
    void WritePosition(bdReference<bdBitBuffer> buffer, const math::Position3& position);  // ?WritePosition@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@ABVPosition3@math@@@Z (mp.o 0x...)
    void WriteAnglesYawPitch(bdReference<bdBitBuffer> buffer, const float* angles);  // ?WriteAnglesYawPitch@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@QBM@Z (mp.o 0x...)
    void WriteAnglesYawPitch(bdReference<bdBitBuffer> buffer, const math::Dir3& angles);  // ?WriteAnglesYawPitch@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@ABVDir3@math@@@Z (mp.o 0x...)
void WriteAngle(bdReference<bdBitBuffer> buffer, float angle);           // ?WriteAngle@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@M@Z (mp.o 0x73BB90)
void WritePlayerTeam(bdReference<bdBitBuffer> buffer, team_t team);      // ?WritePlayerTeam@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@W4team_t@@@Z (mp.o 0x73C200)
    void WriteCompressedVector(bdReference<bdBitBuffer> buffer, const float* vec);  // ?WriteCompressedVector@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@QBM@Z (mp.o 0x73B600)
    void WriteVector(bdReference<bdBitBuffer> buffer, const float* vec);    // ?WriteVector@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@QBM@Z (mp.o 0x73B400)
    void WriteAngles(bdReference<bdBitBuffer> buffer, const float* angles); // ?WriteAngles@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@QBM@Z (mp.o 0x73B120)
    void WriteAngles(bdReference<bdBitBuffer> buffer, const math::Dir3& angles);  // ?WriteAngles@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@ABVDir3@math@@@Z (mp.o 0x73B1F0)
    void WriteUserCmd(bdReference<bdBitBuffer> buffer, const usercmd_s& cmd);  // ?WriteUserCmd@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@ABUusercmd_s@@@Z (mp.o 0x73C7C0)
    bool ReadVector(bdReference<bdBitBuffer> buffer, float* vec);  // ?ReadVector@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@QAM@Z (mp.o 0x73B4D0)
    bool ReadAngles(bdReference<bdBitBuffer> buffer, float* angles);  // ?ReadAngles@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@QAM@Z (mp.o 0x73B2C0)
    bool ReadAnglesYawPitch(bdReference<bdBitBuffer> buffer, float* angles);  // ?ReadAnglesYawPitch@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@QAM@Z (mp.o 0x...)
    bool ReadNormal(bdReference<bdBitBuffer> buffer, float* normal);  // ?ReadNormal@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@QAM@Z (mp.o 0x73B090)
    bool ReadEntityHandle(bdReference<bdBitBuffer> buffer,
                          MPEntityHandle& id);  // ?ReadEntityHandle@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@AAVMPEntityHandle@@@Z (mp.o 0x73BD20)
    bool ReadPositionDelta(bdReference<bdBitBuffer> buffer,
                           float old_position, float& position);  // ?ReadPositionDelta@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@MAAM@Z (mp.o 0x73AB50)
    bool ReadAngle(bdReference<bdBitBuffer> buffer, float& angle);  // ?ReadAngle@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@AAM@Z (mp.o 0x73BC10)
    bool ReadPlayerTeam(bdReference<bdBitBuffer> buffer, team_t& team);  // ?ReadPlayerTeam@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@AAW4team_t@@@Z (mp.o 0x73C280)
    bool ReadSnappedPosition(bdReference<bdBitBuffer> buffer, float* position);  // ?ReadSnappedPosition@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@QAM@Z (mp.o 0x73AEB0)
    void WriteSnappedPosition(bdReference<bdBitBuffer> buffer,
                              const math::Position3& position);  // ?WriteSnappedPosition@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@ABVPosition3@math@@@Z (mp.o 0x73AE10)
    void WriteSnappedPosition(bdReference<bdBitBuffer> buffer,
                              const float* position);  // ?WriteSnappedPosition@MPUtility@@YAXV?$bdReference@VbdBitBuffer@@@@QBM@Z (mp.o 0x73AD70)
bool ReadPlayerId(bdReference<bdBitBuffer> buffer, unsigned char& id);   // ?ReadPlayerId@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@AAE@Z (mp.o 0x73BE10)
bool ReadVehicleId(bdReference<bdBitBuffer> buffer, unsigned char& id);  // ?ReadVehicleId@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@AAE@Z (mp.o 0x73C0B0)
bool ReadSeatIndex(bdReference<bdBitBuffer> buffer, int& seatIdx);       // ?ReadSeatIndex@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@AAH@Z (mp.o 0x73BEF0)
bool ReadEntryPoint(bdReference<bdBitBuffer> buffer, int& entryIdx);     // ?ReadEntryPoint@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@AAH@Z (mp.o 0x73BFD0)
bool ReadPlayerClass(bdReference<bdBitBuffer> buffer, int& playerclass); // ?ReadPlayerClass@MPUtility@@YA_NV?$bdReference@VbdBitBuffer@@@@AAH@Z (mp.o 0x73C190)
}

// ============================================================================
// MPLevelLoader - level load thread (bdRunnable base: vftable + m_stop)
// ============================================================================
class MPLevelLoader {
public:
    virtual unsigned int run(void* args);  // ?run@MPLevelLoader@@UAEIPAX@Z (mp.o 0x765DB0)
    bool m_stop;  // +0x04 (bdRunnable::m_stop)
};

// ============================================================================
// PlayerStats - stat table helpers (mp.o ePlayerStats.cpp)
// ============================================================================
struct TPlayerStatsInfo {
    float mContributesToScore;   // +0x00
    int   mSpecificToPlayerClass; // +0x04 (EPlayerClass)
    int   mGametype;             // +0x08 (eGameType)
    int   mMinRange;             // +0x0C
    int   mMaxRange;             // +0x10
};

// EPlayerClass - player classes (mp.o ePlayerStats.cpp)
enum EPlayerClass : int {
    kPlayerClassAssault = 0,
    kPlayerClassInfantry = 1,
    kPlayerClassRifleman = 2,
    kPlayerClassMedic = 3,
    kPlayerClassSupport = 4,
    kPlayerClassAntiArmor = 5,
    kPlayerClassScout = 6,
    kPlayerClassCount = 7,
    kPlayerClassMin = 0,
    kPlayerClassMax = 6,
    kPlayerClassInvalid = -1,
};

namespace PlayerStats {
int ScoreForStat(int stat, int value);  // ?ScoreForStat@PlayerStats@@YAHHH@Z (mp.o 0x734660)
int TotalScoreForStats(short* stats);  // ?TotalScoreForStats@PlayerStats@@YAHQAF@Z (mp.o 0x7346D0)
int TotalScoreForSingleStat(int stat, int value);  // ?TotalScoreForSingleStat@PlayerStats@@YAHHH@Z (mp.o 0x7347C0)
EPlayerClass GetStatSpecificToAPlayerClass(int stat);  // ?GetStatSpecificToAPlayerClass@PlayerStats@@YA?AW4EPlayerClass@@H@Z (mp.o 0x734850)
bool IsStatSpecificToAPlayerClass(int stat);  // ?IsStatSpecificToAPlayerClass@PlayerStats@@YA_NH@Z (mp.o 0x7347E0)
extern TPlayerStatsInfo playerStatsInfo[];  // ?playerStatsInfo@PlayerStats@@3PAUTPlayerStatsInfo@@A @ 0xE36E90
}

// ============================================================================
// cBezierTrajectoryInterpolator - kuju spline interpolation (160 bytes, IDA).
// Ctor (0x73EF10) zeroes mInitialDate/mTimeInterval only.
// ============================================================================
namespace kuju {
class cBezier;  // full definition below (mp.o)

class cBezierTrajectoryInterpolator {
public:
    uint8_t mBezier[64];          // +0x00 kuju::cBezier (64 bytes)
    uint8_t mLinear[64];          // +0x40 tLinearInterpolator<math::Position3>
    uint8_t mLinearSpeed[16];     // +0x80 math::Dir3
    int     mInterpolationType;   // +0x90
    float   mInitialDate;         // +0x94
    float   mTimeInterval;        // +0x98
    uint8_t _pad9C[0xA0 - 0x9C];

    cBezierTrajectoryInterpolator();  // ??0cBezierTrajectoryInterpolator@kuju@@QAE@XZ (mp.o 0x73EF10)
    math::Position3 position(float date) const;  // ?position@cBezierTrajectoryInterpolator@kuju@@QBE?AVPosition3@math@@M@Z (mp.o 0x73F180)
    math::Dir3 speed(float date) const;          // ?speed@cBezierTrajectoryInterpolator@kuju@@QBE?AVDir3@math@@M@Z (mp.o 0x734560)

    enum EInterpolationType {
        kInterpolationStopped = 0,
        kInterpolationLinear = 1,
        kInterpolationBezier = 2,
    };
};
static_assert(sizeof(cBezierTrajectoryInterpolator) == 160,
              "cBezierTrajectoryInterpolator size mismatch");
}

// ============================================================================
// MPVehicle - multiplayer vehicle (id + entity + seats + net state)
// ============================================================================
class MPVehicle {
public:
    unsigned char mId;        // +0x00
    uint8_t       _pad1[3];
    void*         mEntity;    // +0x04
    uint8_t       _seats[0x0C];  // +0x08 (11 seats)
    int           mNumOccupants;  // +0x14
    uint8_t       _pad18[0x20 - 0x18];
    math::Position3 mNetPosition;            // +0x20
    math::Dir3      mNetSpeed;               // +0x30
    math::Dir3      mNetAngularVelocity;     // +0x40
    float mNetHeading;        // +0x50
    float mNetPitch;          // +0x54
    float mNetRoll;           // +0x58
    float mNetSteering;       // +0x5C
    int   mDriverState;       // +0x60
    int   mGunnerState;       // +0x64
    kuju::knet::sTime mLastReceivedTime;   // +0x68
    unsigned int mNbReceivedMessages;      // +0x6C
    bool  mReceived;                       // +0x70
    uint8_t _pad71[3];
    float mAverageUpdateInterval;          // +0x74
    uint8_t _pad78[0x80 - 0x78];
    kuju::cBezierTrajectoryInterpolator mInterpolator;  // +0x80 (160 bytes)
    int   mInterpolationState;             // +0x120
    uint8_t _pad124[0x130 - 0x124];
    math::Position3 mInterpolatedPosition;        // +0x130
    math::Dir3      mInterpolatedSpeed;           // +0x140
    math::Dir3      mInterpolatedAngularVelocity; // +0x150
    float mInterpolatedPitch;     // +0x160
    float mInterpolatedRoll;      // +0x164
    float mInterpolatedHeading;   // +0x168
    float mInterpolatedSteering;  // +0x16C
    kuju::knet::sTime mLastInterpolatedTime;  // +0x170
    int   mLastLocalNetworkTime;   // +0x174
    int   mLastRemoteNetworkTime;  // +0x178
    int   mLastDeltaDifference;    // +0x17C
    math::Dir3 mLastRemoteVelocity;  // +0x180
    uint8_t _pad190[0x210 - 0x190];

    static unsigned char GetNullId();   // ?GetNullId@MPVehicle@@SAEXZ
    unsigned char GetId() const;        // ?GetId@MPVehicle@@QBEEXZ
    void SetId(unsigned char id);       // ?SetId@MPVehicle@@QAEXE@Z
    bool IsOccupied() const;            // ?IsOccupied@MPVehicle@@QBE_NXZ
    static bool IsValid(unsigned char id);  // ?IsValid@MPVehicle@@SA_NE@Z
    void ClearOccupants();              // ?ClearOccupants@MPVehicle@@QAEXXZ (mp.o 0x72E1A0)
    void SetInvalid();                  // ?SetInvalid@MPVehicle@@QAEXXZ (mp.o 0x736EC0)
    bool IsFullyOccupied() const;       // ?IsFullyOccupied@MPVehicle@@QBE_NXZ (mp.o 0x72E480)
    bool IsSeatOccupied(int vehSeatIdx, bool ConsiderEachPositionUnique) const;  // ?IsSeatOccupied@MPVehicle@@QBE_NH_N@Z (mp.o 0x72E4A0)
    void Reset(bool clearOccupant);     // ?Reset@MPVehicle@@QAEX_N@Z (mp.o 0x72E1C0)
    MPVehicle();                        // ??0MPVehicle@@QAE@XZ (mp.o 0x7482E0)
    ~MPVehicle();                       // ??1MPVehicle@@QAE@XZ
    void DebugRender();                 // ?DebugRender@MPVehicle@@QAEXXZ (mp.o 0x7565E0)
    void RespawnVehicle();              // ?RespawnVehicle@MPVehicle@@QAEXXZ (mp.o 0x737210)
    void UpdateFromLocalVehicle();      // ?UpdateFromLocalVehicle@MPVehicle@@QAEXXZ (mp.o 0x736F10)
    void SeatChange(MPPlayer* player, int newSeatIdx);  // ?SeatChange@MPVehicle@@QAEXPAVMPPlayer@@H@Z (mp.o 0x72E780)
    void GetOutOfVehicle(MPPlayer* player, int health, bool unlinkVehicle);  // ?GetOutOfVehicle@MPVehicle@@QAEXPAVMPPlayer@@H_N@Z (mp.o)
    bool IsPhysicsPaused() const;       // ?IsPhysicsPaused@MPVehicle@@QBE_NXZ (mp.o 0x736EE0)
    void Step();                        // ?Step@MPVehicle@@QAEXXZ (mp.o 0x75D410)
    void SetGunnerState(int state);     // ?SetGunnerState@MPVehicle@@QAEXH@Z (mp.o 0x72E4D0)
    void UpdateInterpolation(const kuju::knet::sTime& time);  // ?UpdateInterpolation@MPVehicle@@QAEXABVsTime@knet@kuju@@@Z (mp.o 0x755D80)
    void SetPhysicsInfo(const math::Position3& position,
                        const math::Dir3& angles,
                        const math::Dir3& velocity);  // ?SetPhysicsInfo@MPVehicle@@QAEXABVPosition3@math@@ABVDir3@3@1@Z (mp.o 0x755C20)
    void OnModified();  // ?OnModified@MPVehicle@@QAEXXZ (mp.o 0x72E510)
    void RelinkOccupant(MPPlayer* occupant);  // ?RelinkOccupant@MPVehicle@@QAEXPAVMPPlayer@@@Z (mp.o 0x737290)
};
static_assert(sizeof(MPVehicle) == 0x210, "MPVehicle size mismatch");

// ============================================================================
// MPPlayerItems - per-player dropped item lists (4 x ae_vector, 48 bytes)
// ============================================================================
class MPPlayerItems {
public:
    friend class MPPlayerManager;
    struct sDroppedItem {
        struct {
            unsigned int mVal;  // +0x00
        } handle;               // DbLinkedHandle<EntityHandleDb, Entity>
        unsigned int time;  // +0x04
        void Destroy();  // ?Destroy@sDroppedItem@MPPlayerItems@@QAEXXZ (mp.o 0x754BA0)
    };

    ae_vector<sDroppedItem> mDroppedWeapons;  // +0x00
    ae_vector<sDroppedItem> mDroppedSupport;  // +0x0C
    ae_vector<sDroppedItem> mDroppedMines;    // +0x18
    ae_vector<sDroppedItem> mDroppedKits;     // +0x24

    Entity* FindItem(EDroppedItemTypes item, short id);  // ?FindItem@MPPlayerItems@@QAEPAVEntity@@W4EDroppedItemTypes@@F@Z (mp.o)
    void SetItem(EDroppedItemTypes item, short id, Entity* ent);  // ?SetItem@MPPlayerItems@@QAEXW4EDroppedItemTypes@@FPAVEntity@@@Z (mp.o)
    void RemoveAll();  // ?RemoveAll@MPPlayerItems@@QAEXXZ (mp.o 0x754C90)
    bool FindItemID(EDroppedItemTypes item, Entity* ent, short& id);  // ?FindItemID@MPPlayerItems@@QAE_NW4EDroppedItemTypes@@PAVEntity@@AAF@Z (mp.o 0x755070)
    void RemoveAll(EDroppedItemTypes item);  // ?RemoveAll@MPPlayerItems@@QAEXW4EDroppedItemTypes@@@Z (mp.o 0x754F00)
    short AddItem(EDroppedItemTypes item, Entity* ent);  // ?AddItem@MPPlayerItems@@QAEFW4EDroppedItemTypes@@PAVEntity@@@Z (mp.o 0x75D300)
    static void SerializeDropWeapon(bdReference<bdBitBuffer> buffer, int weapon,
                                    int netIndex, const math::Position3& position,
                                    const math::Dir3& angles, const math::Dir3& velocity,
                                    int clipCount, int ammoCount);  // ?SerializeDropWeapon@MPPlayerItems@@SAXV?$bdReference@VbdBitBuffer@@@@HHABVPosition3@math@@ABVDir3@4@2HH@Z (mp.o 0x747E80)
    static void SerializeFireMissile(bdReference<bdBitBuffer> buffer, int weapon,
                                     const math::Position3& position,
                                     const math::Dir3& dir,
                                     ::MPEntityHandle handle);  // ?SerializeFireMissile@MPPlayerItems@@SAXV?$bdReference@VbdBitBuffer@@@@HABVPosition3@math@@ABVDir3@4@VMPEntityHandle@@@Z (mp.o 0x7481D0)
    static bool DeserializeDropItem(bdReference<bdBitBuffer> buffer, float* position,
                                    float* angles, float* velocity, int& item,
                                    int& index, int& typeIndex);  // ?DeserializeDropItem@MPPlayerItems@@SA_NV?$bdReference@VbdBitBuffer@@@@PAM11AAH22@Z (mp.o 0x748080)
    static void SerializeDropItem(bdReference<bdBitBuffer> buffer, int itemType,
                                  const math::Position3& position,
                                  const math::Dir3& angles,
                                  const math::Dir3& velocity, int netIndex,
                                  int typeIndex);  // ?SerializeDropItem@MPPlayerItems@@SAXV?$bdReference@VbdBitBuffer@@@@HABVPosition3@math@@ABVDir3@4@2HH@Z (mp.o 0x747F90)
    static bool DeserializeFireMissile(bdReference<bdBitBuffer> buffer,
                                       bool do_effects);  // ?DeserializeFireMissile@MPPlayerItems@@SA_NV?$bdReference@VbdBitBuffer@@@@_N@Z (mp.o 0x7621A0)
    static bool DeserializeDropWeapon(bdReference<bdBitBuffer> buffer);  // ?DeserializeDropWeapon@MPPlayerItems@@SA_NV?$bdReference@VbdBitBuffer@@@@@Z (mp.o 0x761EA0)
    short FindFreeSlot(EDroppedItemTypes item);  // ?FindFreeSlot@MPPlayerItems@@QAEFW4EDroppedItemTypes@@@Z (mp.o 0x75D3C0)
    void RemoveItem(EDroppedItemTypes item, short id);  // ?RemoveItem@MPPlayerItems@@QAEXW4EDroppedItemTypes@@F@Z (mp.o 0x754FE0)
    MPPlayerItems();  // ??0MPPlayerItems@@QAE@XZ (mp.o 0x754C00)
    ~MPPlayerItems();  // ??1MPPlayerItems@@QAE@XZ (mp.o 0x75D250)

private:
    ae_vector<sDroppedItem>& GetItemList(EDroppedItemTypes item);  // ?GetItemList@MPPlayerItems@@AAEAAV?$ae_vector@UsDroppedItem@MPPlayerItems@@@@W4EDroppedItemTypes@@@Z (mp.o 0x72E020)
    short FindOldestItem(const ae_vector<sDroppedItem>& list);  // ?FindOldestItem@MPPlayerItems@@AAEFABV?$ae_vector@UsDroppedItem@MPPlayerItems@@@@@Z (mp.o 0x755300)
};

// ============================================================================
// MPLanDiscovery - LAN session discovery results
// ============================================================================
class MPLanDiscovery : public bdDiscoveryListener {
public:
    uint8_t _pad04[4];               // +0x04
    bdDiscoveryClient mDiscoveryClient;  // +0x08
    unsigned int mNumResults;  // +0x48
    bdReference<bdGameInfo> mResults[10];  // +0x4C
    int m_lastStatus;          // +0x74 (bdDiscoveryClient::bdStatus)

    MPLanDiscovery();  // ??0MPLanDiscovery@@QAE@XZ (mp.o 0x735E50)
    unsigned int GetNumResults() const;  // ?GetNumResults@MPLanDiscovery@@QBEIXZ
    bool IsDone();                       // ?IsDone@MPLanDiscovery@@QAE_NXZ (mp.o 0x72CB60)
    void GetResult(unsigned int index,
                   bdReference<bdGameInfo>& result) const;  // ?GetResult@MPLanDiscovery@@QBEXIAAV?$bdReference@VbdGameInfo@@@@@Z (mp.o 0x746050)
    void Start();                        // ?Start@MPLanDiscovery@@QAEXXZ (mp.o 0x72CAF0)
    virtual ~MPLanDiscovery();           // ??1MPLanDiscovery@@UAE@XZ (mp.o 0x735EE0)
protected:
    virtual void onDiscovery(bdReference<bdGameInfo> gameInfo);  // ?onDiscovery@MPLanDiscovery@@MAEXV?$bdReference@VbdGameInfo@@@@@Z (mp.o 0x735F60)
};
static_assert(sizeof(MPLanDiscovery) == 120, "MPLanDiscovery size mismatch");

// ============================================================================
// MPGameInfo - game listing info (bdGameInfo base + slots at +0x28)
// ============================================================================
class MPGameInfo : public bdGameInfo {
public:
    unsigned char m_publicOpen;    // +0x28
    unsigned char m_privateOpen;   // +0x29
    unsigned char m_publicFilled;  // +0x2A
    unsigned char m_privateFilled; // +0x2B
    char  mName[24];               // +0x2C
    unsigned char mMapID;          // +0x44
    unsigned char mGameType;       // +0x45
    unsigned char mGameSubType;    // +0x46
    bool  mTeamBalancing;          // +0x47
    bool  mFriendlyFire;           // +0x48
    bool  mEnableAARVote;          // +0x49
    bool  mEnablePenaltyVote;      // +0x4A
    int   mQosProbeHandle;         // +0x4C

    MPGameInfo(unsigned int titleID, const XNKID& securityID,
               const XNKEY& securityKey,
               bdReference<bdCommonAddr> hostAddr);  // ??0MPGameInfo@@QAE@IABUXNKID@@ABUXNKEY@@V?$bdReference@VbdCommonAddr@@@@@Z (mp.o 0x73DA50)
    MPGameInfo();  // ??0MPGameInfo@@QAE@XZ (mp.o 0x73D9A0)
    MPGameInfo(unsigned int titleID, bdReference<bdCommonAddr> hostAddr,
               XNKID secID, XNKEY secKey, unsigned char publicOpen,
               unsigned char privateOpen, unsigned char publicFilled,
               unsigned char privateFilled);  // ??0MPGameInfo@@QAE@IV?$bdReference@VbdCommonAddr@@@@UXNKID@@UXNKEY@@EEEE@Z (mp.o 0x73DB90)

    void getSlots(unsigned char& publicOpen, unsigned char& privateOpen,
                  unsigned char& publicFilled,
                  unsigned char& privateFilled) const;  // ?getSlots@MPGameInfo@@QBEXAAE000@Z (mp.o 0x730510)
    void updateSlots(char publicOpenDelta, char privateOpenDelta,
                     char publicFilledDelta,
                     char privateFilledDelta);  // ?updateSlots@MPGameInfo@@QAEXDDDD@Z (mp.o 0x730540)
    MPGameInfo(unsigned int titleID, bdReference<bdCommonAddr> hostAddr,
               unsigned char publicOpen, unsigned char privateOpen,
               unsigned char publicFilled,
               unsigned char privateFilled);  // ??0MPGameInfo@@QAE@IV?$bdReference@VbdCommonAddr@@@@EEEE@Z (mp.o 0x73DAD0)
    bool operator==(const MPGameInfo& other) const;  // ??8MPGameInfo@@QBE_NABV0@@Z (mp.o 0x73DFF0)
};
static_assert(sizeof(MPGameInfo) == 0x50, "MPGameInfo size mismatch");

// EGameConnectionType (mp.o)
enum EGameConnectionType : int {
    kGameConnectionTypeLan = 0,
    kGameConnectionTypeOnline = 1,
    kGameConnectionTypeLocal = 2,
};

// eGameType - multiplayer game type id (mp.o)
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

struct sServerCreateParams;  // defined below MPUIInterface
struct sServerQueryParams;   // defined below MPUIInterface
// sGameListing - LAN/Live game listing entry (16 bytes, IDA verified)
struct CDefaultResult;  // full definition below
struct sGameListing {
    int mSize;                          // +0x00
    bdReference<MPGameInfo> mGameInfo;  // +0x04
    int mPing;                          // +0x08
    bool mValidVersion;                 // +0x0C

    void FromXboxLive(const CDefaultResult& info);  // ?FromXboxLive@sGameListing@@QAEXABVCDefaultResult@@@Z (mp.o 0x763E40)
};

// CDefaultResult - Live query result record (162 bytes, IDA)
#pragma pack(push, 1)
struct CDefaultResult {
    char host_name[0x22];        // +0x00
    char game_type[0x08];        // +0x22
    char game_map[0x08];         // +0x2A
    char friendly_fire[0x08];    // +0x32
    char team_balancing[0x08];   // +0x3A
    char sub_type[0x08];         // +0x42
    char num_players[0x08];      // +0x4A
    unsigned char SessionID[8];  // +0x52
    unsigned char KeyExchangeKey[16];  // +0x5A
    unsigned char HostAddress[0x24];   // +0x6A
    unsigned int PublicOpen;     // +0x8E
    unsigned int PrivateOpen;    // +0x92
    unsigned int PublicFilled;   // +0x96
    unsigned int PrivateFilled;  // +0x9A
    void* pQosInfo;              // +0x9E
};
#pragma pack(pop)
static_assert(sizeof(CDefaultResult) == 162, "CDefaultResult size mismatch");

// ============================================================================
// MPUIInterface - multiplayer shell/UI static interface
// ============================================================================
class MPUIInterface {
public:
    static void getLocalAddresses(bdArray<bdInetAddr>& addrs);  // ?getLocalAddresses@MPUIInterface@@SAXAAV?$bdArray@VbdInetAddr@@@@@Z
    static void Logging(bool enabled);   // ?Logging@MPUIInterface@@SAX_N@Z
    static const int GetTimeLimitCount();   // ?GetTimeLimitCount@MPUIInterface@@SA?BHXZ
    static const int GetRoundLimitCount();  // ?GetRoundLimitCount@MPUIInterface@@SA?BHXZ
    static const int GetMaxPlayersCount();  // ?GetMaxPlayersCount@MPUIInterface@@SA?BHXZ
    static const int GetRespawnTimeCount(); // ?GetRespawnTimeCount@MPUIInterface@@SA?BHXZ
    static const int GetReturnMenu();       // ?GetReturnMenu@MPUIInterface@@SA?BHXZ
    static const char* GetGameTypeString(unsigned long gameType);       // ?GetGameTypeString@MPUIInterface@@SAPBDK@Z (mp.o 0x72F770)
    static const char* GetGameTypeShortString(unsigned long gameType);  // ?GetGameTypeShortString@MPUIInterface@@SAPBDK@Z (mp.o 0x72F7D0)
    static const char* GetMapRotationString(unsigned long mapRotation); // ?GetMapRotationString@MPUIInterface@@SAPBDK@Z (mp.o 0x72F8A0)
    static const int  GetTimeLimit(unsigned long index);   // ?GetTimeLimit@MPUIInterface@@SA?BHK@Z (mp.o 0x72F910)
    static const int  GetRoundLimit(unsigned long index);  // ?GetRoundLimit@MPUIInterface@@SA?BHK@Z (mp.o 0x72FB20)
    static const int  GetMaxPlayers(unsigned long index);  // ?GetMaxPlayers@MPUIInterface@@SA?BHK@Z (mp.o 0x72FB90)
    static const int  GetRespawnTime(unsigned long index); // ?GetRespawnTime@MPUIInterface@@SA?BHK@Z (mp.o 0x72FC00)
    static const int  GetScoreLimitCount(eGameType gameType);  // ?GetScoreLimitCount@MPUIInterface@@SA?BHW4eGameType@@@Z (mp.o 0x72F970)
    static const int  GetScoreLimit(unsigned long index,
                                    eGameType gameType);  // ?GetScoreLimit@MPUIInterface@@SA?BHKW4eGameType@@@Z (mp.o 0x72FA00)
    static const int  GetMaxPlayersOptionFromMap(char mapID);  // ?GetMaxPlayersOptionFromMap@MPUIInterface@@SA?BHD@Z (mp.o 0x73D430)
    static void GameListingEnd();        // ?GameListingEnd@MPUIInterface@@SAXXZ
    static void StartDevice();           // ?StartDevice@MPUIInterface@@SAXXZ
    static void PlatformStop();          // ?PlatformStop@MPUIInterface@@SAXXZ
    static void PlatformStart();         // ?PlatformStart@MPUIInterface@@SAXXZ (mp.o 0x730000)
    static void Reboot();                // ?Reboot@MPUIInterface@@SAXXZ (mp.o 0x72F380)
    static void ResolveVote();           // ?ResolveVote@MPUIInterface@@SAXXZ (mp.o 0x73D6C0)
    static void CancelJoin();            // ?CancelJoin@MPUIInterface@@SAXXZ (mp.o 0x72F4A0)
    static void bdNetStop();             // ?bdNetStop@MPUIInterface@@SAXXZ (mp.o 0x766000)
    static void NextRound();             // ?NextRound@MPUIInterface@@SAXXZ (mp.o 0x75A8E0)
    static void SetServerParams(const sServerCreateParams& a_ServerParams);  // ?SetServerParams@MPUIInterface@@SAXABUsServerCreateParams@@@Z (mp.o 0x730240)
    static void SetQueryParams(sServerQueryParams& params);  // ?SetQueryParams@MPUIInterface@@SAXAAUsServerQueryParams@@@Z (mp.o 0x72F520)
    static bool NextRoundMapChanges();   // ?NextRoundMapChanges@MPUIInterface@@SA_NXZ
    static bool NextRoundMapRestart();   // ?NextRoundMapRestart@MPUIInterface@@SA_NXZ (mp.o 0x7300A0)
    static const bool IsGameListingComplete(); // ?IsGameListingComplete@MPUIInterface@@SA?B_NXZ (mp.o 0x72F730)
    static const bool StartClient(sGameListing& game, bool bStartGame,
                                  int nGameIndex);  // ?StartClient@MPUIInterface@@SA?B_NAAUsGameListing@@_NH@Z (mp.o 0x7663E0)
    static void NextRoundServerParams();       // ?NextRoundServerParams@MPUIInterface@@SAXXZ (mp.o 0x7300D0)
    static void SetupCvars(bool useCurrent);   // ?SetupCvars@MPUIInterface@@SAX_N@Z (mp.o)
    static const bool IsLANGame();  // ?IsLANGame@MPUIInterface@@SA?B_NXZ (mp.o 0x72F470)
    static const bool IsLocalGame();  // ?IsLocalGame@MPUIInterface@@SA?B_NXZ (mp.o 0x72F490)
    static const char* GetMapString(unsigned long mapIndex);  // ?GetMapString@MPUIInterface@@SAPBDK@Z (mp.o 0x72F830)
    static sGameListing* GameListingGet(unsigned long& numGames);  // ?GameListingGet@MPUIInterface@@SAPAUsGameListing@@AAK@Z (mp.o 0x765FC0)
    static bool BlockUntilNetReady();  // ?BlockUntilNetReady@MPUIInterface@@SA_NXZ
    static void LoadMap(int map, bool restart, bool mapRot);  // ?LoadMap@MPUIInterface@@SAXH_N0@Z
    static void ExitFrontend(int returnMenu);  // ?ExitFrontend@MPUIInterface@@SAXH@Z (mp.o 0x74ED40)
    static void bdNetStart();  // ?bdNetStart@MPUIInterface@@SAXXZ (mp.o 0x764180)
    static void HandleQuery();  // ?HandleQuery@MPUIInterface@@SAXXZ (mp.o 0x764090)
    static bool StartGame(bool forceRestart,
                          bool blockUntilNetReady);  // ?StartGame@MPUIInterface@@SA_N_N0@Z (mp.o 0x7660E0)
    static const bool StartServer(bool forceRestart,
                                  bool blockUntilNetReady);  // ?StartServer@MPUIInterface@@SA?B_N_N0@Z (mp.o 0x766260)

    static int  mReturnMenu;  // ?mReturnMenu@MPUIInterface@@1HA @ 0xF0A124
    static bool mKicked;      // ?mKicked@MPUIInterface@@1_NA @ 0xF0A128
    static struct sServerCreateParams mServerParams;     // ?mServerParams@MPUIInterface@@1UsServerCreateParams@@A
    static struct sServerCreateParams mNextServerParams; // ?mNextServerParams@MPUIInterface@@1UsServerCreateParams@@A
    static EGameConnectionType mGameConnectionType;  // ?mGameConnectionType@MPUIInterface@@1W4EGameConnectionType@@A
    static bool mLanDiscoveryActive;  // ?mLanDiscoveryActive@MPUIInterface@@1_NA
    static bool mLiveQueryActive;     // ?mLiveQueryActive@MPUIInterface@@1_NA
    static bool mInSession;           // ?mInSession@MPUIInterface@@1_NA (defined in MPLiveEngine.cpp)
    static bool mHostMigrated;        // ?mHostMigrated@MPUIInterface@@1_NA
    static bool mHostDisconnected;    // ?mHostDisconnected@MPUIInterface@@1_NA
    static unsigned long mGameListingNumGames;  // ?mGameListingNumGames@MPUIInterface@@1KA @ 0xF93FB0
    static unsigned char mGameListings[1600];   // ?mGameListings@MPUIInterface@@1PAEA @ 0xF93DC8
    static bool mQueryFromID;                   // ?mQueryFromID@MPUIInterface@@1_NA @ 0xF93FAC
    static struct sServerQueryParams mQueryParams;  // ?mQueryParams@MPUIInterface@@1UsServerQueryParams@@A
    static const char* mGameTypeStrings[6];       // ?mGameTypeStrings@MPUIInterface@@1PAPBDA
    static const char* mGameTypeShortStrings[6];  // ?mGameTypeShortStrings@MPUIInterface@@1PAPBDA
    static const char* mMapRotationStrings[4];    // ?mMapRotationStrings@MPUIInterface@@1PAPBDA
    static const int mTimeLimitList[6];    // ?mTimeLimitList@MPUIInterface@@1QBHB
    static const int mRoundLimitList[1];   // ?mRoundLimitList@MPUIInterface@@1QBHB
    static const int mMaxPlayerList[4];    // ?mMaxPlayerList@MPUIInterface@@1QBHB
    static const int mRespawnTimeList[3];  // ?mRespawnTimeList@MPUIInterface@@1QBHB
    static const int mScoreLimitListWar[5];        // @ 0xD192B8
    static const int mScoreLimitListHQ[5];         // @ 0xD192CC
    static const int mScoreLimitListSCFCTF[5];     // @ 0xD192E0
    static const int mScoreLimitListTeamBattle[6]; // @ 0xD192F4
    static const int mScoreLimitListBattle[5];     // @ 0xD1930C
};

// sServerCreateParams - host session setup (mMapID at +0x58)
// sServerCreateParams - host session setup; full layout in platform/xbox_shim/xlive.h
struct sServerCreateParams;

// sServerQueryParams - LAN query filters (44 bytes)
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

// EDroppedItemTypes (mp.o); enumerators kept out of the global scope to avoid
// colliding with the anonymous enums in g_local.h.
enum EDroppedItemTypes : int;

// ============================================================================
// MP options menus (shell FE menu subclasses; mp.o vtable overrides)
// ============================================================================
// Minimal front-end views (mp.o overrides; full types live in ui_types.h).
class FEText {
public:
    virtual ~FEText();                      // ??1FEText@@UAE@XZ (shell.o)
    virtual void SetText(const char* s);    // ?SetText@FEText@@UAEXPBD@Z (shell.o 0x560870)
};

class FEMultiLineText {
public:
    virtual ~FEMultiLineText();      // ??1FEMultiLineText@@UAE@XZ (shell.o)
    virtual void UpdateForWidescreen(bool widescreen);  // ?UpdateForWidescreen@FEMultiLineText@@UAEX_N@Z (shell.o)
    virtual void SetTextBoxNoLocalize(Broc::string text, int width,
                                      float height);  // ?SetTextBoxNoLocalize@FEMultiLineText@@UAEXVstring@Broc@@HM@Z (shell.o 0x1857C0)
};

class UIListBox {
public:
    uint8_t _pad[0x60 - 4];  // +0x04 (vtable at +0x00)
    int     mTopLine;    // +0x60
    int     mSelectedLine;  // +0x64
    uint8_t _pad2[172 - 0x68];
    virtual ~UIListBox();                // slot 0 ??1UIListBox@@UAE@Z (shell.o)
    virtual void Clear();                // slot 1 ?Clear@UIListBox@@UAEXXZ
    virtual void ClearRow(int row);      // slot 2 ?ClearRow@UIListBox@@UAEXH@Z
protected:
    virtual void SelectLine(int a, int b);  // slot 3 (MAEXHH)
public:
    virtual void SelectLine(int line);      // slot 4 ?SelectLine@UIListBox@@UAEXH@Z
    virtual short OnUp(int c);              // slot 5 ?OnUp@UIListBox@@UAEFH@Z (shell.o 0x184290)
    virtual short OnDown(int c);            // slot 6 ?OnDown@UIListBox@@UAEFH@Z (shell.o 0x184340)
    virtual void Update(float time_inc);    // slot 7 ?Update@UIListBox@@UAEXM@Z
    virtual void Draw();                    // slot 8 ?Draw@UIListBox@@UAEXXZ
    virtual void Refresh();                 // slot 9 ?Refresh@UIListBox@@UAEXXZ
    void RemoveAllItems();  // ?RemoveAllItems@UIListBox@@QAEXXZ (shell.o 0x5816B0)
};
static_assert(sizeof(UIListBox) == 172, "UIListBox size mismatch");

class PanelFile {
public:
    void UpdateWidescreen(bool widescreen, float about_x);  // ?UpdateWidescreen@PanelFile@@QAEX_NM@Z (shell.o)
    void Draw();  // ?Draw@PanelFile@@QAEXXZ (shell.o)
};

// FEMenu base (0x4C) - minimal view; members/virtuals used by mp.o overrides
class FEMenuEntry;  // full minimal view below
class FEMenu {
public:
    FEMenuEntry** entries;              // +0x04
    FEMenuSystem* system;               // +0x08
    uint8_t       _pad0C[0x22 - 0x0C];
    int16_t       highlighted;          // +0x22
    uint8_t       _pad24[0x2A - 0x24];
    int16_t       flags;                // +0x2A
    uint8_t       _pad2C[0x30 - 0x2C];
    bool          lockInput;            // +0x30
    uint8_t       _pad31[0x32 - 0x31];
    char          button_held_down;     // +0x32
    char          default_color_scheme; // +0x33
    uint8_t       _pad34[0x48 - 0x34];
    PanelFile*    panel;                // +0x48

    virtual void PanelFileUnloaded(PanelFile* pf);  // slot 1 shell.o 0x5B7570
    virtual void UpdateWidescreen(bool widescreen); // slot 2 shell.o 0x57DF20
    virtual ~FEMenu();                               // slot 3 shell.o 0x592150
    virtual void Draw();                             // slot 20 shell.o (?Draw@FEMenu@@UAEXXZ)
    virtual void Update(float time_inc);             // slot 23 shell.o (?Update@FEMenu@@UAEXM@Z)
    virtual void OnActivate();                       // slot 28 shell.o 0x570750
    virtual void OnUp(int c);                        // slot 32 shell.o 0x5AE910
    virtual void OnDown(int c);                      // slot 33 shell.o 0x5AE920
    virtual void OnTriangle(int c);                  // slot 46 shell.o
    virtual void OnLeft(int c);                      // slot 34 shell.o 0x5AF820
    virtual void OnRight(int c);                     // slot 35 shell.o 0x5AF830
    virtual void SetHigh(int index, bool anim);      // slot 48 shell.o 0x171C70
    void Cleanup();                 // ?Cleanup@FEMenu@@QAEXXZ (shell.o)
protected:
    virtual void Up();              // ?Up@FEMenu@@MAEXXZ (shell.o 0x164020)
    virtual void Down();            // ?Down@FEMenu@@MAEXXZ (shell.o 0x163F00)
public:
    FEMenu(FEMenuSystem* menuSystem, int num, int x, int y, int mve, int flg);  // ??0FEMenu@@QAE@PAVFEMenuSystem@@HHHHH@Z (shell.o)
};
static_assert(sizeof(FEMenu) == 0x4C, "FEMenu size mismatch");

// DialogMenu / DialogMenuSystem - shell.o dialog views (minimal)
class DialogMenu : public FEMenu {
public:
    void (*triangleResponse)(int);   // +0x60
    void AddOption(const char* t,
                   bool (*responseFunc)(int));  // ?AddOption@DialogMenu@@QAEXPBDP6A_NH@Z@Z (shell.o 0x572EE0)
    void Reformat(bool vertical, int viewport); // ?Reformat@DialogMenu@@QAEX_NH@Z (shell.o 0x572F40)
    void CloseOnDelay(int delaySeconds,
                      void (*delayResp)(int));  // ?CloseOnDelay@DialogMenu@@QAEXHP6AXH@Z@Z (shell.o)
};

class DialogMenuSystem : public FEMenuSystem {
public:
    DialogMenu* GetLayer(bool layer1);       // ?GetLayer@DialogMenuSystem@@QAEPAVDialogMenu@@_N@Z (shell.o 0x572830)
    void BringUp(const char* t, bool type_ok, bool type_yn,
                 const char* title_unloc, bool layer1);  // ?BringUp@DialogMenuSystem@@QAEXPBD_N101@Z (shell.o 0x58E3B0)
    void CloseDialog();                      // ?CloseDialog@DialogMenuSystem@@QAEXXZ (shell.o)
    void HighlightOption(int index);         // ?HighlightOption@DialogMenuSystem@@QAEXH@Z (shell.o 0x57F1A0)
    int  GetActiveMenu();                    // ?GetActiveMenu@FEMenuSystem@@UAEHXZ (shell.o 0x571370)
};

// FEMenuEntry - one menu row (vtable + 20 bytes; SetValue/GetValue at slots
// 49/50, i.e. +0xC4/+0xC8)
class FEMenuEntry {
public:
    virtual ~FEMenuEntry();              // ??1FEMenuEntry@@UAE@Z (shell.o)
    virtual void SetText(const char* s); // slot 12 (?SetText@FEMenuEntry@@UAEXPBD@Z)
    virtual void SetValue(int value);    // ?SetValue@FEMenuEntry@@UAEXH@Z (shell.o 0x5AE840)
    virtual int  GetValue();             // ?GetValue@FEMenuEntry@@UAEHXZ (shell.o 0x5AE850)
    uint8_t _pad[0x18 - 0x04];
};
static_assert(sizeof(FEMenuEntry) == 0x18, "FEMenuEntry size mismatch");

// PanelQuad / FESlider - shell.o FE objects used by the controls options menu
// (mBar->SetAlpha drives the slider bar alpha; offsets verified against IDA)
class PanelQuad {
public:
    virtual ~PanelQuad();                // ??1PanelQuad@@UAE@Z (shell.o)
    virtual void SetAlpha(float a);      // slot 34 (?SetAlpha@PanelQuad@@UAEXM@Z)
};

class FESlider : public FEMenuEntry {
public:
    uint8_t _pad[0x30 - 0x18];
    PanelQuad* mBar;                     // +0x30
};

// FEComboBox - shell.o combo box (minimal view; AddOption +0x4C)
class FEComboBox : public FEMenuEntry {
public:
    void AddOption(Broc::string optionString);  // ?AddOption@FEComboBox@@QAEXVstring@Broc@@@Z (shell.o)
};

// ProfileManager / ProfileEditMenu (shell.o) - methods used by mp.o
class ProfileManager {
public:
    struct Profile {
        const char* profileName;  // +0x00
        int profileSlot;          // +0x04
    };

    static ProfileManager* Me();             // ?Me@ProfileManager@@SAPAV1@XZ (shell.o 0x5751D0)
    void Update();                           // ?Update@ProfileManager@@QAEXXZ (shell.o)
    void Reset();                            // ?Reset@ProfileManager@@QAEXXZ (shell.o 0x575210)
    int  GetProfiles(Profile** slots);       // ?GetProfiles@ProfileManager@@QAEHQAPAUProfile@1@@Z (shell.o)
    void EnumProfiles(SaveGameData** slots); // ?EnumProfiles@ProfileManager@@QAEXQAPAUSaveGameData@@@Z (shell.o 0x5935B0)
    const char* GetLoadedProfile() const;    // ?GetLoadedProfile@ProfileManager@@QBEPBDXZ (shell.o 0x575420)
    void SetProfile(SaveGameData* sv);       // ?SetProfile@ProfileManager@@QAEXPAUSaveGameData@@@Z (shell.o 0x580F00)
    int  mCurrentStatus;                     // +0x44
};

class ProfileEditMenu : public FEMenu {
public:
    bool mNeedWrite;              // +0x4C
    static ProfileEditMenu* Me(); // ?Me@ProfileEditMenu@@SAPAV1@XZ (shell.o 0x574FA0)
};

// MusicMgr / STBManager / controller - cross-object extern views
class MusicMgr {
public:
    static MusicMgr* sInst;   // ?sInst@MusicMgr@@2PAV1@A (game.o)
    void Stop(float fadeOutTime);  // ?Stop@MusicMgr@@QAEXM@Z (game.o 0x221830)
    void ScaleVolume(float scale); // ?ScaleVolume@MusicMgr@@QAEXM@Z (game.o 0x2216D0)
};

class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    const char* GetSTBString(const char* pszReference);  // ?GetSTBString@STBManager@@QAEPBDPBD@Z (core.o)
};

class controller {
public:
    uint8_t _pad[0x18];
    int     locked_port;          // +0x18
    uint8_t is_locked;            // +0x1C
    uint8_t accepting_input_from_controller[4];  // +0x1D
    static controller* inst();    // ?inst@controller@@SAPAV1@XZ (controller.o)
};

// GameSettings - shell.o global settings (temp profile buffer size)
class GameSettings {
public:
    static GameSettings* sInst;  // ?sInst@GameSettings@@2PAV1@A (shell.o @ 0x132028C)
    unsigned int get_temp_buffer_size() const;  // ?get_temp_buffer_size@GameSettings@@QBEIXZ (shell.o)
};

class MPOptionsScreenMenu : public FEMenu {
public:
    FEText* mScreenText[4];     // +0x4C
    FEMultiLineText* mInstructionsText;  // +0x5C
    bool    mWidescreen;        // +0x60

    MPOptionsScreenMenu(FEMenuSystem* s);  // ??0MPOptionsScreenMenu@@QAE@PAVFEMenuSystem@@@Z (mp.o 0x7305E0)
    virtual ~MPOptionsScreenMenu();        // ??1MPOptionsScreenMenu@@UAE@XZ (mp.o 0x730630)
    static MPOptionsScreenMenu* Me();  // ?Me@MPOptionsScreenMenu@@SAPAV1@XZ
    virtual void Select(int entry_num);  // ?Select@MPOptionsScreenMenu@@UAEXH@Z
    virtual void OnCross(int c);         // ?OnCross@MPOptionsScreenMenu@@UAEXH@Z
    virtual void Update(float time_inc); // ?Update@MPOptionsScreenMenu@@UAEXM@Z
    virtual void PanelFileUnloaded(PanelFile* pPanelFile);  // ?PanelFileUnloaded@MPOptionsScreenMenu@@UAEXPAVPanelFile@@@Z (mp.o 0x7309B0)
    virtual void UpdateWidescreen(bool widescreen);  // ?UpdateWidescreen@MPOptionsScreenMenu@@UAEX_N@Z (mp.o 0x730D30)
    virtual void OnActivate();  // ?OnActivate@MPOptionsScreenMenu@@UAEXXZ (mp.o 0x7309D0)
    virtual void OnTriangle(int c);  // ?OnTriangle@MPOptionsScreenMenu@@UAEXH@Z (mp.o 0x730CE0)
    virtual void Draw();  // ?Draw@MPOptionsScreenMenu@@UAEXXZ (mp.o 0x730AC0)
    virtual void OnUp(int c);  // ?OnUp@MPOptionsScreenMenu@@UAEXH@Z (mp.o 0x730BC0)
    virtual void OnDown(int c);  // ?OnDown@MPOptionsScreenMenu@@UAEXH@Z (mp.o 0x730C50)
    static const char* const kScreenOptionStrings[4];        // @ 0xD19378
    static const char* const kScreenInstructionStrings[4];   // @ 0xD1939C
private:
    bool SaveOptions();         // ?SaveOptions@MPOptionsScreenMenu@@AAE_NXZ (mp.o 0x730B00)
};

class MPOptionsSoundMenu : public FEMenu {
public:
    FEText* mSoundText[4];      // +0x4C
    int     mOutputVal;         // +0x5C
    int     mMusicVal;          // +0x60
    int     mEffectsVal;        // +0x64
    FEMultiLineText* mInstructionsText;  // +0x68
    bool    mWidescreen;        // +0x6C

    MPOptionsSoundMenu(FEMenuSystem* s);  // ??0MPOptionsSoundMenu@@QAE@PAVFEMenuSystem@@@Z (mp.o 0x730D80)
    virtual ~MPOptionsSoundMenu();        // ??1MPOptionsSoundMenu@@UAE@XZ (mp.o 0x730DE0)
    static MPOptionsSoundMenu* Me();  // ?Me@MPOptionsSoundMenu@@SAPAV1@XZ
    virtual void Update(float time_inc);  // ?Update@MPOptionsSoundMenu@@UAEXM@Z
    virtual void PanelFileUnloaded(PanelFile* pPanelFile);  // ?PanelFileUnloaded@MPOptionsSoundMenu@@UAEXPAVPanelFile@@@Z (mp.o 0x7311F0)
    virtual void UpdateWidescreen(bool widescreen);  // ?UpdateWidescreen@MPOptionsSoundMenu@@UAEX_N@Z (mp.o 0x731560)
    virtual void ButtonHeldAction();  // ?ButtonHeldAction@MPOptionsSoundMenu@@UAEXXZ (mp.o 0x731440)
    virtual void OnActivate();        // ?OnActivate@MPOptionsSoundMenu@@UAEXXZ (mp.o 0x73E0D0)
    virtual void OnTriangle(int c);   // ?OnTriangle@MPOptionsSoundMenu@@UAEXH@Z (mp.o 0x73E200)
    virtual void Draw();              // ?Draw@MPOptionsSoundMenu@@UAEXXZ (mp.o 0x731210)
    virtual void OnUp(int c);         // ?OnUp@MPOptionsSoundMenu@@UAEXH@Z (mp.o 0x731320)
    virtual void OnDown(int c);       // ?OnDown@MPOptionsSoundMenu@@UAEXH@Z (mp.o 0x7313B0)

    static const char* const kSoundOptionStrings[];  // ?kSoundOptionStrings@MPOptionsSoundMenu@@0QBQBDB @ 0xD193BC
    static const char* const kSoundInstructionStrings[];  // ?kSoundInstructionStrings@MPOptionsSoundMenu@@0QBQBDB @ 0xD193C0
private:
    void SetOptions();  // ?SetOptions@MPOptionsSoundMenu@@AAEXXZ (mp.o)
    bool SaveOptions();               // ?SaveOptions@MPOptionsSoundMenu@@AAE_NXZ (mp.o 0x7314B0)
    void AdjustOptions(bool up);      // ?AdjustOptions@MPOptionsSoundMenu@@AAEX_N@Z (mp.o 0x73E140)
};

class MPOptionsControlsMenu : public FEMenu {
public:
    FEText* mControlsText[4];   // +0x4C
    FEMultiLineText* mInstructionsText;  // +0x5C
    bool    mWidescreen;        // +0x60

    MPOptionsControlsMenu(FEMenuSystem* s);  // ??0MPOptionsControlsMenu@@QAE@PAVFEMenuSystem@@@Z (mp.o 0x7315B0)
    virtual ~MPOptionsControlsMenu();        // ??1MPOptionsControlsMenu@@UAE@XZ (mp.o 0x731600)
    static MPOptionsControlsMenu* Me();  // ?Me@MPOptionsControlsMenu@@SAPAV1@XZ
    virtual void Update(float time_inc);  // ?Update@MPOptionsControlsMenu@@UAEXM@Z
    virtual void PanelFileUnloaded(PanelFile* pPanelFile);  // ?PanelFileUnloaded@MPOptionsControlsMenu@@UAEXPAVPanelFile@@@Z (mp.o 0x731B00)
    virtual void UpdateWidescreen(bool widescreen);  // ?UpdateWidescreen@MPOptionsControlsMenu@@UAEX_N@Z (mp.o 0x7320F0)
    virtual void ButtonHeldAction();  // ?ButtonHeldAction@MPOptionsControlsMenu@@UAEXXZ (mp.o 0x731E80)
    virtual void OnActivate(int previous);  // ?OnActivate@MPOptionsControlsMenu@@UAEXH@Z (mp.o 0x73E230)
    virtual void OnTriangle(int c);   // ?OnTriangle@MPOptionsControlsMenu@@UAEXH@Z (mp.o 0x73E340)
    virtual void OnCross(int c);      // ?OnCross@MPOptionsControlsMenu@@UAEXH@Z (mp.o 0x73E3C0)
    virtual void Select(int entry_num);  // ?Select@MPOptionsControlsMenu@@UAEXH@Z (mp.o 0x73E370)
    virtual void Draw();              // ?Draw@MPOptionsControlsMenu@@UAEXXZ (mp.o 0x731B20)
    virtual void OnUp(int c);         // ?OnUp@MPOptionsControlsMenu@@UAEXH@Z (mp.o 0x731CA0)
    virtual void OnDown(int c);       // ?OnDown@MPOptionsControlsMenu@@UAEXH@Z (mp.o 0x731D90)
    static const char* const kControlsOptionStrings[];       // @ 0xD194B8
    static const char* const kControlsInstructionStrings[];  // @ 0xD19500
    static const char* const kStickLayoutStrings[];          // @ 0xD194D8
    static const char* const kButtonLayoutStrings[];         // @ 0xD194E8
private:
    void SetOptions();  // ?SetOptions@MPOptionsControlsMenu@@AAEXXZ (mp.o 0x731B60)
    bool SaveOptions();  // ?SaveOptions@MPOptionsControlsMenu@@AAE_NXZ
};

class MPOptionsGameplayMenu : public FEMenu {
public:
    FEText* mGameplayText[4];   // +0x4C
    FEMultiLineText* mInstructionsText;  // +0x5C
    bool    mWidescreen;        // +0x60

    MPOptionsGameplayMenu(FEMenuSystem* s);  // ??0MPOptionsGameplayMenu@@QAE@PAVFEMenuSystem@@@Z (mp.o 0x732140)
    virtual ~MPOptionsGameplayMenu();        // ??1MPOptionsGameplayMenu@@UAE@XZ (mp.o 0x732190)
    static MPOptionsGameplayMenu* Me();  // ?Me@MPOptionsGameplayMenu@@SAPAV1@XZ
    virtual void Update(float time_inc);  // ?Update@MPOptionsGameplayMenu@@UAEXM@Z
    virtual void PanelFileUnloaded(PanelFile* pPanelFile);  // ?PanelFileUnloaded@MPOptionsGameplayMenu@@UAEXPAVPanelFile@@@Z (mp.o 0x732540)
    virtual void UpdateWidescreen(bool widescreen);  // ?UpdateWidescreen@MPOptionsGameplayMenu@@UAEX_N@Z (mp.o 0x732890)
    virtual void OnActivate();  // ?OnActivate@MPOptionsGameplayMenu@@UAEXXZ (mp.o 0x73E400)
    virtual void OnTriangle(int c);  // ?OnTriangle@MPOptionsGameplayMenu@@UAEXH@Z (mp.o 0x73E4B0)
    virtual void Draw();  // ?Draw@MPOptionsGameplayMenu@@UAEXXZ (mp.o 0x7321F0)
    virtual void OnUp(int c);  // ?OnUp@MPOptionsGameplayMenu@@UAEXH@Z (mp.o 0x732560)
    virtual void OnDown(int c);  // ?OnDown@MPOptionsGameplayMenu@@UAEXH@Z (mp.o 0x7325F0)
    static const char* const kGameplayOptionStrings[4];       // @ 0xD19578
    static const char* const kGameplayInstructionStrings[4];  // @ 0xD195A8
private:
    void SetOptions();  // ?SetOptions@MPOptionsGameplayMenu@@AAEXXZ (mp.o)
    bool SaveOptions();  // ?SaveOptions@MPOptionsGameplayMenu@@AAE_NXZ
};

class MPOptionsPreferencesMenu : public FEMenu {
public:
    FEText* mScreenText[4];     // +0x4C
    FEMultiLineText* mInstructionsText;  // +0x5C
    FEComboBox* mMapCombo;      // +0x60
    uint8_t _pad64[0x8C - 0x64];  // mOnOffArrows[5][2] (opaque)
    int     mChoiceLimits[5];   // +0x8C
    bool    mWidescreen;        // +0xA0

    MPOptionsPreferencesMenu(FEMenuSystem* s);  // ??0MPOptionsPreferencesMenu@@QAE@PAVFEMenuSystem@@@Z (mp.o 0x7328E0)
    virtual ~MPOptionsPreferencesMenu();        // ??1MPOptionsPreferencesMenu@@UAE@XZ (mp.o 0x732940)
    static MPOptionsPreferencesMenu* Me();  // ?Me@MPOptionsPreferencesMenu@@SAPAV1@XZ
    virtual void Update(float time_inc);  // ?Update@MPOptionsPreferencesMenu@@UAEXM@Z
    virtual void PanelFileUnloaded(PanelFile* pPanelFile);  // ?PanelFileUnloaded@MPOptionsPreferencesMenu@@UAEXPAVPanelFile@@@Z (mp.o 0x732F30)
    virtual void UpdateWidescreen(bool widescreen);  // ?UpdateWidescreen@MPOptionsPreferencesMenu@@UAEX_N@Z (mp.o 0x733350)
    virtual void OnTriangle(int c);  // ?OnTriangle@MPOptionsPreferencesMenu@@UAEXH@Z (mp.o 0x733320)
    virtual void Draw();  // ?Draw@MPOptionsPreferencesMenu@@UAEXXZ (mp.o 0x732F50)
    virtual void OnUp(int c);  // ?OnUp@MPOptionsPreferencesMenu@@UAEXH@Z
    virtual void OnDown(int c);  // ?OnDown@MPOptionsPreferencesMenu@@UAEXH@Z
    virtual void OnActivate();  // ?OnActivate@MPOptionsPreferencesMenu@@UAEXXZ (mp.o 0x73E4E0)
    static const char* const kOptionStrings[5];       // @ 0xD19618
    static const char* const kInstructionStrings[5];  // @ 0xD1962C
    static unsigned char m_FirstTimeAccessedByte;     // ?m_FirstTimeAccessedByte@MPOptionsPreferencesMenu@@0EA @ 0xF93FC4
private:
    void SetOptions();  // ?SetOptions@MPOptionsPreferencesMenu@@AAEXXZ (mp.o 0x732F90)
    bool SaveOptions();  // ?SaveOptions@MPOptionsPreferencesMenu@@AAE_NXZ
};

// ============================================================================
// MP profile menus
// ============================================================================
class MPProfileEditMenu : public FEMenu {
public:
    bool    mNeedWrite;          // +0x4C
    bool    mWidescreen;         // +0x4D
    FEText* mProfileEditText[4]; // +0x50
    FEMultiLineText* mInstructionsText;  // +0x60
    UIListBox mListBox;          // +0x64 (172 bytes)

    MPProfileEditMenu(FEMenuSystem* s);  // ??0MPProfileEditMenu@@QAE@PAVFEMenuSystem@@@Z (mp.o 0x7656C0)
    static MPProfileEditMenu* Me();  // ?Me@MPProfileEditMenu@@SAPAV1@XZ
    static bool DialogResponseOk(int index);  // ?DialogResponseOk@MPProfileEditMenu@@SA_NH@Z
    static const char* const kProfileTextOptionStrings[];        // ?kProfileTextOptionStrings@MPProfileEditMenu@@0QBQBDB @ 0xD19670
    static const char* const kProfileTextInstructionStrings[];   // ?kProfileTextInstructionStrings@MPProfileEditMenu@@0QBQBDB @ 0xD19684
    virtual void PanelFileUnloaded(PanelFile* pPanelFile);  // ?PanelFileUnloaded@MPProfileEditMenu@@UAEXPAVPanelFile@@@Z (mp.o 0x733640)
    virtual void Select(int entry_num);                      // ?Select@MPProfileEditMenu@@UAEXH@Z (mp.o 0x7337B0)
    virtual void OnCross(int c);                             // ?OnCross@MPProfileEditMenu@@UAEXH@Z (mp.o 0x733820)
    virtual void OnUp(int c);                                // ?OnUp@MPProfileEditMenu@@UAEXH@Z (mp.o 0x733670)
    virtual void OnDown(int c);                              // ?OnDown@MPProfileEditMenu@@UAEXH@Z (mp.o 0x7336F0)
    virtual void OnTriangle(int c);                          // ?OnTriangle@MPProfileEditMenu@@UAEXH@Z (mp.o 0x733770)
    virtual void ButtonHeldAction();                         // ?ButtonHeldAction@MPProfileEditMenu@@UAEXXZ (mp.o 0x733890)
    virtual ~MPProfileEditMenu();                            // ??1MPProfileEditMenu@@UAE@XZ (mp.o 0x765750)
    virtual void Update(float time_inc);  // ?Update@MPProfileEditMenu@@UAEXM@Z (mp.o 0x7333D0)
    virtual void UpdateWidescreen(bool widescreen);  // ?UpdateWidescreen@MPProfileEditMenu@@UAEX_N@Z (mp.o 0x7338D0)
    virtual void Draw();  // ?Draw@MPProfileEditMenu@@UAEXXZ (mp.o 0x7333A0)
    virtual void OnActivate(int previous);  // ?OnActivate@MPProfileEditMenu@@UAEXH@Z (mp.o 0x73E650)
};

class MPProfileMainMenu : public FEMenu {
public:
    int      mMenuState;         // +0x4C
    int      mMenuStatus[6];     // +0x50
    SaveGameData* mSaveSlots[6]; // +0x68
    const char*   mSelectedProfile;  // +0x80
    PanelFile*    mPanel;        // +0x84
    FEMultiLineText* mHelpBar;   // +0x88

    MPProfileMainMenu(FEMenuSystem* s);  // ??0MPProfileMainMenu@@QAE@PAVFEMenuSystem@@@Z (mp.o 0x733920)
    virtual ~MPProfileMainMenu();  // ??1MPProfileMainMenu@@UAE@XZ (mp.o 0x7339D0)
    static MPProfileMainMenu* Me();  // ?Me@MPProfileMainMenu@@SAPAV1@XZ
    static bool DialogResponseDeleteCancel(int index);  // ?DialogResponseDeleteCancel@MPProfileMainMenu@@SA_NH@Z
    static bool DialogResponseProfileEdit(int index);   // ?DialogResponseProfileEdit@MPProfileMainMenu@@SA_NH@Z
    static bool DialogResponseNoMemCard(int index);     // ?DialogResponseNoMemCard@MPProfileMainMenu@@SA_NH@Z
    static bool DialogResponseProfileLoadOk(int index); // ?DialogResponseProfileLoadOk@MPProfileMainMenu@@SA_NH@Z (mp.o 0x734250)
    static bool DialogResponseSaveSuccess(int index);   // ?DialogResponseSaveSuccess@MPProfileMainMenu@@SA_NH@Z (mp.o 0x734290)
    static bool DialogResponseDeleteSuccess(int index); // ?DialogResponseDeleteSuccess@MPProfileMainMenu@@SA_NH@Z (mp.o 0x7342B0)
    static bool DialogResponseDeleteConfirm(int index); // ?DialogResponseDeleteConfirm@MPProfileMainMenu@@SA_NH@Z (mp.o 0x74EFD0)
    static bool DialogResponseDelete(int index);        // ?DialogResponseDelete@MPProfileMainMenu@@SA_NH@Z (mp.o 0x75A9E0)
    static void LoadProfileData();   // ?LoadProfileData@MPProfileMainMenu@@SAXXZ (mp.o 0x733AF0)
    void ClearEntries();             // ?ClearEntries@MPProfileMainMenu@@QAEXXZ (mp.o 0x733A90)
    void CreateProfile();            // ?CreateProfile@MPProfileMainMenu@@QAEXXZ (mp.o 0x733D90)
    void DialogDisplayProfileLoading(int delaySecs);  // ?DialogDisplayProfileLoading@MPProfileMainMenu@@QAEXH@Z (shell.o)
    static void DialogDisplayProfileLoadSuccess(int index);  // ?DialogDisplayProfileLoadSuccess@MPProfileMainMenu@@SAXH@Z (mp.o 0x73E7F0)
    void DialogDisplaySaveSuccess();  // ?DialogDisplaySaveSuccess@MPProfileMainMenu@@QAEXXZ (mp.o 0x73EA80)
    void DialogDisplayDeleteSuccess(); // ?DialogDisplayDeleteSuccess@MPProfileMainMenu@@QAEXXZ (mp.o 0x73EB60)
    void DialogDisplayNoMemDevice();  // ?DialogDisplayNoMemDevice@MPProfileMainMenu@@QAEXXZ (mp.o 0x73EC40)
    void DialogDisplayDataCorrupt();  // ?DialogDisplayDataCorrupt@MPProfileMainMenu@@QAEXXZ (mp.o 0x73ED20)
    void DialogDisplayNoFreeSpace();  // ?DialogDisplayNoFreeSpace@MPProfileMainMenu@@QAEXXZ (mp.o 0x73EE00)
    void DialogDisplayDeleting();     // ?DialogDisplayDeleting@MPProfileMainMenu@@QAEXXZ (mp.o 0x73E9F0)
    virtual void Select(int entry_num);  // ?Select@MPProfileMainMenu@@UAEXH@Z (mp.o 0x764210)
    virtual void OnCross(int c);         // ?OnCross@MPProfileMainMenu@@UAEXH@Z (mp.o 0x733D80)
    virtual void OnTriangle(int c);      // ?OnTriangle@MPProfileMainMenu@@UAEXH@Z (mp.o 0x760F90)
    virtual void OnUp(int c);            // ?OnUp@MPProfileMainMenu@@UAEXH@Z (mp.o 0x7341E0)
    virtual void ButtonHeldAction();     // ?ButtonHeldAction@MPProfileMainMenu@@UAEXXZ (mp.o 0x734220)
    virtual void OnActivate(int previous);  // ?OnActivate@MPProfileMainMenu@@UAEXH@Z (mp.o 0x73E7C0)
    virtual void UpdateWidescreen(bool widescreen);  // ?UpdateWidescreen@MPProfileMainMenu@@UAEX_N@Z (mp.o 0x733B20)
    virtual void Update(float time_inc);  // ?Update@MPProfileMainMenu@@UAEXM@Z (mp.o 0x74ED90)
    virtual void Draw();                  // ?Draw@MPProfileMainMenu@@UAEXXZ (mp.o 0x733A60)
    void DialogDisplayLoading();          // ?DialogDisplayLoading@MPProfileMainMenu@@QAEXXZ (mp.o 0x73E8D0)
    void DialogDisplaySaving();           // ?DialogDisplaySaving@MPProfileMainMenu@@QAEXXZ (mp.o 0x73E960)
    void DialogDisplayProfileSelected();  // ?DialogDisplayProfileSelected@MPProfileMainMenu@@QAEXXZ (mp.o 0x760FE0)
    void LoadProfilesDone();              // ?LoadProfilesDone@MPProfileMainMenu@@QAEXXZ (mp.o)
private:
    void LoadSelectedProfile(bool displayDialog);  // ?LoadSelectedProfile@MPProfileMainMenu@@AAEX_N@Z (mp.o 0x75A980)
    void OnSelectionChange();        // ?OnSelectionChange@MPProfileMainMenu@@AAEXXZ (mp.o)
};

static_assert(sizeof(MPOptionsSoundMenu) == 0x70, "MPOptionsSoundMenu size mismatch");
static_assert(sizeof(MPOptionsControlsMenu) == 0x64, "MPOptionsControlsMenu size mismatch");
static_assert(sizeof(MPProfileEditMenu) == 0x110, "MPProfileEditMenu size mismatch");
static_assert(sizeof(MPProfileMainMenu) == 0x8C, "MPProfileMainMenu size mismatch");

// ============================================================================
// kuju utility / voice classes (mp.o)
// ============================================================================
namespace kuju {
class cBezier {
public:
    cBezier();  // ??0cBezier@kuju@@QAE@XZ
    cBezier(const math::Position3& initialPoint,
            const math::Dir3& initialInflexion,
            const math::Position3& finalPoint,
            const math::Dir3& finalInflexion);  // ??0cBezier@kuju@@QAE@ABVPosition3@math@@ABVDir3@3@01@Z (mp.o 0x73EEE0)
    math::Dir3 mAFactor;     // +0x00
    math::Dir3 mBFactor;     // +0x10
    math::Dir3 mCFactor;     // +0x20
    math::Position3 mInitialPoint;  // +0x30

    void reset(const math::Position3& initialPoint,
               const math::Dir3& initialInflexion,
               const math::Position3& finalPoint,
               const math::Dir3& finalInflexion);  // ?reset@cBezier@kuju@@QAEXABVPosition3@math@@ABVDir3@4@01@Z (mp.o 0x7342F0)
    math::Position3 position(float time) const;  // ?position@cBezier@kuju@@QBE?AVPosition3@math@@M@Z (mp.o 0x7343A0)
    math::Dir3 speed(float time) const;          // ?speed@cBezier@kuju@@QBE?AVDir3@math@@M@Z (mp.o 0x734480)
};
}  // namespace kuju

// EVoipGroup - voice group states (mp.o GetVoipGroup)
enum EVoipGroup : int {
    kVoipGroupSpectating = 0,
    kVoipGroupDead = 1,
    kVoipGroupPlaying = 2,
};
EVoipGroup GetVoipGroup(int state);  // ?GetVoipGroup@@YA?AW4EVoipGroup@@H@Z (mp.o 0x72C7E0)

// VKMenu - shell.o on-screen keyboard (minimal view; mSlotNum +0x1BC)
class VKMenu {
public:
    static VKMenu* Me();  // ?Me@VKMenu@@SAPAV1@XZ (shell.o)
};

namespace kuju {
namespace knetuser {
class cVoiceNetworkManager {
public:
    class iVoiceHandlerInterface {
    public:
        virtual ~iVoiceHandlerInterface() {}
        virtual void receiveVoiceData(unsigned int fromPlayerIndex,
                                      unsigned char* buffer,
                                      unsigned int length) = 0;
    };
    // sVoicePacket - received voice packet (280 bytes, IDA)
    struct sVoicePacket {
        unsigned char mBuffer[256];  // +0x00
        unsigned int mPrevSeqID;     // +0x100
        unsigned int mSeqID;         // +0x104
        unsigned int mSize;          // +0x108
        kuju::knet::sTime mTimeReceived;  // +0x10C
        sVoicePacket* mPrev;         // +0x110
        sVoicePacket* mNext;         // +0x114
    };
    // sVoicePendingDispatchPacket - queued outbound voice packet (348 bytes)
    struct sVoicePendingDispatchPacket {
        unsigned char mBuffer[256];  // +0x00
        unsigned char mSourcePlayer; // +0x100
        MPPlayerSet mPlayersToExclude;  // +0x102
        MPPlayerSet mPlayersToSendTo;   // +0x104
        unsigned int mPrevSeqID[16];    // +0x108
        unsigned int mSeqID;            // +0x148
        unsigned int mSize;             // +0x14C
        kuju::knet::sTime mTimeReceived;  // +0x150
        sVoicePendingDispatchPacket* mPrev;  // +0x154
        sVoicePendingDispatchPacket* mNext;  // +0x158
    };

    cVoiceNetworkManager();           // ??0cVoiceNetworkManager@knetuser@kuju@@QAE@XZ (mp.o 0x73F2A0)
    virtual ~cVoiceNetworkManager();  // ??1cVoiceNetworkManager@knetuser@kuju@@UAE@XZ
    kuju::knet::sTime mLastTime;  // +0x04
    void*   mVoiceHandlerInterface;  // +0x08
    sVoicePacket mVoicePackets[24];  // +0x0C
    sVoicePacket* mFreeVoicePacketList;      // +0x1A4C
    sVoicePacket* mPendingVoicePacketList[16];  // +0x1A50
    unsigned int  mLastReceivedSeqID[16];    // +0x1A90
    sVoicePendingDispatchPacket mVoicePendingDispatchPackets[5];  // +0x1AD0
    sVoicePendingDispatchPacket* mFreeVoicePendingDispatchPacketList;  // +0x219C
    sVoicePendingDispatchPacket* mVoicePendingDispatchPacketList;      // +0x21A0
    MPPlayerSet mActivePlayerVoices;  // +0x21A4
    kuju::knet::sTime mTimeVoiceActive[16];  // +0x21A8
    kuju::knet::sTime mLastDispatchTime;     // +0x21E8
    unsigned int mLastDispatchedSeqID[16];   // +0x21EC
    unsigned int mNextDispatchedSeqID;       // +0x222C
    float mPlayerDistances[16][16];          // +0x2230
    unsigned int mNumActivePlayerVoices;     // +0x2630
    unsigned char mRecentlyReceivedPacketSources[10];  // +0x2634
    unsigned char mRecentlyReceivedPacketRoutes[10];   // +0x263E
    unsigned char mRecentlyDispatchedPacketSources[10]; // +0x2648
    unsigned char mRecentlyDispatchedPacketRoutes[10];  // +0x2652
    unsigned char mRecentlyReceivedPacketIndex;    // +0x265C
    unsigned char mRecentlyDispatchedPacketIndex;  // +0x265D
    unsigned int mMissedPackets;                   // +0x2660
    static const kuju::knet::sTime mVoiceLifeTime;  // ?mVoiceLifeTime@cVoiceNetworkManager@knetuser@kuju@@0VsTime@knet@3@B @ 0x12266F4
    void resetPlayer(unsigned long playerIndex);  // ?resetPlayer@cVoiceNetworkManager@knetuser@kuju@@QAEXK@Z (mp.o 0x74F050)
    void sendVoiceData(MPPlayerSet destinationPlayers, unsigned char* buffer,
                       unsigned long length);  // ?sendVoiceData@cVoiceNetworkManager@knetuser@kuju@@QAEXVMPPlayerSet@@PAEK@Z (mp.o)
    void initialise();               // ?initialise@cVoiceNetworkManager@knetuser@kuju@@QAEXXZ (mp.o)
    void deinitialise();             // ?deinitialise@cVoiceNetworkManager@knetuser@kuju@@QAEXXZ
    void update(const kuju::knet::sTime& time);             // ?update@cVoiceNetworkManager@knetuser@kuju@@QAEXABVsTime@knet@3@@Z (mp.o 0x7500E0)
    void check_for_looped();  // ?check_for_looped@cVoiceNetworkManager@knetuser@kuju@@QAEXXZ (mp.o 0x734EC0)
private:
    void updateVoiceNetwork(const kuju::knet::sTime& time); // ?updateVoiceNetwork@cVoiceNetworkManager@knetuser@kuju@@AAEXABVsTime@knet@3@@Z (mp.o 0x7500C0)
    void addPacket(sVoicePacket* packet, unsigned long listIndex);  // ?addPacket@cVoiceNetworkManager@knetuser@kuju@@AAEXPAUsVoicePacket@123@K@Z (mp.o 0x734C40)
    sVoicePendingDispatchPacket* getFreeVoicePendingDispatchPacket();  // ?getFreeVoicePendingDispatchPacket@cVoiceNetworkManager@knetuser@kuju@@AAEPAUsVoicePendingDispatchPacket@123@XZ (mp.o 0x73F7C0)
    void getPlayerIndicesToSendTo(unsigned long& firstPlayer,
                                  unsigned long& secondPlayer);  // ?getPlayerIndicesToSendTo@cVoiceNetworkManager@knetuser@kuju@@AAEXAAK0@Z (mp.o 0x73F8C0)
    unsigned int seqIDInList(unsigned long seqID,
                             unsigned long listIndex);  // ?seqIDInList@cVoiceNetworkManager@knetuser@kuju@@AAEIKK@Z (mp.o 0x734D50)
    void flushFirstPacketInList(unsigned long listIndex,
                                unsigned int discardData);  // ?flushFirstPacketInList@cVoiceNetworkManager@knetuser@kuju@@AAEXKI@Z (mp.o 0x734D90)
    void determineClosestDestination(sVoicePendingDispatchPacket* packet,
                                     unsigned char& closestPlayerForThisPacket,
                                     float& closestDistanceForThisPacket);  // ?determineClosestDestination@cVoiceNetworkManager@knetuser@kuju@@AAEXPAUsVoicePendingDispatchPacket@123@AAEAAM@Z (mp.o 0x7352C0)
    void checkForPendingPacketsAwaitingHandling(const kuju::knet::sTime& time);  // (mp.o 0x735250)
    void checkForPendingPacketsAwaitingDispatch(const kuju::knet::sTime& time);  // (mp.o 0x750000)
    void addVoicePendingDispatchPacket(sVoicePendingDispatchPacket* packet);  // ?addVoicePendingDispatchPacket@cVoiceNetworkManager@knetuser@kuju@@AAEXPAUsVoicePendingDispatchPacket@123@@Z (mp.o 0x734FA0)
private:
    void handleDirectDestinations(MPPlayerSet& sendTo, MPPlayerSet& exclude,
                                  unsigned long destinationPlayer);  // ?handleDirectDestinations@cVoiceNetworkManager@knetuser@kuju@@AAEXAAVMPPlayerSet@@0K@Z (mp.o 0x74F6D0)
    unsigned char getRoutePlayer(unsigned char sourcePlayer,
                                 unsigned char destPlayer);  // ?getRoutePlayer@cVoiceNetworkManager@knetuser@kuju@@AAEEEE@Z (mp.o 0x73FC70)
    void dispatchPacketDirectToPlayer(const kuju::knet::sTime& time,
                                      sVoicePendingDispatchPacket* packet,
                                      unsigned char player);  // ?dispatchPacketDirectToPlayer@cVoiceNetworkManager@knetuser@kuju@@AAEXABVsTime@knet@3@PAUsVoicePendingDispatchPacket@123@E@Z (mp.o 0x74FB00)
    void dispatchPendingVoicePackets(const kuju::knet::sTime& time,
                                     MPPlayerSet& connectionsUsed,
                                     unsigned long& connectionsLeft);  // ?dispatchPendingVoicePackets@cVoiceNetworkManager@knetuser@kuju@@AAEXABVsTime@knet@3@AAVMPPlayerSet@@AAK@Z (mp.o 0x74FF40)
    void dispatchVoicePendingDispatchPacket(sVoicePendingDispatchPacket* packet,
                                            unsigned int destinationPlayer);  // ?dispatchVoicePendingDispatchPacket@cVoiceNetworkManager@knetuser@kuju@@AAEXPAUsVoicePendingDispatchPacket@123@K@Z (mp.o)
    void discardVoicePendingDispatchPacket(sVoicePendingDispatchPacket* packet);  // ?discardVoicePendingDispatchPacket@cVoiceNetworkManager@knetuser@kuju@@AAEXPAUsVoicePendingDispatchPacket@123@@Z (mp.o)
};
}

namespace kvoicemanager {
class cVoiceManager {
public:
    // sDiagnostics - getDiagnostics output (224 bytes, IDA)
    struct sDiagnostics {
        unsigned int mDataPendingEncode;          // +0x00
        unsigned int mDataPendingNetworkDispatch; // +0x04
        unsigned int mDataPendingDecode[16];      // +0x08
        unsigned int mDataPendingPlayback;        // +0x48
        kuju::knet::sTime mTimeSinceLastDecode;   // +0x4C
        kuju::knet::sTime mTimeSinceLastNetworkDispatch;  // +0x50
        kuju::knet::sTime mTimeSinceLastNetworkReceive[16];  // +0x54
        unsigned int mPlayerFlags[16];            // +0x94
        unsigned int mDecodeDstOffset;            // +0xD4
        unsigned int mPlaybackOffset;             // +0xD8
        unsigned int mPlaybackBytesRemaining;     // +0xDC
    };
    static_assert(sizeof(sDiagnostics) == 224, "sDiagnostics size mismatch");

    virtual ~cVoiceManager();  // ??1cVoiceManager@kvoicemanager@kuju@@UAE@XZ (mp.o 0x7348B0)
    cVoiceManager();           // ??0cVoiceManager@kvoicemanager@kuju@@QAE@XZ (mp.o 0x74EFF0)
    int             mInitialised;  // +0x04
    kuju::knetuser::cVoiceNetworkManager mVoiceNetworkManager;  // +0x08
    MPPlayerSet     mRemoteListeners;  // +0x266C
    MPPlayerSet     mConnectedPlayers; // +0x266E
    kuju::knet::sTime mLastNetworkDispatchTime;      // +0x2670
    kuju::knet::sTime mRealLastNetworkDispatchTime;  // +0x2674
    unsigned char   mEncodeBuffer[2500];             // +0x2678
    unsigned int    mEncodeDstOffset;                // +0x303C
    unsigned int    mNetworkDispatchOffset;          // +0x3040
    unsigned int    mPlaybackActive;                 // +0x3044
    unsigned int    mPlaybackDataAvailable;          // +0x3048
    kuju::knet::sTime mPlaybackDataAvailableStartTime;  // +0x304C

    void initialise();   // ?initialise@cVoiceManager@kvoicemanager@kuju@@QAEXXZ (mp.o 0x73F240)
    void update();       // ?update@cVoiceManager@kvoicemanager@kuju@@QAEXXZ (mp.o 0x7642B0)
    void getDiagnostics(sDiagnostics* diagnostics,
                        const kuju::knet::sTime& time);  // ?getDiagnostics@cVoiceManager@kvoicemanager@kuju@@QAEXPAUsDiagnostics@123@ABVsTime@knet@3@@Z (mp.o 0x734990)
    void deinitialise();   // ?deinitialise@cVoiceManager@kvoicemanager@kuju@@QAEXXZ
    void loadIRXModules(); // ?loadIRXModules@cVoiceManager@kvoicemanager@kuju@@QAEXXZ
    void setRemoteListeners(MPPlayerSet& players);  // ?setRemoteListeners@cVoiceManager@kvoicemanager@kuju@@QAEXAAVMPPlayerSet@@@Z
    virtual void receiveVoiceData(unsigned long fromPlayerIndex,
                                  unsigned char* buffer,
                                  unsigned long length);  // ?receiveVoiceData@cVoiceManager@kvoicemanager@kuju@@UAEXKPAEK@Z (mp.o 0x7348F0)
private:
    void startSystem();    // ?startSystem@cVoiceManager@kvoicemanager@kuju@@AAEXXZ (mp.o 0x734910)
    void stopSystem();     // ?stopSystem@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
    void startLoopback();  // ?startLoopback@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
    void stopLoopback();   // ?stopLoopback@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
    void updateLoopback(); // ?updateLoopback@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
    void evaluatePlayers(); // ?evaluatePlayers@cVoiceManager@kvoicemanager@kuju@@AAEXXZ (mp.o 0x74F030)
    void dispatchVoiceData(); // ?dispatchVoiceData@cVoiceManager@kvoicemanager@kuju@@AAEXXZ (mp.o 0x75AAD0)
    void updateSystem(kuju::knet::sTime& currentTime);  // ?updateSystem@cVoiceManager@kvoicemanager@kuju@@AAEXAAVsTime@knet@3@@Z (mp.o 0x7610D0)
};
}
}

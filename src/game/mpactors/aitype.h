// ============================================================================
// aitype.h - AIType / CharacterType / AITypeManager (mp_actors.o)
// Sizes verified against IDA (codmp_xboxr.xbe.h local types).
// ============================================================================

#pragma once

#include "engine/broc_types.h"
#include "game/game_types.h"

// ============================================================================
// CharacterType - AI character archetype (0x18 bytes)
// ============================================================================
struct CharacterType {
    InplaceString mBodyName;        // +0x00
    InplaceString mVoiceName;       // +0x04
    InplaceString mGearName;        // +0x08
    InplaceString mHeadName;        // +0x0C
    InplaceString mHelmetName;      // +0x10
    InplaceString mPopedHelmetName; // +0x14
};
static_assert(sizeof(CharacterType) == 0x18,
              "CharacterType size mismatch");

// ============================================================================
// AIType - AI archetype (0x48 bytes) - IDA verified
// ============================================================================
class AIType {
public:
    enum AITypeTeam {
        AITYPE_TEAM_AXIS = 0,
        AITYPE_TEAM_ALLIES = 1,
        AITYPE_TEAM_NEUTRAL = 2,
    };

    InplaceString mName;               // +0x00
    AITypeTeam    mTeam;               // +0x04
    float         mAccuracyVsAI;       // +0x08
    float         mAccuracyVsPlayer;   // +0x0C
    int           mHealth;             // +0x10
    InplaceString mWeapon;             // +0x14
    InplaceString mSecondaryWeapon;    // +0x18
    InplaceString mGrenadeWeapon;      // +0x1C
    float         mScariness;          // +0x20
    float         mBravery;            // +0x24
    int           mGrenadeAmmo;        // +0x28
    InplaceVector<CharacterType> mCharacters;  // +0x2C (8 bytes)

    void Spawner(Entity* ent);   // ?Spawner@AIType@@QAEXPAVEntity@@@Z
    void InitPlayer(Entity* ent, TPakId pakId);  // ?InitPlayer@AIType@@QAEXPAVEntity@@W4TPakId@@@Z
    void InitEnt(Entity* ent);   // ?InitEnt@AIType@@QAEXPAVEntity@@@Z
    void InitActor(actor_s* actor);  // ?InitActor@AIType@@QAEXPAUactor_s@@@Z
};
static_assert(sizeof(AIType) == 0x34, "AIType size mismatch");

// ============================================================================
// AITypeManager - owns AITypeBank array (inherits AssetBankSet vtable)
// ============================================================================
class AITypeManager {
private:
    AITypeManager();             // ??0AITypeManager@@AAE@XZ
    virtual ~AITypeManager();    // ??1AITypeManager@@EAE@XZ
public:
    static AITypeManager* sInst;  // ?sInst@AITypeManager@@2PAV1@A (defined in pakmanager.cpp)
    static AITypeManager* Inst(); // ?Inst@AITypeManager@@SAPAV1@XZ (scr.o 0x5E9F80)
    static void* operator new(unsigned int size, void* p);
    static void CreateInst();      // ?CreateInst@AITypeManager@@SAXXZ
    static void DeleteInst();      // ?DeleteInst@AITypeManager@@SAXXZ
    IVPointer<AIType> GetAIType(TPakId pak_id, const char* name,
                                int nameOffset);  // ?GetAIType@AITypeManager@@QAE?AV?$IVPointer@VAIType@@@@W4TPakId@@PBDH@Z
    void*   mBankArray[99];      // +0x04 (ae_array<AITypeBank*,99>)
};
static_assert(sizeof(AITypeManager) == 0x190,
              "AITypeManager size mismatch");

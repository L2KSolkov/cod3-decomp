// ============================================================================
// g_checkpoint.cpp - game.o CheckpointMgr helpers (checkpointmgr.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <new>
#include <stdlib.h>
#include <string.h>

extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void  tlMemFree(void* ptr);
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file,
                                 int line);
extern void  mem_heap_free(void* ptr);

// PathNode / checkpoint support externs (mp_actors.o / streamer.o)
extern void Path_RelinquishNodePermanently(
    PathNodes::PathNode* pNode, sentient_s* pClaimer);
    // ?Path_RelinquishNodePermanently@@YAXPAUPathNode@PathNodes@@PAUsentient_s@@@Z
extern void PathNodeMgr_DissociateSentient(void* self,
                                           sentient_s* pSentient);
    // ?DissociateSentient@PathNodeMgr@@QAEXPAUsentient_s@@@Z
extern const PathNodes::PathNode* PathNodes_NodeHandle_deref(
    const PathNodes::NodeHandle* self);  // ??DNodeHandle (mp_actors.o)

struct world_t {
    uint8_t _pad[0x0C];
    char    baseName[128];   // +0x0C (verified vs RestoreSceneEntity disasm)
};
world_t s_worldDataLocal;     // unnamed 0x1364098 object; distinct from
                              // streamer's ?s_worldData@@3Uworld_t@@A @ 0xF74B98

// SceneBank persistent storage accessor (streamer.o; opaque layout)
class SceneEntity {
public:
    uint8_t _pad[0xCC];
    int16_t m_persistent_index;  // +0xCC (IDA SceneEntity type)
    uint8_t _tail[0x16];         // SceneEntity size 0xE4 (IDA type)
};
extern unsigned char* SceneBank_PersistentStorage(void* self,
                                                  unsigned int index);
    // InplaceVector<unsigned char>::operator[] (streamer.o)
extern TPakId CurPakId();  // ?CurPakId@@YA?AW4TPakId@@XZ
extern void ValidatePakId(TPakId pakId);  // ?ValidatePakId@@YAXW4TPakId@@@Z
// ?GetDestructible@DestructibleBankManager@@QAE?AV?$IVPointer@VDestructible@@@@W4TPakId@@PBD@Z
// (physics.o 0x705CD0; canonical member is in g_physics.cpp)
IVPointer<Destructible> DestructibleBankManager_GetDestructible(
    void* self, TPakId pak_id, const char* name)
{
    DestructibleBankManager* manager =
        self != nullptr ? static_cast<DestructibleBankManager*>(self)
                        : DestructibleBankManager::sInst;
    return manager->GetDestructible(pak_id, name);
}
extern void Destructible_CheckpointExplode(Destructible* self);
    // ?CheckpointExplode@Destructible@@QAEXXZ

// Minimal Destructible view (destructible.cpp; mFlags at +0x00 verified vs
// RestoreExplodedExploders disasm)
struct DestructibleView {
    struct Flags {
        unsigned int mMask;  // +0x00
    } mFlags;                // +0x00
};

// Checkpoint stub-save buffer (game.o .data @ 0xF317B0..0xF32ABC).
// Offsets verified against disasm of SaveCheckpoint (0x631560) /
// LoadCheckpointFromStubData (0x632120).
struct CheckpointStub {
    unsigned char  saveExists;            // +0x000 (byte_F317B0)
    uint8_t        _pad1[3];
    int            ammo[92];              // +0x004 (unk_F317B4, 0x170)
    int            ammoclip[92];          // +0x174 (unk_F31924, 0x170)
    int            weapons[2];            // +0x2E4 (dword_F31A94/A98)
    char           weaponslots[12];       // +0x2EC (dword_F31A9C/AA0, word_F31AA4)
    int            weaponrechamber[2];    // +0x2F8 (dword_F31AA8/AAC)
    int            weapon;                // +0x300 (dword_F31AB0)
    int            playerHealth;          // +0x304 (dword_F31AB4)
    float          playerOrientation[3];  // +0x308 (dword_F31AB8/ABC/AC0)
    float          origin[3];             // +0x314 (dword_F31AC4/AC8/ACC)
    uint8_t        _pad320[0x380];
    int            friendlyCount;         // +0x6A0 (dword_F31E50)
    uint8_t        _pad6A4[0x20];
    char           checkpointName[32];    // +0x6C4 (byte_F31E74)
    char           eventName[32];         // +0x6E4 (Destination)
    int            gameVarCount;          // +0x704 (dword_F31EB4)
    SCheckpointGameVar gameVars[256];     // +0x708 (iElement)
    int            explodedCount;         // +0xB08 (dword_F32AB8)
    int            exploded[256];         // +0xB0C (dword_F32ABC)
};
static_assert(sizeof(CheckpointStub) == 0x170C, "CheckpointStub size mismatch");
static CheckpointStub* sCheckpointStub = (CheckpointStub*)0xF317B0;
extern SaveGameData gSaveGameData[4];  // ?gSaveGameData@@3PAUSaveGameData@@A

// Broc runtime API (gpBrocAPI -> BrocAPI struct; mGetEnt at +0x94).
// g_local.h's minimal BrocAPI lacks mGetEnt, so call the entry via the
// exported function pointer address.
typedef unsigned int (__cdecl* BrocGetEntFn)(
    const Broc::string*, int, unsigned int*, int, int);
static BrocGetEntFn BrocAPI_mGetEnt()
{
    return *(BrocGetEntFn*)((char*)gpBrocAPI + 0x94);
}
extern Entity* GetPlayer(int idx);  // ?GetPlayer@@YAPAVEntity@@H@Z (g.o)

// ============================================================================
// CheckpointMenu - ea: 0x6392D0..0x639370 (checkpointmenu.cpp)
// ============================================================================
extern const PakInfoNode* PakManager_GetPakInfo(void* self, TPakId pakId);
    // ?GetPakInfo@PakManager@@QBEPBUPakInfoNode@@W4TPakId@@@Z
extern void Cvar_Set(const char* var_name, const char* value);  // core.o
extern int gCurCheckpoint;       // @ 0xF4F44C
extern int gDebounce;            // @ 0xF4F450

// Minimal controller view (mirrors g_cmd.cpp PadAliasMgr twin)
class controller { public:
public:
    enum ButtonIndex {
        LEFTBUTTON = 0,
        DOWNBUTTON = 1,
        RIGHTBUTTON = 2,
        UPBUTTON = 3,
        SQUARE = 4,
        X = 5,
        CIRCLE = 6,
        TRIANGLE = 7,
        R1 = 8,
        L1 = 9,
        R2 = 10,
        L2 = 11,
        R3 = 12,
        L3 = 13,
        START = 14,
        SELECT = 15,
    };
    static controller* inst();                     // controller_xbox.o
    int  button_value(int i_controller_num, ButtonIndex i_button);
    bool is_locked;          // +0x? (locked state)
    int  locked_port;        // +0x?
};

// ea: 0x006392D0
namespace CheckpointMenu {
// ?gCheckpointMenuActive@CheckpointMenu@@3_NA (game.o data @ 0xF4F454)
bool gCheckpointMenuActive = false;

void RestartAtCheckpoint(int num)
{
    if (Cvar_Get("letterbox_enabled", "0", 0)->integer != 0)
    {
        Com_Printf(
            "Wait until letterbox complete, before attempting to skip to a checkpoint.\n");
    }
    else
    {
        const PakInfoNode* PakInfo =
            PakManager::sInst->GetPakInfo(CurPakId());
        if (PakInfo != nullptr)
        {
            const char* name =
                PakInfo->checkPointNames.mList[(unsigned int)num].mStr;
            Com_Printf("Menu command to restart at checkpoint %s\n", name);
            Cvar_Set("checkpoint",
                     PakInfo->checkPointNames.mList[(unsigned int)num].mStr);
            CheckpointMgr::sInst->mGameVars.mSize = 0;
        }
    }
}
}  // namespace CheckpointMenu

// ============================================================================
// CheckpointMgr::SaveCheckpoint - ea: 0x631560 (checkpointmgr.cpp)
// ============================================================================
// ea: 0x00631560
void CheckpointMgr::SaveCheckpoint(const char* checkpointName,
                                   bool calledFromScript)
{
    if (Cvar_Get("letterbox_enabled", "0", 0)->integer != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
        AeAssert::gCurrentLine = 475;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error(
                " SAVE CHECKPOINT CANNOT OCCUR DURING LETTERBOX "))
            __debugbreak();
        return;
    }
    if (checkpointName == nullptr || *checkpointName == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
        AeAssert::gCurrentLine = 482;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && !AeAssert::Warning(
                "CHECKPOINT NAME SPECIFIED is either null or empty"))
            goto LABEL_101;
        __debugbreak();
        goto LABEL_101;
    }
    Broc::string::Block* mBlock = this->mEvent.mBlock;
    if (mBlock != nullptr && (Broc::string::Block*)mBlock + 1 != nullptr
        && *((char*)((Broc::string::Block*)mBlock + 1)) != 0
        && Broc::operator==(this->mEvent, checkpointName))
    {
        Com_Printf("Checkpoint <%s> already reached, just letting you know\n",
                   checkpointName);
        return;
    }
    Entity* scriptOrigin = nullptr;
    if (!calledFromScript)
    {
        Broc::string v42(checkpointName);
        unsigned int v7 = BrocAPI_mGetEnt()(
            &v42, HashString::CalcHash("targetname"), nullptr, 0, 0);
        v42.~string();
        scriptOrigin = (Entity*)EntityHandleDb::sInst.GetObject(v7);
        if (scriptOrigin == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
            AeAssert::gCurrentLine = 508;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && !AeAssert::Warning(
                    "NO RESTART SCRIPT_ORIGIN ATTACHED TO CHECK POINT"))
                goto LABEL_101;
            __debugbreak();
            goto LABEL_101;
        }
        if (scriptOrigin->mScriptNoteworthy.mBlock == nullptr
            || scriptOrigin->mScriptNoteworthy.is_empty())
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
            AeAssert::gCurrentLine = 518;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && !AeAssert::Warning(
                    "Invalid checkpoint name stored in script_noteworthy pair "
                    "on script origin"))
                goto LABEL_101;
            __debugbreak();
            goto LABEL_101;
        }
        if (Broc::operator==(this->mEvent,
                             scriptOrigin->mScriptNoteworthy))
        {
            const char* v10 =
                scriptOrigin->mScriptNoteworthy.mBlock != nullptr
                    ? (const char*)(
                          scriptOrigin->mScriptNoteworthy.mBlock + 1)
                    : defaultFileName;
            Com_Printf(
                "Checkpoint <%s> already reached, just letting you know\n",
                v10);
            return;
        }
        this->mEvent = scriptOrigin->mScriptNoteworthy;
        this->mCurrentMapName = s_worldDataLocal.baseName;
        this->mPlayerOrientation[0] = 0.0f;
        this->mPlayerOrientation[1] =
            scriptOrigin->r.currentAngles.v.m128_f32[1];
        this->mPlayerOrientation[2] = 0.0f;
        this->mOrigin.v.m128_f32[0] =
            scriptOrigin->r.currentOrigin.v.m128_f32[0];
        this->mOrigin.v.m128_f32[1] =
            scriptOrigin->r.currentOrigin.v.m128_f32[1];
        this->mOrigin.v.m128_f32[2] =
            scriptOrigin->r.currentOrigin.v.m128_f32[2];
        goto LABEL_31;
    }
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player != nullptr)
    {
        this->mCurrentMapName = s_worldDataLocal.baseName;
        this->mEvent = checkpointName;
        this->mPlayerOrientation[0] =
            Player->r.currentAngles.v.m128_f32[0];
        this->mPlayerOrientation[1] =
            Player->r.currentAngles.v.m128_f32[1];
        this->mPlayerOrientation[2] =
            Player->r.currentAngles.v.m128_f32[2];
        this->mOrigin.v.m128_f32[0] =
            Player->r.currentOrigin.v.m128_f32[0];
        this->mOrigin.v.m128_f32[1] =
            Player->r.currentOrigin.v.m128_f32[1];
        this->mOrigin.v.m128_f32[2] =
            Player->r.currentOrigin.v.m128_f32[2];
        goto LABEL_31;
    }
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
    AeAssert::gCurrentLine = 550;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && !AeAssert::Warning(
            "No player found while trying to save checkpoint"))
        goto LABEL_101;
    __debugbreak();
LABEL_101:
    Com_Printf("INVALID CHECKPOINT - PROGRESS NOT SAVED");
    return;

LABEL_31:
    Client* client = GetPlayer(currCl)->client;
    if (client->ps.stats[0] <= client->ps.stats[2]
        && client->ps.stats[0] != 0)
    {
        this->mPlayerHealth = client->ps.stats[0];
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
        AeAssert::gCurrentLine = 574;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                "CRS - CURRENT HEALTH GREATER THAN MAX HEALTH - CAPPING "
                "HEALTH AUTOMATICALLY, PLAYER PROBABLY HAS MAGIC BULLET "
                "SHIELD ON"))
            __debugbreak();
        this->mPlayerHealth = client->ps.stats[2];
    }
    Com_Printf("blah blah Checkpoint Reached.\n");
    if (this->mPlayerHealth == 0)
        this->mPlayerHealth = client->ps.stats[2];
    Client* v14 = GetPlayer(currCl)->client;
    memcpy(this->ammo, v14->ps.ammo, sizeof(this->ammo));
    memcpy(this->ammoclip, v14->ps.ammoclip, sizeof(this->ammoclip));
    memcpy(this->weaponslots, v14->ps.weaponslots,
           sizeof(this->weaponslots));
    this->weaponrechamber[0] = v14->ps.weaponrechamber[0];
    this->weaponrechamber[1] = v14->ps.weaponrechamber[1];
    if (BG_GetInfoForWeapon(v14->ps.weapon)->weapClass != WEAPCLASS_GRENADE)
    {
        this->weapon = v14->ps.weapon;
    }
    else if (BG_GetInfoForWeapon(v14->ps.lastWeapon)
                 ->weapClass != WEAPCLASS_GRENADE)
    {
        this->weapon = v14->ps.lastWeapon;
    }
    else
    {
        this->weapon = 0;
    }
    this->mCheckpointSaveExists = 1;
    ++this->mCheckpointIndex;
    InplaceVector<unsigned char>* mPersistantStorage =
        SceneManager::sInst->mPersistantStorage;
    if (mPersistantStorage != nullptr)
    {
        for (unsigned int i = 0; i < mPersistantStorage->mSize; ++i)
        {
            if ((*mPersistantStorage)[i] == 1)
                (*mPersistantStorage)[i] = 2;
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
        AeAssert::gCurrentLine = 623;
        AeAssert::gCurrentExpr = "persistantStorage";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("no persistant storage!"))
            __debugbreak();
    }
    memcpy(&this->mCheckpointScriptExploded, &this->mCurrentScriptExploded,
           sizeof(this->mCheckpointScriptExploded));
    this->mFriendlyCount = 0;
    Entity** itCur = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity** itEnd = itCur + EntityHandleDb::sInst.mActiveList.m_size;
    if (itCur != itEnd)
    {
        do
        {
            Entity* v22 = *itCur;
            if (v22 != nullptr && v22->s.eType == 11
                && G_GetActorFriendlyIndex(v22) >= 0)
            {
                Broc::string::Block* v23 = v22->targetname.mBlock;
                if (v23 != nullptr && v23->mLength != 0)
                {
                    int mFriendlyCount = this->mFriendlyCount;
                    if (mFriendlyCount >= 16)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\checkpointmgr.cpp";
                        AeAssert::gCurrentLine = 661;
                        AeAssert::gCurrentExpr = "0";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(
                                "Friendlies to save exceeded max count of "
                                "%d",
                                16))
                            __debugbreak();
                        break;
                    }
                    this->mFriendlies[mFriendlyCount].mOrigin[0] =
                        v22->r.currentOrigin.v.m128_f32[0];
                    this->mFriendlies[mFriendlyCount].mOrigin[1] =
                        v22->r.currentOrigin.v.m128_f32[1];
                    this->mFriendlies[mFriendlyCount].mOrigin[2] =
                        v22->r.currentOrigin.v.m128_f32[2];
                    this->mFriendlies[mFriendlyCount].mOrientation[0] =
                        v22->r.currentAngles.v.m128_f32[0];
                    this->mFriendlies[mFriendlyCount].mOrientation[1] =
                        v22->r.currentAngles.v.m128_f32[1];
                    this->mFriendlies[mFriendlyCount].mOrientation[2] =
                        v22->r.currentAngles.v.m128_f32[2];
                    const char* v26 = v22->targetname.mBlock != nullptr
                        ? (const char*)(v22->targetname.mBlock + 1)
                        : defaultFileName;
                    strcpy(this->mFriendlies[mFriendlyCount].mTargetname,
                           v26);
                    this->mFriendlyCount++;
                }
                else
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\checkpointmgr.cpp";
                    AeAssert::gCurrentLine = 654;
                    AeAssert::gCurrentExpr = nullptr;
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Warning(
                            "Friendly doesn't have a targetname, so his "
                            "position can't be saved.  Tell Stavro right "
                            "away!  He's very interested in this kind of "
                            "thing."))
                        __debugbreak();
                }
            }
            ++itCur;
        } while (itCur != itEnd);
    }
    sCheckpointStub->saveExists = 1;
    strncpy(sCheckpointStub->eventName, checkpointName, 31);
    strncpy(sCheckpointStub->checkpointName, s_worldDataLocal.baseName, 31);
    sCheckpointStub->weapon = this->weapon;
    memcpy(sCheckpointStub->ammo, v14->ps.ammo, 0x170);
    memcpy(sCheckpointStub->ammoclip, v14->ps.ammoclip, 0x170);
    sCheckpointStub->weapons[0] = v14->ps.weapons[0];
    sCheckpointStub->weapons[1] = v14->ps.weapons[1];
    memcpy(sCheckpointStub->weaponslots, v14->ps.weaponslots, 10);
    sCheckpointStub->weaponrechamber[0] = v14->ps.weaponrechamber[0];
    sCheckpointStub->weaponrechamber[1] = v14->ps.weaponrechamber[1];
    sCheckpointStub->playerHealth = this->mPlayerHealth;
    sCheckpointStub->origin[0] = this->mOrigin.v.m128_f32[0];
    sCheckpointStub->origin[1] = this->mOrigin.v.m128_f32[1];
    sCheckpointStub->origin[2] = this->mOrigin.v.m128_f32[2];
    sCheckpointStub->playerOrientation[0] = this->mPlayerOrientation[0];
    sCheckpointStub->playerOrientation[1] = this->mPlayerOrientation[1];
    sCheckpointStub->playerOrientation[2] = this->mPlayerOrientation[2];
    sCheckpointStub->friendlyCount = this->mFriendlyCount;
    sCheckpointStub->explodedCount = 0;
    int v34 = 0;
    unsigned short* pExploded = this->mCurrentScriptExploded.mElements;
    int nExploded = this->mCurrentScriptExploded.m_size;
    if (pExploded != pExploded + nExploded)
    {
        while (1)
        {
            if (v34 >= 256)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\checkpointmgr.cpp";
                AeAssert::gCurrentLine = 715;
                AeAssert::gCurrentExpr = nullptr;
                if (AeAssert::Error("Wankery!! Tell Stavro!"))
                    __debugbreak();
            }
            else
            {
                sCheckpointStub->exploded[sCheckpointStub->explodedCount] =
                    pExploded[0];
                ++sCheckpointStub->explodedCount;
            }
            ++pExploded;
            if (pExploded == pExploded + nExploded)
                break;
            v34 = sCheckpointStub->explodedCount;
        }
    }
    int v37 = 0;
    for (sCheckpointStub->gameVarCount = 0;
         v37 < this->mGameVars.mSize;
         sCheckpointStub->gameVarCount = v37)
    {
        if (v37 >= 256)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\checkpointmgr.cpp";
            AeAssert::gCurrentLine = 728;
            AeAssert::gCurrentExpr = nullptr;
            if (AeAssert::Error("Wankery!! Tell Stavro!"))
                __debugbreak();
        }
        else
        {
            if (v37 < 0 || v37 >= this->mGameVars.mSize)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
                AeAssert::gCurrentLine = 167;
                AeAssert::gCurrentExpr =
                    "iIndex >= 0 && iIndex < mSize";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
                v37 = sCheckpointStub->gameVarCount;
            }
            const SCheckpointGameVar* v39 =
                &this->mGameVars.mElements[v37];
            sCheckpointStub->gameVars[v37].mHashVarName =
                v39->mHashVarName;
            sCheckpointStub->gameVars[v37].mVal = v39->mVal;
            sCheckpointStub->gameVars[v37].mDataSize = v39->mDataSize;
        }
        v37 = sCheckpointStub->gameVarCount + 1;
    }
}

// ea: 0x00639370
namespace CheckpointMenu {
void RenderCheckpointMenu()
{
    const PakInfoNode* PakInfo =
        PakManager::sInst->GetPakInfo(CurPakId());
    if (PakInfo != nullptr || *(int*)0xD4 == 0)
    {
        if (gDebounce != 0)
            --gDebounce;
        unsigned int mSize = PakInfo->checkPointNames.mSize;
        unsigned int v5 = gCurCheckpoint < 0 ? 0 : gCurCheckpoint;
        gCurCheckpoint = (int)v5;
        if (v5 == mSize)
        {
            v5 = mSize - 1;
            gCurCheckpoint = (int)(mSize - 1);
        }
        const char* mStr =
            PakInfo->checkPointNames.mList[v5].mStr;
        char buf[256];
        buf[0] = 0;
        if (gCurCheckpoint > 0)
            sprintf(buf, "<- ");
        sprintf(buf, "%s %s ", buf, mStr);
        if (gCurCheckpoint < (int)PakInfo->checkPointNames.mSize - 1)
            sprintf(buf, "%s ->", buf);
        char col[16];
        strcpy(col, "fff?fff?fff?");
        ((unsigned char*)col)[4] = 0;
        *(unsigned short*)((char*)col + 4) = 16256;  // 0x3F80 (1.0f) hi
        DebugRender::RenderText(
            buf, 32, 32,
            Color(*(const float*)(col + 0), *(const float*)(col + 4),
                  *(const float*)(col + 8), *(const float*)(col + 12)),
            0.0f, 1.0f);
        int locked_port = 0;
        if (controller::inst()->is_locked)
            locked_port = controller::inst()->locked_port;
        if (gDebounce == 0)
        {
            if (controller::inst()->button_value(
                    locked_port, controller::SELECT) != 0)
            {
                if (controller::inst()->button_value(
                        locked_port, controller::RIGHTBUTTON) != 0)
                {
                    ++gCurCheckpoint;
                    gDebounce = 7;
                }
                else if (controller::inst()->button_value(
                             locked_port, controller::LEFTBUTTON) != 0)
                {
                    --gCurCheckpoint;
                    gDebounce = 7;
                }
                else if (controller::inst()->button_value(
                             locked_port, controller::UPBUTTON) != 0)
                {
                    gCheckpointMenuActive = false;
                    RestartAtCheckpoint(gCurCheckpoint);
                }
            }
            else
            {
                gCheckpointMenuActive = false;
            }
        }
    }
}
}  // namespace CheckpointMenu


template <typename T>
static void CheckpointVectorResize(CheckpointVector<T>* v, int iNewSize);

// ============================================================================
// CheckpointMgr ctor/dtor/ReInit - ea: 0x6220C0..0x632120
// ============================================================================

// ea: 0x006220C0
CheckpointMgr::CheckpointMgr()
{
    for (int i = 0; i < 6; ++i)
        new (&this->mWeapons[i]) Broc::string((Broc::string::Block*)nullptr);
    this->mCurrentScriptExploded.m_size = 0;
    for (int i = 0; i < 16; ++i)
    {
        this->mFriendlies[i].mOrientation[0] = 0.0f;
        this->mFriendlies[i].mOrientation[1] = 0.0f;
        this->mFriendlies[i].mOrientation[2] = 0.0f;
        this->mFriendlies[i].mOrigin[0] = 0.0f;
        this->mFriendlies[i].mOrigin[1] = 0.0f;
        this->mFriendlies[i].mOrigin[2] = 0.0f;
        this->mFriendlies[i].mTargetname[0] = 0;
    }
    new (&this->mEvent) Broc::string((Broc::string::Block*)nullptr);
    new (&this->mCurrentMapName) Broc::string((Broc::string::Block*)nullptr);
    this->mGameVars.mElements = nullptr;
    this->mGameVars.mCapacity = 0;
    this->mGameVars.mSize = 0;
    this->mCheckpointScriptExploded.m_size = 0;
    this->mCheckpointSaveExists = false;
    this->mUsingCheckpoints = false;
    this->mFriendlyCount = 0;
    this->mTimeRemainingForHudText = 0.0f;
    this->mCheckpointIndex = 0;
    this->mCurrentlySavingCheckpoint = false;
    this->mCheckpointFromStorage = false;
    Cvar_Set("checkpoint", "0");
    this->mPlayerHealth = 0;
    for (int i = 0; i < 6; ++i)
    {
        this->mWeaponAmmo[i] = 0;
        this->mWeaponClipAmmo[i] = 0;
    }
}

// ea: 0x006221F0
CheckpointMgr::~CheckpointMgr()
{
    if (this->mGameVars.mElements != nullptr)
    {
        tlMemFree(this->mGameVars.mElements);
        this->mGameVars.mElements = nullptr;
        this->mGameVars.mCapacity = 0;
    }
    this->mCurrentMapName.~string();
    this->mEvent.~string();
    for (int i = 0; i < 6; ++i)
        this->mWeapons[i].~string();
}

// ea: 0x004DDDB0
CheckpointMgr* CheckpointMgr::CreateInst()
{
    CheckpointMgr* result = nullptr;
    if (CheckpointMgr::sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CheckpointMgr.h";
        AeAssert::gCurrentLine = 36;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }
    result = static_cast<CheckpointMgr*>(
        mem_heap_malloc_ctx(0xB18u, 4, "core",
                            "c:\\cod\\code\\game\\CheckpointMgr.h", 36));
    if (result != nullptr)
    {
        result = new (result) CheckpointMgr();
        CheckpointMgr::sInst = result;
    }
    else
    {
        CheckpointMgr::sInst = nullptr;
    }
    return result;
}

// ea: 0x004E2AD0
void CheckpointMgr::DeleteInst()
{
    CheckpointMgr* instance = CheckpointMgr::sInst;
    if (instance == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CheckpointMgr.h";
        AeAssert::gCurrentLine = 36;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }
    if (instance != nullptr)
    {
        instance->~CheckpointMgr();
        mem_heap_free(instance);
    }
    CheckpointMgr::sInst = nullptr;
}

// ea: 0x00631490
void CheckpointMgr::ReInit()
{
    this->mCheckpointSaveExists = false;
    this->mUsingCheckpoints = false;
    this->mCheckpointFromStorage = false;
    this->mCurrentlySavingCheckpoint = false;
    this->mFriendlyCount = 0;
    this->mTimeRemainingForHudText = 0.0f;
    this->mCheckpointIndex = 0;
    this->mEvent.clear();
    this->mCurrentMapName.clear();
    Cvar_Set("checkpoint", "0");
    CheckpointVectorResize(&this->mGameVars, 0);
    this->mPlayerHealth = 0;
    this->weapon = 0;
    memset(this->ammo, 0, sizeof(this->ammo));
    memset(this->ammoclip, 0, sizeof(this->ammoclip));
    this->weapons[0] = 0;
    this->weapons[1] = 0;
    this->weaponslots[0] = 0;
    this->weaponslots[4] = 0;
    this->weaponslots[8] = 0;
    this->weaponrechamber[0] = 0;
    this->weaponrechamber[1] = 0;
    this->mCheckpointScriptExploded.m_size = 0;
    this->mCurrentScriptExploded.m_size = 0;
}

// ea: 0x00632110
void CheckpointMgr::ClearGameVars()
{
    CheckpointVectorResize(&this->mGameVars, 0);
}

// game.o save-state globals (persisted checkpoint storage)
unsigned char byte_F317B0;   // checkpoint save exists
unsigned char byte_F31E54;   // checkpoint from storage
int dword_F31AB4;            // player health
int dword_F31E50;            // friendly count
int dword_F32AB8;            // exploded exploder count
int dword_F31EB4;            // game var count

// ea: 0x00640E60
void CheckpointMgr::ClearSavedCheckpointData()
{
    this->ReInit();
    byte_F317B0 = 0;
    byte_F31E54 = 0;
    dword_F31AB4 = 0;
    dword_F31E50 = 0;
    dword_F32AB8 = 0;
    dword_F31EB4 = 0;
}

// ============================================================================
// CheckpointVector<SCheckpointGameVar> - ae_vector COMDATs (game.o inlines)
// Verified against IDA (construct_array 0x65E970, push_back 0x660AA0,
// erase 0x65D6D0, resize 0x661190, clear 0x661EC0)
// ============================================================================
template <typename T>
static T* CheckpointVectorConstruct(int iCapacity, int iSize)
{
    T* p = (T*)tlMemAlloc(iCapacity * sizeof(T), 8, 0);
    for (T* q = p; q != p + iSize; ++q)
        memset(q, 0, sizeof(T));
    return p;
}

template <typename T>
static T* CheckpointVectorConstruct(int iNumber)
{
    T* p = (T*)tlMemAlloc(iNumber * sizeof(T), 8, 0);
    for (T* i = p; i != p + iNumber; ++i)
        memset(i, 0, sizeof(T));
    return p;
}

template SCheckpointGameVar*
CheckpointVectorConstruct<SCheckpointGameVar>(int);

template <typename T>
static void CheckpointVectorResize(CheckpointVector<T>* v, int iNewSize)
{
    if (iNewSize > v->mCapacity)
    {
        T* newElements = CheckpointVectorConstruct<T>(iNewSize, 0);
        for (int i = 0; i < v->mSize; ++i)
            newElements[i] = v->mElements[i];
        if (v->mElements != nullptr)
        {
            tlMemFree(v->mElements);
            v->mElements = nullptr;
            v->mCapacity = 0;
        }
        v->mElements = newElements;
        v->mCapacity = iNewSize;
    }
    else if (iNewSize > v->mSize)
    {
        for (T* q = v->mElements + v->mSize; q != v->mElements + iNewSize;
             ++q)
            memset(q, 0, sizeof(T));
    }
    v->mSize = iNewSize;
}

template <typename T>
static void CheckpointVectorPushBack(CheckpointVector<T>* v, const T& iElement)
{
    if (v->mSize >= v->mCapacity)
    {
        int newCapacity = v->mSize + 4;
        if (v->mSize <= 3)
            newCapacity = v->mSize + 1;
        T* newElements = CheckpointVectorConstruct<T>(newCapacity, v->mSize + 1);
        for (int i = 0; i < v->mSize; ++i)
            newElements[i] = v->mElements[i];
        if (v->mElements != nullptr)
        {
            tlMemFree(v->mElements);
            v->mElements = nullptr;
            v->mCapacity = 0;
        }
        v->mElements = newElements;
        v->mCapacity = newCapacity;
    }
    v->mElements[v->mSize] = iElement;
    ++v->mSize;
}

template <typename T>
static void CheckpointVectorErase(CheckpointVector<T>* v, T* iBeginErase,
                                  T* iEndErase)
{
    if (v->mSize <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 286;
        AeAssert::gCurrentExpr = "mSize > 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Can't erase in empty vector"))
            __debugbreak();
    }
    T* end = v->mElements + v->mSize;
    T* dst = iBeginErase;
    if (iBeginErase != end)
    {
        T* src = iEndErase;
        while (src != end)
            *dst++ = *src++;
    }
    v->mSize -= (int)(iEndErase - iBeginErase);
}

// ea: 0x00609240
void CheckpointMgr::RestorePlayerHealth()
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player != nullptr && Player->client != nullptr)
    {
        if (this->mCheckpointSaveExists && this->mUsingCheckpoints)
        {
            int minHealth[3];
            minHealth[0] = (int)(0.75 * Player->client->ps.stats[2]);
            minHealth[1] = (int)(0.5 * Player->client->ps.stats[2]);
            minHealth[2] = (int)(0.25 * Player->client->ps.stats[2]);
            int v6 = minHealth[g_gameskill->integer];
            if (this->mPlayerHealth < v6)
            {
                Player->client->ps.stats[0] = v6;
                Player->health = v6;
                return;
            }
            Player->client->ps.stats[0] = this->mPlayerHealth;
            Player->health = this->mPlayerHealth;
        }
        else
        {
            Player->client->ps.stats[0] = Player->client->ps.stats[2];
            Player->health = Player->client->ps.stats[2];
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
        AeAssert::gCurrentLine = 157;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                "INVALID/NON_PLAYER ENTITY PASSED INTO RestorePlayerHealth"))
            __debugbreak();
    }
}

// ea: 0x00609330
bool CheckpointMgr::Restart()
{
    this->mTimeRemainingForHudText = 0.0f;
    return true;
}

// ea: 0x00609340
void CheckpointMgr::SetTosserValues()
{
}

// ea: 0x00609350
void CheckpointMgr::RestoreScriptExploders()
{
    if (this->mUsingCheckpoints && this->mCheckpointSaveExists)
        memcpy(&this->mCurrentScriptExploded, &this->mCheckpointScriptExploded,
               sizeof(this->mCurrentScriptExploded));
}

// ea: 0x00609380
void CheckpointMgr::SetCheckpointCvar()
{
    Broc::string::Block* mBlock = this->mEvent.mBlock;
    if (mBlock != nullptr)
        Cvar_Set("checkpoint", (const char*)&mBlock[1]);
    else
        Cvar_Set("checkpoint", defaultFileName);
}

// ea: 0x00618120
void CheckpointMgr::SetEvent(const char* checkpointName)
{
    this->mEvent = checkpointName;
}

// ea: 0x00618130
void CheckpointMgr::RestoreLastCheckpoint()
{
    if (this->mCheckpointSaveExists && this->mUsingCheckpoints)
    {
        Entity* Player = EntityManager::sInst->GetPlayer(currCl);
        memcpy(Player->client->ps.ammo, this->ammo, sizeof(Player->client->ps.ammo));
        memcpy(Player->client->ps.ammoclip, this->ammoclip,
               sizeof(Player->client->ps.ammoclip));
        Client* client = Player->client;
        client->ps.weapons[0] = this->weapons[0];
        client->ps.weapons[1] = this->weapons[1];
        memcpy(Player->client->ps.weaponslots, this->weaponslots,
               sizeof(this->weaponslots));
        Client* v5 = Player->client;
        v5->ps.weaponrechamber[0] = this->weaponrechamber[0];
        v5->ps.weaponrechamber[1] = this->weaponrechamber[1];
        int weapon = this->weapon;
        int v7 = 0;
        int v8 = 0;
        if (weapon == 0
            || ((1 << (this->weapon & 0x1F)) & this->weapons[weapon >> 5]) == 0)
        {
            if (this->weapons[0] != 0)
            {
                v7 = this->weapons[0];
            }
            else if (this->weapons[1] != 0)
            {
                v7 = this->weapons[1];
                v8 = 32;
            }
            else
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
                AeAssert::gCurrentLine = 793;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                        "Checkpoint couldn't restore weapon - Weapon index of 0...  Missing weapon about to occur"))
                    __debugbreak();
            }
            int v9 = 0;
            while (((1 << v9) & v7) == 0)
            {
                if (++v9 > 31)
                    goto LABEL_17;
            }
            this->weapon = v8 + v9;
        }
    LABEL_17:
        BG_SelectWeaponIndex(this->weapon, currCl);
        this->RestorePlayerHealth();
    }
}

// ea: 0x006182C0
bool CheckpointMgr::GetGameVar(unsigned int hashVarName, unsigned int* val,
                               unsigned int dataSize)
{
    if (dataSize == 0)
        return false;
    SCheckpointGameVar* mElements = this->mGameVars.mElements;
    SCheckpointGameVar* v6 = &mElements[this->mGameVars.mSize];
    if (mElements == v6)
        return false;
    while (mElements->mHashVarName == 0
           || mElements->mHashVarName != hashVarName)
    {
        if (++mElements == v6)
            return false;
    }
    *val = mElements->mVal;
    unsigned int v7 = 1;
    if (dataSize <= 1)
        return true;
    while (1)
    {
        unsigned int mHashVarName = mElements[1].mHashVarName;
        ++mElements;
        if (mHashVarName != 0)
            break;
        val[v7++] = mElements->mVal;
        if (v7 >= dataSize)
            return true;
    }
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
    AeAssert::gCurrentLine = 891;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning(
            "ATTEMPTED TO RETRIEVE GAME VARIABLE AS TYPE LARGER THAN IT WAS SAVED AS!!!"))
        __debugbreak();
    return false;
}

// ea: 0x00622290
void CheckpointMgr::SetGameVar(unsigned int hashVarName, unsigned int* val,
                               unsigned int dataSize)
{
    CheckpointVector<SCheckpointGameVar>* p_mGameVars = &this->mGameVars;
    int mSize = this->mGameVars.mSize;
    SCheckpointGameVar var;
    var.mVal = *val;
    var.mHashVarName = hashVarName;
    var.mDataSize = dataSize;
    SCheckpointGameVar* mElements = p_mGameVars->mElements;
    SCheckpointGameVar* v7 = &p_mGameVars->mElements[mSize];
    if (p_mGameVars->mElements != v7)
    {
        while (mElements->mHashVarName != hashVarName)
        {
            if (++mElements == v7)
                goto LABEL_6;
        }
        CheckpointVectorErase(p_mGameVars, mElements,
                              &mElements[mElements->mDataSize]);
    }
LABEL_6:
    CheckpointVectorPushBack(p_mGameVars, var);
    unsigned int v9 = 1;
    if (dataSize > 1)
    {
        var.mHashVarName = 0;
        var.mDataSize = 0;
        do
        {
            var.mVal = val[v9];
            CheckpointVectorPushBack(p_mGameVars, var);
            ++v9;
        } while (v9 < dataSize);
    }
}

// ea: 0x00622330
bool CheckpointMgr::ExploderCheckpointExploded(int exploderId)
{
    unsigned short* mElements = this->mCurrentScriptExploded.mElements;
    int mSize = this->mCurrentScriptExploded.m_size;
    if (mSize == 0)
        return false;
    for (int i = 0; i < mSize; ++i)
    {
        if (mElements[i] == exploderId)
            return true;
    }
    return false;
}

// ea: 0x00622370
bool CheckpointMgr::PrecludeExploderPiece(const char* exploderType,
                                          int exploderId)
{
    if (exploderType == nullptr || *exploderType == 0)
        return false;
    return this->ExploderCheckpointExploded(exploderId)
        && (strcmp(exploderType, "exploder_swap_out") == 0
            || strcmp(exploderType, "exploder_piece") == 0
            || strcmp(exploderType, "exploder_piece_visible") == 0);
}

// ============================================================================
// CheckpointMgr scene restore - ea: 0x609010..0x632400 (checkpointmgr.cpp)
// ============================================================================

// ea: 0x00609010
void CheckpointMgr::RestoreSceneEntity(Entity* pEnt)
{
    if (pEnt == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
        AeAssert::gCurrentLine = 105;
        AeAssert::gCurrentExpr = "pEnt";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "NULL ENTITY PASSED INTO CHECKPOINT MANAGER!!!"))
            __debugbreak();
    }
    if (!this->mCheckpointSaveExists)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
        AeAssert::gCurrentLine = 106;
        AeAssert::gCurrentExpr = "mCheckpointSaveExists";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "Trying to restore entity to checkpoint, when checkpoint has not been reached"))
            __debugbreak();
    }
    const char* mapName = this->mCurrentMapName.mBlock != nullptr
        ? (const char*)(this->mCurrentMapName.mBlock + 1)
        : defaultFileName;
    if (_stricmp(mapName, s_worldDataLocal.baseName) == 0)
    {
        Broc::string::Block* eventBlock = this->mEvent.mBlock;
        if (eventBlock != nullptr)
        {
            const char* evt = (const char*)(eventBlock + 1);
            if (evt != nullptr && *evt != 0)
            {
                if (pEnt->mClassName == "info_player_start")
                {
                    pEnt->r.currentOrigin.v.m128_f32[0] = this->mOrigin.v.m128_f32[0];
                    pEnt->r.currentOrigin.v.m128_f32[1] = this->mOrigin.v.m128_f32[1];
                    pEnt->r.currentOrigin.v.m128_f32[2] = this->mOrigin.v.m128_f32[2];
                    pEnt->r.currentAngles.v.m128_f32[0] = this->mPlayerOrientation[0];
                    pEnt->r.currentAngles.v.m128_f32[1] = this->mPlayerOrientation[1];
                    pEnt->r.currentAngles.v.m128_f32[2] = this->mPlayerOrientation[2];
                }
                int v8 = 0;
                if (this->mFriendlyCount > 0)
                {
                    const SEntitySaveInfo* pEnta = this->mFriendlies;
                    while (!(pEnt->targetname == pEnta->mTargetname))
                    {
                        ++v8;
                        ++pEnta;
                        if (v8 >= this->mFriendlyCount)
                            goto done;
                    }
                    pEnt->r.currentOrigin.v.m128_f32[0] = pEnta->mOrigin[0];
                    pEnt->r.currentOrigin.v.m128_f32[1] = pEnta->mOrigin[1];
                    pEnt->r.currentOrigin.v.m128_f32[2] = pEnta->mOrigin[2];
                    pEnt->r.currentAngles.v.m128_f32[0] = pEnta->mOrientation[0];
                    pEnt->r.currentAngles.v.m128_f32[1] = pEnta->mOrientation[1];
                    pEnt->r.currentAngles.v.m128_f32[2] = pEnta->mOrientation[2];
                }
            done:
                sentient_s* sentient = pEnt->sentient;
                if (sentient != nullptr)
                {
                    uint16_t mValue = sentient->mClaimedNode.mValue;
                    if (mValue != 0
                        && mValue != 0xFFFF
                        && PathNodes_NodeHandle_deref(
                               (const PathNodes::NodeHandle*)
                                   &sentient->mClaimedNode)
                            != nullptr)
                    {
                        Path_RelinquishNodePermanently(
                            (PathNodes::PathNode*)
                                PathNodes_NodeHandle_deref(
                                    (const PathNodes::NodeHandle*)
                                        &sentient->mClaimedNode),
                            sentient);
                        sentient->mClaimedNode.mValue = 0;
                    }
                    PathNodeMgr_DissociateSentient(PathNodeMgr::sInst,
                                                   sentient);
                }
            }
        }
    }
}

// ea: 0x006180F0
class SceneBank;
bool CheckpointMgr::SceneEntityWasDeletedBeforeCheckpoint(class SceneEntity* pSceneEnt, class SceneBank* pScnBank)
{
    return *SceneBank_PersistentStorage(
               pScnBank, pSceneEnt->m_persistent_index)
        < this->mCheckpointIndex;
}

// ea: 0x00632400
void CheckpointMgr::RestoreExplodedExploders()
{
    if (this->mCheckpointSaveExists && this->mUsingCheckpoints)
    {
        unsigned short* mElements = this->mCurrentScriptExploded.mElements;
        int mSize = this->mCurrentScriptExploded.m_size;
        for (int i = 0; i < mSize; ++i)
        {
            Broc::string exploderStr((int)mElements[i]);
            const char* name = exploderStr.mBlock != nullptr
                ? (const char*)(exploderStr.mBlock + 1)
                : defaultFileName;
            IVPointer<Destructible> d =
                DestructibleBankManager_GetDestructible(
                    DestructibleBankManager::sInst, CurPakId(), name);
            ValidatePakId((TPakId)d.mPakId);
            if (d.mValue != nullptr)
            {
                ValidatePakId((TPakId)d.mPakId);
                if (((DestructibleView*)d.mValue)->mFlags.mMask & 0x400000)
                {
                    ValidatePakId((TPakId)d.mPakId);
                    Destructible_CheckpointExplode(d.mValue);
                }
            }
            exploderStr.~string();
        }
    }
}

// ea: 0x00632120
void CheckpointMgr::LoadCheckpointFromStubData()
{
    this->ReInit();
    if (sCheckpointStub->saveExists != 0)
    {
        this->mCheckpointSaveExists = sCheckpointStub->saveExists != 0;
        this->weapon = sCheckpointStub->weapon;
        this->mEvent = sCheckpointStub->eventName;
        this->mCurrentMapName = sCheckpointStub->checkpointName;
        const char* mapName = this->mCurrentMapName.mBlock != nullptr
            ? (const char*)(this->mCurrentMapName.mBlock + 1)
            : defaultFileName;
        strcpy(sCheckpointStub->checkpointName, mapName);
        sCheckpointStub->weapon = this->weapon;
        memcpy(this->ammo, sCheckpointStub->ammo, sizeof(this->ammo));
        memcpy(this->ammoclip, sCheckpointStub->ammoclip,
               sizeof(this->ammoclip));
        this->weapons[0] = sCheckpointStub->weapons[0];
        this->weapons[1] = sCheckpointStub->weapons[1];
        memcpy(this->weaponslots, sCheckpointStub->weaponslots,
               sizeof(this->weaponslots));
        this->weaponrechamber[0] = sCheckpointStub->weaponrechamber[0];
        this->weaponrechamber[1] = sCheckpointStub->weaponrechamber[1];
        this->mPlayerHealth = sCheckpointStub->playerHealth;
        this->mOrigin.v.m128_f32[0] = sCheckpointStub->origin[0];
        this->mOrigin.v.m128_f32[1] = sCheckpointStub->origin[1];
        this->mOrigin.v.m128_f32[2] = sCheckpointStub->origin[2];
        this->mPlayerOrientation[0] = sCheckpointStub->playerOrientation[0];
        this->mPlayerOrientation[1] = sCheckpointStub->playerOrientation[1];
        this->mPlayerOrientation[2] = sCheckpointStub->playerOrientation[2];
        this->mFriendlyCount = 0;
        int i = 0;
        if (sCheckpointStub->friendlyCount > 0)
        {
            // Friendly block lives in gSaveGameData.savedState.Data at
            // gSaveGameData + 0x7E0, 0x38-byte stride: targetname +0x00,
            // orientation +0x20, origin +0x2C (verified vs disasm).
            unsigned char* src = (unsigned char*)gSaveGameData + 0x7E0;
            for (; i < sCheckpointStub->friendlyCount; ++i)
            {
                strcpy(this->mFriendlies[i].mTargetname, (const char*)src);
                this->mFriendlies[i].mOrientation[0] =
                    *(float*)(src + 0x20);
                this->mFriendlies[i].mOrientation[1] =
                    *(float*)(src + 0x24);
                this->mFriendlies[i].mOrientation[2] =
                    *(float*)(src + 0x28);
                this->mFriendlies[i].mOrigin[0] = *(float*)(src + 0x2C);
                this->mFriendlies[i].mOrigin[1] = *(float*)(src + 0x30);
                this->mFriendlies[i].mOrigin[2] = *(float*)(src + 0x34);
                src += 56;
                ++this->mFriendlyCount;
            }
        }
        CheckpointVectorResize(&this->mGameVars, 0);
        int v18 = 0;
        if (sCheckpointStub->gameVarCount > 0)
        {
            const SCheckpointGameVar* v19 =
                (const SCheckpointGameVar*)sCheckpointStub->gameVars;
            do
            {
                CheckpointVectorPushBack(&this->mGameVars, *v19);
                ++v18;
                ++v19;
            } while (v18 < sCheckpointStub->gameVarCount);
        }
        int v20 = 0;
        if (sCheckpointStub->explodedCount > 0)
        {
            do
            {
                unsigned short elt =
                    (unsigned short)sCheckpointStub->exploded[v20];
                this->mCheckpointScriptExploded.mElements
                    [this->mCheckpointScriptExploded.m_size] = elt;
                ++this->mCheckpointScriptExploded.m_size;
                this->mCurrentScriptExploded.mElements
                    [this->mCurrentScriptExploded.m_size] = elt;
                ++this->mCurrentScriptExploded.m_size;
                ++v20;
            } while (v20 < sCheckpointStub->explodedCount);
        }
    }
}

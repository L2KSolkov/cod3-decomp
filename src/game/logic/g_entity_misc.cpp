// ============================================================================
// g_entity_misc.cpp - game.o stat monitor + Entity DObj/enemy helpers
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// Stat monitor (StatMon_*.cpp)
// ============================================================================
struct statmonitor_s {
    int endtime;   // +0x00
    void* shader;  // +0x04 (nglTexture*)
};
static statmonitor_s stats[64];   // 0xF3C100 (game.o data)
static int statCount;             // 0xF3C300 (game.o data)
extern cvar_t* com_statmon;       // ?com_statmon@@3PAUcvar_t@@A (core.o)
extern int dword_F170E0;          // game.o data
extern void* GetTextureData(const char* name, int image_type,
                            const char* fromPak);  // ?GetTextureData (render.o)
extern int Sys_Milliseconds();    // ?Sys_Milliseconds@@YAHXZ

// ============================================================================
// StatMon_Warning - ea: 0x611C20
// ============================================================================
// ea: 0x00611C20
void StatMon_Warning(int type, int duration, const char* pszShaderName)
{
    if (com_statmon->integer != 0)
    {
        if (type >= 0x40)
            Com_Error(ERR_DROP, "StatMon_UpdateEntry: invalid entry '%i'",
                      type);
        stats[type].endtime = duration + Sys_Milliseconds();
        if (stats[type].shader == 0 && dword_F170E0 != 0)
            stats[type].shader =
                GetTextureData(pszShaderName, 0, "mp_frontEnd");
        if (type >= statCount)
            statCount = type + 1;
    }
}

// ============================================================================
// StatMon_GetStatsArray - ea: 0x611CA0
// ============================================================================
// ea: 0x00611CA0
void StatMon_GetStatsArray(const statmonitor_s** array, int* count)
{
    *array = stats;
    *count = statCount;
}

// ============================================================================
// StatMon_Reset - ea: 0x611CC0
// ============================================================================
// ea: 0x00611CC0
int StatMon_Reset()
{
    memset(stats, 0, sizeof(stats));
    statCount = 0;
    return 0;
}

// ============================================================================
// Entity::CreateDObj - ea: 0x611CE0
// ============================================================================
extern void register_dobj(DbLinkedHandle<EntityHandleDb, Entity> handle);  // ?register_dobj (g.o)
extern void DObjCreate(DObjModel* models, unsigned short numModels,
                       XAnimTree* tree, DObj* dobj,
                       unsigned short gameId);  // ?DObjCreate (render.o)
extern biped_phys_info* create_biped_phys_info(Entity* owner);  // physics.o
extern void destroy_biped_phys_info(biped_phys_info* bp_info);  // physics.o
extern void Entity_set_bp_info(Entity* self, biped_phys_info* bpInfo);  // ?set_bp_info@Entity@@QAEXPAVbiped_phys_info@@@Z (game.o)

void Entity::CreateDObj(DObjModel* dobjModels, unsigned short numModels,
                        XAnimTree* tree, unsigned short gameId)
{
    if (this->mDObj == nullptr)
    {
        void* v6 = DObj::operator new(0xE8u);
        if (v6 != nullptr)
            this->mDObj = new (v6) DObj(this->mPakId);
        else
            this->mDObj = nullptr;
        register_dobj(this->mHandle);
    }
    this->mDObj->mEntity = this;
    DObjCreate(dobjModels, numModels, tree, this->mDObj, gameId);
    if (this->client != nullptr && this->mBPInfo == nullptr)
    {
        biped_phys_info* bp = create_biped_phys_info(this);
        Entity_set_bp_info(this, bp);
    }
}

// ============================================================================
// Entity::FreeDObj - ea: 0x611DB0
// ============================================================================
extern void DObjFree(void* obj, int bClearTree);  // ?DObjFree (render.o)
extern void unregister_dobj(DbLinkedHandle<EntityHandleDb, Entity> handle);  // ?unregister_dobj (g.o)

void Entity::FreeDObj(bool deleteDObjs)
{
    DObj* mDObj = this->mDObj;
    if (mDObj != nullptr)
    {
        DObjFree(mDObj, 0);
        if (deleteDObjs)
        {
            if (this->mBPInfo != nullptr)
            {
                destroy_biped_phys_info(this->mBPInfo);
                this->mBPInfo = nullptr;
            }
            DObj* v4 = this->mDObj;
            if (v4 != nullptr)
            {
                this->mDObj->~DObj();
                DObj::operator delete(v4);
            }
            this->mDObj = nullptr;
            unregister_dobj(this->mHandle);
        }
    }
}

// ============================================================================
// Entity::IsEnemy - ea: 0x611E30
// ============================================================================
bool Entity::IsEnemy(Entity* ent)
{
    if (ent == nullptr || ent->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 618;
        AeAssert::gCurrentExpr = "ent && ent->sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    if (this->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 619;
        AeAssert::gCurrentExpr = "sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    team_t eTeam = (team_t)this->sentient->eTeam;
    return eTeam != TEAM_NEUTRAL
        && (team_t)ent->sentient->eTeam != TEAM_NEUTRAL
        && eTeam != (team_t)ent->sentient->eTeam;
}

// ============================================================================
// DecodeStub - ea: 0x611B60
// ============================================================================
// ea: 0x00611B60
void DecodeStub()
{
}

// ============================================================================
// CGBankManager::UnloadAll - ea: 0x611C00
// ============================================================================
void CGBankManager::UnloadAll()
{
    this->mCount = 0;
}

// ============================================================================
// render_brush - ea: 0x611C10
// ============================================================================
// ea: 0x00611C10
void render_brush()
{
}

// ============================================================================
// Entity helpers - ea: 0x611F00..0x639170
// ============================================================================
extern void AnglesToAxis(const math::Position3* angles,
                         const math::Position3* origin,
                         math::Mat43* mat);  // core.o (3-arg variant)
extern int g_DOBJF_NOT_RENDERED_LAST_FRAME;  // ?g_DOBJF_NOT_RENDERED_LAST_FRAME (core.o)

// ============================================================================
// Entity::CalcRotTranMat43 - ea: 0x611F40
// ============================================================================
// ea: 0x00611F40
const math::Mat43 Entity::CalcRotTranMat43()
{
    math::Mat43 result;
    AnglesToAxis(&this->r.currentAngles, &this->r.currentOrigin,
                 &this->r.currentMat);
    result = this->r.currentMat;
    return result;
}

// ============================================================================
// Entity::IsVisible - ea: 0x6122A0
// ============================================================================
// ea: 0x006122A0
int Entity::IsVisible() const
{
    DObj* mDObj = this->mDObj;
    return mDObj != nullptr
        && (g_DOBJF_NOT_RENDERED_LAST_FRAME & mDObj->mFlags) == 0;
}

// ============================================================================
// Entity::IsDoingPhysics - ea: 0x6122D0
// ============================================================================
// ea: 0x006122D0
bool Entity::IsDoingPhysics()
{
    bool result = false;
    if (this->client != nullptr)
    {
        biped_phys_info* mBPInfo = this->mBPInfo;
        if (mBPInfo != nullptr && mBPInfo->m_bp_sys != nullptr)
            return true;
    }
    return result;
}

// ============================================================================
// Entity::IsInRagdoll - ea: 0x612300
// ============================================================================
// ea: 0x00612300
bool Entity::IsInRagdoll()
{
    return (this->flags & 0x400000) != 0;
}

// ============================================================================
// Entity::IsLocalPlayer - ea: 0x612310
// ============================================================================
// ea: 0x00612310
bool Entity::IsLocalPlayer() const
{
    Client* client = this->client;
    return client != nullptr
        && (int)client->mServerClientIndex >= 0
        && *(int*)((char*)&svs.clients[client->mServerClientIndex].netchan[8])
            == 2;
}

// ============================================================================
// Entity::GetPlayerIndex - ea: 0x612340
// ============================================================================
// ea: 0x00612340
int Entity::GetPlayerIndex() const
{
    if (this->client == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 1041;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "Calling GetPlayerIndex on a non-player entity"))
            __debugbreak();
    }
    Client* client = this->client;
    if (client != nullptr)
        return client->mServerClientIndex;
    return -1;
}

// ============================================================================
// Entity::FreeAllDObjs - ea: 0x62A8A0
// ============================================================================
// ea: 0x0062A8A0
void Entity::FreeAllDObjs(bool deleteDObjs)
{
    Entity** p_mActiveList = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity** v2 = &EntityHandleDb::sInst.mActiveList.m_elements[
        EntityHandleDb::sInst.mActiveList.m_size];
    if (v2 != p_mActiveList)
    {
        do
        {
            if (*p_mActiveList != nullptr)
                (*p_mActiveList)->FreeDObj(deleteDObjs);
            ++p_mActiveList;
        } while (p_mActiveList != v2);
    }
}

// ============================================================================
// Entity::GetRenderEntity - ea: 0x62AF30
// ============================================================================
// ea: 0x0062AF30
trRefEntity& Entity::GetRenderEntity()
{
    if (this->mRenderEntity == nullptr)
    {
        trRefEntity* v2 = gRefEntFreeList.Alloc();
        this->mRenderEntity = v2 != nullptr
            ? new (v2) trRefEntity(0)
            : nullptr;
    }
    return *this->mRenderEntity;
}

// ============================================================================
// Entity::SetInSnapshot - ea: 0x639170
// ============================================================================
// ea: 0x00639170
void Entity::SetInSnapshot()
{
    trRefEntity& RenderEntity = GetRenderEntity();
    RenderEntity.SetInSnapshot();
}

// ============================================================================
// Entity::IsInSnapshot - ea: 0x612280
// ============================================================================
// ea: 0x00612280
bool Entity::IsInSnapshot() const
{
    trRefEntity* mRenderEntity = this->mRenderEntity;
    return mRenderEntity != nullptr && mRenderEntity->IsInSnapshot();
}

// ============================================================================
// Entity::IsCameraTweening - ea: 0x6123B0
// ============================================================================
// ea: 0x006123B0
bool Entity::IsCameraTweening() const
{
    unsigned int PlayerIndex = (unsigned int)GetPlayerIndex();
    if (PlayerIndex >= 2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 1052;
        AeAssert::gCurrentExpr = "index >= 0 && index <= 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "IsCameraTweening has bad player index"))
            __debugbreak();
    }
    return gCamera[PlayerIndex].IsTweening();
}

// ============================================================================
// GamePause - ea: 0x612680..0x612690
// ============================================================================
GamePause::GamePauseData GamePause::mData;

// ea: 0x00612680
GamePause::GamePauseData::GamePauseData()
{
    mGamePaused[0] = false;
}

// ea: 0x00612690
void GamePause::SetAllPaused(bool paused)
{
    GamePause::mData.mGamePaused[0] = paused;
}

// ============================================================================
// Entity::ExecScriptHandler - ea: 0x611F10
// ============================================================================
// ScriptEventHandler lives in g_game2_misc.cpp (game2.o port); params are
// ScriptEventParams* in the binary, void* in the tree's game2.o port.
struct ScriptEventHandler {
    unsigned char m_dlist_node[8];      // +0x00
    unsigned char mEvents[0x38];        // +0x08 (ScriptEvent mEvents[7])
    ScriptEventHandler* mNext;          // +0x40
    bool ExecEvents(Entity* ent, HashString h, void* params);  // game2.o 0x4F5A50
};

// ea: 0x00611F10
void Entity::ExecScriptHandler(HashString h, void* params)
{
    ScriptEventHandler* mScriptEventHandler = this->mScriptEventHandler;
    if (mScriptEventHandler != nullptr)
        mScriptEventHandler->ExecEvents(this, h, params);
}

// ============================================================================
// Entity::CalcOriginAnglesFromMat - ea: 0x611FE0
// ============================================================================
extern void Axis4ToAngles(const float (*axis)[4], float* angles);  // core.o

// ea: 0x00611FE0
void Entity::CalcOriginAnglesFromMat()
{
    Client* client = this->client;
    this->s.pos.trDelta[0] = 0.0f;
    this->s.pos.trDelta[1] = 0.0f;
    this->s.pos.trDelta[2] = 0.0f;
    this->s.apos.trDelta[0] = 0.0f;
    this->s.apos.trDelta[1] = 0.0f;
    this->s.apos.trDelta[2] = 0.0f;
    this->s.pos.trTime = 0;
    this->s.pos.trDuration = 0;
    this->s.apos.trTime = 0;
    this->s.apos.trDuration = 0;
    if (client != nullptr)
    {
        this->s.apos.trBase[0] = this->r.currentAngles.v.m128_f32[0];
        this->s.apos.trBase[1] = this->r.currentAngles.v.m128_f32[1];
        this->s.apos.trBase[2] = this->r.currentAngles.v.m128_f32[2];
    }
    float tmp = this->r.currentAngles.v.m128_f32[0];
    float v7 = this->r.currentAngles.v.m128_f32[1];
    float v8 = this->r.currentAngles.v.m128_f32[2];
    Axis4ToAngles((const float(*)[4])&this->r.currentMat, &tmp);
    this->r.currentAngles.v.m128_f32[0] = tmp;
    this->r.currentAngles.v.m128_f32[1] = v7;
    this->r.currentAngles.v.m128_f32[2] = v8;
    this->r.currentAngles.v.m128_f32[0] =
        AngleNormalize180(this->r.currentAngles.v.m128_f32[0]);
    this->r.currentOrigin.v.m128_f32[0] = this->r.currentMat.w.v.m128_f32[0];
    this->r.currentOrigin.v.m128_f32[1] = this->r.currentMat.w.v.m128_f32[1];
    float v3 = this->r.currentMat.w.v.m128_f32[3];
    this->r.currentOrigin.v.m128_f32[2] = this->r.currentMat.w.v.m128_f32[2];
    this->r.currentOrigin.v.m128_f32[3] = v3;
    this->s.pos.trBase[0] = this->r.currentOrigin.v.m128_f32[0];
    this->s.pos.trBase[1] = this->r.currentOrigin.v.m128_f32[1];
    Client* v4 = this->client;
    this->s.pos.trBase[2] = this->r.currentOrigin.v.m128_f32[2];
    float v5 = this->r.currentAngles.v.m128_f32[0];
    if (v4 != nullptr)
    {
        this->s.apos.trDelta[0] = v5;
        this->s.apos.trDelta[1] = this->r.currentAngles.v.m128_f32[1];
        this->s.apos.trDelta[2] = this->r.currentAngles.v.m128_f32[2];
    }
    else
    {
        this->s.apos.trBase[0] = v5;
        this->s.apos.trBase[1] = this->r.currentAngles.v.m128_f32[1];
        this->s.apos.trBase[2] = this->r.currentAngles.v.m128_f32[2];
    }
    this->s.pos.trType = TR_STATIONARY;
    this->s.apos.trType = TR_STATIONARY;
    if (v4 != nullptr)
    {
        v4->oldOrigin.v.m128_f32[0] = v4->ps.origin.v.m128_f32[0];
        this->client->oldOrigin.v.m128_f32[1] =
            this->client->ps.origin.v.m128_f32[1];
        this->client->oldOrigin.v.m128_f32[2] =
            this->client->ps.origin.v.m128_f32[2];
        this->client->ps.origin.v.m128_f32[0] =
            this->r.currentOrigin.v.m128_f32[0];
        this->client->ps.origin.v.m128_f32[1] =
            this->r.currentOrigin.v.m128_f32[1];
        this->client->ps.origin.v.m128_f32[2] =
            this->r.currentOrigin.v.m128_f32[2];
    }
    g_LinkEntity(this);
}

// ============================================================================
// Entity::GetParentBoneIndex - ea: 0x6121A0
// ============================================================================
// ea: 0x006121A0
int Entity::GetParentBoneIndex(int boneIndex)
{
    if (this->mDObj == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 750;
        AeAssert::gCurrentExpr = "mDObj";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(""))
            __debugbreak();
    }
    return this->mDObj->GetBoneParent(boneIndex);
}

// ============================================================================
// Entity::GetBaseRelMat - ea: 0x612210
// ============================================================================
// ea: 0x00612210
const math::Mat43::Packed& Entity::GetBaseRelMat(int boneIndex)
{
    if (this->mDObj == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 757;
        AeAssert::gCurrentExpr = "mDObj";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(""))
            __debugbreak();
    }
    return this->mDObj->GetBaseRelMat(boneIndex);
}

// ============================================================================
// MusicMgr - ea: 0x612E30
// ============================================================================
struct MusicMgr {
    Handle mMusic;           // +0x00
    Handle mMusicIndoor;     // +0x04
    float  mVolScale;        // +0x08
    float  mOutsideScale;    // +0x0C
    float  mIndoorScale;     // +0x10
    float  mIndoorFadeTime;  // +0x14
    float  mDelayCount;      // +0x18
    int    mCrossFadeType;   // +0x1C
    MusicMgr();              // ??0MusicMgr@@QAE@XZ
};

// ea: 0x00612E30
MusicMgr::MusicMgr()
{
    this->mMusic.mVal = 0;
    this->mMusicIndoor.mVal = 0;
    this->mVolScale = 1.0f;
    this->mOutsideScale = 1.0f;
    this->mIndoorScale = 1.0f;
    this->mIndoorFadeTime = 1.0f;
    this->mDelayCount = 0.0f;
    this->mCrossFadeType = 0;
}

// ============================================================================
// CGBankManager::~CGBankManager - ea: 0x611B70
// ============================================================================
extern void Cmd_RemoveCommand(const char* cmd_name);  // game.o g_cmd.cpp

// ea: 0x00611B70
CGBankManager::~CGBankManager()
{
    Cmd_RemoveCommand("cg");
    Cmd_RemoveCommand("cggraph");
    Cmd_RemoveCommand("cgperf");
    Cmd_RemoveCommand("cgzoomin");
    Cmd_RemoveCommand("cgzoomout");
    Cmd_RemoveCommand("teleport");
}

// ============================================================================
// AnimNotifyTask::Find - ea: 0x612790
// ============================================================================
class AnimNotifyTask {
    static ae_vector<unsigned int> mKeys;  // ?mKeys@AnimNotifyTask@@0V?$ae_vector@I@@A
    static int Find(unsigned int key);     // ?Find@AnimNotifyTask@@CAHI@Z
};
ae_vector<unsigned int> AnimNotifyTask::mKeys;

// ea: 0x00612790
int AnimNotifyTask::Find(unsigned int key)
{
    unsigned int* mElements = AnimNotifyTask::mKeys.mElements;
    unsigned int* v2 =
        &AnimNotifyTask::mKeys.mElements[AnimNotifyTask::mKeys.mSize];
    int result = 0;
    if (AnimNotifyTask::mKeys.mElements == v2)
        return -1;
    while (*mElements != key)
    {
        ++mElements;
        ++result;
        if (mElements == v2)
            return -1;
    }
    return result;
}

// ============================================================================
// AudioBankMgr - ea: 0x6127D0..0x612980
// ============================================================================
typedef int nflFileID;  // filesystem/nfl.cpp / core_systems.h use int
extern nslWaveID nslGetWave(const char* name);   // ?nslGetWave (nsl)
extern void nslFreeBank(nslBankID bankID);       // ?nslFreeBank (nsl)
extern void nflCloseFile(nflFileID file);        // filesystem/nfl.cpp

class AudioBankMgr {
public:
    enum eState {
        kUnloaded = 0,
        kLoading = 1,     // verified vs disasm IsFinished
        kLoaded = 2,
        kUnloading = 3,   // verified vs disasm IsFinished
    };
    struct WbkEntry {
        uint8_t   _pad0[0x2C];      // +0x00
        int       state[6];         // +0x2C
        nflFileID fileID[6];        // +0x44
        nslBankID bankId[6];        // +0x5C (bankId[5] aliases next entry +0x04)
    };
    static_assert(sizeof(WbkEntry) == 0x74, "WbkEntry view size mismatch");
    uint8_t  mAvailableWbks[0x6C0];    // +0x00 (16 * 0x6C stride)
    uint8_t  _pad6C0[0x6C8 - 0x6C0];
    int      m_size;                   // +0x6C8
    static AudioBankMgr* sInst;        // ?sInst@AudioBankMgr@@2PAV1@A
    virtual ~AudioBankMgr();           // ??1AudioBankMgr@@UAE@XZ
    bool IsFinished() const;           // ?IsFinished@AudioBankMgr@@QBE_NXZ
};
AudioBankMgr* AudioBankMgr::sInst = nullptr;

// ea: 0x006127D0
AudioBankMgr::~AudioBankMgr()
{
    int i = 0;
    if (this->m_size > 0)
    {
        unsigned int v2 = 0;
        for (unsigned int j = 0;; v2 = j)
        {
            if (v2 >= 0x6C0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 154;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            WbkEntry* entry =
                (WbkEntry*)((char*)this->mAvailableWbks + v2);
            for (int k = 0; k < 6; ++k)
            {
                if (entry->bankId[k] != NSL_BANK_ID_INVALID)
                {
                    nslFreeBank(entry->bankId[k]);
                    nflCloseFile(entry->fileID[k]);
                    entry->bankId[k] = NSL_BANK_ID_INVALID;
                    entry->fileID[k] = (nflFileID)-1;
                }
            }
            bool v5 = ++i < this->m_size;
            j += 108;
            if (!v5)
                break;
        }
    }
}

// ea: 0x006128E0
bool AudioBankMgr::IsFinished() const
{
    int v2 = 0;
    if (this->m_size > 0)
    {
        unsigned int v3 = 0;
        while (1)
        {
            if (v3 >= 0x6C0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 148;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            const WbkEntry* entry =
                (const WbkEntry*)((char*)this->mAvailableWbks + v3);
            int v4 = 0;
            do
            {
                if (entry->state[v4] == kLoading
                    || entry->state[v4] == kUnloading)
                    return false;
                ++v4;
            } while (v4 < 6);
            ++v2;
            v3 += 108;
            if (v2 >= this->m_size)
                break;
        }
    }
    return true;
}

// ============================================================================
// SoundDevice::FindWave - ea: 0x612980
// ============================================================================
// ea: 0x00612980
nslWaveID SoundDevice::FindWave(char* name)
{
    nslWaveID Wave = nslGetWave(name);
    if (AudioBankMgr::sInst->m_size > 0 && Wave == NSL_WAVE_ID_INVALID)
        strncmp(name, "loading_", 8u);
    return Wave;
}

// ============================================================================
// GetSurfaceTypeSounds - ea: 0x612DB0
// ============================================================================
extern const char* Com_SurfaceTypeToName(int iTypeIndex);  // core.o common.cpp

// ea: 0x00612DB0
void GetSurfaceTypeSounds(const char* pszType, nslWaveID* sounds)
{
    char szAliasName[256];
    for (int i = 0; i < 23; ++i)
    {
        const char* v3 = Com_SurfaceTypeToName(i);
        sprintf(szAliasName, "%s_%s", pszType, v3);
        nslWaveID Wave = nslGetWave(szAliasName);
        if (AudioBankMgr::sInst->m_size > 0 && Wave == NSL_WAVE_ID_INVALID)
            strncmp(szAliasName, "loading_", 8u);
        sounds[i] = Wave;
    }
}

// ============================================================================
// EntityManager - ea: 0x612420..0x612630 (inline COMDATs from g.o 0x4A6990)
// ============================================================================
extern int dword_F6A28C[];  // game.o data

// ea: 0x00612420
EntityManager::EntityManager()
{
    this->mWorld = nullptr;
    for (int i = 0; i < 16; ++i)
        this->mPlayers[i] = nullptr;
}

// ea: 0x00612470
EntityManager::~EntityManager()
{
}

// ea: 0x00612480
void EntityManager::CreatePlayers()
{
    Entity** mPlayers = this->mPlayers;
    if (this->mPlayers[0] == nullptr)
    {
        for (int i = 16; i != 0; --i)
        {
            TPakId PakId =
                PakManager::sInst->FindPakId(kPakTypeGlobal);
            *mPlayers++ = G_Spawn(PakId);
        }
    }
}

// ea: 0x006124C0
void EntityManager::CreateWorld()
{
    if (this->mWorld != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EntityManager.cpp";
        AeAssert::gCurrentLine = 42;
        AeAssert::gCurrentExpr = "mWorld == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("World already created!"))
            __debugbreak();
    }
    TPakId PakId =
        PakManager::sInst->FindPakId(kPakTypeGlobal);
    this->mWorld = G_Spawn(PakId);
}

// ea: 0x00612530
int EntityManager::GetPlayerIndex(Entity* entity)
{
    int result = 0;
    Entity** i = this->mPlayers;
    while (entity != *i)
    {
        ++i;
        if (++result >= 16)
            return -1;
    }
    return result;
}

// ea: 0x00612560
int EntityManager::GetEntityController(Entity* entity)
{
    Client* client = entity->client;
    if (client == nullptr)
        return 0;
    int mServerClientIndex = client->mServerClientIndex;
    if (mServerClientIndex < 0
        || *(int*)((char*)&svs.clients[mServerClientIndex].netchan[8]) != 2)
        return 0;
    int v4 = 0;
    Entity** i = this->mPlayers;
    while (entity != *i)
    {
        ++i;
        if (++v4 >= 16)
        {
            v4 = -1;
            return dword_F6A28C[802 * v4];
        }
    }
    return dword_F6A28C[802 * v4];
}

// ea: 0x006125C0
bool EntityManager::IsLocalPlayer(Entity* entity)
{
    if (entity == nullptr)
        return false;
    Client* client = entity->client;
    return client != nullptr
        && (int)client->mServerClientIndex >= 0
        && *(int*)((char*)&svs.clients[client->mServerClientIndex].netchan[8])
            == 2;
}

// ea: 0x00612610
Entity* EntityManager::GetFirstLocalPlayer()
{
    int LocalClientIndex = LocalClient::FirstLocalClientIndex();
    return this->GetPlayer(LocalClientIndex);
}

// ea: 0x00612630
void EntityManager::SwapPlayers(int eA, int eB)
{
    Entity* v3 = this->mPlayers[eA];
    this->mPlayers[eA] = this->mPlayers[eB];
    this->mPlayers[eB] = v3;
    this->mPlayers[eA]->client->mServerClientIndex = eA;
    this->mPlayers[eB]->client->mServerClientIndex = eB;
}

// g.o inline COMDATs (EntityManager.h)
// ea: 0x004A6990
EntityManager* EntityManager::Inst()
{
    return EntityManager::sInst;
}

// ea: 0x004A6A60
Entity* EntityManager::GetPlayer(int idx)
{
    if (idx >= 16)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EntityManager.h";
        AeAssert::gCurrentLine = 19;
        AeAssert::gCurrentExpr = "idx<16";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
            __debugbreak();
    }
    return this->mPlayers[idx];
}

// ea: 0x004A6AE0
Entity* EntityManager::GetWorld()
{
    return this->mWorld;
}

// ============================================================================
// SoundDevice::Sound - ea: 0x6129C0..0x612A10
// ============================================================================
// ea: 0x00612A10
SoundDevice::Sound::Sound()
{
    this->mEntHandle.mVal = 0;
    this->mHandle.mVal = 0;
    this->mDialogNotify.mHash = 0;
    this->mMinRange = 50.0f;
    this->mSource = -1;
    this->mWave = -1;
    this->mPaused = false;
    this->mAutoRelease = true;
    this->mPitch = 1.0f;
    this->mVolume = 1.0f;
    this->mMaxRange = 1500.0f;
    this->mGroupVolume = 1.0f;
    this->mDialogNotify.mHash = 0;
    this->mPoPtr = nullptr;
}

// ea: 0x006129C0
void SoundDevice::Sound::Reset()
{
    this->mMinRange = 50.0f;
    this->mSource = -1;
    this->mWave = -1;
    this->mPaused = false;
    this->mAutoRelease = true;
    this->mPitch = 1.0f;
    this->mVolume = 1.0f;
    this->mMaxRange = 1500.0f;
    this->mGroupVolume = 1.0f;
    this->mDialogNotify.mHash = 0;
    this->mPoPtr = nullptr;
}

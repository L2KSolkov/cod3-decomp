// ============================================================================
// g_entity_misc.cpp - game.o stat monitor + Entity DObj/enemy helpers
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"
#include "core/PoolAllocator.h"

#include <new>
#include <stdio.h>
#include <string.h>

extern int g_uniqueEntityIndex;  // ?g_uniqueEntityIndex@@3HA (game.o @ 0xF4F444)

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
// Entity notify plumbing - ea: 0x62AD70..0x62AFA0 (Entity.cpp)
// The reserved_dlist layout is verified here: m_size +0x00, m_head +0x04,
// m_end +0x08, m_tail +0x0C; node m_next +0x00, m_prev +0x04.
// ============================================================================
// Layout twins of core_systems.h (core_systems.h can't be included with
// g_local.h). ctors/allocators are provided by core.o (ctor_dtor.cpp).
struct WaitTilOutput;
struct EntityNotify {
    unsigned char m_dlist_node[8];   // +0x00
    unsigned int  mStr;              // +0x08
    DbLinkedHandle<EntityHandleDb, Entity> mOwner;  // +0x0C
    WaitTilOutput* mParam;           // +0x10

    EntityNotify(unsigned int hashStr,
                 DbLinkedHandle<EntityHandleDb, Entity> ent,
                 WaitTilOutput* param);  // core.o 0x4BDAA0
    static PoolAllocator* sAllocator;    // core.o @ 0xF00E28
};
struct EntityNotifySet {
    unsigned char m_dlist_node[8];   // +0x00
    DbLinkedHandle<void, void> mEnt; // +0x08
    unsigned char mStrings[0x10];    // +0x0C
    unsigned char mEndOnList[0x10];  // +0x1C

    EntityNotifySet(Entity* e);      // core.o 0x4C1D80
    static PoolAllocator* sAllocator;    // core.o @ 0xF00E2C
};

struct NotifyDList {
    int   m_size;  // +0x00
    void* m_head;  // +0x04
    void* m_end;   // +0x08
    void* m_tail;  // +0x0C
};
struct NotifyNode {
    NotifyNode* m_next;  // +0x00
    NotifyNode* m_prev;  // +0x04
};

// ea: 0x0062AD70
void Entity::AddNotify(EntityNotify* notify)
{
    if (this->mNotifySet == nullptr)
    {
        void* v3 = EntityNotifySet::sAllocator->Allocate(0x2C, false);
        EntityNotifySet* v4 =
            v3 != nullptr ? new (v3) EntityNotifySet(this) : nullptr;
        this->mNotifySet = v4;
    }
    NotifyDList* strings =
        (NotifyDList*)((char*)this->mNotifySet + 0x0C);
    NotifyNode* node = (NotifyNode*)&notify->m_dlist_node;
    node->m_next = (NotifyNode*)strings->m_end;
    node->m_prev = (NotifyNode*)strings->m_tail;
    ((NotifyNode*)strings->m_tail)->m_next = node;
    strings->m_tail = node;
    ++strings->m_size;
}

// game.o data (Entity.cpp)
static int          sNotifyInitFlags;  // $S69_1 @ 0xF58C38
static unsigned int footstep;          // ?footstep @ 0xF58C34
static unsigned int step;              // ?step @ 0xF58C30

// ea: 0x0062AE00
void Entity::Notify(HashString h)
{
    if (h.mHash == 0)
        return;
    if (this->client != nullptr)
    {
        if ((sNotifyInitFlags & 1) == 0)
        {
            sNotifyInitFlags |= 1;
            footstep = HashString::CalcHash("footstep");
        }
        if ((sNotifyInitFlags & 2) == 0)
        {
            sNotifyInitFlags |= 2;
            step = HashString::CalcHash("step");
        }
        if (h.mHash == step || h.mHash == footstep)
            this->FootStep();
    }
    void* v3 = EntityNotify::sAllocator->Allocate(0x14, false);
    EntityNotify* v4 =
        v3 != nullptr
            ? new (v3) EntityNotify(h.mHash, this->mHandle, nullptr)
            : nullptr;
    NotifyDList* pending =
        (NotifyDList*)((char*)&AeThreadManager::sInst + 0x24);
    NotifyNode* node = (NotifyNode*)&v4->m_dlist_node;
    node->m_next = (NotifyNode*)pending->m_end;
    node->m_prev = (NotifyNode*)pending->m_tail;
    ((NotifyNode*)pending->m_tail)->m_next = node;
    pending->m_tail = node;
    ++pending->m_size;
    ScriptEventHandler* mScriptEventHandler = this->mScriptEventHandler;
    if (mScriptEventHandler != nullptr)
        mScriptEventHandler->ExecEvents(this, h, nullptr);
}

// refEntity_t - leading member of trRefEntity (+0x00) - matches cg_local.h
struct refEntity_t {
    int   reType;          // +0x00
    int   renderfx;        // +0x04
    float lightingOrigin[3]; // +0x08
    float axis[3][3];      // +0x14
    float scale;           // +0x38
    float origin[3];       // +0x3C
    float oldorigin[3];    // +0x48
    void* obj;             // +0x54
    Entity* entity;        // +0x58
    void* pStaticModel;    // +0x5C
};

// ea: 0x0062AFA0
refEntity_t& Entity::GetRefEntity()
{
    return (refEntity_t&)this->GetRenderEntity();
}

// ============================================================================
// EntityState / EntityShared / Entity ctors - ea: 0x620280..0x62AD65
// ============================================================================

// ea: 0x00620280
EntityShared::EntityShared()
{
    this->linked = 0;
    this->svFlags = 0;
    this->mSingleClient.mHandle.mVal = 0;
    this->bmodel = nullptr;
    this->mins.v = _mm_setzero_ps();
    this->maxs.v = _mm_setzero_ps();
    this->absmin.v = _mm_setzero_ps();
    this->absmax.v = _mm_setzero_ps();
    this->contents = 0;
    this->currentOrigin.v = _mm_setzero_ps();
    this->currentAngles.v = _mm_setzero_ps();
    this->currentMat.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    this->currentMat.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    this->currentMat.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    this->currentMat.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    this->mOwner.mHandle.mVal = 0;
    this->eventType = 0;
    this->eventTime = 0;
    this->worldSector = nullptr;
    this->nextEntityInWorldSector = nullptr;
    this->numClusters = 0;
    this->lastCluster = 0;
    this->areanum = 0;
    this->areanum2 = 0;
    this->linkcontents = 0;
    for (int i = 0; i < 16; ++i)
        this->clusternums[i] = 0;
    this->linkmin[0] = 0.0f;
    this->linkmin[1] = 0.0f;
    this->linkmax[0] = 0.0f;
    this->linkmax[1] = 0.0f;
}

// ea: 0x00620480
EntityState::EntityState()
{
    this->eType = 0;
    this->loopSound = 0;
    this->surfType = 0;
    this->weapon = 0;
    this->eventParm = 0;
    this->scale = 0;
    this->mOtherEntity.mHandle.mVal = 0;
    this->mGroundEntity.mHandle.mVal = 0;
    this->eFlags = 0;
    this->pos.trType = TR_STATIONARY;
    this->pos.trTime = 0;
    this->pos.trDuration = 0;
    this->pos.trGravityOverride = 0;
    this->pos.trBase[0] = 0.0f;
    this->pos.trBase[1] = 0.0f;
    this->pos.trBase[2] = 0.0f;
    this->pos.trDelta[0] = 0.0f;
    this->pos.trDelta[1] = 0.0f;
    this->pos.trDelta[2] = 0.0f;
    this->apos.trType = TR_STATIONARY;
    this->apos.trTime = 0;
    this->apos.trDuration = 0;
    this->apos.trGravityOverride = 0;
    this->apos.trBase[0] = 0.0f;
    this->apos.trBase[1] = 0.0f;
    this->apos.trBase[2] = 0.0f;
    this->apos.trDelta[0] = 0.0f;
    this->apos.trDelta[1] = 0.0f;
    this->apos.trDelta[2] = 0.0f;
    this->lerpOrigin.v = _mm_setzero_ps();
    this->lerpAngles.v = _mm_setzero_ps();
    this->origin2.v = _mm_setzero_ps();
    this->angles2.v = _mm_setzero_ps();
    this->constantLight = 0;
    this->solid = 0;
    this->eventSequence = 0;
    this->leanf = 0.0f;
    this->dmgFlags = 0;
    this->useCount = 0;
    this->eTeam = 0;
    this->brushmodel = 0;
    unsigned char* eventParms = this->eventParms;
    for (int i = 4; i != 0; --i)
    {
        *(eventParms - 4) = 0;
        *eventParms++ = 0;
    }
}

// ea: 0x0062A8E0
Entity::Entity(TPakId pakId)
    : s(), r(),
      mClassName((Broc::string::Block*)nullptr),
      targetname((Broc::string::Block*)nullptr),
      mTarget((Broc::string::Block*)nullptr),
      mGroupName((Broc::string::Block*)nullptr),
      mScriptNoteworthy((Broc::string::Block*)nullptr),
      mAnimName((Broc::string::Block*)nullptr),
      team((Broc::string::Block*)nullptr),
      mSpawnItem((Broc::string::Block*)nullptr)
{
    this->mPakId = pakId;
    this->mHandle.mHandle.mVal = 0;
    this->mEntityArrayIndex = -1;
    this->mDObj = nullptr;
    this->mNotifySet = nullptr;
    this->mScriptEventHandler = nullptr;
    this->mBPInfo = nullptr;
    this->mDestructible.mValue = nullptr;
    this->mDestructible.mPakId = (unsigned int)PAK_ID_INVALID;
    this->client = nullptr;
    this->actor = nullptr;
    this->sentient = nullptr;
    this->scr_vehicle = nullptr;
    this->pTurretInfo = nullptr;
    this->mRenderEntity = nullptr;
    this->pAnimTree = nullptr;
    this->mModel.mValue = nullptr;
    this->mModel.mPakId = (unsigned int)PAK_ID_INVALID;
    this->modelscale = 1.0f;
    this->mClassNameHash.mHash = 0;
    this->targetnameHash = 0;
    this->mTargetHash = 0;
    this->mGroupNameHash = 0;
    this->mHintString = 0;
    this->physicsObject = 0;
    this->noise_index = 0;
    this->active = 0;
    this->moverState = 0;
    this->attachIgnoreCollision = 0;
    this->takedamage = 0;
    this->invulnerability_timeout = 0;
    this->spawnflags = 0;
    this->flags = 0;
    this->mFlags = 0;
    this->clipmask = 0;
    this->processedFrame = 0;
    this->parentHandle.mHandle.mVal = 0;
    this->timestamp = 0;
    this->angle = 0.0f;
    this->speed = 0.0f;
    this->closespeed = 0.0f;
    this->gDuration = 0;
    this->gDurationBack = 0;
    this->nextthink = 0;
    this->think = THINK__NULL;
    this->reached = 0;
    this->blocked = 0;
    this->touch = 0;
    this->use = 0;
    this->pain = 0;
    this->die = 0;
    this->entinfo = 0;
    this->controller = 0;
    this->health = 0;
    this->maxHealth = 0;
    this->damage = 0;
    this->methodOfDeath = 0;
    this->splashMethodOfDeath = 0;
    this->count = 0;
    this->enemy = nullptr;
    this->activator = nullptr;
    this->teamchain = nullptr;
    this->teammaster = nullptr;
    this->wait = 0.0f;
    this->random = 0.0f;
    this->delay = 0.0f;
    this->item = nullptr;
    this->key = 0;
    this->cell_index = -1;
    this->mPersistentIndex = -1;
    this->count2 = 0;
    this->grenadeExplodeTime = 0;
    this->snd_wait.notifyHash.mHash = 0;
    this->snd_wait.soundName.mHash = 0;
    this->curve = nullptr;
    this->tagInfo = nullptr;
    this->tagChildren = nullptr;
    this->scripted = nullptr;
    for (int i = 0; i < 7; ++i)
        new (&this->mAttachModels[i]) AttachModelInfo();
    this->disconnectedLinks = 0;
    this->iDisconnectTime = 0;
    this->currentValid = 0;
    this->fireSndDelay = 0;
    this->isFiring = 0;
    this->effectLoopingFire.mVal = 0;
    this->previousEventSequence = 0;
    this->previousPreEventSequence = 0;
    this->mAnimDebug = nullptr;
    this->mBrocExtendedEntity = nullptr;
    this->proximity_data = nullptr;
    this->mClassNameHash.mHash = 0;
    this->snd_wait.notifyHash.mHash = 0;
    this->snd_wait.soundName.mHash = 0;
    this->pos1.v = _mm_setzero_ps();
    this->pos2.v = _mm_setzero_ps();
    this->pos3.v = _mm_setzero_ps();
    this->movedir.v = _mm_setzero_ps();
    this->rotate.v = _mm_setzero_ps();
    this->TargetAngles.v = _mm_setzero_ps();
    this->uniqueIndex = g_uniqueEntityIndex++;
    UpdateEntityHash(this);
    this->mClassName = str_const.noclass;
    HashString hs(this->mClassName);
    this->mClassNameHash.mHash = hs.mHash;
    this->r.mOwner.mHandle.mVal = 0;
    this->parentHandle.mHandle.mVal = 0;
    this->r.eventType = 0;
    this->r.eventTime = 0;
    this->spawnflags = 0;
    this->r.pos_cache.v.m128_f32[3] = 0.0f;
    this->mScriptNoteworthy.clear();
    this->targetname.clear();
    this->previousPreEventSequence = 0;
    this->previousEventSequence = 0;
    this->speed = -1.0f;
    UpdateEntityHash(this);
    EntityHandleDb::sInst.AssignHandle(*this);
    this->mFlags |= 1u;
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
    void ScaleVolume(float scale);  // ?ScaleVolume@MusicMgr@@QAEXM@Z (game.o 0x62D6D0)
    void Stop(const float fadeOutTime);  // ?Stop@MusicMgr@@QAEXM@Z (game.o 0x62D830)
    void Update(float dt);       // ?Update@MusicMgr@@QAEXM@Z (game.o 0x62D8A0)
    bool IsMusicPlaying();       // ?IsMusicPlaying@MusicMgr@@QAE_NXZ (game.o 0x6217F0)
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
// SoundMediaMgr - ea: 0x603EF0..0x603F20 (SoundMediaMgr.cpp)
// ============================================================================
struct SoundMediaMgr {
    SoundMediaMgr();              // ??0SoundMediaMgr@@QAE@XZ (game.o 0x603EF0)
    ~SoundMediaMgr();             // ??1SoundMediaMgr@@QAE@XZ (game.o 0x603F00)
    void RegisterSounds();        // ?RegisterSounds@SoundMediaMgr@@QAEXXZ (game.o 0x603F10)
    void PlayLandingSound(Entity* entity, int surfaceType,
                          bool damage);  // game.o 0x603F20
};

extern struct CollisionDesc {
    math::Position3 coord;    // +0x00
    math::Position3 normal;   // +0x10
    int material;             // +0x20
};
extern Handle PostEffectEventLanding(const Entity* ent,
                                     const CollisionDesc* col_desc);

// ea: 0x00603EF0
SoundMediaMgr::SoundMediaMgr()
{
}

// ea: 0x00603F00
SoundMediaMgr::~SoundMediaMgr()
{
}

// ea: 0x00603F10
void SoundMediaMgr::RegisterSounds()
{
}

// ea: 0x00603F20
void SoundMediaMgr::PlayLandingSound(Entity* entity,
                                     int surfaceType,
                                     bool damage)
{
    CollisionDesc v5;
    v5.coord.v.m128_f32[0] = entity->s.pos.trBase[0];
    v5.coord.v.m128_f32[1] = entity->s.pos.trBase[1];
    v5.coord.v.m128_f32[2] = entity->s.pos.trBase[2];
    memset(&v5.coord.v.m128_f32[3], 0, 20);
    v5.material = surfaceType;
    PostEffectEventLanding(entity, &v5);
}

extern float nslGetWaveParam(nslWaveID wave, int b, float c);  // nsl_xboxr

static SoundDevice::Sound* SoundFromHandle(Handle h)
{
    unsigned int idx = h.mVal & 0xFFF;
    if (idx < 0x200
        && h.mVal >> 12 == SoundDevice::SoundHandleDb::sInst.mElements[idx].mKey)
        return SoundDevice::SoundHandleDb::sInst.mElements[idx].mObject;
    return nullptr;
}

// ea: 0x0062D6D0
void MusicMgr::ScaleVolume(float scale)
{
    unsigned int mVal = this->mMusic.mVal;
    this->mVolScale = scale;
    SoundDevice::Sound* mObject = SoundFromHandle(this->mMusic);
    if (mObject != nullptr)
    {
        float newVolume = nslGetWaveParam((nslWaveID)mObject->mWave, 0, 1.0f)
            * this->mOutsideScale * this->mVolScale;
        mObject->SetVolume(newVolume);
    }
    SoundDevice::Sound* v10 = SoundFromHandle(this->mMusicIndoor);
    if (v10 != nullptr)
    {
        float newVolumea = nslGetWaveParam((nslWaveID)v10->mWave, 0, 1.0f)
            * this->mIndoorScale * this->mVolScale;
        v10->SetVolume(newVolumea);
    }
}

// ea: 0x0062D830
void MusicMgr::Stop(const float fadeOutTime)
{
    SoundDevice::Sound* mObject = SoundFromHandle(this->mMusic);
    if (mObject != nullptr)
    {
        mObject->Stop();
        this->mMusic.mVal = 0;
    }
}

// ea: 0x0062D8A0
void MusicMgr::Update(float dt)
{
    if (this->mCrossFadeType == 1)
    {
        float v4 = dt + this->mDelayCount;
        float v6 = v4 / this->mIndoorFadeTime;
        this->mDelayCount = v4;
        this->mIndoorScale = v6;
        this->mOutsideScale = 1.0f - v6;
        if (v4 >= this->mIndoorFadeTime)
        {
            this->mIndoorScale = 1.0f;
            this->mOutsideScale = 0.0f;
            this->mCrossFadeType = 0;
        }
        SoundDevice::Sound* mObject = SoundFromHandle(this->mMusic);
        if (mObject != nullptr)
        {
            float newVolume = nslGetWaveParam((nslWaveID)mObject->mWave, 0,
                                              1.0f)
                * this->mVolScale * this->mOutsideScale;
            mObject->SetVolume(newVolume);
        }
        SoundDevice::Sound* v14 = SoundFromHandle(this->mMusicIndoor);
        if (v14 != nullptr)
        {
            float newVolumea = nslGetWaveParam((nslWaveID)v14->mWave, 0, 1.0f)
                * this->mVolScale * this->mIndoorScale;
            v14->SetVolume(newVolumea);
        }
    }
    else if (this->mCrossFadeType == 2)
    {
        float v15 = dt + this->mDelayCount;
        float v16 = v15 / this->mIndoorFadeTime;
        this->mDelayCount = v15;
        this->mOutsideScale = v16;
        this->mIndoorScale = 1.0f - v16;
        if (v15 >= this->mIndoorFadeTime)
        {
            this->mOutsideScale = 1.0f;
            this->mCrossFadeType = 0;
            SoundDevice::Sound* indoor = SoundFromHandle(this->mMusicIndoor);
            if (indoor != nullptr)
            {
                indoor->Stop();
                this->mMusicIndoor.mVal = 0;
            }
        }
        SoundDevice::Sound* v20 = SoundFromHandle(this->mMusic);
        if (v20 != nullptr)
        {
            float newVolumeb = nslGetWaveParam((nslWaveID)v20->mWave, 0, 1.0f)
                * this->mVolScale * this->mOutsideScale;
            v20->SetVolume(newVolumeb);
        }
        SoundDevice::Sound* v22 = SoundFromHandle(this->mMusicIndoor);
        if (v22 != nullptr)
        {
            float newVolumec = nslGetWaveParam((nslWaveID)v22->mWave, 0, 1.0f)
                * this->mVolScale * this->mIndoorScale;
            v22->SetVolume(newVolumec);
        }
    }
}

// ea: 0x006217F0
bool MusicMgr::IsMusicPlaying()
{
    unsigned int mVal = this->mMusic.mVal;
    unsigned int v2 = mVal & 0xFFF;
    return v2 < 0x200
        && mVal >> 12
            == (unsigned int)SoundDevice::SoundHandleDb::sInst.mElements[v2].mKey
        && SoundDevice::SoundHandleDb::sInst.mElements[v2].mObject != nullptr;
}

// ============================================================================
// SoundDevice::Sound::GetDebugString - ea: 0x6216F0
// ============================================================================
// nsl sound API declarations (shared by the SoundDevice helpers below)
enum nslSourceState {
    NSL_SOURCE_STATE_INVALID = 0,
    NSL_SOURCE_STATE_QUEUING = 2,
    NSL_SOURCE_STATE_QUEUED = 3,
    NSL_SOURCE_STATE_PLAYING = 4,
    NSL_SOURCE_STATE_PAUSED = 5,
};
extern nslSourceState nslGetSourceState(nslSourceID sid);   // nsl
extern unsigned int nslWaveGetHash(nslWaveID waveID);       // nsl
extern void nslStopSource(nslSourceID sid);                 // nsl
extern void nslFreeSource(nslSourceID sid);                 // nsl
extern void nslSetSourceParam(nslSourceID sid, int index,
                              float value);                 // nsl
extern void nslSetSourcePosition(nslSourceID sid,
                                 const float* position);    // nsl
extern void nslSetSourceVelocity(nslSourceID sid,
                                 const float* velocity);    // nsl
extern const char* nslWaveGetName(nslWaveID waveID);        // nsl
extern nslSourceID g_break_on_stop;  // ?g_break_on_stop@@3W4nslSourceID@@A (game.o)
extern void tlWarning(const char* fmt, ...);                // tl_xboxr
extern "C" int __fpclass(float);
extern const char* nslGetSourceName(nslSourceID sid);       // nsl
extern float nslGetSourceParam(nslSourceID sid, int index,
                               float defaultValue);         // nsl
extern unsigned int nslGetSourceLength(nslSourceID sid);    // nsl
extern int nslIsWaveLooped(nslWaveID a);                    // nsl
extern int nslGetWaveLength(nslWaveID waveID);              // nsl
extern void nslSetSourceEffectOn(nslSourceID sid);          // nsl
extern void nslSetSourceEffectOff(nslSourceID sid);         // nsl
extern void nslSetMasterVolume(float newVolume);            // nsl
extern void nslPauseSource(nslSourceID sid);                // nsl
extern void nslUnpauseSource(nslSourceID sid);              // nsl
extern void nslPlaySource(nslSourceID sid);                 // nsl
extern void nslDampenGuardSource(nslSourceID sid);          // nsl
extern void nslSetNumberOfListeners(int listeners);         // nsl
extern int nslAreAllBanksLoaded();                          // nsl
extern int nslNumBanksInUse();                              // nsl

// ea: 0x006216F0
ae_fixed_string<1024, unsigned short>
SoundDevice::Sound::GetDebugString() const
{
    const char* SourceName = nslGetSourceName((nslSourceID)this->mSource);
    const char* v11 = "loop";
    nslSourceState SourceState;
    if (((this->mSource == NSL_SOURCE_ID_INVALID
          || (SourceState = nslGetSourceState((nslSourceID)this->mSource))
                 != NSL_SOURCE_STATE_PLAYING
             && SourceState != NSL_SOURCE_STATE_QUEUING
             && SourceState != NSL_SOURCE_STATE_QUEUED
             && SourceState != NSL_SOURCE_STATE_PAUSED)
         && !this->mPaused)
        || nslIsWaveLooped((nslWaveID)this->mWave) == 0)
    {
        v11 = "one-shot";
    }
    if (SourceName == nullptr)
        SourceName = "(unknown)";
    unsigned int SourceLength =
        nslGetSourceLength((nslSourceID)this->mSource);
    float SourceParam =
        nslGetSourceParam((nslSourceID)this->mSource, 1, -1.0f);
    float v7 = nslGetSourceParam((nslSourceID)this->mSource, 0, -1.0f);
    char buf[1024];
    sprintf(buf, "%s V%.2f P%.2f L%.2f %s", SourceName, v7, SourceParam,
            SourceLength * 0.001f, v11);
    return ae_fixed_string<1024, unsigned short>(buf);
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
typedef int ELanguage;  // core_globals.h ABI twin (core_systems.h can't load)
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
        uint8_t   _pad0[0x2C];      // +0x00 (name: tlFixedString, 32 bytes)
        int       state[6];         // +0x2C
        nflFileID fileID[6];        // +0x44
        nslBankID bankId[6];        // +0x5C (bankId[5] aliases next entry +0x04)
    };
    static_assert(sizeof(WbkEntry) == 0x74, "WbkEntry view size mismatch");
    uint8_t  _pad0[4];                 // +0x00 (vftable)
    bool     mDoUnloadNotify;          // +0x04
    bool     mDoLoadNotify;            // +0x05
    uint8_t  _pad06[2];                // +0x06
    uint8_t  mAvailableWbks[0x6C0];    // +0x08 (16 * 0x6C stride)
    int      m_size;                   // +0x6C8
    static AudioBankMgr* sInst;        // ?sInst@AudioBankMgr@@2PAV1@A
    virtual ~AudioBankMgr();           // ??1AudioBankMgr@@UAE@XZ
    bool IsFinished() const;           // ?IsFinished@AudioBankMgr@@QBE_NXZ
    const char* LanguageStr(ELanguage id) const;  // ?LanguageStr@AudioBankMgr@@ABEPBDW4ELanguage@@@Z
    void NotifyLoaded();               // ?NotifyLoaded@AudioBankMgr@@AAEXXZ (game.o 0x62B9C0)
    void NotifyUnloaded();             // ?NotifyUnloaded@AudioBankMgr@@AAEXXZ (game.o 0x62B9F0)
    void Update();                     // ?Update@AudioBankMgr@@QAEXXZ (game.o 0x62BA20)
    void FinishLoading();              // ?FinishLoading@AudioBankMgr@@QAEXXZ (game.o 0x62BC40)
    void LoadWbkInternal(WbkEntry* wbk, const char* path, ELanguage lang,
                         bool async);  // ?LoadWbkInternal@AudioBankMgr@@AAEXAAUWbkEntry@1@PBDW4ELanguage@@_N@Z (game.o 0x62BD50)
    void FreeWbk(const void* name, bool async);  // ?FreeWbk@AudioBankMgr@@QAEXABVtlFixedString@@_N@Z (game.o 0x62BE30)
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
// AudioBankMgr load/notify/update - ea: 0x62B9C0..0x62C010
// ============================================================================
extern void codNflUpdate();                    // nfl_xboxr
extern void nslUpdateBanks();                  // nsl_xboxr
extern int  nslGetBankState(nslBankID bankID); // nsl_xboxr
extern nslBankID nslLoadBank(unsigned int flags, nflFileID file,
                             unsigned int fileOffset);  // nsl_xboxr
extern void tlPrintf(const char* fmt, ...);    // tl_xboxr

// ELanguage values (verified vs AudioBankMgr::LanguageStr disasm)
enum {
    kLanguageEnglish = 0,
    kLanguageGerman = 1,
    kLanguageFrench = 2,
    kLanguageSpanish = 3,
    kLanguageItalian = 4,
    kLanguageUnlocalized = 5,
};

// ea: 0x006023F0
const char* AudioBankMgr::LanguageStr(ELanguage id) const
{
    const char* result;
    switch (id)
    {
    case kLanguageEnglish:
        result = "English";
        break;
    case kLanguageGerman:
        result = "German";
        break;
    case kLanguageFrench:
        result = "French";
        break;
    case kLanguageSpanish:
        result = "Spanish";
        break;
    case kLanguageItalian:
        result = "Italian";
        break;
    case kLanguageUnlocalized:
        result = "<Unlocalized>";
        break;
    default:
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AudioBankManager.cpp";
        AeAssert::gCurrentLine = 63;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Unknown language id"))
            __debugbreak();
        result = "<unknown language id>";
        break;
    }
    return result;
}

// ea: 0x0062B9C0
void AudioBankMgr::NotifyLoaded()
{
    Entity* mWorld = EntityManager::sInst->mWorld;
    if (mWorld != nullptr)
    {
        HashString v2;
        v2.mHash = HashString::CalcHash("wbk_loaded");
        mWorld->Notify(v2);
    }
}

// ea: 0x0062B9F0
void AudioBankMgr::NotifyUnloaded()
{
    Entity* mWorld = EntityManager::sInst->mWorld;
    if (mWorld != nullptr)
    {
        HashString v2;
        v2.mHash = HashString::CalcHash("wbk_unloaded");
        mWorld->Notify(v2);
    }
}

// ea: 0x0062BA20
void AudioBankMgr::Update()
{
    codNflUpdate();
    nslUpdateBanks();
    if (this->mDoUnloadNotify)
    {
        Entity* mWorld = EntityManager::sInst->mWorld;
        if (mWorld != nullptr)
        {
            HashString v3;
            v3.mHash = HashString::CalcHash("wbk_unloaded");
            mWorld->Notify(v3);
        }
    }
    if (this->mDoLoadNotify)
    {
        Entity* v4 = EntityManager::sInst->mWorld;
        if (v4 != nullptr)
        {
            HashString v5;
            v5.mHash = HashString::CalcHash("wbk_loaded");
            v4->Notify(v5);
        }
    }
    int m_size = this->m_size;
    this->mDoUnloadNotify = false;
    this->mDoLoadNotify = false;
    if (m_size > 0)
    {
        unsigned int v7 = 0;
        unsigned int v17 = 0;
        for (int i = 0;;)
        {
            if (v7 >= 0x6C0)
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
                (WbkEntry*)((char*)this->mAvailableWbks + v7);
            for (int j = 6; j != 0; --j)
            {
                if (entry->bankId[j - 1] == NSL_BANK_ID_INVALID)
                    continue;
                int BankState = nslGetBankState(entry->bankId[j - 1]);
                int state = entry->state[j - 1];
                if (state == kLoading)
                {
                    if (BankState != 0)
                    {
                        if (BankState != -1)
                            continue;
                        AeAssert::gCurrentAuthor = AeAssert::ARO;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\AudioBankManager.cpp";
                        AeAssert::gCurrentLine = 131;
                        AeAssert::gCurrentExpr = nullptr;
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Warning(
                                "Problem loading wbk '%s'",
                                (const char*)entry + 4))
                            __debugbreak();
                        nflCloseFile(entry->fileID[j - 1]);
                        entry->state[j - 1] = kUnloaded;
                        entry->fileID[j - 1] = (nflFileID)-1;
                        entry->bankId[j - 1] = NSL_BANK_ID_INVALID;
                        Entity* v10 = EntityManager::sInst->mWorld;
                        if (v10 != nullptr)
                        {
                            HashString v11;
                            v11.mHash = HashString::CalcHash("wbk_loaded");
                            v10->Notify(v11);
                        }
                    }
                    else
                    {
                        entry->state[j - 1] = kLoaded;
                        Entity* v10 = EntityManager::sInst->mWorld;
                        if (v10 != nullptr)
                        {
                            HashString v11;
                            v11.mHash = HashString::CalcHash("wbk_loaded");
                            v10->Notify(v11);
                        }
                    }
                }
                else if (state == kUnloading && BankState == -1)
                {
                    entry->state[j - 1] = kUnloaded;
                    entry->bankId[j - 1] = NSL_BANK_ID_INVALID;
                    Entity* v10 = EntityManager::sInst->mWorld;
                    if (v10 != nullptr)
                    {
                        HashString v11;
                        v11.mHash = HashString::CalcHash("wbk_unloaded");
                        v10->Notify(v11);
                    }
                }
            }
            ++i;
            v7 = v17 + 108;
            v17 += 108;
            if (i >= this->m_size)
                break;
        }
    }
}

// ea: 0x0062BC40
void AudioBankMgr::FinishLoading()
{
    do
    {
        this->Update();
        bool v3 = true;
        for (int i = 0; i < this->m_size; ++i)
        {
            const WbkEntry* entry =
                (const WbkEntry*)((char*)this->mAvailableWbks + 108 * i);
            for (int k = 0; k < 6; ++k)
            {
                if (entry->state[k] == kLoading
                    || entry->state[k] == kUnloading)
                    v3 = false;
            }
        }
        if (v3)
            break;
    } while (1);
}

// ea: 0x0062BD50
void AudioBankMgr::LoadWbkInternal(WbkEntry* wbk, const char* path,
                                   ELanguage lang, bool async)
{
    if (wbk->fileID[lang] == (nflFileID)-1
        || wbk->bankId[lang] != NSL_BANK_ID_INVALID)
    {
        this->mDoLoadNotify = true;
    }
    else
    {
        const char* v6 = this->LanguageStr(lang);
        tlPrintf("[wbk] loading wbk [%s]: %s\n",
                 (const char*)wbk + 4, v6);
        nslBankID Bank = nslLoadBank(0, wbk->fileID[lang], 0);
        wbk->bankId[lang] = Bank;
        wbk->state[lang] = kLoading;
        if (Bank == NSL_BANK_ID_INVALID)
        {
            this->mDoLoadNotify = true;
            nflCloseFile(wbk->fileID[lang]);
            wbk->fileID[lang] = (nflFileID)-1;
            wbk->state[lang] = kUnloaded;
        }
        else if (!async)
        {
            if (nslGetBankState(Bank) == 1)
            {
                do
                    this->Update();
                while (nslGetBankState(wbk->bankId[lang]) == 1);
            }
            PakManager::sInst->SetSoundProgress(1.0f);
        }
    }
}

// ea: 0x0062BE30
void AudioBankMgr::FreeWbk(const void* name, bool async)
{
    unsigned int v5 = 0;
    int i = 0;
    if (this->m_size > 0)
    {
        while (1)
        {
            if (v5 >= 0x6C0)
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
                (WbkEntry*)((char*)this->mAvailableWbks + v5);
            // tlFixedString name compare (8 dwords = 32 bytes)
            if (memcmp(entry, name, 32) == 0)
            {
                for (int lang = 6; lang != 0; --lang)
                {
                    int state = entry->state[lang - 1];
                    if (state == kLoaded)
                    {
                        tlPrintf("[wbk] freeing wbk: %s\n",
                                 (const char*)entry + 4);
                        nslFreeBank(entry->bankId[lang - 1]);
                        nflCloseFile(entry->fileID[lang - 1]);
                        entry->state[lang - 1] = kUnloading;
                        entry->fileID[lang - 1] = (nflFileID)-1;
                        if (!async
                            && nslGetBankState(entry->bankId[lang - 1]) >= 0)
                        {
                            do
                                this->Update();
                            while (nslGetBankState(
                                       entry->bankId[lang - 1])
                                   >= 0);
                        }
                    }
                    else if (state == kLoading)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::ARO;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\AudioBankManager.cpp";
                        AeAssert::gCurrentLine = 363;
                        AeAssert::gCurrentExpr =
                            "wbk.state[lang] != WbkEntry::kLoading";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("not handling unloading case"))
                            __debugbreak();
                    }
                    this->mDoUnloadNotify = true;
                }
                return;
            }
            v5 += 108;
            if (++i >= this->m_size)
                break;
        }
    }
    AeAssert::gCurrentAuthor = AeAssert::ARO;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AudioBankManager.cpp";
    AeAssert::gCurrentLine = 372;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("trying to free unknown wbk '%s'",
                             (const char*)name + 4))
        __debugbreak();
    this->mDoUnloadNotify = true;
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
// SoundDevice::SetListenerVectors - ea: 0x612A70
// ============================================================================
extern "C" int __fpclass(float);
extern void nslListenerSetPosition(unsigned int listenerIndex,
                                   const float* pos);  // nsl_xboxr
extern void nslListenerSetOrientation(unsigned int listenerIndex,
                                      const float* frt,
                                      const float* top);  // nsl_xboxr
extern void tlWarning(const char* Format, ...);  // tl_xboxr

// ea: 0x00612A70
void SoundDevice::SetListenerVectors(int listener,
                                     const math::Position3& position,
                                     const math::Dir3& front,
                                     const math::Dir3& up)
{
    if ((__fpclass(position.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(position.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(position.v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(front.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(front.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(front.v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(up.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(up.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(up.v.m128_f32[2]) & 0x297) != 0)
    {
        tlWarning(
            "A NAN was passed into the sound system while trying to adjust "
            "listener position\n");
        return;
    }
    float upv[3];
    upv[0] = position.v.m128_f32[0];
    upv[1] = position.v.m128_f32[1];
    upv[2] = position.v.m128_f32[2];
    float fwv[3];
    fwv[0] = front.v.m128_f32[0];
    fwv[1] = front.v.m128_f32[1];
    fwv[2] = front.v.m128_f32[2];
    float v12[3];
    v12[0] = up.v.m128_f32[0];
    v12[1] = up.v.m128_f32[1];
    v12[2] = up.v.m128_f32[2];
    nslListenerSetPosition((unsigned int)listener, upv);
    nslListenerSetOrientation((unsigned int)listener, fwv, v12);
    this->mDebugListenerPosition[0] = position.v.m128_f32[0];
    this->mDebugListenerPosition[1] = position.v.m128_f32[1];
    this->mDebugListenerPosition[2] = position.v.m128_f32[2];
    this->mDebugListenerForward[0] = front.v.m128_f32[0];
    this->mDebugListenerForward[1] = front.v.m128_f32[1];
    this->mDebugListenerForward[2] = front.v.m128_f32[2];
    this->mDebugListenerUp[0] = up.v.m128_f32[0];
    this->mDebugListenerUp[1] = up.v.m128_f32[1];
    this->mDebugListenerUp[2] = up.v.m128_f32[2];
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

extern bool gCareAboutCheckpoint;  // ?gCareAboutCheckpoint@@3_NA (game.o 0xDD74C8)
extern TPakId CurPakId(void);      // sv.o

// ea: 0x0062B0B0
void EntityManager::UnloadBank(TPakId pakId)
{
    Entity* const* p = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity* const* end = p + EntityHandleDb::sInst.mActiveList.m_size;
    while (p != end)
    {
        Entity* v4 = *p;
        if (v4 != nullptr)
        {
            TPakId mPakId = (TPakId)v4->mPakId;
            if (mPakId == PAK_ID_INVALID)
                mPakId = CurPakId();
            if (mPakId == pakId)
            {
                gCareAboutCheckpoint = false;
                G_FreeEntity(v4, 0);
            }
        }
        ++p;
    }
}

// ea: 0x0062B110
void EntityManager::DeleteAllEntities()
{
    Entity* const* p = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity* const* end = p + EntityHandleDb::sInst.mActiveList.m_size;
    while (p != end)
    {
        Entity* v4 = *p;
        if (v4 != nullptr)
        {
            gCareAboutCheckpoint = false;
            G_FreeEntity(v4, 0);
        }
        ++p;
    }
    EntityHandleDb::sInst.Compact();
    for (int i = 0; i < 16; ++i)
        this->mPlayers[i] = nullptr;
    this->mWorld = nullptr;
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

// ============================================================================
// SoundDevice / Sound helpers - ea: 0x6025A0..0x6029D0 (SoundDevice.cpp)
// ============================================================================

// ea: 0x006025A0
float SoundDevice::GetWaveDuration(nslWaveID wave)
{
    if (wave == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 255;
        AeAssert::gCurrentExpr = "wave";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid wave ptr"))
            __debugbreak();
    }
    return (float)nslGetWaveLength(wave);
}

// ea: 0x00602600
float SoundDevice::Sound::GetPlaybackPosition() const
{
    return 0.0f;
}

// ea: 0x00602610
void SoundDevice::Sound::SetReverb(bool on)
{
    nslSourceID mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
        goto LABEL_6;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
    AeAssert::gCurrentLine = 452;
    AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
        __debugbreak();
    mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
    {
    LABEL_6:
        if (on)
            nslSetSourceEffectOn(mSource);
        else
            nslSetSourceEffectOff(mSource);
    }
}

// ea: 0x00602690
float SoundDevice::Sound::GetVolume() const
{
    if (this->mSource == -1)
        return -2.0f;
    return nslGetSourceParam((nslSourceID)this->mSource, 0, -1.0f);
}

// ea: 0x006026B0
const char* SoundDevice::Sound::GetSourceName() const
{
    if (this->mSource == -1)
        return nullptr;
    return nslGetSourceName((nslSourceID)this->mSource);
}

// ea: 0x00602820
bool SoundDevice::Sound::IsQueuing() const
{
    nslSourceID mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
        return nslGetSourceState(mSource) == NSL_SOURCE_STATE_QUEUING;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
    AeAssert::gCurrentLine = 708;
    AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
        __debugbreak();
    mSource = (nslSourceID)this->mSource;
    return this->mSource != -1
        && nslGetSourceState(mSource) == NSL_SOURCE_STATE_QUEUING;
}

// ea: 0x00602890
bool SoundDevice::Sound::IsQueued() const
{
    nslSourceID mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
        return nslGetSourceState(mSource) == NSL_SOURCE_STATE_QUEUED;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
    AeAssert::gCurrentLine = 720;
    AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
        __debugbreak();
    mSource = (nslSourceID)this->mSource;
    return this->mSource != -1
        && nslGetSourceState(mSource) == NSL_SOURCE_STATE_QUEUED;
}

// ea: 0x00602900
bool SoundDevice::Sound::IsPlaying() const
{
    if (this->mSource == -1)
        return false;
    nslSourceState SourceState =
        nslGetSourceState((nslSourceID)this->mSource);
    return SourceState == NSL_SOURCE_STATE_PLAYING
        || SourceState == NSL_SOURCE_STATE_QUEUING
        || SourceState == NSL_SOURCE_STATE_QUEUED
        || SourceState == NSL_SOURCE_STATE_PAUSED;
}

// ea: 0x00602930
bool SoundDevice::Sound::IsPaused() const
{
    return this->mPaused;
}

// ea: 0x00602940
bool SoundDevice::Sound::IsFinished() const
{
    bool result = false;
    if (!this->mPaused)
    {
        if (this->mSource == -1)
            return true;
        nslSourceState SourceState =
            nslGetSourceState((nslSourceID)this->mSource);
        if (SourceState != NSL_SOURCE_STATE_PLAYING
            && SourceState != NSL_SOURCE_STATE_QUEUING
            && SourceState != NSL_SOURCE_STATE_QUEUED
            && SourceState != NSL_SOURCE_STATE_PAUSED)
            return true;
    }
    return result;
}

// ea: 0x00602980
bool SoundDevice::Sound::IsLooped() const
{
    nslSourceState s = nslGetSourceState((nslSourceID)this->mSource);
    return (this->mSource != -1
            && (s == NSL_SOURCE_STATE_PLAYING
                || s == NSL_SOURCE_STATE_QUEUING
                || s == NSL_SOURCE_STATE_QUEUED
                || s == NSL_SOURCE_STATE_PAUSED)
            || this->mPaused)
        && nslIsWaveLooped((nslWaveID)this->mWave) != 0;
}

// ea: 0x006029D0
float SoundDevice::Sound::GetLength() const
{
    if (this->mSource == -1)
        return 0.0f;
    return (float)nslGetSourceLength((nslSourceID)this->mSource);
}

// ea: 0x006026D0
void SoundDevice::Sound::PlayQueued()
{
    nslSourceID mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
        goto LABEL_6;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
    AeAssert::gCurrentLine = 667;
    AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
        __debugbreak();
    mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
    LABEL_6:
        nslPlaySource(mSource);
}

// ea: 0x00602730
void SoundDevice::Sound::Pause()
{
    nslSourceID mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
        goto LABEL_6;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
    AeAssert::gCurrentLine = 677;
    AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
        __debugbreak();
    mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
    {
    LABEL_6:
        nslPauseSource(mSource);
        this->mPaused = true;
    }
}

// ea: 0x006027A0
void SoundDevice::Sound::Unpause()
{
    nslSourceID mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
        goto LABEL_6;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
    AeAssert::gCurrentLine = 688;
    AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
        __debugbreak();
    mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
    {
    LABEL_6:
        nslUnpauseSource(mSource);
        this->mPaused = false;
    }
}

// ea: 0x00602810
void SoundDevice::Sound::DampenGuard()
{
    if (this->mSource != -1)
        nslDampenGuardSource((nslSourceID)this->mSource);
}

// ============================================================================
// SoundDevice manager methods - ea: 0x602A10..0x602B40
// ============================================================================
// ea: 0x00602A10
void SoundDevice::ScaleVolume(float scale)
{
    this->mVolScale = scale;
    if (scale <= 0.0f)
        this->mVolScale = 0.0f;
    if (this->mVolScale >= 1.0f)
        this->mVolScale = 1.0f;
    nslSetMasterVolume(1.0f - ((1.0f - this->mVolScale)
                               * (1.0f - this->mVolScale)));
}

// ea: 0x00602A80
void SoundDevice::PauseAllSounds()
{
    Sound* p = this->mSounds;
    for (int i = 512; i != 0; --i)
    {
        if (p->mSource != -1)
            p->Pause();
        ++p;
    }
}

// ea: 0x00602AB0
SoundDevice::Sound* SoundDevice::GetSoundFromSourceId(nslSourceID id)
{
    int v2 = 0;
    for (Sound* i = this->mSounds; i->mSource != (int)id; ++i)
    {
        if (++v2 >= 0x200)
            return nullptr;
    }
    return &this->mSounds[v2];
}

// ea: 0x00602AE0
void SoundDevice::UnpauseAllSounds()
{
    Sound* p = this->mSounds;
    for (int i = 512; i != 0; --i)
    {
        if (p->mSource != -1)
            p->Unpause();
        ++p;
    }
}

// ea: 0x00602B10
int SoundDevice::GetNumberOfListeners()
{
    return this->mNumberOfListeners;
}

// ea: 0x00602B20
void SoundDevice::SetNumberOfListeners(int listeners)
{
    this->mNumberOfListeners = listeners;
    nslSetNumberOfListeners(listeners);
}

// ea: 0x00602B40
bool SoundDevice::IsSoundReady()
{
    return nslAreAllBanksLoaded() != 0 && nslNumBanksInUse() >= 2;
}

// ============================================================================
// SoundDevice::SetReverb - ea: 0x602BA0 (reverb preset table, bits exact)
// ============================================================================
// ea: 0x00602BA0
void SoundDevice::SetReverb(const char* preset, bool immediate)
{
    if (preset != nullptr && *preset != 0)
    {
        int _newFx[14];
        int v3, v4, v5;
        if (_stricmp(preset, "Preset_Alley") == 0)
        {
            _newFx[3] = 1069463634;
            v3 = 1063004406;
            _newFx[1] = -270;
            _newFx[5] = -1204;
            _newFx[7] = -4;
            goto LABEL_55;
        }
        if (_stricmp(preset, "Preset_Arena") == 0)
        {
            _newFx[3] = 1088925204;
            _newFx[4] = 1051260355;
            _newFx[6] = 1017370378;
            v4 = 1022739087;
            _newFx[1] = -698;
            _newFx[5] = -1166;
            _newFx[7] = 16;
            goto LABEL_56;
        }
        if (_stricmp(preset, "Preset_Auditorium") == 0)
        {
            _newFx[3] = 1082801521;
            _newFx[4] = 1058474557;
            _newFx[6] = 1017370378;
            v4 = 1022739087;
            _newFx[1] = -476;
            _newFx[5] = -789;
            _newFx[7] = -289;
            goto LABEL_56;
        }
        if (_stricmp(preset, "Preset_Bathroom") == 0)
        {
            _newFx[3] = 1069463634;
            _newFx[4] = 1057635697;
            _newFx[6] = 1004888130;
            _newFx[8] = 1010055512;
            _newFx[9] = 1120403456;
            v5 = 1114636288;
            _newFx[0] = -1000;
            _newFx[1] = -1200;
            _newFx[5] = -370;
            _newFx[7] = 1030;
            goto LABEL_58;
        }
        if (_stricmp(preset, "Preset_CarpetedHallway") == 0)
        {
            _newFx[3] = 1050253722;
            _newFx[4] = 1036831949;
            _newFx[6] = 990057071;
            v4 = 1022739087;
            _newFx[1] = -4000;
            _newFx[5] = -1831;
            _newFx[7] = -1630;
            goto LABEL_56;
        }
        if (_stricmp(preset, "Preset_Cave") == 0)
        {
            _newFx[3] = 1077558641;
            _newFx[4] = 1067869798;
            _newFx[6] = 1014350479;
            v4 = 1018444120;
            _newFx[1] = 0;
            _newFx[5] = -602;
            _newFx[7] = -302;
            goto LABEL_56;
        }
        if (_stricmp(preset, "Preset_City") == 0)
        {
            _newFx[3] = 1069463634;
            _newFx[4] = 1059816735;
            _newFx[6] = 1004888130;
            _newFx[8] = 1010055512;
            _newFx[9] = 1112014848;
            v5 = 1120403456;
            _newFx[0] = -1000;
            _newFx[1] = -800;
            _newFx[5] = -2273;
            _newFx[7] = -2217;
            goto LABEL_58;
        }
        if (_stricmp(preset, "Preset_ConcertHall") == 0)
        {
            _newFx[3] = 1081794888;
            _newFx[4] = 1060320051;
            _newFx[6] = 1017370378;
            v4 = 1022202216;
            _newFx[1] = -500;
            _newFx[5] = -1230;
            _newFx[7] = -2;
            goto LABEL_56;
        }
        if (_stricmp(preset, "Preset_Default2") == 0)
        {
            _newFx[3] = 1065353216;
            _newFx[4] = 1056964608;
            _newFx[6] = 1017370378;
            _newFx[8] = 1025758986;
            _newFx[1] = 0;
            _newFx[9] = 1120403456;
            _newFx[10] = 1120403456;
            goto LABEL_60;
        }
        if (_stricmp(preset, "Preset_Default") == 0)
        {
            _newFx[3] = 1069463634;
            _newFx[4] = 1062501089;
            _newFx[6] = 1004888130;
            _newFx[8] = 1010055512;
            _newFx[0] = -1000;
            _newFx[1] = -100;
            _newFx[5] = -2602;
            _newFx[7] = 200;
            _newFx[9] = 1120403456;
            _newFx[10] = 1120403456;
            goto LABEL_61;
        }
        if (_stricmp(preset, "Preset_Forest") == 0)
        {
            _newFx[3] = 1069463634;
            _newFx[4] = 1057635697;
            _newFx[6] = 1042670420;
            _newFx[8] = 1035221336;
            _newFx[9] = 1117650944;
            v5 = 1120403456;
            _newFx[0] = -1000;
            _newFx[1] = -3300;
            _newFx[5] = -2560;
            _newFx[7] = -613;
            goto LABEL_58;
        }
        if (_stricmp(preset, "Preset_Generic") == 0)
        {
            _newFx[3] = 1069463634;
            v3 = 1062501089;
            _newFx[1] = -100;
            _newFx[5] = -2602;
            _newFx[7] = 200;
            goto LABEL_55;
        }
        if (_stricmp(preset, "Preset_Hallway") == 0)
        {
            _newFx[3] = 1069463634;
            v3 = 1058474557;
            _newFx[1] = -300;
            _newFx[5] = -1219;
            _newFx[7] = 441;
            goto LABEL_55;
        }
        if (_stricmp(preset, "Preset_Hangar") == 0)
        {
            _newFx[3] = 1092668621;
            _newFx[4] = 1047233823;
            _newFx[6] = 1017370378;
            v4 = 1022739087;
            _newFx[0] = -1000;
            _newFx[1] = -1000;
            _newFx[5] = -602;
            _newFx[7] = 198;
            goto LABEL_57;
        }
        if (_stricmp(preset, "Preset_LivingRoom") == 0)
        {
            _newFx[3] = 1056964608;
            _newFx[4] = 1036831949;
            _newFx[6] = 994352038;
            v4 = 998445679;
            _newFx[1] = -6000;
            _newFx[5] = -1376;
            _newFx[7] = -1104;
            goto LABEL_56;
        }
        if (_stricmp(preset, "Preset_Mountains") == 0)
        {
            _newFx[3] = 1069463634;
            _newFx[4] = 1045891645;
            _newFx[6] = 1050253722;
            _newFx[8] = 1036831949;
            _newFx[9] = 1104674816;
            v5 = 1120403456;
            _newFx[0] = -1000;
            _newFx[1] = -2500;
            _newFx[5] = -2780;
            _newFx[7] = -2014;
            goto LABEL_58;
        }
        if (_stricmp(preset, "Preset_NoReverb") != 0)
        {
            if (_stricmp(preset, "Preset_PaddedCell") == 0)
            {
                _newFx[3] = 1043207291;
                _newFx[4] = 1036831949;
                _newFx[6] = 981668463;
                v4 = 990057071;
                _newFx[1] = -6000;
                _newFx[5] = -1204;
                _newFx[7] = 207;
                goto LABEL_56;
            }
            if (_stricmp(preset, "Preset_ParkingLot") == 0)
            {
                _newFx[3] = 1070805811;
                _newFx[4] = 1069547520;
                _newFx[6] = 1006834287;
                v4 = 1011129254;
                _newFx[1] = 0;
                _newFx[5] = -1363;
                _newFx[7] = -1153;
                goto LABEL_56;
            }
            if (_stricmp(preset, "Preset_Plain") == 0)
            {
                _newFx[3] = 1069463634;
                _newFx[4] = 1056964608;
                _newFx[6] = 1043811271;
                _newFx[8] = 1036831949;
                _newFx[9] = 1101529088;
                v5 = 1120403456;
                _newFx[0] = -1000;
                _newFx[1] = -2000;
                _newFx[5] = -2466;
                _newFx[7] = -2514;
                goto LABEL_58;
            }
            if (_stricmp(preset, "Preset_Quarry") == 0)
                goto LABEL_43;
            if (_stricmp(preset, "Preset_Room") == 0)
            {
                _newFx[3] = 1053609165;
                _newFx[4] = 1062501089;
                _newFx[6] = 990057071;
                v4 = 994352038;
                _newFx[1] = -454;
                _newFx[5] = -1646;
                _newFx[7] = 53;
                goto LABEL_56;
            }
            if (_stricmp(preset, "Preset_SewerPipe") == 0)
            {
                _newFx[3] = 1077139210;
                _newFx[4] = 1041194025;
                _newFx[6] = 1013276738;
                _newFx[8] = 1017907249;
                _newFx[9] = 1117782016;
                v5 = 1114636288;
                _newFx[0] = -1000;
                _newFx[1] = -1000;
                _newFx[5] = 429;
                _newFx[7] = 648;
                goto LABEL_58;
            }
            if (_stricmp(preset, "Preset_Quarry") == 0)
            {
            LABEL_43:
                _newFx[3] = 1069463634;
                _newFx[4] = 1062501089;
                _newFx[6] = 1031396131;
                v4 = 1020054733;
                _newFx[0] = -1000;
                _newFx[1] = -1000;
                _newFx[5] = -10000;
                _newFx[7] = 500;
                goto LABEL_57;
            }
            if (_stricmp(preset, "Preset_StoneRoom") == 0)
            {
                _newFx[3] = 1075042058;
                _newFx[4] = 1059313418;
                _newFx[6] = 1011129254;
                v4 = 1015759766;
                _newFx[1] = -300;
                _newFx[5] = -711;
                _newFx[7] = 83;
                goto LABEL_56;
            }
            if (_stricmp(preset, "Preset_StoneCorridor") == 0)
            {
                _newFx[3] = 1076677837;
                _newFx[4] = 1061830001;
                _newFx[6] = 1012202996;
                v4 = 1017370378;
                _newFx[1] = -237;
                _newFx[5] = -1214;
                _newFx[7] = 395;
                goto LABEL_56;
            }
            if (_stricmp(preset, "Preset_Underwater") == 0)
            {
                _newFx[3] = 1069463634;
                v3 = 1036831949;
                _newFx[1] = -4000;
                _newFx[5] = -449;
                _newFx[7] = 1700;
                goto LABEL_55;
            }
        }
        _newFx[1] = -10000;
        _newFx[3] = 1065353216;
        _newFx[4] = 1065353216;
        _newFx[6] = 0;
        memset(&_newFx[8], 0, 12);
        goto LABEL_60;
    LABEL_55:
        _newFx[4] = v3;
        _newFx[6] = 1004888130;
        v4 = 1010055512;
        goto LABEL_56;
    LABEL_56:
        _newFx[0] = -1000;
        goto LABEL_57;
    LABEL_57:
        _newFx[8] = v4;
        v5 = 1120403456;
        _newFx[9] = 1120403456;
        goto LABEL_58;
    LABEL_58:
        _newFx[10] = v5;
        _newFx[12] = 25;
        goto LABEL_62;
    LABEL_60:
        _newFx[0] = -10000;
        _newFx[5] = -10000;
        _newFx[7] = -10000;
        goto LABEL_61;
    LABEL_61:
        _newFx[12] = -1;
        goto LABEL_62;
    LABEL_62:
        _newFx[13] = 0;
        _newFx[11] = 1167867904;
        _newFx[2] = 0;
        memcpy(this->mTargetReverb, _newFx, sizeof(this->mTargetReverb));
        memcpy(this->mCurrentReverb, _newFx, sizeof(this->mCurrentReverb));
        this->mUpdateReverb = true;
        this->mRemainingReverbBlendTime = 0.0f;
    }
}

// ea: 0x0062C020
SoundDevice::Sound::~Sound()
{
    nslSourceState SourceState =
        nslGetSourceState((nslSourceID)this->mSource);
    if (this->mSource != NSL_SOURCE_ID_INVALID
        && (this->mPaused || SourceState == NSL_SOURCE_STATE_PLAYING
            || SourceState == NSL_SOURCE_STATE_QUEUING
            || SourceState == NSL_SOURCE_STATE_QUEUED
            || SourceState == NSL_SOURCE_STATE_PAUSED))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 288;
        AeAssert::gCurrentExpr =
            "mSource == NSL_SOURCE_ID_INVALID || IsFinished()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("source destructed while still playing"))
            __debugbreak();
    }
    unsigned int v3 = this->mEntHandle.mVal & 0xFFF;
    if (v3 < 0x540
        && this->mEntHandle.mVal >> 12
            == EntityHandleDb::sInst.mElements[v3].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v3].mObject;
        if (mObject != nullptr && this->mDialogNotify.mHash != 0)
            mObject->Notify(this->mDialogNotify);
    }
}

// ea: 0x0062C0D0
void SoundDevice::Sound::Stop()
{
    if (this->mSource != NSL_SOURCE_ID_INVALID)
    {
        nslWaveGetHash((nslWaveID)this->mWave);
        if (this->mSource == g_break_on_stop)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
            AeAssert::gCurrentLine = 421;
            AeAssert::gCurrentExpr = "mSource != g_break_on_stop";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("no!"))
                __debugbreak();
        }
        SoundDevice::SoundHandleDb::sInst.ReleaseHandle(this->mHandle);
        this->mHandle.mVal = 0;
        nslStopSource((nslSourceID)this->mSource);
        nslFreeSource((nslSourceID)this->mSource);
        this->mSource = NSL_SOURCE_ID_INVALID;
    }
    unsigned int v2 = this->mEntHandle.mVal & 0xFFF;
    if (v2 < 0x540
        && this->mEntHandle.mVal >> 12
            == EntityHandleDb::sInst.mElements[v2].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v2].mObject;
        if (mObject != nullptr)
        {
            HashString v4;
            v4.mHash = nslWaveGetHash((nslWaveID)this->mWave);
            EffectEventSys::sInst->SendSpecificSoundNotify(mObject, v4);
            if (this->mDialogNotify.mHash != 0)
                mObject->Notify(this->mDialogNotify);
        }
    }
    this->mMinRange = 50.0f;
    this->mPaused = false;
    this->mDialogNotify.mHash = 0;
    this->mPoPtr = nullptr;
    this->mSource = NSL_SOURCE_ID_INVALID;
    this->mWave = NSL_WAVE_ID_INVALID;
    this->mAutoRelease = true;
    this->mPitch = 1.0f;
    this->mVolume = 1.0f;
    this->mMaxRange = 1500.0f;
    this->mGroupVolume = 1.0f;
}

// ea: 0x0062C210
void SoundDevice::Sound::SetVolume(float vol)
{
    if (this->mSource == NSL_SOURCE_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 474;
        AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
            __debugbreak();
    }
    if (this->mSource != NSL_SOURCE_ID_INVALID)
    {
        if ((__fpclass(vol) & 0x297) != 0)
        {
            const char* Name = nslWaveGetName((nslWaveID)this->mWave);
            tlWarning("A NAN was passed into the sound system while trying to adjust the volume on %s\n", Name);
            this->Stop();
        }
        else
        {
            nslSetSourceParam((nslSourceID)this->mSource, 0,
                              this->mGroupVolume * vol);
            this->mVolume = vol;
        }
    }
}

// ea: 0x0062C2D0
void SoundDevice::Sound::SetPitch(float pitch)
{
    if ((__fpclass(pitch) & 0x297) != 0)
    {
        const char* Name = nslWaveGetName((nslWaveID)this->mWave);
        tlWarning("A NAN was passed into the sound system while trying to adjust the pitch on %s\n", Name);
        this->Stop();
    }
    else
    {
        nslSourceID mSource = (nslSourceID)this->mSource;
        if (this->mSource == NSL_SOURCE_ID_INVALID)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
            AeAssert::gCurrentLine = 528;
            AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
                __debugbreak();
            mSource = (nslSourceID)this->mSource;
        }
        if (this->mSource != NSL_SOURCE_ID_INVALID)
        {
            nslSetSourceParam(mSource, 1, pitch);
            this->mPitch = pitch;
        }
    }
}

// ea: 0x0062C380
void SoundDevice::Sound::SetRange(float min, float max)
{
    if ((__fpclass(min) & 0x297) != 0 || (__fpclass(max) & 0x297) != 0)
    {
        const char* Name = nslWaveGetName((nslWaveID)this->mWave);
        tlWarning("A NAN was passed into the sound system while trying to adjust the min or max on %s\n", Name);
        this->Stop();
    }
    else
    {
        if (this->mSource == NSL_SOURCE_ID_INVALID)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
            AeAssert::gCurrentLine = 552;
            AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
                __debugbreak();
        }
        if (this->mSource != NSL_SOURCE_ID_INVALID)
        {
            nslSetSourceParam((nslSourceID)this->mSource, 25, min);
            nslSetSourceParam((nslSourceID)this->mSource, 26, max);
            this->mMinRange = min;
            this->mMaxRange = max;
        }
    }
}

// ea: 0x0062C470
void SoundDevice::Sound::SetPosition(const math::Position3& pos)
{
    if (this->mSource == NSL_SOURCE_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 566;
        AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
            __debugbreak();
    }
    if (this->mSource != NSL_SOURCE_ID_INVALID)
    {
        float v5[3];
        v5[0] = -pos.v.m128_f32[1];
        v5[1] = pos.v.m128_f32[2];
        v5[2] = pos.v.m128_f32[0];
        if ((__fpclass(v5[0]) & 0x297) != 0 || (__fpclass(v5[1]) & 0x297) != 0
            || (__fpclass(v5[2]) & 0x297) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
            AeAssert::gCurrentLine = 574;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning(
                    "Invalid position detected while setting sound position- this is fatal on xbox"))
                __debugbreak();
            this->Stop();
        }
        else
        {
            nslSetSourcePosition((nslSourceID)this->mSource, v5);
            this->mDebugPos[0] = pos.v.m128_f32[0];
            this->mDebugPos[1] = pos.v.m128_f32[1];
            this->mDebugPos[2] = pos.v.m128_f32[2];
        }
    }
}

// ea: 0x0062C630
void SoundDevice::Sound::SetVelocity(const math::Dir3& vel)
{
    if (this->mSource == NSL_SOURCE_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 590;
        AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
            __debugbreak();
    }
    if (this->mSource != NSL_SOURCE_ID_INVALID)
    {
        float v5[3];
        v5[0] = vel.v.m128_f32[0];
        v5[1] = vel.v.m128_f32[1];
        v5[2] = vel.v.m128_f32[2];
        if ((__fpclass(v5[0]) & 0x297) != 0 || (__fpclass(v5[1]) & 0x297) != 0
            || (__fpclass(v5[2]) & 0x297) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
            AeAssert::gCurrentLine = 596;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning(
                    "Invalid position detected while setting sound velocity- this is fatal on xbox"))
                __debugbreak();
            this->Stop();
        }
        else
        {
            nslSetSourceVelocity((nslSourceID)this->mSource, v5);
        }
    }
}

// ea: 0x0062C7A0
void SoundDevice::Sound::Update()
{
    if (this->mSource == NSL_SOURCE_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 614;
        AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
            __debugbreak();
    }
    if (this->mSource != NSL_SOURCE_ID_INVALID)
    {
        this->SetVolume(this->mVolume);
        this->SetPitch(this->mPitch);
        this->SetRange(this->mMinRange, this->mMaxRange);
        unsigned int v3 = this->mEntHandle.mVal & 0xFFF;
        Entity* mObject = nullptr;
        if (v3 < 0x540
            && this->mEntHandle.mVal >> 12
                == EntityHandleDb::sInst.mElements[v3].mKey)
            mObject = EntityHandleDb::sInst.mElements[v3].mObject;
        const math::Mat43* mPoPtr = (const math::Mat43*)this->mPoPtr;
        if (mPoPtr != nullptr)
        {
            if ((__fpclass(mPoPtr->w.v.m128_f32[0]) & 0x297) == 0
                && (__fpclass(mPoPtr->w.v.m128_f32[1]) & 0x297) == 0
                && (__fpclass(mPoPtr->w.v.m128_f32[2]) & 0x297) == 0)
            {
                this->SetPosition(mPoPtr->w);
                math::Dir3 zeroVel;
                zeroVel.v = _mm_setzero_ps();
                this->SetVelocity(zeroVel);
                return;
            }
            tlWarning("A NAN was passed into the sound system update an sound position\n");
            goto LABEL_22;
        }
        if (mObject != nullptr)
        {
            math::Position3 v10;
            v10.v = mObject->r.currentOrigin.v;
            if ((__fpclass(v10.v.m128_f32[0]) & 0x297) == 0
                && (__fpclass(v10.v.m128_f32[1]) & 0x297) == 0
                && (__fpclass(v10.v.m128_f32[2]) & 0x297) == 0)
            {
                this->SetPosition(v10);
                math::Dir3 zeroVel;
                zeroVel.v = _mm_setzero_ps();
                this->SetVelocity(zeroVel);
                return;
            }
            tlWarning("A NAN was passed into the sound system update an entity position\n");
        LABEL_22:
            this->Stop();
        }
    }
}

// ea: 0x0062C9B0
void SoundDevice::ReleaseSound(Sound* s)
{
    s->Stop();
}

// ea: 0x0062C9C0
void SoundDevice::ReleaseSound(
    DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> s)
{
    unsigned int v2 = s.mHandle.mVal & 0xFFF;
    if (v2 < 0x200
        && s.mHandle.mVal >> 12
            == SoundDevice::SoundHandleDb::sInst.mElements[v2].mKey
        && SoundDevice::SoundHandleDb::sInst.mElements[v2].mObject != nullptr)
    {
        Sound* mObject = nullptr;
        if ((s.mHandle.mVal & 0xFFF) < 0x200
            && s.mHandle.mVal >> 12
                == SoundDevice::SoundHandleDb::sInst.mElements[v2].mKey)
            mObject = SoundDevice::SoundHandleDb::sInst.mElements[v2].mObject;
        mObject->Stop();
    }
}

// ea: 0x0062CA10
void SoundDevice::StopAllSoundsNotPaused()
{
    Sound* s = this->mSounds;
    for (int i = 512; i != 0; --i)
    {
        if (!s->mPaused)
            s->Stop();
        ++s;
    }
}

// ea: 0x0062CA40
void SoundDevice::UpdateCrossFade(float deltaTime)
{
    for (int i = 16; i != 0; --i)
    {
        CrossFadeInfo* info = &this->mCrossFadeInfo[16 - i];
        if (info->mRemainingTime > 0.0f)
        {
            info->mRemainingTime -= deltaTime;
            if (info->mRemainingTime < 0.0f)
                info->mRemainingTime = 0.0f;
            unsigned int v4 = info->mSound1.mVal & 0xFFF;
            Sound* mObject = nullptr;
            float adjustVolume1 = info->mAdjustVolume1 * deltaTime;
            float adjustVolume2 = info->mAdjustVolume2 * deltaTime;
            if (v4 < 0x200
                && info->mSound1.mVal >> 12
                    == SoundDevice::SoundHandleDb::sInst.mElements[v4].mKey)
                mObject = SoundDevice::SoundHandleDb::sInst.mElements[v4].mObject;
            unsigned int v8 = info->mSound2.mVal & 0xFFF;
            Sound* v9 = nullptr;
            if (v8 < 0x200
                && info->mSound2.mVal >> 12
                    == SoundDevice::SoundHandleDb::sInst.mElements[v8].mKey)
                v9 = SoundDevice::SoundHandleDb::sInst.mElements[v8].mObject;
            if (info->mRemainingTime <= 0.0f)
            {
                if (mObject != nullptr)
                {
                    mObject->SetVolume(0.0f);
                    mObject->Stop();
                }
                if (v9 != nullptr)
                    v9->SetVolume(1.0f);
            }
            else
            {
                if (mObject != nullptr)
                {
                    float v10;
                    if (mObject->mSource == NSL_SOURCE_ID_INVALID)
                        v10 = -2.0f;
                    else
                        v10 = nslGetSourceParam((nslSourceID)mObject->mSource,
                                                0, -1.0f);
                    float newVolume = v10 + adjustVolume1;
                    if (newVolume < 0.0f)
                        newVolume = 0.0f;
                    mObject->SetVolume(newVolume);
                }
                if (v9 != nullptr)
                {
                    float v11;
                    if (v9->mSource == NSL_SOURCE_ID_INVALID)
                        v11 = -2.0f;
                    else
                        v11 = nslGetSourceParam((nslSourceID)v9->mSource, 0,
                                                -1.0f);
                    float v15 = v11 + adjustVolume2;
                    if (v15 > 1.0f)
                        v15 = 1.0f;
                    v9->SetVolume(v15);
                }
            }
        }
    }
}

// ea: 0x0062CBE0
void SoundDevice::CrossFade(unsigned int sound1, unsigned int sound2,
                            float crossFadeTime)
{
    int v4 = -1;
    int v6 = 0;
    CrossFadeInfo* info = this->mCrossFadeInfo;
    while (v4 == -1)
    {
        if (info[0].mRemainingTime == 0.0f)
        {
            v4 = v6;
            break;
        }
        if (info[1].mRemainingTime == 0.0f)
        {
            v4 = v6 + 1;
            break;
        }
        if (info[2].mRemainingTime == 0.0f)
        {
            v4 = v6 + 2;
            break;
        }
        if (info[3].mRemainingTime == 0.0f)
        {
            v4 = v6 + 3;
            break;
        }
        if (info[4].mRemainingTime == 0.0f)
        {
            v4 = v6 + 4;
            break;
        }
        if (info[5].mRemainingTime == 0.0f)
        {
            v4 = v6 + 5;
            break;
        }
        if (info[6].mRemainingTime == 0.0f)
        {
            v4 = v6 + 6;
            break;
        }
        if (info[7].mRemainingTime == 0.0f)
            v4 = v6 + 7;
        v6 += 8;
        info += 8;
        if (v6 >= 16)
            break;
    }
    unsigned int v9 = sound1 & 0xFFF;
    Sound* mObject = nullptr;
    if (v9 < 0x200
        && sound1 >> 12 == SoundDevice::SoundHandleDb::sInst.mElements[v9].mKey)
        mObject = SoundDevice::SoundHandleDb::sInst.mElements[v9].mObject;
    unsigned int v11 = sound2 & 0xFFF;
    Sound* sound1a = nullptr;
    if (v11 < 0x200
        && sound2 >> 12 == SoundDevice::SoundHandleDb::sInst.mElements[v11].mKey)
        sound1a = SoundDevice::SoundHandleDb::sInst.mElements[v11].mObject;
    if (v4 > -1)
    {
        CrossFadeInfo* v12 = &this->mCrossFadeInfo[v4];
        v12->mSound1.mVal = sound1;
        v12->mSound2.mVal = sound2;
        v12->mAdjustVolume1 = 0.0f;
        v12->mAdjustVolume2 = 0.0f;
        if (mObject != nullptr)
            v12->mAdjustVolume1 =
                (-1.0f / crossFadeTime) * mObject->mVolume;
        if (sound1a != nullptr)
        {
            v12->mAdjustVolume2 = sound1a->mVolume / crossFadeTime;
            sound1a->SetVolume(0.0f);
        }
        v12->mRemainingTime = crossFadeTime;
    }
}



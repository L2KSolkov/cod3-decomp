// ============================================================================
// g_entity_misc.cpp - game.o stat monitor + Entity DObj/enemy helpers
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

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
math::Mat43 Entity::CalcRotTranMat43()
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

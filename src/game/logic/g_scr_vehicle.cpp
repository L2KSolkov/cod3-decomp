// ============================================================================
// g_scr_vehicle.cpp - scripted vehicle stubs (g.o: g_scr_vehicle.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <string.h>

// ea: 0x00452BC0
void VehicleNodeAllocator::Initialize()
{
    m_numNodes = 0;
    m_numBlocks = 0;
    m_currentBlockIndex = 0;
    for (int i = 0; i < 16; ++i)
        m_pNodeBlocks[i] = nullptr;
}

// ea: 0x00452C10
vehicle_node_t* VehicleNodeAllocator::AllocNode()
{
    int16_t m_numBlocks = this->m_numBlocks;
    if (m_numBlocks == 0 || this->m_currentBlockIndex >= 128)
    {
        if (m_numBlocks >= 16)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_vehicle_path.cpp";
            AeAssert::gCurrentLine = 1605;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Out of vehicle Nodes - Tell MikeA"))
                __debugbreak();
        }
        this->m_pNodeBlocks[this->m_numBlocks] = mem_heap_malloc(16, 0x2000u);
        vehicle_node_t* v3 = (vehicle_node_t*)this->m_pNodeBlocks[this->m_numBlocks];
        memset(v3, 0, 0x2000);
        for (int i = 128; i != 0; --i)
        {
            if (v3 != nullptr)
            {
                v3->mName = Broc::string();
                v3->mTarget = Broc::string();
                v3->script_noteworthy = Broc::string();
            }
            ++v3;
        }
        ++this->m_numBlocks;
        this->m_currentBlockIndex = 0;
    }
    uint16_t m_currentBlockIndex = this->m_currentBlockIndex;
    vehicle_node_t* result =
        (vehicle_node_t*)this->m_pNodeBlocks[this->m_numBlocks] + m_currentBlockIndex;
    this->m_currentBlockIndex = m_currentBlockIndex + 1;
    s_nodes[this->m_numNodes++] = result;
    return result;
}

// ea: 0x0045F2B0
void VehicleNodeAllocator::FreeAll()
{
    if (this->m_numBlocks > 0)
    {
        for (int i = 0; i < this->m_numBlocks; ++i)
        {
            vehicle_node_t* v3 = (vehicle_node_t*)m_pNodeBlocks[i];
            for (int j = 128; j != 0; --j)
                v3++->~vehicle_node_t();
            mem_heap_free(m_pNodeBlocks[i]);
        }
    }
    m_numNodes = 0;
    m_numBlocks = 0;
    m_currentBlockIndex = 0;
    for (int i = 0; i < 16; ++i)
        m_pNodeBlocks[i] = nullptr;
}

// ea: 0x0044D8C0
void VEH_SetupCollmap(Entity* ent)
{
    ent->s.brushmodel = 0;
    SV_SetBrushModel(ent);
    ent->r.contents = 0xA00000;
}

// ea: 0x0044F3A0
float VEH_GetMaxSpeed(scr_vehicle_t* veh)
{
    rb_vehicle* mRBVeh = (rb_vehicle*)veh->mRBVeh;
    if (mRBVeh != nullptr)
        return *(float*)(*(void**)((char*)mRBVeh + 0x250));  // m_parameter->m_speed_max
    return s_vehicleInfos[veh->infoIdx]->maxSpeed;
}

// ea: 0x0044F520
int scr_vehicle_t::GetStageAnim(Client* client)
{
    return animMap->stages[animMap->routes[client->mVehicleAnimRoute].stages[client->mVehicleAnimStage]].animRow;
}

// ea: 0x00452840
void G_VehFreePathPos(vehicle_pathpos_t* vpp)
{
    vpp->switchNode[0].mName.clear();
    vpp->switchNode[0].mTarget.clear();
    vpp->switchNode[1].mName.clear();
    vpp->switchNode[1].mTarget.clear();
}

// ea: 0x0045E900
vehicle_info_t* G_GetVehicleInfo(Entity* ent)
{
    if (ent != nullptr && ent->scr_vehicle != nullptr)
        return s_vehicleInfos[ent->scr_vehicle->infoIdx];
    return nullptr;
}

// ea: 0x00457F90
void G_FreeAnimTreeInstances(void)
{
    for (int i = 0; i < 16; ++i)
        g_scr_data.actorCorpseInfo[i].mEntity.mHandle.mVal = 0;
}

// ea: 0x00490EA0
void VEH_PlayerInteractionEntry(Entity* vehicle)
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    VEH_LinkPlayer(vehicle, Player, 0, 0, 0);
}

// ea: 0x0044C7E0
void UpdatePaths(Entity* ent)
{
    if ((ent->flags & 0x1000) != 0)
    {
        if (ent->moverState == 7)
        {
            if (ent->key != 0)
            {
                PathNodeMgr::sInst->DisconnectPathsForEntity(ent);
                return;
            }
        }
        else if (ent->moverState == 8)
        {
            PathNodeMgr::sInst->DisconnectPathsForEntity(ent);
            return;
        }
        PathNodeMgr::sInst->ConnectPathsForEntity(ent);
    }
}

// ea: 0x0044D320
int IsVehicleTank(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    return scr_vehicle != nullptr && s_vehicleInfos[scr_vehicle->infoIdx]->type == 2;
}

// ea: 0x0044EFD0
void G_FreeScrVehicleInfo(void)
{
    for (int i = 0; i < s_numVehicleInfos; mem_heap_free(s_vehicleInfos[i++]))
        ;
    s_numVehicleInfos = 0;
}

// ea: 0x0045E240
void G_FreeScrVehicles(void)
{
    if (level.MaxVehicles != 0)
    {
        for (int v0 = 0; v0 < level.MaxVehicles; ++v0)
        {
            Broc::string* v2 = (Broc::string*)&s_vehicles[v0];
            v2[14].clear();
            v2[15].clear();
            v2[30].clear();
            v2[31].clear();
        }
    }
}

// ea: 0x004523F0
int G_FreeVehiclePaths(void)
{
    int result = s_numNodes;
    int v1 = 0;
    if (s_numNodes > 0)
    {
        result = 0;
        do
        {
            vehicle_node_t* v2 = s_nodes[result];
            v2->mName.clear();
            v2->mTarget.clear();
            result = ++v1;
        } while (v1 < s_numNodes);
    }
    s_numNodes = 0;
    return result;
}

// ea: 0x0046F300
bool scr_vehicle_t::IsPhysicsPaused()
{
    rb_vehicle* mRBVeh = (rb_vehicle*)this->mRBVeh;
    if (mRBVeh != nullptr)
        return (*(unsigned int*)((char*)mRBVeh + 0x280) & 1) != 0;  // m_flags.mMask
    Entity* mObject = HandleDbToEnt(
        *(DbLinkedHandle<EntityHandleDb, Entity>*)((char*)this + 0x1E0));  // seats[0].occupant
    return mObject == nullptr;
}

// ea: 0x004890C0
void scr_vehicle_t::CollisionDamage(Entity* ent, const math::Position3* pos,
                                    const math::Position3* dir, float intensity)
{
    float damage = *(float*)((char*)s_vehicleInfos[infoIdx] + 0x68) * intensity;
    G_Damage(ent, nullptr, nullptr, dir->v.m128_f32, pos->v.m128_f32,
             (int)damage, 32, 27, HITLOC_NONE, -1);
}

// ea: 0x0044FB30
vehicle_info_t* VEH_GetVehicleInfo(unsigned int iIndex)
{
    if (iIndex > 0x40)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10667;
        AeAssert::gCurrentExpr = "(iIndex >= 0) && (iIndex <= 64)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return s_vehicleInfos[iIndex];
}

// ea: 0x0044F620
int scr_vehicle_t::GetSwitchPosRoute(int seatIdx, int fromPos, bool hasFlag)
{
    vehicleAnimMap_t* animMap = this->animMap;
    if (animMap == nullptr)
        return -1;
    int numRoutes = animMap->numRoutes;
    int result = 0;
    if (numRoutes <= 0)
        return -1;
    for (vehicleAnimRoute_t* i = animMap->routes;
         i->vehPosSrc != fromPos || i->vehPosDest != seatIdx
             || ((i->flags & 4) != 0 && !hasFlag);
         ++i)
    {
        if (++result >= numRoutes)
            return -1;
    }
    return result;
}

// ea: 0x0044D480
int VEH_GetVehicleInfo(const char* name)
{
    if (name == nullptr || *name == 0)
        return -1;
    int v1 = 0;
    if (s_numVehicleInfos <= 0)
        return -1;
    while (_stricmp(name, s_vehicleInfos[v1]->name.c_str()) != 0)
    {
        if (++v1 >= s_numVehicleInfos)
            return -1;
    }
    return v1;
}

// ea: 0x0044D4E0
int16_t VEH_GetPlayerVehicleInfo(const char* name)
{
    if (name == nullptr || *name == 0)
        return -1;
    int16_t v1 = 0;
    if (s_numVehicleInfos <= 0)
        return -1;
    while (_stricmp(name, s_vehicleInfos[v1]->name.c_str()) != 0)
    {
        if (++v1 >= s_numVehicleInfos)
            return -1;
    }
    return v1;
}

// ea: 0x0046A370
void VEH_SetPosition(Entity* ent, const math::Position3* origin,
                     const math::Position3* angles, const float* vel)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    if (ent->takedamage != 0)
    {
        ent->s.pos.trBase[0] = ent->r.currentOrigin.v.m128_f32[0];
        ent->s.pos.trBase[1] = ent->r.currentOrigin.v.m128_f32[1];
        ent->s.pos.trBase[2] = ent->r.currentOrigin.v.m128_f32[2];
        ent->s.pos.trDelta[0] = origin->v.m128_f32[0];
        ent->s.pos.trDelta[1] = origin->v.m128_f32[1];
        ent->s.pos.trDelta[2] = origin->v.m128_f32[2];
        if (IS_NAN(ent->r.currentOrigin.v.m128_f32[0])
            || IS_NAN(ent->r.currentOrigin.v.m128_f32[1])
            || IS_NAN(ent->r.currentOrigin.v.m128_f32[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 1292;
            AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        ent->r.currentOrigin.v.m128_f32[0] = origin->v.m128_f32[0];
        ent->r.currentOrigin.v.m128_f32[1] = origin->v.m128_f32[1];
        ent->r.currentOrigin.v.m128_f32[2] = origin->v.m128_f32[2];
        ent->s.apos.trBase[0] = ent->r.currentAngles.v.m128_f32[0];
        ent->s.apos.trBase[1] = ent->r.currentAngles.v.m128_f32[1];
        ent->s.apos.trBase[2] = ent->r.currentAngles.v.m128_f32[2];
        ent->s.apos.trDelta[0] = angles->v.m128_f32[0];
        ent->s.apos.trDelta[1] = angles->v.m128_f32[1];
        ent->s.apos.trDelta[2] = angles->v.m128_f32[2];
        ent->r.currentAngles.v.m128_f32[0] = angles->v.m128_f32[0];
        ent->r.currentAngles.v.m128_f32[1] = angles->v.m128_f32[1];
        ent->r.currentAngles.v.m128_f32[2] = angles->v.m128_f32[2];
        ent->s.pos.trType = TR_INTERPOLATE;
        ent->s.apos.trType = TR_INTERPOLATE;
        if (ent->takedamage != 0)
            g_LinkEntity(ent);
        Entity* v7 = HandleDbToEnt(scr_vehicle->mIdleSndEnt);
        if (v7 != nullptr)
        {
            G_SetOrigin(v7, origin);
            G_SetAngle(v7, angles);
            v7->s.pos.trType = TR_INTERPOLATE;
            v7->s.apos.trType = TR_INTERPOLATE;
            g_LinkEntity(v7);
        }
        Entity* v9 = HandleDbToEnt(scr_vehicle->mEngineSndEnt);
        if (v9 != nullptr)
        {
            G_SetOrigin(v9, origin);
            G_SetAngle(v9, angles);
            v9->s.pos.trType = TR_INTERPOLATE;
            v9->s.apos.trType = TR_INTERPOLATE;
            g_LinkEntity(v9);
        }
        if ((ent->flags & 0x20000000) != 0)
            G_SetEntityOceanHeight(ent);
    }
    else
    {
        SV_UnlinkEntity(ent);
    }
}

// ea: 0x0046DBF0
void G_SetupScrVehicles(void)
{
    int v0 = 0;
    if (level.MaxVehicles != 0)
    {
        int v1 = 0;
        do
        {
            unsigned int mVal = s_vehicles[v1].mEntity.mHandle.mVal;
            if (mVal != 0)
            {
                Entity* Entity = VEH_GetEntity(mVal);
                Entity->s.brushmodel = 0;
                SV_SetBrushModel(Entity);
                Entity->r.contents = 0xA00000;
            }
            v1 = ++v0;
        } while (v0 < level.MaxVehicles);
    }
}

// ea: 0x0045F350
void shotgunrandom(float* x, float* y, float randomA, float randomB)
{
    float sinT;
    FastSinCos(((randomA * 360.0f) * 3.1415927f) * 0.0055555557f, &sinT, &randomA);
    *x = randomA * randomB;
    *y = sinT * randomB;
}

// ea: 0x0046F2A0
bool scr_vehicle_t::IsPhysicsStable()
{
    rb_vehicle* mRBVeh = (rb_vehicle*)this->mRBVeh;
    if (mRBVeh != nullptr)
    {
        rb_extra_info* m_chassis_rbinf = *(rb_extra_info**)((char*)mRBVeh + 0x274);  // m_chassis_rbinf
        if (m_chassis_rbinf != nullptr)
            return (*(unsigned int*)((char*)m_chassis_rbinf->m_rb + 0x280) & 4) != 0;
    }
    Entity* mObject = HandleDbToEnt(
        *(DbLinkedHandle<EntityHandleDb, Entity>*)((char*)this + 0x1E0));  // seats[0].occupant
    return mObject == nullptr;
}

// ea: 0x0044F330
bool IsVehFlipped(Entity* ent)
{
    bool result = false;
    if (ent != nullptr)
    {
        scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
        if (scr_vehicle != nullptr
            && scr_vehicle->mRBVeh != nullptr
            && ent->r.currentMat.z.v.m128_f32[2] < 0.2f)
        {
            return true;
        }
    }
    return result;
}

// ea: 0x0044F720
int scr_vehicle_t::GetMantleHintStringIndex()
{
    if (s_vehicleInfos[infoIdx] == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10113;
        AeAssert::gCurrentExpr = "s_vehicleInfos[ infoIdx ]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid info pointer in vehicle"))
            __debugbreak();
    }
    return s_vehicleInfos[infoIdx]->mMantleHintStringIndex;
}

// ea: 0x0046F350
bool scr_vehicle_t::IsOppositeTeamInVehicle(int team)
{
    for (int v2 = 0; v2 < 11; ++v2)
    {
        Entity* mObject = HandleDbToEnt(seats[v2].occupant);
        if (mObject != nullptr)
        {
            sentient_s* sentient = mObject->sentient;
            if (sentient != nullptr && sentient->eTeam != team)
                return true;
        }
    }
    return false;
}

// ea: 0x0044F6A0
void scr_vehicle_t::Mantled(Entity* player)
{
    if (mMantleTime != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10101;
        AeAssert::gCurrentExpr = "mMantleTime == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Vehicle has already been mantled"))
            __debugbreak();
    }
    mMantleTime = level.time + 1800;
    mMantleEntity.mHandle.mVal = player->mHandle.mHandle.mVal;
}

// ea: 0x0045E1D0
int G_InitScrVehicles(void)
{
    int result = 0;
    if (level.MaxVehicles != 0)
    {
        int v0 = 0;
        int v1 = 0;
        do
        {
            int v2 = v1;
            G_VehInitPathPos(&s_vehicles[v2].pathPos);
            ++v0;
            s_vehicles[v2].mEntity.mHandle.mVal = 0;
            result = level.MaxVehicles;
            v1 = v0;
        } while (v0 < level.MaxVehicles);
    }
    level.vehicles = s_vehicles;
    return result;
}

// ea: 0x0044DBD0
void VEH_StopWheelEffects(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* v2 = s_vehicleInfos[scr_vehicle->infoIdx];
    int count = 2 * (v2->type != 1) + 4;
    if (count != -4)
    {
        for (int i = 0; i < count; ++i)
        {
            if (scr_vehicle->mWheel_ParticleEffectHandle[i].mVal != 0)
            {
                EffectEventStopEmitting(scr_vehicle->mWheel_ParticleEffectHandle[i].mVal);
                scr_vehicle->mWheel_ParticleEffectHandle[i].mVal = 0;
            }
        }
    }
    if (scr_vehicle->mRumbleEffectHandle.mVal != 0)
    {
        EffectEventStopEmitting(scr_vehicle->mRumbleEffectHandle.mVal);
        scr_vehicle->mRumbleEffectHandle.mVal = 0;
    }
}

// ea: 0x0046E040
Entity* G_IsVehicleUnusable(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
        return nullptr;
    if ((0x100000 & client->ps.eFlags) == 0)
        return nullptr;
    Entity* v4 = HandleDbToEnt(player->r.mOwner);
    if (v4 == nullptr)
        return nullptr;
    return (0x200000 & v4->r.contents) != 0 ? v4 : nullptr;
}

// ea: 0x0046E200
bool G_IsPlayerVehicleGunner(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
        return false;
    int eFlags = client->ps.eFlags;
    if ((0x100000 & eFlags) == 0 || (0x400000 & eFlags) != 0)
        return false;
    Entity* mOwner = HandleDbToEnt(player->r.mOwner);
    if (mOwner != nullptr && mOwner->scr_vehicle != nullptr)
        return client->ps.vehPos == 1;
    return false;
}

// ea: 0x0046F9F0
int G_EntryPointSeatAssociation(Entity* vehicle, int entryPosition)
{
    scr_vehicle_t* scr_vehicle = vehicle->scr_vehicle;
    if (s_vehicleInfos[scr_vehicle->infoIdx]->type != 2)
        return sEntryPointSeatAssociation[entryPosition];
    Entity* mObject = HandleDbToEnt(scr_vehicle->seats[0].occupant);
    return mObject != nullptr;
}

// ea: 0x00470490
vehicle_info_t* VEH_GetPlayerVehicleInfo(void)
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player == nullptr)
        return nullptr;
    Entity* mObject = HandleDbToEnt(Player->r.mOwner);
    if (mObject == nullptr)
        return nullptr;
    scr_vehicle_t* scr_vehicle = mObject->scr_vehicle;
    if (scr_vehicle == nullptr)
        return nullptr;
    vehicle_info_t* result = s_vehicleInfos[scr_vehicle->infoIdx];
    if (result == nullptr)
        return nullptr;
    return result;
}

// ea: 0x0046F460
bool scr_vehicle_t::LetHatchClose()
{
    if (s_vehicleInfos[infoIdx]->type != 2)
        return true;
    int v2 = 0;
    for (int v4 = 0; v4 < 11; ++v4)
    {
        Entity* mObject = HandleDbToEnt(seats[v4].occupant);
        if (mObject == nullptr)
            continue;
        if (v4 != 0)
        {
            if (v4 == 7 && mObject->client->mVehicleAnimStage < 2)
                continue;
            return false;
        }
        if (!IsPlayerFullySeatedInVehicle(mObject))
            return false;
        v2 = 1;
    }
    return v2 != 0;
}

// ea: 0x0044F2A0
int G_GetVehicleOccupantCount(Entity* ent)
{
    if (ent->scr_vehicle != nullptr)
        return ent->scr_vehicle->playersAttached;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
    AeAssert::gCurrentLine = 7731;
    AeAssert::gCurrentExpr = "ent->scr_vehicle";
    if (AeAssert::IsIgnored() || AeAssert::Assert("G_GetVehicleSeatCount: Entity not a vehicle"))
        __debugbreak();
    return ent->scr_vehicle->playersAttached;
}

// ea: 0x0045E930
int G_GetVehicleSeatCount(Entity* ent)
{
    if (ent->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7725;
        AeAssert::gCurrentExpr = "ent->scr_vehicle";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("G_GetVehicleSeatCount: Entity not a vehicle"))
            __debugbreak();
    }
    if (ent->scr_vehicle != nullptr)
        return s_vehicleInfos[ent->scr_vehicle->infoIdx]->numSeats;
    return 0;
}

// ea: 0x004811F0
void Scr_Vehicle_GetOut(Entity* vehicle, Entity* occupant, int health)
{
    Entity* v3 = occupant;
    if (occupant != nullptr && occupant->IsLocalPlayer())
    {
        int PlayerIndex = occupant->GetPlayerIndex();
        if (InteractionController_Inst(PlayerIndex) != nullptr
            && *(void**)InteractionController_Inst(PlayerIndex) != nullptr)
        {
            InteractionController_EndInteraction(InteractionController_Inst(occupant->GetPlayerIndex()), 1);
        }
    }
    VEH_UnlinkPlayer(v3, true);
    vehicle->health = health;
    if (EntityManager::sInst->IsLocalPlayer(v3))
    {
        unsigned int occupantHandle = v3->mHandle.mHandle.mVal;
        vehicle->Notify(hash_const.deactivate, &occupantHandle);
    }
}

// ea: 0x004807D0
void VEH_UnlinkPlayerDropped(Entity* ent)
{
    if (ent->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6449;
        AeAssert::gCurrentExpr = "ent->scr_vehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    ent->s.eFlags &= 0xFFEFFFFF;
    ent->active = 0;
    ent->r.mOwner.mHandle.mVal = 0;
    Scr_Notify(ent, hash_const.player_off_vehicle, 0);
}

// ea: 0x0046E150
bool G_IsPlayerDrivingVehicle(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
        return false;
    int eFlags = client->ps.eFlags;
    if ((0x100000 & eFlags) == 0 || (0x400000 & eFlags) != 0)
        return false;
    Entity* v3 = HandleDbToEnt(player->r.mOwner);
    if (v3 == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7337;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        return false;
    }
    if (v3->scr_vehicle == nullptr)
        return false;
    return client->ps.vehPos == 0;
}

// ea: 0x004918E0
void Scr_Vehicle_GetIn(Entity* vehicle, Entity* occupant, int health,
                       unsigned int seatIdx, int entryIdx)
{
    unsigned int v5 = seatIdx;
    if (seatIdx == 0)
        vehicle->scr_vehicle->AssignPhysics(occupant);
    VEH_LinkPlayer(vehicle, occupant, (int)v5, entryIdx, 0);
    scr_vehicle_t* scr_vehicle = vehicle->scr_vehicle;
    if (scr_vehicle != nullptr && s_vehicleInfos[scr_vehicle->infoIdx]->type == 2)
        scr_vehicle->noEntryTime = level.time + 4000;
    vehicle->health = health;
    if (EntityManager::sInst->IsLocalPlayer(occupant))
    {
        unsigned int h = occupant->mHandle.mHandle.mVal;
        vehicle->Notify(hash_const.activate, &h);
    }
}

// ea: 0x00480CF0
void VEH_RotateWheels(Entity* self, vehicle_info_t* info)
{
    scr_vehicle_t* scr_vehicle = self->scr_vehicle;
    float trans[3] = { 0.0f, 0.0f, 0.0f };
    float steerAngles[3] = { 0.0f, 0.0f, 0.0f };
    int numWheels = 2 * (info->type != 1) + 4;
    if (2 * (info->type != 1) != -4)
    {
        int v4 = 0;
        int* wheel = scr_vehicle->boneIndex.wheel;
        do
        {
            int v7 = *wheel;
            if (*wheel > 0)
            {
                steerAngles[1] = scr_vehicle->current.mSteeringAngle;
                steerAngles[0] = scr_vehicle->wheelPitch;
                if (v4 != TAG_WHEEL_FRONT_LEFT && v4 != TAG_WHEEL_FRONT_RIGHT)
                    steerAngles[1] = 0.0f;
                G_DObjSetLocalTagInternal_0(trans, steerAngles, v7, self, 0);
            }
            ++v4;
            ++wheel;
        } while (v4 < numWheels);
    }
}

// ea: 0x0044DC50
void VEH_StopAllEffects(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* v2 = s_vehicleInfos[scr_vehicle->infoIdx];
    int count = 2 * (v2->type != 1) + 4;
    if (2 * (v2->type != 1) != -4)
    {
        for (int i = 0; i < count; ++i)
        {
            if (scr_vehicle->mWheel_ParticleEffectHandle[i].mVal != 0)
            {
                EffectEventStopEmitting(scr_vehicle->mWheel_ParticleEffectHandle[i].mVal);
                scr_vehicle->mWheel_ParticleEffectHandle[i].mVal = 0;
            }
        }
    }
    if (scr_vehicle->mRumbleEffectHandle.mVal != 0)
    {
        EffectEventStopEmitting(scr_vehicle->mRumbleEffectHandle.mVal);
        scr_vehicle->mRumbleEffectHandle.mVal = 0;
    }
    for (int i = 6; i != 0; --i)
    {
        if (scr_vehicle->mSoundEffectHandle[6 - i].mVal != 0)
            EffectEventStopEmitting(scr_vehicle->mSoundEffectHandle[6 - i].mVal);
        scr_vehicle->mSoundEffectHandle[6 - i].mVal = 0;
    }
}

// ea: 0x0046DEA0
void G_FreeVehicleRefs(Entity* ent)
{
    int i = 0;
    if (level.MaxVehicles != 0)
    {
        int v1 = 0;
        do
        {
            scr_vehicle_t* v2 = &s_vehicles[v1];
            if (HandleDbToEnt(s_vehicles[v1].mEntity) != nullptr)
            {
                if (v2->mIdleSndEnt.mHandle.mVal == ent->mHandle.mHandle.mVal)
                    v2->mIdleSndEnt.mHandle.mVal = 0;
                if (v2->mEngineSndEnt.mHandle.mVal == ent->mHandle.mHandle.mVal)
                    v2->mEngineSndEnt.mHandle.mVal = 0;
                if (v2->mTargetEnt.mHandle.mVal == ent->mHandle.mHandle.mVal)
                    v2->mTargetEnt.mHandle.mVal = 0;
            }
            ++v1;
            ++i;
        } while (i < level.MaxVehicles);
    }
}

// ea: 0x0044F100
vehicle_info_t* G_GetVehicleInfoName(int16_t index)
{
    if (index < 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7097;
        AeAssert::gCurrentExpr = "index >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (index < s_numVehicleInfos)
        return s_vehicleInfos[index];
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
    AeAssert::gCurrentLine = 7098;
    AeAssert::gCurrentExpr = "index < s_numVehicleInfos";
    if (AeAssert::IsIgnored() || AeAssert::Assert("old cod assert"))
        __debugbreak();
    return s_vehicleInfos[index];
}

// ea: 0x0045E9B0
void Scr_Vehicle_Pain(Entity* pSelf, Entity* pAttacker, int /*damage*/,
                      const float* /*point*/, int mod, const float* dir,
                      hitLocation_t /*hitLoc*/)
{
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    if ((scr_vehicle == nullptr || scr_vehicle->mRBVeh == nullptr) && pAttacker != nullptr)
    {
        switch (mod)
        {
        case 3:
        case 4:
        case 5:
        case 6:
        case 9:
        case 10:
        case 17:
        case 18:
        case 27:
        case 28:
        case 32:
        {
            math::Position3 v8;
            v8.v.m128_f32[0] = dir[0];
            v8.v.m128_f32[1] = dir[1];
            v8.v.m128_f32[2] = dir[2];
            v8.v.m128_f32[3] = 0.0f;
            VEH_JoltBody(pSelf, &v8, 1.0f, 0.0f, 0.0f);
            break;
        }
        default:
            return;
        }
    }
}

// ea: 0x00480880
Client* G_IsVehicleUsable(Entity* ent, Entity* player, bool speedCheck)
{
    Client* result = player->client;
    if (result != nullptr)
    {
        if (ent->health < 0
            || (ent->s.eFlags & 0x80000) != 0
            || (result->ps.eFlags & 0x100000) != 0
            || HandleDbToEnt(player->r.mOwner) != nullptr
            || ent->scr_vehicle->noEntryTime + 200 > level.time)
        {
            return nullptr;
        }
        vehicle_info_t* VehicleInfo = G_GetVehicleInfo(ent);
        scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
        bool canUse = CanMantleVehicle(scr_vehicle, player)
                      || (scr_vehicle->playersAttached < VehicleInfo->numSeats
                          && (VehicleInfo->type != 2
                              || player->sentient == nullptr
                              || !scr_vehicle->IsOppositeTeamInVehicle(player->sentient->eTeam)));
        if (canUse
            && (ent->r.contents & 0x200000) != 0
            && (!speedCheck || (ent->speed <= 100.0f && ent->health > 0)))
        {
            return result;
        }
        return nullptr;
    }
    return result;
}

// ea: 0x0044F010
int16_t G_GetVehicleInfoIndex(const char* name)
{
    if (name == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7080;
        AeAssert::gCurrentExpr = "name";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (*name == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7081;
        AeAssert::gCurrentExpr = "name[0]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int16_t VehicleInfo = (int16_t)VEH_GetVehicleInfo(name);
    if (VehicleInfo == -1)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7084;
        AeAssert::gCurrentExpr = "index != -1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return VehicleInfo;
}

// ea: 0x0046E540
DbLinkedHandle<EntityHandleDb, Entity> G_GetTankEntNum(int index)
{
    DbLinkedHandle<EntityHandleDb, Entity> result;
    result.mHandle.mVal = 0;
    if (index >= level.MaxVehicles)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7601;
        AeAssert::gCurrentExpr = "index < level.MaxVehicles";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle_t* v2 = &level.vehicles[index];
    Entity* v4 = HandleDbToEnt(v2->mEntity);
    if (v4 != nullptr
        && s_vehicleInfos[v2->infoIdx]->type == 2
        && v2->drawOnCompass != 0
        && v4->scr_vehicle != nullptr
        && v4->health > 0
        && HandleDbToEnt(v4->r.mOwner) == nullptr)
    {
        result.mHandle.mVal = v4->mHandle.mHandle.mVal;
    }
    return result;
}

// ea: 0x00488BE0
void Scr_Vehicle_Die(Entity* pSelf, Entity* pInflictor, Entity* pAttacker,
                     int /*damage*/, int mod, int weapon, const float* position,
                     const float* dir, hitLocation_t hitLoc)
{
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    scr_vehicle->playEngineSound = 0;
    VEH_StopWheelEffects(pSelf);
    for (int seat = 0; seat < 11; ++seat)
    {
        Entity* mObject = HandleDbToEnt(scr_vehicle->seats[seat].occupant);
        if (mObject != nullptr)
        {
            Scr_Vehicle_GetOut(pSelf, mObject, pSelf->health);
            G_Damage(mObject, pInflictor, pAttacker, dir, position, 9999, 160,
                     mod, hitLoc, weapon);
        }
    }
    Entity* v16 = pAttacker;
    if (pAttacker == nullptr)
    {
        v16 = pInflictor;
        if (pInflictor == nullptr)
            v16 = pSelf;
    }
    Scr_NotifyFromEnt(pSelf, hash_const.death, v16);
}

// ea: 0x00480970
void Scr_Vehicle_Controller(Entity* pSelf)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7746;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7747;
        AeAssert::gCurrentExpr = "pSelf->scr_vehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    float bodyAngles[3];
    bodyAngles[0] = scr_vehicle->next.mBodyPosition.v.m128_f32[0];
    bodyAngles[1] = 0.0f;
    bodyAngles[2] = scr_vehicle->next.mBodyPosition.v.m128_f32[2];
    int body = scr_vehicle->boneIndex.body;
    if (body >= 0)
        G_DObjSetLocalTagInternal_0(vec3_origin, bodyAngles, body, pSelf, 0);
    float turretAngles[3] = { 0.0f, scr_vehicle->next.mTurretAngles.v.m128_f32[1], 0.0f };
    float barrelAngles[3] = { scr_vehicle->next.mTurretAngles.v.m128_f32[0], 0.0f, 0.0f };
    int turret = scr_vehicle->boneIndex.turret;
    if (turret >= 0)
        G_DObjSetLocalTagInternal_0(vec3_origin, turretAngles, turret, pSelf, 0);
    int barrel = scr_vehicle->boneIndex.barrel;
    if (barrel >= 0)
        G_DObjSetLocalTagInternal_0(vec3_origin, barrelAngles, barrel, pSelf, 0);
}

// ea: 0x0044FB90
bool VEH_VehicleTouchesMine(Entity* vehicle, EntityState* item)
{
    float v12[3];
    v12[0] = item->pos.trBase[0];
    v12[1] = item->pos.trBase[1];
    v12[2] = item->pos.trBase[2];
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(item->weapon);
    if (InfoForWeapon->iTriggerRadius == 0)
        return false;
    rb_vehicle* mRBVeh = (rb_vehicle*)vehicle->scr_vehicle->mRBVeh;
    if (mRBVeh == nullptr)
        return false;
    int v4 = InfoForWeapon->iTriggerRadius * InfoForWeapon->iTriggerRadius;
    float dx = v12[0] - vehicle->r.currentOrigin.v.m128_f32[0];
    float dy = v12[1] - vehicle->r.currentOrigin.v.m128_f32[1];
    float dz = v12[2] - vehicle->r.currentOrigin.v.m128_f32[2];
    if (((dx * dx) + (dy * dy)) + (dz * dz)
        > (vehicle->scr_vehicle->mUseRadius * vehicle->scr_vehicle->mUseRadius) + v4)
    {
        return false;
    }
    for (int v7 = 0; v7 < 6; ++v7)
    {
        rigid_body_constraint_wheel* wheel = mRBVeh->m_wheels[v7];
        if (wheel != nullptr)
        {
            float wx = v12[0] - wheel->m_origin[0];
            float wy = v12[1] - wheel->m_origin[1];
            float wz = v12[2] - wheel->m_origin[2];
            if (v4 > ((wx * wx) + (wy * wy)) + (wz * wz))
                return true;
        }
    }
    return false;
}

// ea: 0x004526D0
void G_VehInitPathPos(vehicle_pathpos_t* vpp)
{
    vpp->nodeIdx = -1;
    vpp->endOfPath = 0;
    vpp->frac = 0.0f;
    vpp->speed = 0.0f;
    vpp->lookAhead = 0.0f;
    vpp->slide = 0.0f;
    vpp->origin[0] = 0.0f;
    vpp->origin[1] = 0.0f;
    vpp->angles[0] = 0.0f;
    vpp->angles[1] = 0.0f;
    vpp->lookPos[0] = 0.0f;
    vpp->lookPos[1] = 0.0f;
    for (int i = 0; i < 2; ++i)
    {
        vehicle_path_node_t* node = &vpp->switchNode[i];
        node->mName.clear();
        node->mTarget.clear();
        node->speed = -1.0f;
        node->lookAhead = -1.0f;
        node->origin[0] = 0.0f;
        node->origin[1] = 0.0f;
        node->dir[0] = 0.0f;
        node->dir[1] = 0.0f;
        node->angles[0] = s_invalidAngles[0];
        node->angles[1] = dword_DD7418;
        node->angles[2] = dword_DD741C;
        node->length = 0.0f;
        node->nextIdx = 0xFFFFFFF;
    }
}

// ea: 0x00452870
void G_VehSetUpPathPos(vehicle_pathpos_t* vpp, int16_t nodeIdx)
{
    vehicle_node_t* v3 = s_nodes[nodeIdx];
    vpp->nodeIdx = nodeIdx;
    vpp->endOfPath = 0;
    vpp->frac = 0.0f;
    vpp->speed = v3->speed;
    vpp->lookAhead = v3->lookAhead;
    vpp->slide = (v3->nextIdx & 0x30000000) != 0 ? 1.0f : 0.0f;
    vpp->origin[0] = v3->origin[0];
    vpp->origin[1] = v3->origin[1];
    vpp->origin[2] = v3->origin[2];
    vpp->angles[0] = v3->angles[0];
    vpp->angles[1] = v3->angles[1];
    vpp->angles[2] = v3->angles[2];
    vpp->lookPos[0] = v3->origin[0];
    vpp->lookPos[1] = v3->origin[1];
    vpp->lookPos[2] = v3->origin[2];
    for (int i = 0; i < 2; ++i)
    {
        vehicle_path_node_t* node = &vpp->switchNode[i];
        node->mName.clear();
        node->mTarget.clear();
        node->speed = -1.0f;
        node->lookAhead = -1.0f;
        node->origin[0] = 0.0f;
        node->origin[1] = 0.0f;
        node->dir[0] = 0.0f;
        node->dir[1] = 0.0f;
        node->angles[0] = s_invalidAngles[0];
        node->angles[1] = dword_DD7418;
        node->angles[2] = dword_DD741C;
        node->length = 0.0f;
        node->nextIdx = 0xFFFFFFF;
    }
}

// ea: 0x00452A10
void G_VehSetSwitchNode(vehicle_pathpos_t* vpp, int16_t srcNodeIdx, uint16_t dstNodeIdx)
{
    for (int i = 0; i < 2; ++i)
    {
        vehicle_path_node_t* node = &vpp->switchNode[i];
        node->mName.clear();
        node->mTarget.clear();
        node->speed = -1.0f;
        node->lookAhead = -1.0f;
        node->origin[0] = 0.0f;
        node->origin[1] = 0.0f;
        node->dir[0] = 0.0f;
        node->dir[1] = 0.0f;
        node->angles[0] = s_invalidAngles[0];
        node->angles[1] = dword_DD7418;
        node->angles[2] = dword_DD741C;
        node->length = 0.0f;
        node->nextIdx = 0xFFFFFFF;
    }
    if (srcNodeIdx >= 0 && (dstNodeIdx & 0x8000u) == 0)
    {
        vehicle_node_t* v6 = s_nodes[srcNodeIdx];
        vehicle_node_t* dstNode = s_nodes[dstNodeIdx];
        VP_CopyNode(v6, &vpp->switchNode[0]);
        VP_CopyNode(v6, &vpp->switchNode[1]);
        vpp->switchNode[0].nextIdx ^= (dstNodeIdx ^ vpp->switchNode[0].nextIdx) & 0x3FFF;
        vpp->switchNode[0].dir[0] = dstNode->origin[0] - v6->origin[0];
        vpp->switchNode[0].dir[1] = dstNode->origin[1] - v6->origin[1];
        vpp->switchNode[0].dir[2] = dstNode->origin[2] - v6->origin[2];
        vpp->switchNode[0].length = VectorNormalize(vpp->switchNode[0].dir);
    }
}

// ea: 0x0045F1A0 (VP_CopyNode)
void VP_CopyNode(vehicle_node_t* src, vehicle_path_node_t* dst)
{
    dst->mName = src->mName;
    dst->mTarget = src->mTarget;
    dst->speed = src->speed;
    dst->lookAhead = src->lookAhead;
    dst->script_noteworthy = src->script_noteworthy;
    memcpy(dst->origin, src->origin, sizeof(dst->origin));
    memcpy(dst->dir, src->dir, sizeof(dst->dir));
    memcpy(dst->angles, src->angles, sizeof(dst->angles));
    dst->length = src->length;
    dst->nextIdx = src->nextIdx;
}

// ea: 0x00488280
int G_SpawnVehicle(Entity* ent, const char* typeName)
{
    scr_vehicle_t* v3;
    if (ent->scr_vehicle != nullptr)
    {
        v3 = ent->scr_vehicle;
    }
    else
    {
        v3 = nullptr;
        int16_t v4 = 0;
        if (level.MaxVehicles != 0)
        {
            do
            {
                int v5 = v4;
                unsigned int mVal = s_vehicles[v5].mEntity.mHandle.mVal;
                v3 = &s_vehicles[v5];
                if (mVal & 0xFFF >= 0x540
                    || mVal >> 12 != EntityHandleDb::sInst.mElements[mVal & 0xFFF].mKey
                    || EntityHandleDb::sInst.mElements[mVal & 0xFFF].mObject == nullptr)
                {
                    break;
                }
                ++v4;
            } while (v4 < level.MaxVehicles);
        }
        if (v4 == level.MaxVehicles)
            Com_Error(ERR_DROP, "Too many vehicles");
    }
    int16_t v8 = v3->infoIdx;
    memset(v3, 0, sizeof(scr_vehicle_t));
    v3->mTargetEnt.mHandle.mVal = 0;
    v3->mIdleSndEnt.mHandle.mVal = 0;
    v3->mEngineSndEnt.mHandle.mVal = 0;
    int16_t infoIdxa = v8;
    v3->playEngineSound = 1;
    if (typeName != nullptr && (infoIdxa = (int16_t)VEH_GetVehicleInfo(typeName)) < 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6803;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Can't find info for script vehicle [%s]\n", typeName))
            __debugbreak();
        return 0;
    }
    VEH_InitEntity(ent, v3, infoIdxa);
    VEH_InitVehicle(v3);
    ent->s.brushmodel = 0;
    SV_SetBrushModel(ent);
    ent->r.contents = 0xA00000;
    if (ent->scr_vehicle != nullptr
        && s_vehicleInfos[ent->scr_vehicle->infoIdx]->type == 3)
    {
        ent->SetAlwaysRender(true);
    }
    return 1;
}

// ea: 0x0045F030
int G_VehUpdatePathPos(Entity* pEnt, vehicle_pathpos_t* vpp, bool overrideSpeed,
                       int msec, int waitNode)
{
    if (vpp->endOfPath != 0)
    {
        if (vpp->switchNode[0].mName.mBlock == nullptr
            || vpp->switchNode[0].mName.c_str()[0] == 0)
        {
            return 0;
        }
    }
    vehicle_path_node_t* switchNode = &vpp->switchNode[0];
    if (switchNode->mName.mBlock != nullptr && switchNode->mName.c_str()[0] != 0)
    {
        int NodeIndex = VP_GetNodeIndex(&switchNode->mName, nullptr);
        if (NodeIndex >= 0)
        {
            VP_CopyNode(s_nodes[NodeIndex], &vpp->switchNode[0]);
        }
    }
    VP_GetLookAheadXYZ(vpp, vpp->lookPos);
    float lookDir[3];
    lookDir[0] = vpp->lookPos[0] - vpp->origin[0];
    lookDir[1] = vpp->lookPos[1] - vpp->origin[1];
    lookDir[2] = vpp->lookPos[2] - vpp->origin[2];
    float dist = VectorNormalize(lookDir);
    if (dist <= 0.0f)
    {
        vpp->endOfPath = 1;
    }
    else
    {
        vectoangles(lookDir, vpp->angles);
        vpp->angles[0] = AngleNormalize180(vpp->angles[0]);
        vpp->angles[1] = AngleNormalize180(vpp->angles[1]);
        vpp->angles[2] = AngleNormalize180(vpp->angles[2]);
        float v14 = (msec * 0.001f) * vpp->speed;
        if (v14 > dist)
        {
            int next = s_nodes[vpp->nodeIdx]->nextIdx;
            if (next >= 0 && (s_nodes[next]->nextIdx & 0x2000) == 0)
                v14 = dist;
        }
        vpp->origin[0] += v14 * lookDir[0];
        vpp->origin[1] += v14 * lookDir[1];
        vpp->origin[2] += v14 * lookDir[2];
        VP_UpdatePathPos(pEnt, vpp, lookDir, overrideSpeed, waitNode);
        VP_GetAngles(vpp, vpp->angles);
    }
    if (switchNode->mName.mBlock != nullptr
        && switchNode->mName.mBlock != (Broc::string::Block*)-12
        && switchNode->mName.c_str()[0] != 0)
    {
        int v18 = VP_GetNodeIndex(&switchNode->mName, nullptr);
        if (v18 >= 0)
            VP_CopyNode(s_nodes[v18], &vpp->switchNode[1]);
    }
    return 0;
}

// ea: 0x00491980
unsigned int Scr_Vehicle_SeatChange(Entity* occupant, unsigned int newSeatIdx)
{
    if (newSeatIdx > 0xA)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9369;
        AeAssert::gCurrentExpr = "newSeatIdx > VEHPOS_UNKNOWN && newSeatIdx <= VEHPOS_LAST_USABLE";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid seat index"))
            __debugbreak();
    }
    Entity* mObject = HandleDbToEnt(occupant->r.mOwner);
    if (mObject == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9371;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("G_VehicleOccupantChangeSeat:  specified occupant not attached to vehicle"))
            __debugbreak();
        return 0;
    }
    int vehPos = occupant->client->ps.vehPos;
    int fromPos = vehPos;
    if (occupant->IsLocalPlayer())
    {
        int PlayerIndex = occupant->GetPlayerIndex();
        if (InteractionController_Inst(PlayerIndex) != nullptr
            && *(void**)InteractionController_Inst(PlayerIndex) != nullptr)
        {
            InteractionController_EndInteraction(InteractionController_Inst(PlayerIndex), 1);
        }
    }
    scr_vehicle_t* scr_vehicle = mObject->scr_vehicle;
    if (scr_vehicle != nullptr
        && s_vehicleInfos[scr_vehicle->infoIdx]->type == 1
        && vehPos == 1
        && occupant == EntityManager::sInst->GetPlayer(currCl))
    {
        SetClientViewAngle(occupant, mObject->r.currentAngles.v.m128_f32);
        scr_vehicle->current.mGunnerAngles.v.m128_f32[0] = 0.0f;
        scr_vehicle->current.mGunnerAngles.v.m128_f32[1] = 0.0f;
        scr_vehicle->next.mGunnerAngles.v.m128_f32[0] = 0.0f;
        scr_vehicle->next.mGunnerAngles.v.m128_f32[1] = 0.0f;
    }
    VEH_UnlinkPlayer(occupant, false);
    if (newSeatIdx == 0)
        mObject->scr_vehicle->AssignPhysics(occupant);
    VEH_LinkPlayer(mObject, occupant, (int)newSeatIdx, 0, vehPos);
    return newSeatIdx;
}

// ea: 0x00488ED0
void VEH_RespawnVehicle(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    VEH_StopAllEffects(ent);
    void* mRBVeh = scr_vehicle->mRBVeh;
    float respawn_origin[3];
    respawn_origin[0] = scr_vehicle->respawn_origin.v.m128_f32[0];
    respawn_origin[1] = scr_vehicle->respawn_origin.v.m128_f32[1];
    respawn_origin[2] = scr_vehicle->respawn_origin.v.m128_f32[2];
    float respawn_angles[3];
    respawn_angles[0] = scr_vehicle->respawn_angles.v.m128_f32[0];
    respawn_angles[1] = scr_vehicle->respawn_angles.v.m128_f32[1];
    respawn_angles[2] = scr_vehicle->respawn_angles.v.m128_f32[2];
    if (mRBVeh != nullptr)
    {
        VEH_RemoveVehicle(mRBVeh);
        scr_vehicle->mRBVeh = nullptr;
    }
    ent->s.eFlags &= ~0x80u;
    G_SpawnVehicle(ent, nullptr);
    scr_vehicle->phys.origin.v.m128_f32[0] = respawn_origin[0];
    scr_vehicle->phys.origin.v.m128_f32[1] = respawn_origin[1];
    scr_vehicle->phys.origin.v.m128_f32[2] = respawn_origin[2];
    scr_vehicle->phys.prevOrigin.v.m128_f32[0] = respawn_origin[0];
    scr_vehicle->phys.prevOrigin.v.m128_f32[1] = respawn_origin[1];
    scr_vehicle->phys.prevOrigin.v.m128_f32[2] = respawn_origin[2];
    ent->r.currentOrigin.v.m128_f32[0] = respawn_origin[0];
    ent->r.currentOrigin.v.m128_f32[1] = respawn_origin[1];
    ent->r.currentOrigin.v.m128_f32[2] = respawn_origin[2];
    scr_vehicle->phys.angles.v.m128_f32[0] = respawn_angles[0];
    scr_vehicle->phys.angles.v.m128_f32[1] = respawn_angles[1];
    scr_vehicle->phys.angles.v.m128_f32[2] = respawn_angles[2];
    scr_vehicle->phys.prevAngles.v.m128_f32[0] = respawn_angles[0];
    scr_vehicle->phys.prevAngles.v.m128_f32[1] = respawn_angles[1];
    scr_vehicle->phys.prevAngles.v.m128_f32[2] = respawn_angles[2];
    ent->r.currentAngles.v.m128_f32[0] = respawn_angles[0];
    ent->r.currentAngles.v.m128_f32[1] = respawn_angles[1];
    ent->r.currentAngles.v.m128_f32[2] = respawn_angles[2];
    float vel[3] = { 0.0f, 0.0f, 0.0f };
    VEH_SetPosition(ent, &scr_vehicle->phys.origin, &scr_vehicle->phys.angles, vel);
    scr_vehicle->respawn_origin.v.m128_f32[0] = respawn_origin[0];
    scr_vehicle->respawn_origin.v.m128_f32[1] = respawn_origin[1];
    scr_vehicle->respawn_origin.v.m128_f32[2] = respawn_origin[2];
    ent->rotate.v.m128_f32[0] = respawn_origin[0];
    ent->rotate.v.m128_f32[1] = respawn_origin[1];
    ent->rotate.v.m128_f32[2] = respawn_origin[2];
    scr_vehicle->respawn_angles.v.m128_f32[0] = respawn_angles[0];
    scr_vehicle->respawn_angles.v.m128_f32[1] = respawn_angles[1];
    scr_vehicle->respawn_angles.v.m128_f32[2] = respawn_angles[2];
}

// ea: 0x00488CE0
void SP_script_vehicle(Entity* pSelf)
{
    static unsigned char s_init = 0;
    static unsigned int vehicletype_hash = 0;
    if (!(s_init & 1))
    {
        s_init |= 1;
        vehicletype_hash = HashString::CalcHash("vehicletype");
    }
    const char* typeName = nullptr;
    G_SpawnString(vehicletype_hash, nullptr, &typeName);
    if (s_numVehicleInfos <= 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10231;
        AeAssert::gCurrentExpr = "s_numVehicleInfos > 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("No vehicle type files are loaded.\nDo all the vehicles in your level\nhave the \"vehicletype\" key?"))
            __debugbreak();
    }
    if (typeName == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10232;
        AeAssert::gCurrentExpr = "typeName";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid vehicletype found in level.\nDo all the vehicles in your level\nhave the \"vehicletype\" key?"))
            __debugbreak();
    }
    if (G_SpawnVehicle(pSelf, typeName) != 0)
    {
        scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
        if (s_vehicleInfos[scr_vehicle->infoIdx]->type == 2)
        {
            VEH_Backup(pSelf);
            if ((pSelf->flags & 0x400000) == 0)
                VEH_SetPosition(pSelf, &scr_vehicle->phys.origin, &scr_vehicle->phys.angles,
                                scr_vehicle->phys.vel.v.m128_f32);
        }
        pSelf->scr_vehicle->respawn_origin.v.m128_f32[0] = pSelf->r.currentOrigin.v.m128_f32[0];
        pSelf->scr_vehicle->respawn_origin.v.m128_f32[1] = pSelf->r.currentOrigin.v.m128_f32[1];
        pSelf->scr_vehicle->respawn_origin.v.m128_f32[2] = pSelf->r.currentOrigin.v.m128_f32[2];
        pSelf->rotate.v.m128_f32[0] = pSelf->r.currentOrigin.v.m128_f32[0];
        pSelf->rotate.v.m128_f32[1] = pSelf->r.currentOrigin.v.m128_f32[1];
        pSelf->rotate.v.m128_f32[2] = pSelf->r.currentOrigin.v.m128_f32[2];
        pSelf->scr_vehicle->respawn_angles.v.m128_f32[0] = pSelf->r.currentAngles.v.m128_f32[0];
        pSelf->scr_vehicle->respawn_angles.v.m128_f32[1] = pSelf->r.currentAngles.v.m128_f32[1];
        pSelf->scr_vehicle->respawn_angles.v.m128_f32[2] = pSelf->r.currentAngles.v.m128_f32[2];
    }
}

// ea: 0x0046FD50
int scr_vehicle_t::GetEntryHintStringIndex(Entity* vehicle, unsigned int entryPosition)
{
    if (entryPosition >= 6)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10121;
        AeAssert::gCurrentExpr = "entryPosition >= 0 && entryPosition < NUM_ENTRY_POINTS";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid entry point index"))
            __debugbreak();
    }
    if (s_vehicleInfos[infoIdx] == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10122;
        AeAssert::gCurrentExpr = "s_vehicleInfos[ infoIdx ]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid info pointer in vehicle"))
            __debugbreak();
    }
    if (s_vehicleInfos[infoIdx]->type != 2)
    {
        if (sEntryPointHintIndicies[entryPosition] == -1)
        {
            G_GetHintStringIndex(&sEntryPointHintIndicies[entryPosition],
                                 sEntryPointHintText[entryPosition]);
            if (sEntryPointHintIndicies[entryPosition] < 0)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                AeAssert::gCurrentLine = 10136;
                AeAssert::gCurrentExpr = "sEntryPointHintIndicies[entryPosition]>= 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Invalid vehicle entry hint string."))
                    __debugbreak();
            }
        }
        if (sEntryPointSeatAssociation[entryPosition] != 1
            || *BG_GetInfoForWeapon((int)gunnerWeapon)->szUseHintString == 0)
        {
            return sEntryPointHintIndicies[entryPosition];
        }
        return BG_GetInfoForWeapon((int)gunnerWeapon)->iUseHintStringIndex;
    }
    if (HandleDbToEnt(vehicle->scr_vehicle->seats[0].occupant) != nullptr)
        return BG_GetInfoForWeapon((int)gunnerWeapon)->iUseHintStringIndex;
    Entity* mObject = HandleDbToEnt(mEntity);
    return BG_GetInfoForWeapon(mObject->s.weapon)->iUseHintStringIndex;
}

// ea: 0x00480AC0
void Scr_Vehicle_Init(Entity* pSelf, int /*msec*/)
{
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    bool v4 = pSelf->active == 2;
    if (!v4 || s_clientThink != 0)
    {
        float* wheelZPos = scr_vehicle->phys.wheelZPos;
        for (int i = 0; i < 6; ++i)
        {
            int BoneIndex = SV_DObjGetBoneIndex(pSelf, s_wheelTagHashes[i]);
            if (BoneIndex >= 0)
            {
                DObjSkelMat mtx;
                G_DObjGetWorldBoneIndexMatrix(pSelf, BoneIndex, &mtx);
                wheelZPos[i] = mtx.origin[2];
            }
            scr_vehicle->mWheel_ParticleEffectHandle[i].mVal = 0;
        }
        int turretPitch = scr_vehicle->boneIndex.turret;
        if (turretPitch >= 0)
        {
            DObjSkelMat mtx;
            G_DObjGetWorldBoneIndexMatrix(pSelf, turretPitch, &mtx);
            float turretPos[3] = { mtx.origin[0], mtx.origin[1], mtx.origin[2] };
            int turretSpan = scr_vehicle->boneIndex.barrel;
            if (turretSpan >= 0)
            {
                DObjSkelMat mtx2;
                G_DObjGetWorldBoneIndexMatrix(pSelf, turretSpan, &mtx2);
                float spanPos[3] = { mtx2.origin[0], mtx2.origin[1], mtx2.origin[2] };
                scr_vehicle->mUseRadius = VectorDistance(turretPos, spanPos);
            }
        }
        int type = info->type;
        if ((type == 1 || type == 2) && Entity_has_zone_collision(pSelf))
            VEH_GroundPlant(pSelf, 0, 10000);
        float vel[3] = { 0.0f, 0.0f, 0.0f };
        VEH_SetPosition(pSelf, &scr_vehicle->phys.origin, &scr_vehicle->phys.angles, vel);
        scr_vehicle->phys.prevOrigin.v.m128_f32[0] = scr_vehicle->phys.origin.v.m128_f32[0];
        scr_vehicle->phys.prevOrigin.v.m128_f32[1] = scr_vehicle->phys.origin.v.m128_f32[1];
        scr_vehicle->phys.prevOrigin.v.m128_f32[2] = scr_vehicle->phys.origin.v.m128_f32[2];
        scr_vehicle->phys.prevAngles.v.m128_f32[0] = scr_vehicle->phys.angles.v.m128_f32[0];
        scr_vehicle->phys.prevAngles.v.m128_f32[1] = scr_vehicle->phys.angles.v.m128_f32[1];
        scr_vehicle->phys.prevAngles.v.m128_f32[2] = scr_vehicle->phys.angles.v.m128_f32[2];
        math::Position3 angles = scr_vehicle->phys.angles;
        MultiplayerMgr::sInst->ApplyLocalPhysicsToVehicle(pSelf, &scr_vehicle->phys.origin,
                                                          &angles, vel);
        collision_context_t context;
        context.__vftable = nullptr;
        context.pass_entity1.mHandle.mVal = 0;
        context.pass_entity2.mHandle.mVal = 0;
        context.pass_owner1.mHandle.mVal = 0;
        context.pass_owner2.mHandle.mVal = 0;
        context.contentmask = -1;
        G_DoTouchTriggers(pSelf, &pSelf->r.currentOrigin, nullptr, &context);
        pSelf->think = THINK__Scr_Vehicle_Init;
        pSelf->nextthink = level.time + 1;
    }
    else
    {
        pSelf->nextthink = level.time;
    }
}

// ea: 0x0044D370
int VEH_ParseSpecificField(unsigned char* pStruct, const char* pValue, int fieldType)
{
    if (fieldType == 8)
    {
        int v6 = 0;
        while (_stricmp(pValue, s_vehicleTypeNames[v6]) != 0)
        {
            if (++v6 >= 6)
                break;
        }
        if (v6 == 6)
            Com_Error(ERR_DROP, "unknown vehicle type '%s'", pValue);
        *(pStruct + 16) = (unsigned char)v6;
        return 1;
    }
    if (fieldType == 9)
    {
        int v5 = 0;
        while (_stricmp(pValue, s_vehicleSubTypeNames[v5]) != 0)
        {
            if (++v5 >= 9)
                break;
        }
        if (v5 == 9)
        {
            Com_Error(ERR_DROP, "unknown vehicle subtype '%s'", pValue);
            return 1;
        }
        *(pStruct + 17) = (unsigned char)v5;
        return 1;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
    AeAssert::gCurrentLine = 1002;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored())
    {
        if (AeAssert::Warning(va("Bad vehicle field type %i\n", fieldType)))
            __debugbreak();
    }
    Com_Error(ERR_DROP, "Bad vehicle field type %i", fieldType);
    return 0;
}

// ea: 0x0046C9E0
void VEH_InvalidateCaches(void)
{
    if (level.vehicles != nullptr)
    {
        for (int i = 0; i < level.MaxVehicles; ++i)
        {
            Entity* mObject = HandleDbToEnt(level.vehicles[i].mEntity);
            if (mObject != nullptr && mObject->proximity_data != nullptr)
            {
                mObject->proximity_data->lo.v.m128_f32[0] = 3.4028235e38f;
                mObject->proximity_data->lo.v.m128_f32[1] = 3.4028235e38f;
                mObject->proximity_data->lo.v.m128_f32[2] = 3.4028235e38f;
                mObject->proximity_data->lo.v.m128_f32[3] = 3.4028235e38f;
                mObject->proximity_data->hi.v.m128_f32[0] = -3.4028235e38f;
                mObject->proximity_data->hi.v.m128_f32[1] = -3.4028235e38f;
                mObject->proximity_data->hi.v.m128_f32[2] = -3.4028235e38f;
                mObject->proximity_data->hi.v.m128_f32[3] = -3.4028235e38f;
            }
        }
    }
}

// ea: 0x0046E0B0
bool G_IsPlayerInVehicle(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
        return false;
    int eFlags = client->ps.eFlags;
    if ((0x100000 & eFlags) == 0 || (0x400000 & eFlags) != 0)
        return false;
    Entity* v3 = HandleDbToEnt(player->r.mOwner);
    if (v3 == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7302;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        return false;
    }
    return v3->scr_vehicle != nullptr;
}

// ea: 0x0045F210
vehicle_node_t* SP_create_info_vehicle_node(void)
{
    vehicle_node_t* v0 = g_vehicleNodeManager.AllocNode();
    v0->mName.clear();
    v0->mTarget.clear();
    int v1 = v0->nextIdx;
    v0->speed = -1.0f;
    v0->lookAhead = -1.0f;
    v0->origin[0] = 0.0f;
    v0->origin[1] = 0.0f;
    v0->dir[0] = 0.0f;
    v0->dir[1] = 0.0f;
    v1 &= 0xCFFFFFFF;
    v0->nextIdx = v1;
    v0->angles[0] = s_invalidAngles[0];
    v0->angles[1] = dword_DD7418;
    float v2 = dword_DD741C;
    v0->nextIdx = v1 | 0xFFFFFFF;
    v0->angles[2] = v2;
    v0->length = 0.0f;
    ++s_numNodes;
    return v0;
}

// ea: 0x0044F3D0
float Scr_Vehicle_CalcSpeed(const scr_vehicle_t* pVehicle)
{
    if (pVehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 8963;
        AeAssert::gCurrentExpr = "pVehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vehicle"))
            __debugbreak();
        return 0.0f;
    }
    rb_vehicle* mRBVeh = (rb_vehicle*)pVehicle->mRBVeh;
    if (mRBVeh == nullptr)
        return 0.0f;
    float v4[3];
    rb_vehicle_get_velocity(mRBVeh, v4);
    return sqrtf(v4[0] * v4[0] + v4[1] * v4[1] + v4[2] * v4[2]);
}

// ea: 0x0046F3C0
void scr_vehicle_t::AssignPhysics(Entity* player)
{
    if (mRBVeh != nullptr)
    {
        if (EntityManager::sInst->IsLocalPlayer(player)
            && mPhysicsOwner.mHandle.mVal != player->mHandle.mHandle.mVal)
        {
            if (IsPhysicsPaused())
                rb_vehicle_unpause_physics((rb_vehicle*)mRBVeh);
            rb_vehicle_update_from_network((rb_vehicle*)mRBVeh,
                                           &phys.origin, &phys.angles, &phys.vel,
                                           &phys.rotVel);
        }
    }
    mPhysicsOwner.mHandle.mVal = player->mHandle.mHandle.mVal;
}

// ea: 0x0044F580
int scr_vehicle_t::GetEntryRoute(int seatIdx, int entryIdx, bool hasFlag)
{
    vehicleAnimMap_t* animMap = this->animMap;
    if (animMap == nullptr)
        return -1;
    int numRoutes = animMap->numRoutes;
    int result = 0;
    if (numRoutes > 0)
    {
        do
        {
            if (animMap->routes[result].vehPosSrc == -1
                && animMap->routes[result].vehPosDest == seatIdx)
            {
                if (this->animMap->stages[this->animMap->routes[result].stages[0]].startTag
                        == this->animMap->entryTags[entryIdx]
                    && ((animMap->routes[result].flags & 4) == 0 || hasFlag))
                {
                    return result;
                }
                numRoutes = animMap->numRoutes;
            }
            ++result;
        } while (result < numRoutes);
    }
    return -1;
}

// ea: 0x004639D0
void G_ParseScrVehicleInfo(void)
{
    ConfigStringManager* v0 = ConfigStringManager::sInst;
    s_numVehicleInfos = 0;
    TPakId v1 = CurPakId();
    v0->CallbackSearch(v1, "VEHICLEFILE", ParseVehicleConfigString);
    ConfigStringManager* v2 = ConfigStringManager::sInst;
    TPakId v3 = CurPakId();
    v2->CallbackSearch(v3, "VEHICLEPHYSICSFILE", ParseVehiclePhysicsConfigString);
}

// ea: 0x0044D2E0
vehicle_info_t* VEH_GetInfo(int idx)
{
    return s_vehicleInfos[idx];
}

// ea: 0x0044F290
int G_GetNonPVSTankInfo(float* /*origin*/, DbLinkedHandle<EntityHandleDb, Entity> /*ent*/)
{
    return 0;
}

// ea: 0x0044F470
void G_FreeVehicleSeat(Entity* /*ent*/, Entity* /*veh*/, int /*seat*/)
{
    ;
}

// ea: 0x0044F480
int G_RequestVehicleSeat(Entity* /*ent*/, Entity* /*veh*/, HashString /*seat*/, bool /*bForce*/)
{
    return -1;
}

// ea: 0x0044F490
int G_RequestVehicleBestSeat(Entity* /*ent*/, Entity* /*veh*/, bool /*bForce*/,
                             bool /*bPassenger*/, bool /*bCanDrive*/)
{
    return -1;
}

// ea: 0x0044F4A0
int G_StealVehicleSeat(Entity* /*ent*/, Entity* /*veh*/, int /*seat*/, bool /*bForce*/)
{
    return -1;
}

// ea: 0x0044F4B0
void Scr_Vehicle_OccupantStartEntering(scr_vehicle_t* /*veh*/, const Entity* /*ent*/,
                                       int /*seat*/)
{
    ;
}

// ea: 0x0044F4C0
void Scr_Vehicle_OccupantIsSeat(scr_vehicle_t* /*veh*/, const Entity* /*ent*/, int /*seat*/)
{
    ;
}

// ea: 0x0044F4D0
void Scr_Vehicle_OccupantStartExiting(scr_vehicle_t* /*veh*/, const Entity* /*ent*/,
                                      int /*seat*/)
{
    ;
}

// ea: 0x0044F4E0
void Scr_Vehicle_OccupantIsOut(scr_vehicle_t* /*veh*/, Entity* /*ent*/, int /*seat*/)
{
    ;
}

// ea: 0x004517F0
vehicle_node_t* GetVehicleNode(int idx)
{
    return s_nodes[idx];
}

// ea: 0x004523E0
void G_InitVehiclePaths(void)
{
    s_numNodes = 0;
}

// ea: 0x0044D2B0
void SP_script_prop_collmap(Entity* pSelf)
{
    pSelf->r.contents = 0;
    pSelf->s.eType = 17;
}

// ea: 0x0044D2F0
int Is4WheeledVehicle(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    if (scr_vehicle == nullptr)
        return 0;
    vehicle_info_t* v2 = s_vehicleInfos[scr_vehicle->infoIdx];
    if (v2->type != 1)
        return 0;
    return 1;
}

// ea: 0x0044F1D0
vehicle_info_t* G_GetVehicleInfo(scr_vehicle_t* veh)
{
    if (veh != nullptr)
        return s_vehicleInfos[veh->infoIdx];
    return nullptr;
}

// ea: 0x0044F4F0
vehicleAnimStage_t* scr_vehicle_t::GetRouteStage(int routeIdx, int stage)
{
    return &animMap->stages[animMap->routes[routeIdx].stages[stage]];
}

// ea: 0x0044F560
float scr_vehicle_t::GetAnimSpeedScale(Client* client)
{
    return animMap->routes[client->mVehicleAnimRoute].animSpeedScale;
}

// ea: 0x0044F680
float scr_vehicle_t::GetThrottle()
{
    rb_vehicle* mRBVeh = (rb_vehicle*)this->mRBVeh;
    if (mRBVeh != nullptr)
        return mRBVeh->m_throttle;
    return 0.0f;
}

// ea: 0x0044F790
void SP_script_vehicle_collmap(Entity* pSelf)
{
    pSelf->r.contents = 0;
    pSelf->s.eType = 16;
}

// ea: 0x0044F7B0
void vehicle_FreeDynamicBuffers(void)
{
    if (s_vehicles != nullptr)
    {
        mem_heap_free(s_vehicles);
        s_vehicles = nullptr;
        level.MaxVehicles = 0;
    }
}

// ea: 0x0045EA80
void scr_vehicle_t::ReleasePhysics(Entity* player)
{
    if (mPhysicsOwner.mHandle.mVal == player->mHandle.mHandle.mVal)
        mPhysicsOwner.mHandle.mVal = 0;
}

// ea: 0x00480860
void VEH_PlayerInteractionExit(void)
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    VEH_UnlinkPlayer(Player, true);
}

static float rate = 1.0f;          // @ 0xDD7FDC (g_scr_vehicle.cpp local)
static float s_sndLerpMin = 0.1f;  // @ 0xDD7FE0 (g_scr_vehicle.cpp local)
static scr_vehicle_t s_backup;     // @ 0xEE60A0 (bss, g_scr_vehicle.cpp local)
static float intensity_scale = 500.0f;  // @ 0xDD7F48
static float hit_offset = 30.0f;        // @ 0xDD7F4C

// ea: 0x0044DD00
void VEH_Backup(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    scr_vehicle->phys.prevOrigin.v.m128_f32[0] = ent->r.currentOrigin.v.m128_f32[0];
    scr_vehicle->phys.prevOrigin.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
    scr_vehicle->phys.prevOrigin.v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2];
    scr_vehicle->phys.prevAngles.v.m128_f32[0] = ent->r.currentAngles.v.m128_f32[0];
    scr_vehicle->phys.prevAngles.v.m128_f32[1] = ent->r.currentAngles.v.m128_f32[1];
    scr_vehicle->phys.prevAngles.v.m128_f32[2] = ent->r.currentAngles.v.m128_f32[2];
    s_backup = *scr_vehicle;
}

// ea: 0x0044D8F0
void VEH_JoltBody(Entity* ent, const math::Position3* dir, float intensity,
                  float speedFrac, float decel)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    if (scr_vehicle != nullptr)
    {
        rb_vehicle* mRBVeh = (rb_vehicle*)scr_vehicle->mRBVeh;
        vehicle_info_t* v8 = s_vehicleInfos[scr_vehicle->infoIdx];
        if (mRBVeh != nullptr && v8->type == 2)
        {
            math::Position3 hitp;
            hitp.v = _mm_setzero_ps();
            hitp.v.m128_f32[2] = hit_offset;
            math::Position3 hitd;
            hitd.v = dir->v;
            ApplyPhysics(ent, &hitp, (const math::Dir3*)&hitd,
                         intensity_scale * intensity, true, HITLOC_TORSO_UPR);
        }
        if (intensity < 0.0f)
            intensity = 0.0f;
        else if (intensity > 1.0f)
            intensity = 1.0f;
        float axis[3][3];
        AnglesToAxis(&scr_vehicle->phys.angles, axis);
        scr_vehicle->joltDir[0] = (dir->v.m128_f32[0] * axis[0][0])
                                + (dir->v.m128_f32[1] * axis[0][1])
                                + (dir->v.m128_f32[2] * axis[0][2]);
        scr_vehicle->joltDir[1] = -((dir->v.m128_f32[0] * axis[1][0])
                                  + (dir->v.m128_f32[1] * axis[1][1])
                                  + (dir->v.m128_f32[2] * axis[1][2]));
        scr_vehicle->joltTime = 0.80000001f;
        scr_vehicle->joltWave = 0.0f;
        VectorNormalize2D(scr_vehicle->joltDir);
        scr_vehicle->joltDir[0] = (v8->maxBodyPitch * scr_vehicle->joltDir[0]) * intensity;
        scr_vehicle->joltDir[1] = (v8->maxBodyRoll * scr_vehicle->joltDir[1]) * intensity;
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 1699;
        AeAssert::gCurrentExpr = "veh";
        if (!AeAssert::IsIgnored())
        {
            const char* v6 = ent->mClassName.mBlock != nullptr
                                 ? (const char*)&ent->mClassName.mBlock[1]
                                 : defaultFileName;
            if (AeAssert::Assert("Non vehicle entity %s passed to VEH_JoltBody", v6))
                __debugbreak();
        }
    }
}

// ea: 0x0046CB00
void Svcmd_VehicleList_f()
{
    int v0 = 0;
    for (int i = 0; i < level.MaxVehicles; ++i)
    {
        unsigned int v3 = s_vehicles[i].mEntity.mHandle.mVal & 0xFFF;
        if (v3 < 0x540
            && s_vehicles[i].mEntity.mHandle.mVal >> 12
                   == EntityHandleDb::sInst.mElements[v3].mKey
            && EntityHandleDb::sInst.mElements[v3].mObject != nullptr)
            ++v0;
    }
    G_Printf("vehicles %d\n", v0);
}

// ea: 0x0046D2B0
void VEH_NetAltWeaponStatus(Entity* ent, int status)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    Entity* mObject = HandleDbToEnt(ent->r.mOwner);
    if (!EntityManager::sInst->IsLocalPlayer(mObject))
        scr_vehicle->seats[0].firing = (status != 0);
}

// ea: 0x0044DB90 (file-local)
static Handle VEH_StartWheelEffect(Entity* ent, unsigned int wheel_tag_hash,
                                   int mat)
{
    vehicle_info_t* info = s_vehicleInfos[ent->scr_vehicle->infoIdx];
    if (info->type == 2)
        wheel_tag_hash = 0;
    return PostEffectEventVehicleWheel(ent, (const char*)info,
                                       0x29 /* kActionVEHICLE_HORN */, mat,
                                       wheel_tag_hash);
}

// ea: 0x0045C7F0
void VEH_UpdateWheelParticleEffects(Entity* ent, int wheelIndex)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    if (info->maxSpeed <= 88.0f)
        return;
    if (scr_vehicle->engineSndLerp > s_sndLerpMin
        && scr_vehicle->phys.wheelSurfType[wheelIndex] != 0)
    {
        if (scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex].mVal == 0)
        {
            scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex] =
                VEH_StartWheelEffect(ent, s_wheelTagHashes[wheelIndex],
                                     scr_vehicle->phys.wheelSurfType[wheelIndex]);
        }
        float v8 = (ent->speed - 88.0f) / (info->maxSpeed - 88.0f);
        if (v8 < 0.0f)
            v8 = 0.0f;
        else if (v8 > 1.0f)
            v8 = 1.0f;
        float scaleValue2 = (float)(fabs(scr_vehicle->phys.rotVel.v.m128_f32[1])
                                    / info->rotRate);
        float v7 = (scr_vehicle->brakeSndLerp > 0.5f) ? 1.0f : 0.0f;
        float v9 = (v8 <= scaleValue2) ? scaleValue2 : v8;
        if (v9 <= v7)
            v8 = v7;
        else if (v8 <= scaleValue2)
            v8 = scaleValue2;
        EffectEventAdjustEffect_Scale(scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex],
                                      "EmissionRate", rate * v8);
    }
    else
    {
        if (scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex].mVal != 0)
            EffectEventStopEmitting(scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex].mVal);
        scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex].mVal = 0;
    }
}

// ea: 0x0045C3A0
void VEH_DebugBox(const math::Position3* pos, float width, float r, float g, float b)
{
    float color[4] = {1.0f,
                      pos->v.m128_f32[0] + width * 0.5f,
                      pos->v.m128_f32[1] + width * 0.5f,
                      pos->v.m128_f32[2] + width * 0.5f};
    float mins[3] = {pos->v.m128_f32[0] - width * 0.5f,
                     pos->v.m128_f32[1] - width * 0.5f,
                     pos->v.m128_f32[2] - width * 0.5f};
    float boxColor[3] = {r, g, b};
    G_DebugBox(&color[1], mins, boxColor, 1, 0, 0);
}

// ea: 0x0045CA30
void VEH_FillFollowHistoryBuffer(scr_vehicle_t* veh)
{
    float facing[3];
    AnglesToForward(veh->phys.angles.v.m128_f32, facing);
    VectorNormalizeFast(facing);
    int v1 = 117;
    int v2 = -36;
    do
    {
        for (int b = 0; b < 10; ++b)
        {
            float dist = (float)(v2 + 18 - 18 * b);
            float* dst = veh->follow->positionHistory[v1 / 3 - b];
            dst[0] = veh->phys.origin.v.m128_f32[0] + dist * facing[0];
            dst[1] = veh->phys.origin.v.m128_f32[1] + dist * facing[1];
            dst[2] = veh->phys.origin.v.m128_f32[2] + dist * facing[2];
        }
        v2 -= 180;
        v1 -= 30;
    } while (v2 >= -576);
    veh->follow->numHistoryBufferEntries = 40;
    veh->follow->historyBufferFront = 0;
}

// ea: 0x0047F630
void VEH_UpdateOverHeat(Entity* self, int msec)
{
    scr_vehicle_t* scr_vehicle = self->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    for (int i = 0; i < 11; ++i)
    {
        vehicleSeat_t& seat = scr_vehicle->seats[i];
        if (seat.gunMounted)
        {
            if (seat.weapon != 0)
            {
                weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(seat.weapon);
                if (seat.heat < 1.0f)
                {
                    if (seat.overheating && seat.heat <= 0.5f)
                    {
                        Handle v9;
                        v9.mVal = seat.overheatEffect.mVal;
                        seat.overheating = false;
                        if (v9.mVal != 0)
                        {
                            EffectEventSys::sInst->StopEffect(v9.mVal, false);
                            seat.overheatEffect.mVal = 0;
                        }
                    }
                }
                else
                {
                    seat.overheating = true;
                    PostEffectEventWeapon(self, InfoForWeapon->szInternalName,
                                          0x37 /* kActionMax|kActionWEAPON_PICKUP */);
                    Scr_Notify(self, hash_const.overheated, 0);
                    Handle v8;
                    v8.mVal = seat.overheatEffect.mVal;
                    if (v8.mVal != 0)
                        EffectEventSys::sInst->AdjustEffect_Scale(v8.mVal,
                                                                  "EmissionRate", 200.0f);
                }
                float heat = seat.heat;
                if (heat <= 0.0f)
                    seat.heat = 0.0f;
                else
                    seat.heat = heat - (msec * InfoForWeapon->fCooldownRate) * 0.001f;
                Handle v11;
                v11.mVal = seat.overheatEffect.mVal;
                if (seat.heat <= 0.25f)
                {
                    if (v11.mVal != 0)
                    {
                        EffectEventSys::sInst->StopEffect(v11.mVal, false);
                        seat.overheatEffect.mVal = 0;
                    }
                }
                else
                {
                    if (v11.mVal == 0)
                        seat.overheatEffect =
                            PostEffectEventWeapon(self, InfoForWeapon->szInternalName,
                                                  0x36 /* kActionMax|kActionWEAPON_LAST_SHOT_EJECT */);
                    float scale = (seat.heat - 0.25f) * 1.333333333333333f * emissionRate_0;
                    EffectEventSys::sInst->AdjustEffect_Scale(seat.overheatEffect.mVal,
                                                              "EmissionRate", scale);
                }
            }
        }
        else if (info->type == 1 && i == 0)
        {
            if (scr_vehicle->seats[0].heat < 1.0f)
            {
                if (scr_vehicle->seats[0].overheating
                    && scr_vehicle->seats[0].heat <= 0.5f)
                    scr_vehicle->seats[0].overheating = false;
            }
            else
            {
                scr_vehicle->seats[0].overheating = true;
            }
            float heat = scr_vehicle->seats[0].heat;
            if (heat <= 0.0f)
                scr_vehicle->seats[0].heat = 0.0f;
            else
                scr_vehicle->seats[0].heat = heat - msec * 0.00025000001f;
        }
    }
}

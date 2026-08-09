// ============================================================================
// g_scr_vehicle.cpp - scripted vehicle stubs (g.o: g_scr_vehicle.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

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

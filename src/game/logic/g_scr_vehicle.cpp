// ============================================================================
// g_scr_vehicle.cpp - scripted vehicle stubs (g.o: g_scr_vehicle.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

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

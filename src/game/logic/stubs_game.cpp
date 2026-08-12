// AUTO-GENERATED STUBS — Game core logic (g.o)
// 0 non-inline functions to port
// When ported, functions move from here to their real .cpp files.

#include <stdio.h>

#define COD3_UNIMPLEMENTED(lib) \
    fprintf(stderr, "COD3 UNIMPLEMENTED: %s\n", lib)

void __cod3_stub_game_logic(void) {
    COD3_UNIMPLEMENTED("game_logic");
}

// g.o vehicle-path helpers (port later; VP_* are real g.o functions).
class Entity;
struct vehicle_pathpos_t;
namespace math { class Position3; }

// ea: 0x45EDB0 (g.o) - stub
int VP_UpdatePathPos(Entity* pEnt, vehicle_pathpos_t* vpp, float* dir,
                     bool overrideSpeed, int waitNode)
{
    (void)pEnt; (void)vpp; (void)dir;
    (void)overrideSpeed; (void)waitNode;
    return 0;
}

// ea: 0x452090 (g.o) - stub
void VP_GetAngles(vehicle_pathpos_t* vpp, float* angles)
{
    (void)vpp; (void)angles;
}

// ea: 0x4521F0 (g.o) - stub
void VP_GetLookAheadXYZ(const vehicle_pathpos_t* vpp, float* lookXYZ)
{
    (void)vpp; (void)lookXYZ;
}

// ScriptMover_Updatemove (g.o) - stub
int ScriptMover_Updatemove(float speed, float time, math::Position3* dest)
{
    (void)speed; (void)time; (void)dest;
    return 0;
}

// AUTO-GENERATED STUBS — Game core logic (g.o)
// 0 non-inline functions to port
// When ported, functions move from here to their real .cpp files.

#include <stdio.h>

#define COD3_UNIMPLEMENTED(lib) \
    fprintf(stderr, "COD3 UNIMPLEMENTED: %s\n", lib)

void __cod3_stub_game_logic(void) {
    COD3_UNIMPLEMENTED("game_logic");
}

// g.o vehicle-path helpers that remain pending IDA reconstruction.
class Entity;
struct vehicle_pathpos_t;
namespace math { class Position3; }

// ea: 0x0045EDB0 (g.o) - stub
int VP_UpdatePathPos(Entity* pEnt, vehicle_pathpos_t* vpp, float* dir,
                     bool overrideSpeed, int waitNode)
{
    (void)pEnt; (void)vpp; (void)dir;
    (void)overrideSpeed; (void)waitNode;
    return 0;
}

// ea: 0x00452090 (g.o) - stub
void VP_GetAngles(vehicle_pathpos_t* vpp, float* angles)
{
    (void)vpp; (void)angles;
}

// ScriptMover_Updatemove (g.o) - stub; address pending IDA confirmation.
int ScriptMover_Updatemove(float speed, float time, math::Position3* dest)
{
    (void)speed; (void)time; (void)dest;
    return 0;
}

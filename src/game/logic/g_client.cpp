// ============================================================================
// g_client.cpp - client-side game logic (g.o: g_client.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

// ea: 0x00448BD0
void G_SetClientSound(Entity* ent)
{
    ent->s.loopSound = 0;
}

// ea: 0x00448E40
void G_RunClient(Entity* /*ent*/)
{
    ;
}

// ea: 0x00448F00
int ClientInactivityTimer(Entity* /*ent*/)
{
    return 1;
}

// ea: 0x00448F10
int ClientSpectatorInactivityTimer(Entity* /*ent*/)
{
    return 1;
}

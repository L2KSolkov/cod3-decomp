// ============================================================================
// COD3 Server Types — client_s, serverStatic_t, server_t, msg_t, netadr_t,
// netchan_t, reliableCommands_t, vm_s
// Reconstructed from IDA local types (PDB symbol data).
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

#include "game/game_types.h"
#include "game/player_types.h"
#include "game/trace_types.h"
#include "game/cvar_types.h"
#include "core/math_types.h"
#include "engine/broc_types.h"
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// netadrtype_t — address type enum
// ============================================================================
enum netadrtype_t {
    NA_BAD = 0,
    NA_LOOPBACK = 1,
    NA_BOT = 2,
    NA_IP = 3,
    NA_BROADCAST = 4,
};

// ============================================================================
// netsrc_t — network source enum
// ============================================================================
enum netsrc_t {
    NS_CLIENT = 0,
    NS_SERVER = 1,
};

// ============================================================================
// netadr_t — network address (20 bytes)
// Size: 0x14 (20 bytes) — verified against IDA
// ============================================================================
struct netadr_t {
    netadrtype_t type;             // +0x00
    uint8_t      ip[4];            // +0x04
    uint8_t      ipx[10];          // +0x08
    uint16_t     port;             // +0x12
};
static_assert(sizeof(netadr_t) == 0x14, "netadr_t size mismatch");
static_assert(offsetof(netadr_t, port) == 0x12, "netadr_t::port offset mismatch");

// ============================================================================
// serverState_t — server state enum
// ============================================================================
enum serverState_t {
    SS_DEAD = 0,
    SS_LOADING = 1,
    SS_GAME = 2,
};

// ============================================================================
// reliableCommands_t — per-client reliable command buffer (20 bytes)
// Size: 0x14 (20 bytes) — verified against IDA
// ============================================================================
struct reliableCommands_t {
    int    bufSize;          // +0x00
    char*  buf;              // +0x04
    char** commands;         // +0x08
    int*   commandLengths;   // +0x0C
    char*  rover;            // +0x10
};
static_assert(sizeof(reliableCommands_t) == 0x14, "reliableCommands_t size mismatch");

// ============================================================================
// netchan_t — network channel (2096 bytes)
// Size: 0x830 (2096 bytes) — verified against IDA
// ============================================================================
struct netchan_t {
    netsrc_t       sock;                  // +0x00
    int            dropped;               // +0x04
    netadr_t       remoteAddress;         // +0x08
    int            qport;                 // +0x1C
    int            incomingSequence;      // +0x20
    int            outgoingSequence;      // +0x24
    int            fragmentSequence;      // +0x28
    int            fragmentLength;        // +0x2C
    uint8_t        fragmentBuffer[2048];  // +0x30
};
static_assert(sizeof(netchan_t) == 0x830, "netchan_t size mismatch");
static_assert(offsetof(netchan_t, remoteAddress) == 0x08, "netchan_t::remoteAddress offset mismatch");
static_assert(offsetof(netchan_t, fragmentBuffer) == 0x30, "netchan_t::fragmentBuffer offset mismatch");

// ============================================================================
// msg_t — message buffer (20 bytes)
// Size: 0x14 (20 bytes) — verified against IDA
// ============================================================================
struct msg_t {
    int         overflowed;  // +0x00
    uint8_t*    data;        // +0x04
    int         maxsize;     // +0x08
    int         cursize;     // +0x0C
    int         readcount;   // +0x10
};
static_assert(sizeof(msg_t) == 0x14, "msg_t size mismatch");

// ============================================================================
// client state enum
// ============================================================================
enum clientState_t {
    CS_FREE = 0,
    CS_ACTIVE = 1,
    CS_ZOMBIE = 2,
    CS_CONNECTED = 3,
};

// ============================================================================
// client_s — per-client server state (4976 bytes)
// Size: 0x1370 (4976 bytes) — verified against IDA
// ============================================================================
struct client_s {
    clientState_t       state;                    // +0x000
    reliableCommands_t  reliableCommands;         // +0x004
    int                 reliableSequence;         // +0x018
    int                 reliableAcknowledge;      // +0x01C
    int                 reliableSent;             // +0x020
    int                 messageAcknowledge;       // +0x024
    int                 serverId;                 // +0x028
    int                 gamestateMessageNum;      // +0x02C
    usercmd_s           lastUsercmd;              // +0x030
    int                 lastMessageNum;           // +0x060
    int                 lastClientCommand;        // +0x064
    char                lastClientCommandString[256];  // +0x068
    DbLinkedHandle<EntityHandleDb, Entity> mEntityHandle;   // +0x168
    int                 deltaMessage;             // +0x16C
    PlayerState         frames[1];                // +0x170
    uint8_t             netchan[3120];            // +0x740
};
static_assert(sizeof(client_s) == 0x1370, "client_s size mismatch");
static_assert(offsetof(client_s, state) == 0x000, "client_s::state offset mismatch");
static_assert(offsetof(client_s, reliableCommands) == 0x004, "client_s::reliableCommands offset mismatch");
static_assert(offsetof(client_s, reliableSequence) == 0x018, "client_s::reliableSequence offset mismatch");
static_assert(offsetof(client_s, lastUsercmd) == 0x030, "client_s::lastUsercmd offset mismatch");
static_assert(offsetof(client_s, lastClientCommandString) == 0x068, "client_s::lastClientCommandString offset mismatch");
static_assert(offsetof(client_s, mEntityHandle) == 0x168, "client_s::mEntityHandle offset mismatch");
static_assert(offsetof(client_s, frames) == 0x170, "client_s::frames offset mismatch");
static_assert(offsetof(client_s, netchan) == 0x740, "client_s::netchan offset mismatch");

// ============================================================================
// serverStatic_t — static server state (60 bytes)
// Size: 0x3C (60 bytes) — verified against IDA
// ============================================================================
struct serverStatic_t {
    int        initialized;            // +0x00
    int        snapFlagServerBit;      // +0x04
    client_s*  clients;                // +0x08
    int        numSnapshotEntities;    // +0x0C
    int        nextSnapshotEntities;   // +0x10
    netadr_t   redirectAddress;        // +0x14
    netadr_t   authorizeAddress;       // +0x28
};
static_assert(sizeof(serverStatic_t) == 0x3C, "serverStatic_t size mismatch");
static_assert(offsetof(serverStatic_t, clients) == 0x08, "serverStatic_t::clients offset mismatch");

// ============================================================================
// server_t — server state (4112 bytes)
// Size: 0x1010 (4112 bytes) — verified against IDA
// ============================================================================
struct server_t {
    int             checksum;            // +0x000
    serverState_t   state;               // +0x004
    int             serverId;            // +0x008
    int             restartedServerId;   // +0x00C
    Broc::string    configstrings[1024]; // +0x010
};
static_assert(sizeof(server_t) == 0x1010, "server_t size mismatch");
static_assert(offsetof(server_t, checksum) == 0x000, "server_t::checksum offset mismatch");
static_assert(offsetof(server_t, state) == 0x004, "server_t::state offset mismatch");
static_assert(offsetof(server_t, configstrings) == 0x010, "server_t::configstrings offset mismatch");

// ============================================================================
// vm_s — virtual machine instance (140 bytes)
// Size: 0x8C (140 bytes) — verified against IDA
// ============================================================================
struct vm_s {
    int (__cdecl* systemCall)(int*);  // +0x00
    char    name[128];                // +0x04
    void*   dllHandle;                // +0x84
    int (*entryPoint)(int, ...);      // +0x88
};
static_assert(sizeof(vm_s) == 0x8C, "vm_s size mismatch");

// ============================================================================
// moveclip_t — clip move context (176 bytes)
// Size: 0xB0 (176 bytes) — verified against IDA
// ============================================================================
struct moveclip_t {
    math::Position3 mins;                      // +0x00
    math::Position3 maxs;                      // +0x10
    math::Position3 outerSize;                 // +0x20
    math::Position3 start;                     // +0x30
    math::Position3 end;                       // +0x40
    trace_t         trace;                     // +0x50
    DbLinkedHandle<EntityHandleDb, Entity> mPassEntity;  // +0xA0
    DbLinkedHandle<EntityHandleDb, Entity> mPassOwner;   // +0xA4
    int             contentmask;               // +0xA8
    int             capsule;                   // +0xAC
};
static_assert(sizeof(moveclip_t) == 0xB0, "moveclip_t size mismatch");

// ============================================================================
// pointtrace_t — point trace context (144 bytes)
// Size: 0x90 (144 bytes) — verified against IDA
// ============================================================================
struct pointtrace_t {
    math::Position3 start;                     // +0x00
    math::Position3 end;                       // +0x10
    trace_t         trace;                     // +0x20
    DbLinkedHandle<EntityHandleDb, Entity> mPassEntity;  // +0x70
    DbLinkedHandle<EntityHandleDb, Entity> mPassOwner;   // +0x74
    int             contentmask;               // +0x78
    int             bLocational;               // +0x7C
    float           mAngleTangent;             // +0x80
    unsigned char*  priorityMap;               // +0x84
};
static_assert(sizeof(pointtrace_t) == 0x90, "pointtrace_t size mismatch");

// ============================================================================
// sightclip_t — sight clip context (112 bytes)
// Size: 0x70 (112 bytes) — verified against IDA
// ============================================================================
struct sightclip_t {
    math::Position3 mins;                      // +0x00
    math::Position3 maxs;                      // +0x10
    math::Position3 outerSize;                 // +0x20
    math::Position3 start;                     // +0x30
    math::Position3 end;                       // +0x40
    DbLinkedHandle<EntityHandleDb, Entity> mPassEntity1; // +0x50
    DbLinkedHandle<EntityHandleDb, Entity> mPassEntity2; // +0x54
    DbLinkedHandle<EntityHandleDb, Entity> mPassOwner1;  // +0x58
    DbLinkedHandle<EntityHandleDb, Entity> mPassOwner2;  // +0x5C
    int             contentmask;               // +0x60
    int             capsule;                   // +0x64
};
static_assert(sizeof(sightclip_t) == 0x70, "sightclip_t size mismatch");

// ============================================================================
// sightpointtrace_t — sight point trace context (64 bytes)
// Size: 0x40 (64 bytes) — verified against IDA
// ============================================================================
struct sightpointtrace_t {
    math::Position3 start;                     // +0x00
    math::Position3 end;                       // +0x10
    DbLinkedHandle<EntityHandleDb, Entity> mPassEntity1; // +0x20
    DbLinkedHandle<EntityHandleDb, Entity> mPassEntity2; // +0x24
    DbLinkedHandle<EntityHandleDb, Entity> mPassOwner1;  // +0x28
    DbLinkedHandle<EntityHandleDb, Entity> mPassOwner2;  // +0x2C
    int             contentmask;               // +0x30
};
static_assert(sizeof(sightpointtrace_t) == 0x40, "sightpointtrace_t size mismatch");

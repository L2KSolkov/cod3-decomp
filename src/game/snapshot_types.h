// Snapshot types shared by the client and client-game ports.
// Layouts are taken from the IDA type database.
#pragma once

struct snapshot_t {
    int snapFlags;                 // +0x00
    int serverTime;                // +0x04
    unsigned char _pad08[0x08];    // +0x08
    unsigned char ps[0x5D0];       // +0x10
    int numEntities;               // +0x5E0
    int numServerCommands;         // +0x5E4
    int serverCommandSequence;     // +0x5E8
    unsigned char _tail5EC[4];      // +0x5EC
};
static_assert(sizeof(snapshot_t) == 0x5F0, "snapshot_t size mismatch");

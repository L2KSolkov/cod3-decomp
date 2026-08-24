// cg_global.h - shared cgGlobal_t layout from IDA local types
#pragma once

#include <stdint.h>

// IDA cgGlobal_t layout: 0x34 bytes at ?cgGlobal@@3UcgGlobal_t@@A.
struct cgGlobal_t {
    int frametime;       // +0x00
    int time;            // +0x04
    int oldTime;         // +0x08
    int cubemapShot;     // +0x0C
    int cubemapSize;     // +0x10
    bool teamGame;       // +0x14
    bool showScore;      // +0x15
    uint8_t _pad16[0x18 - 0x16];
    float gameTime;      // +0x18
    float gameTimeStartTime; // +0x1C
    int teamScores[5];   // +0x20
};
static_assert(sizeof(cgGlobal_t) == 0x34, "cgGlobal_t size mismatch");

extern cgGlobal_t cgGlobal;

// ============================================================================
// cg_local.h - client game (cg.o) shared types and renderer exports
// ============================================================================

#pragma once

#include <stdint.h>

#include "core/math_types.h"

struct Entity;

// Renderer export table (refexport_t; matches the layout used by core.o
// common.cpp's re_export_view and cl_parse.cpp's refexport_t2)
struct re_export_view {
    void (*Shutdown)(int);
    void (*BeginRegistration)(void*);
    void* (*RegisterModel)(void* result, const char*, int, int);
    int (*RegisterShader)(const char*, int);
    int (*RegisterShaderNoMip)(const char*, int);
    void (*LoadWorld)(const char*, int*);
    void (*SetFXImageMemory)(int);
    int (*GetFXImageMemory)();
    int (*GetImageMemory)();
    float (*GetFarPlaneDist)();
    void (*EndRegistration)();
    void (*ClearScene)();
    void (*AddPolyToScene)(void*, int, const void*);
    void (*AddLightToScene)(const float*, float, float, float, float);
    void (*SetCullDist)(float);
    void (*SetFog)(int, int, int, float, float, float, float);
    void (*RenderScene)(const void*);
    void (*ClearFlares)();
    void (*SetColor)(const float*);
    void (*DrawStretchPic)(float, float, float, float, float, float, float,
                           float, void*);
    void (*DrawStretchPicGradient)(float, float, float, float, float, float,
                                   float, float, void*, const float*, int);
    void (*DrawStretchPicRotate)(float, float, float, float, float, float,
                                 float, float, float, void*);
    void (*DrawQuadPic)(const float (*)[2], const float (*)[2], void*);
    void (*DrawStretchRaw)(int, int, int, int, int, int,
                           const unsigned char*, int, int);
    void (*UploadCinematic)(int, int, int, int, const unsigned char*, int, int);
    void (*BeginFrame)();
    void (*EndFrame)(int*, int*);
    void (*SaveScreen)();
    void (*TrackStatistics)(void*);
    int (*PickShader)(const float*, const float*, char*, char*, char*, int);
    void (*ResetImageAllocations)();
    void (*FreeImageAllocations)();
    void (*CubemapShot)(const char*, int, int, float, float);
    void (*CubemapWaterShot)(const char*, int, int, float*, float*);
    void (*LocateDebugStrings)(void*, int);
    void (*LocateDebugLines)(void*, int);
    int (*Text_Width)(const char*, int, float, float, int);
    int (*Text_Height)(int, float);
    void (*Text_Paint)(float, float, int, float, const float*, const char*,
                       float, int, int);
    int (*Text_ConsoleWidth)(const short*, int, float, float, int);
    void (*Text_ConsolePaint)(float, float, int, float, const float*,
                              const short*, float, int, int);
    void (*Text_PaintWithCursor)(float, float, int, float, const float*,
                                 const char*, int, char, float, int, int);
};

extern re_export_view re;

extern void* _Z_MallocInternal(unsigned int size);
extern void  _Z_FreeInternal(void* ptr);

// EntityHandleDb - entity handle database (elements at +0xA8)
class EntityHandleDb {
public:
    struct DbElement {
        Entity* mObject;  // +0x00
        int     mKey;     // +0x04
    };
    unsigned char _pad[0xA8];
    DbElement     mElements[0x540];
    static EntityHandleDb sInst;  // ?sInst@EntityHandleDb@@0V1@A
};

struct weaponFileInfo_t {
    char  szKillIcon[64];      // +0x00
    char  szInternalName[64];  // +0x40
    unsigned char bWideKillIcon;  // +0x80
};

// PlayerState - subset of the fields cg.o touches (full size 0x5D0)
struct PlayerState {
    math::Position3 origin;            // +0x00
    int pm_flags;                      // +0x2C
    float fWeaponPosFrac;              // +0xAC
    int lastWeapon;                    // +0xC4
    int eFlags;                        // +0xF4
    int ammoclip[92];                  // +0x2B4
    int weapons[2];                    // +0x424
    unsigned char weaponslots[10];     // +0x42C
    int weapon;                        // +0xA4
    int vehPos;                        // +0x524
    int vehType;                       // +0x528
    unsigned int mFlags;               // +0x5C8
    unsigned char _pad[0x5D0 - 0x5CC];
};

struct AnimIKFireEvent {
    int fireTime;    // +0x00
    int fireWeapon;  // +0x04
};

struct Client {
    PlayerState ps;  // +0x00
    AnimIKFireEvent AnimIKFireEvents[15];  // +0x5D0
};

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

#define CG_ASSERT(expr, file, line)                                       \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)

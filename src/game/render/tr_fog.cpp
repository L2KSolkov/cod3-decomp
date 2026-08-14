// ============================================================================
// tr_fog.cpp - render.o fog helpers (tr_fog.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"

#include <string.h>

// glfog_t (IDA type; size 0x40)
struct glfog_t {
    int mode;           // +0x00
    int hint;           // +0x04
    int startTime;      // +0x08
    int finishTime;     // +0x0C
    float color[4];     // +0x10
    float start;        // +0x20
    float end;          // +0x24
    int useEndForClip;  // +0x28
    float density;      // +0x2C
    int registered;     // +0x30
    int drawsky;        // +0x34
    int clearscreen;    // +0x38
    int dirty;          // +0x3C
};
static_assert(sizeof(glfog_t) == 0x40, "glfog_t size mismatch");

// glfogType_t (IDA enum; codmp_xboxr.xbe.h)
enum glfogType_t {
    FOG_NONE = 0x0,
    FOG_SKY = 0x1,
    FOG_PORTALVIEW = 0x2,
    FOG_MAP = 0x3,
    FOG_SERVER = 0x4,
    FOG_CURRENT = 0x5,
    FOG_LAST = 0x6,
    FOG_TARGET = 0x7,
    FOG_CMD_SWITCHFOG = 0x8,
    NUM_FOGS = 0x9,
};

glfog_t glfogsettings[NUM_FOGS];  // ?glfogsettings@@3PAUglfog_t@@A @ 0xF73F38
glfogType_t glfogNum;             // ?glfogNum@@3W4glfogType_t@@A @ 0xF74424

// trRefdef_t view (refdef at tr+0x26C; time +0x18)
struct trRefdef_t {
    int x;                     // +0x00
    int y;                     // +0x04
    int width;                 // +0x08
    int height;                // +0x0C
    float fov_x;               // +0x10
    float fov_y;               // +0x14
    int time;                  // +0x18
    int rdflags;               // +0x1C
    short num_world_dlights;   // +0x20
    short num_model_dlights;   // +0x22
};

struct trGlobals_t {
    uint8_t _pad[0x26C];
    trRefdef_t refdef;         // +0x26C
};
extern trGlobals_t tr;         // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0

// ea: 0x006C0840
void R_Fog(glfog_t* fog)
{
    (void)fog;
}

// ea: 0x006C0850
void R_FogOff()
{
}

// ea: 0x006C0860
void R_FogOn()
{
}

// ea: 0x006C0870
void R_SetFogColor()
{
}

// ea: 0x006C0880
void R_SetFog(int fogvar, int var1, int var2, float r, float g, float b,
              float density)
{
    if (fogvar == FOG_CMD_SWITCHFOG)
    {
        if (var1 == FOG_MAP)
        {
            if (glfogsettings[FOG_CURRENT].registered != 0)
                glfogsettings[FOG_LAST] = glfogsettings[FOG_CURRENT];
            memset(&glfogsettings[FOG_MAP], 0, sizeof(glfog_t));
            memset(&glfogsettings[FOG_TARGET], 0, sizeof(glfog_t));
            glfogNum = FOG_NONE;
        }
        else
        {
            if (glfogsettings[var1].registered == 1)
            {
                glfogNum = (glfogType_t)var1;
                if (glfogsettings[FOG_CURRENT].registered == 0)
                    glfogsettings[FOG_LAST] = glfogsettings[var1];
                else
                    glfogsettings[FOG_LAST] = glfogsettings[FOG_CURRENT];
                glfogsettings[FOG_TARGET] = glfogsettings[var1];
                if (var2 != 0)
                {
                    glfogsettings[FOG_TARGET].startTime = tr.refdef.time;
                    glfogsettings[FOG_TARGET].finishTime = var2 + tr.refdef.time;
                }
                else
                {
                    glfogsettings[FOG_TARGET].startTime = 0;
                    glfogsettings[FOG_TARGET].finishTime = 0;
                    glfogsettings[FOG_TARGET].dirty = 1;
                    glfogsettings[FOG_CURRENT].dirty = 1;
                }
            }
        }
    }
    else if (var1 != FOG_NONE || var2 != 0)
    {
        glfogsettings[fogvar].color[0] = r;
        glfogsettings[fogvar].color[1] = g;
        glfogsettings[fogvar].color[2] = b;
        glfogsettings[fogvar].color[3] = 1.0f;
        glfogsettings[fogvar].start = (float)var1;
        glfogsettings[fogvar].end = (float)var2;
        glfogsettings[fogvar].clearscreen = 0;
        glfogsettings[fogvar].drawsky = 1;
        if (density < 1.0f)
            glfogsettings[fogvar].density = density;
        else
            glfogsettings[fogvar].density = 1.0f;
        glfogsettings[fogvar].registered = 1;
    }
    else
    {
        glfogsettings[fogvar].registered = 0;
    }
}

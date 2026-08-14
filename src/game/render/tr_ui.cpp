// ============================================================================
// tr_ui.cpp - render.o UI bridge helpers (tr_ui.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include <stdint.h>

struct nglTexture;
struct polyVert_t;
struct refdef_s;

extern "C" {
int (*syscall_0)(int, ...);      // @ 0xDFB154
void* _Z_MallocInternal(int size);
void  _Z_FreeInternal(void* ptr);
}

// refexport_t view (re; members at IDA offsets)
struct refexport_t {
    uint8_t _pad0[0x30];
    void (*AddPolyToScene)(nglTexture*, int, const polyVert_t*);  // +0x30
    uint8_t _pad1[0x40 - 0x34];
    void (*RenderScene)(const refdef_s*);                         // +0x40
    uint8_t _pad2[0x4C - 0x44];
    void (*DrawStretchPic)(float, float, float, float, float, float,
                           float, float, nglTexture*);            // +0x4C
};
extern refexport_t re;           // ?re@@3Urefexport_t@@A @ 0xF0CC50

// ============================================================================
// ui_dllEntry - ea: 0x006C2CF0
// ============================================================================
void ui_dllEntry(int (*syscallptr)(int, ...))
{
    syscall_0 = syscallptr;
}

// ============================================================================
// trap_R_AddPolyToScene - ea: 0x006C2DA0
// ============================================================================
void trap_R_AddPolyToScene(nglTexture* tex, int numVerts, const polyVert_t* verts)
{
    re.AddPolyToScene(tex, numVerts, verts);
}

// ============================================================================
// trap_R_AddLightToScene - ea: 0x006C2DB0
// ============================================================================
void trap_R_AddLightToScene(const float* const origin, float r, float g,
                            float b, float intensity)
{
    (void)origin; (void)r; (void)g; (void)b; (void)intensity;
}

// ============================================================================
// trap_R_RenderScene - ea: 0x006C2DC0
// ============================================================================
void trap_R_RenderScene(const refdef_s* fd)
{
    re.RenderScene(fd);
}

// ============================================================================
// trap_R_DrawStretchPic - ea: 0x006C2DE0
// ============================================================================
void trap_R_DrawStretchPic(float x, float y, float w, float h, float s1,
                           float t1, float s2, float t2, nglTexture* tex,
                           float unused)
{
    (void)unused;
    re.DrawStretchPic(x, y, w, h, s1, t1, s2, t2, tex);
}

// ============================================================================
// ui_Z_MallocInternal - ea: 0x006C2DF0
// ============================================================================
void* ui_Z_MallocInternal(int size)
{
    return _Z_MallocInternal(size);
}

// ============================================================================
// ui_Z_FreeInternal - ea: 0x006C2E00
// ============================================================================
void ui_Z_FreeInternal(void* ptr)
{
    _Z_FreeInternal(ptr);
}

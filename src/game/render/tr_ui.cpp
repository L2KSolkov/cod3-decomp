// ============================================================================
// tr_ui.cpp - render.o UI bridge helpers (tr_ui.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include <stdint.h>
#include <string.h>

struct nglTexture;
struct polyVert_t;
struct refdef_s;
struct glconfig_t;
struct trStatistics_t;
struct trDebugString_t;
struct trDebugLine_t;

extern "C" {
int (*syscall_0)(int, ...);      // @ 0xDFB154
void* _Z_MallocInternal(int size);
void  _Z_FreeInternal(void* ptr);
}

// refexport_t view (re; members at IDA offsets)
struct refexport_t {
    void (*Shutdown)(int);                                      // +0x00
    void (*BeginRegistration)(glconfig_t*);                     // +0x04
    void* (*RegisterModel)(void*, const char*, int, int);       // +0x08
    uint8_t _pad0[0x14 - 0x0C];
    void (*LoadWorld)(const char*, int*);                       // +0x14
    uint8_t _pad0b[0x24 - 0x18];
    float (*GetFarPlaneDist)();                                 // +0x24
    void (*EndRegistration)();                                  // +0x28
    void (*ClearScene)();                                       // +0x2C
    void (*AddPolyToScene)(nglTexture*, int, const polyVert_t*);  // +0x30
    uint8_t _pad1[0x38 - 0x34];
    void (*SetCullDist)(float);                                 // +0x38
    void (*SetFog)(int, int, int, float, float, float, float);  // +0x3C
    void (*RenderScene)(const refdef_s*);                         // +0x40
    uint8_t _pad2[0x48 - 0x44];
    void (*SetColor)(const float*);                             // +0x48
    void (*DrawStretchPic)(float, float, float, float, float, float,
                           float, float, nglTexture*, float);     // +0x4C
    uint8_t _pad3[0x54 - 0x50];
    void (*DrawStretchPicRotate)(float, float, float, float, float,
                                 float, float, float, float,
                                 nglTexture*);                  // +0x54
    void (*DrawQuadPic)(const float (*)[2], const float (*)[2],
                        nglTexture*);                           // +0x58
    uint8_t _pad4[0x64 - 0x5C];
    void (*BeginFrame)();                                       // +0x64
    void (*EndFrame)(int*, int*);                               // +0x68
    void (*SaveScreen)();                                       // +0x6C
    void (*TrackStatistics)(trStatistics_t*);                   // +0x70
    uint8_t _pad5[0x80 - 0x74];
    void (*CubemapShot)(const char*, int, int, float, float);   // +0x80
    void (*CubemapWaterShot)(const char*, int, int, float*,
                             float*);                           // +0x84
    void (*LocateDebugStrings)(trDebugString_t*, int);          // +0x88
    void (*LocateDebugLines)(trDebugLine_t*, int);              // +0x8C
    int (*Text_Width)(const char*, int, float, float, int);     // +0x90
    int (*Text_Height)(int, float);                             // +0x94
    void (*Text_Paint)(float, float, int, float, const float*,
                       const char*, float, int, int);           // +0x98
    int (*Text_ConsoleWidth)(const short*, int, float, float,
                             int);                              // +0x9C
    void (*Text_ConsolePaint)(float, float, int, float,
                              const float*, const short*, float,
                              int, int);                        // +0xA0
    void (*Text_PaintWithCursor)(float, float, int, float, const float*,
                                 const char*, int, char, float, int,
                                 int);                             // +0xA4
};
extern refexport_t re;           // ?re@@3Urefexport_t@@A @ 0xF0CC50

// refimport_t full view (IDA layout; GetRefAPI copies it wholesale)
struct refimport_t {
    void (*Printf)(int, const char*, ...);              // +0x00
    void (*Error)(int, const char*, ...);               // +0x04
    uint8_t _pad[0x28 - 0x08];
    void (*Cvar_Set)(const char*, const char*);         // +0x28
    void (*Cmd_AddCommand)(const char*, void (__cdecl*)());  // +0x2C
    uint8_t _pad3[0x80 - 0x30];
    void (*AdjustFrom640)(float*, float*, float*, float*);   // +0x80
    void* (*UI_GetFontInfo)(int, float);                // +0x84
};
extern refimport_t ri;           // ?ri@@3Urefimport_t@@A @ 0xF741E8

// ============================================================================
// ui_dllEntry - ea: 0x006C2CF0
// ============================================================================
void ui_dllEntry(int (*syscallptr)(int, ...))
{
    syscall_0 = syscallptr;
}

// ============================================================================
// GetRefAPI - ea: 0x006DCCB0
// ============================================================================
void RE_Shutdown(int destroyWindow);       // tr_dobj2.cpp
void RE_BeginRegistration(glconfig_t* glconfigOut);  // tr_gl.cpp
void* RE_RegisterModel(void* result, const char* name, int pakId,
                       int a4);  // tr_aeps2.cpp (sret IVPointer<XModel>)
void RE_LoadWorldMap(const char* name, int* checksum);  // r_stubs.cpp
float RE_GetFarPlaneDist();                // r_stubs.cpp
void RE_EndRegistration();                 // render.o
void RE_BeginFrame();                      // render.o
void RE_EndFrame(int* a, int* b);          // tr_main.cpp
void RE_ClearScene();                      // tr_scene.cpp
void RE_AddPolyToScene(nglTexture* tex, int numVerts, const polyVert_t* verts);  // tr_scene.cpp
void RE_SetCullDist(float dist);           // r_stubs.cpp
void R_SetFog(int a1, int a2, int a3, float a4, float a5, float a6, float a7);  // render.o
void RE_RenderScene(const refdef_s* fd);   // render.o
void RE_SetColor(const float* rgba);       // r_stubs.cpp
void RE_StretchPic(float x, float y, float w, float h, float s1, float t1,
                   float s2, float t2, nglTexture* tex, float z);  // re_quad.cpp
void RE_StretchPicRotate(float x, float y, float w, float h, float s1,
                         float t1, float s2, float t2, float fRot,
                         nglTexture* tex);  // re_quad.cpp
void RE_DrawQuadPic(const float (*vVerts)[2], const float (*vST)[2],
                    nglTexture* tex);      // re_quad.cpp
void RE_LocateDebugStrings(trDebugString_t* strings, int numStrings);  // r_stubs.cpp
void RE_LocateDebugLines(trDebugLine_t* lines, int numLines);          // r_stubs.cpp
void RE_TrackStatistics(trStatistics_t* statistics);  // r_stubs.cpp
int RE_Text_Width(const char* text, int font, float scale, float charWidth,
                  int limit);              // tr_font.cpp
int RE_Text_Height(int font, float scale); // render.o
void RE_Text_Paint(float x, float y, int font, float scale,
                   const float* color, const char* text, float charWidth,
                   int limit, int style);  // render.o
int RE_Text_ConsoleWidth(const short* text, int font, float scale,
                         float charWidth, int limit);  // render.o
void RE_Text_ConsolePaint(float x, float y, int font, float scale,
                          const float* color, const short* psString,
                          float charWidth, int limit, int style);  // tr_font.cpp
void RE_Text_PaintWithCursor(float x, float y, int font, float scale,
                             const float* const color, const char* text, int cursorPos,
                             char cursor, float depth, int limit,
                             int style);   // tr_text.cpp

// RE_Text_Paint - ea: 0x006D1FE0
void RE_Text_Paint(float x, float y, int font, float scale,
                   const float* color, const char* text, float charWidth,
                   int limit, int style)
{
    RE_Text_PaintWithCursor(x, y, font, scale, color, text, -1, 0,
                            charWidth, limit, style);
}
// IDA labels these three renderer slots as nullsub forwarding thunks.
void j_nullsub_110() {}
void j_nullsub_111() {}
void j_nullsub_112() {}

refexport_t* GetRefAPI(int apiVersion, refimport_t* rimp)
{
    memcpy(&ri, rimp, sizeof(ri));
    memset(&re, 0, sizeof(re));
    if (apiVersion == 14)
    {
        re.Shutdown = RE_Shutdown;
        re.BeginRegistration = RE_BeginRegistration;
        re.RegisterModel = RE_RegisterModel;
        re.LoadWorld = RE_LoadWorldMap;
        re.EndRegistration = RE_EndRegistration;
        re.GetFarPlaneDist = RE_GetFarPlaneDist;
        re.BeginFrame = RE_BeginFrame;
        re.EndFrame = RE_EndFrame;
        re.SaveScreen = j_nullsub_110;
        re.ClearScene = RE_ClearScene;
        re.AddPolyToScene = RE_AddPolyToScene;
        re.SetCullDist = RE_SetCullDist;
        re.SetFog = R_SetFog;
        re.RenderScene = RE_RenderScene;
        re.SetColor = RE_SetColor;
        re.DrawStretchPic = (void (*)(float, float, float, float, float,
                                      float, float, float, nglTexture*,
                                      float))RE_StretchPic;
        re.DrawStretchPicRotate = RE_StretchPicRotate;
        re.DrawQuadPic = RE_DrawQuadPic;
        re.CubemapShot = (void (*)(const char*, int, int, float,
                                   float))j_nullsub_111;
        re.CubemapWaterShot = (void (*)(const char*, int, int, float*,
                                        float*))j_nullsub_112;
        re.LocateDebugStrings = RE_LocateDebugStrings;
        re.LocateDebugLines = RE_LocateDebugLines;
        re.TrackStatistics = RE_TrackStatistics;
        re.Text_Width = RE_Text_Width;
        re.Text_Height = RE_Text_Height;
        re.Text_Paint = RE_Text_Paint;
        re.Text_ConsoleWidth = RE_Text_ConsoleWidth;
        re.Text_ConsolePaint = RE_Text_ConsolePaint;
        re.Text_PaintWithCursor = RE_Text_PaintWithCursor;
        return &re;
    }
    ri.Printf(0, "Mismatched REF_API_VERSION: expected %i, got %i\n", 14,
              apiVersion);
    return nullptr;
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
    re.DrawStretchPic(x, y, w, h, s1, t1, s2, t2, tex, unused);
}

// ============================================================================
// trap_R_Text_PaintWithCursor - ea: 0x006C2D20
// ============================================================================
void trap_R_Text_PaintWithCursor(float x, float y, int font, float scale,
                                 const float* const color, const char* text,
                                 int cursorPos, char cursor, int limit,
                                 int style)
{
    re.Text_PaintWithCursor(x, y, font, scale, color, text, cursorPos,
                            cursor, 0.0f, limit, style);
}

// trap_R_Text_Paint - ea: 0x006C2D10
void trap_R_Text_Paint(float x, float y, int font, float scale,
                       const float* color, const char* text,
                       float charWidth, int limit, int style)
{
    re.Text_Paint(x, y, font, scale, color, text, charWidth, limit, style);
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

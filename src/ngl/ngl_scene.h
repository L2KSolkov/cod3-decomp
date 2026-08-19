// ============================================================================
// ngl_scene.h - NGL scene types + scene API (ngl_xboxr:ngl_scene.o).
// Source: src/ngl_scene.cpp
// Layouts verified against IDA local types.
// ============================================================================
#ifndef COD3_NGL_NGL_SCENE_H
#define COD3_NGL_NGL_SCENE_H

#include "core/math_types.h"

struct nglTexture;
struct nglLightContext;
class nglRenderNode;

// ============================================================================
// nglSortInfo - 8 bytes
// ============================================================================
struct nglSortInfo {
    enum Type : int {
        NGLSORT_OPAQUE = 0x0,
        NGLSORT_TRANSLUCENT = 0x1,
    };
    Type Type;      // +0x00
    union {
        float        Dist;   // +0x04
        unsigned int Hash;   // +0x04
    };
};
static_assert(sizeof(nglSortInfo) == 8, "nglSortInfo size mismatch");

// ============================================================================
// nglSceneParamType
// ============================================================================
enum nglSceneParamType {
    NGLSCENE_DEFAULTS = 0,
    NGLSCENE_PARENT = 1,
    NGLSCENE_ROOT = 2,
};

// ============================================================================
// nglProjType
// ============================================================================
enum nglProjType {
    NGLPROJ_ORTHOGRAPHIC = 0,
    NGLPROJ_PERSPECTIVE = 1,
};

// ============================================================================
// nglMatrixType
// ============================================================================
enum nglMatrixType {
    NGLMTX_VIEW_TO_WORLD = 0,
    NGLMTX_VIEW_TO_SCREEN = 1,
    NGLMTX_WORLD_TO_VIEW = 2,
    NGLMTX_WORLD_TO_SCREEN = 3,
    NGLMTX_PROJECTION = 4,
    NGLMTX_UI = 5,
};

// ============================================================================
// nglSceneCallback - 8 bytes
// ============================================================================
struct nglSceneCallback {
    void (*Fn)(void*);   // +0x00
    void* Data;          // +0x04
};
static_assert(sizeof(nglSceneCallback) == 8, "nglSceneCallback size mismatch");

enum nglSceneCallbackType {
    NGLSCENE_PRE = 0,
    NGLSCENE_MID = 1,
    NGLSCENE_POST = 2,
    NGLSCENE_STARTSCENE = 3,
    NGLSCENE_SETUPSCENE = 4,
};

// ============================================================================
// nglLockedTextureNode - 12 bytes
// ============================================================================
struct nglLockedTextureNode {
    nglTexture*            Tex;       // +0x00
    unsigned int           OldFormat; // +0x04
    nglLockedTextureNode*  Next;      // +0x08
};
static_assert(sizeof(nglLockedTextureNode) == 0x0C, "nglLockedTextureNode size mismatch");

// ============================================================================
// nglParamSet / nglSceneParamSet (4 bytes each, verified against IDA).
// ============================================================================
class nglParamSet {
public:
    unsigned int* Array;  // +0x00
};
static_assert(sizeof(nglParamSet) == 4, "nglParamSet size mismatch");

struct nglSceneParamSet : nglParamSet {
public:
    static unsigned int NumParams;  // ngl_params.o
};
static_assert(sizeof(nglSceneParamSet) == 4, "nglSceneParamSet size mismatch");
struct nglScene {
    unsigned int      ClearStencil;       // +0x000 (ngliScene base)
    int               CubeMapFace;        // +0x004
    uint8_t           _pad8[8];           // +0x008
    math::Mat44       Projection;         // +0x010
    math::Mat44       View;               // +0x050
    math::Mat44       Device;             // +0x090
    math::Mat43       ViewToWorld;        // +0x0D0
    math::Mat44       ViewToScreen;       // +0x110
    math::Mat43       WorldToView;        // +0x150
    math::Mat44       WorldToScreen;      // +0x190
    math::Mat44       ViewportToWorld;    // +0x1D0
    math::Mat44       UIToDevice;         // +0x210
    math::Position3   ViewPos;            // +0x250
    math::Dir3        ViewDir;            // +0x260
    math::Vector4     ClipPlanes[6];      // +0x270
    nglScene*         Parent;             // +0x2D0
    nglScene*         NextSibling;        // +0x2D4
    nglScene*         FirstChild;         // +0x2D8
    nglScene*         LastChild;          // +0x2DC
    nglSceneCallback  Pre;                // +0x2E0
    nglSceneCallback  Mid;                // +0x2E8
    nglSceneCallback  Post;               // +0x2F0
    nglSceneCallback  StartScene;         // +0x2F8
    nglSceneCallback  SetupScene;         // +0x300
    nglTexture*       RenderTarget;       // +0x308
    nglTexture*       ZTarget;            // +0x30C
    nglProjType       ProjType;           // +0x310
    nglRenderNode*    OpaqueRenderList;   // +0x314
    nglRenderNode*    TransRenderList;    // +0x318
    unsigned int      OpaqueListCount;    // +0x31C
    unsigned int      TransListCount;     // +0x320
    nglLockedTextureNode* LockedTextures; // +0x324
    bool              UploadFB;           // +0x328
    bool              DownloadFB;         // +0x329
    bool              UploadZ;            // +0x32A
    bool              DownloadZ;          // +0x32B
    float             ForceSlope_bias;    // +0x32C
    float             ForceDepth_bias;    // +0x330
    int               ForceCullMode;      // +0x334
    int               Tiles;              // +0x338
    nglLightContext*  LightContext;       // +0x33C
    int               ViewX1;             // +0x340
    int               ViewY1;             // +0x344
    int               ViewX2;             // +0x348
    int               ViewY2;             // +0x34C
    float             vx1;                // +0x350
    float             vy1;                // +0x354
    float             vx2;                // +0x358
    float             vy2;                // +0x35C
    float             sx1;                // +0x360
    float             sy1;                // +0x364
    float             sx2;                // +0x368
    float             sy2;                // +0x36C
    float             sx1p;               // +0x370
    float             sy1p;               // +0x374
    float             sx2p;               // +0x378
    float             sy2p;               // +0x37C
    float             h2;                 // +0x380
    float             v2;                 // +0x384
    unsigned int      ClearFlags;         // +0x388
    float             ClearZ;             // +0x38C
    math::Vector4     ClearColor;         // +0x390
    unsigned int      FBWriteMask;        // +0x3A0
    bool              ZWriteEnable;       // +0x3A4
    bool              ZTestEnable;        // +0x3A5
    bool              FogEnabled;         // +0x3A6
    uint8_t           _pad3A7;            // +0x3A7
    float             FogNear;            // +0x3A8
    float             FogFar;             // +0x3AC
    float             FogMin;             // +0x3B0
    float             FogMax;             // +0x3B4
    uint8_t           _pad3B8[8];         // +0x3B8
    math::Vector4     FogColor;           // +0x3C0
    bool              DepthOfFieldEnabled;// +0x3D0
    uint8_t           _pad3D1[3];         // +0x3D1
    float             FocusDepth;         // +0x3D4
    bool              MatricesDirty;      // +0x3D8
    uint8_t           _pad3D9[3];         // +0x3D9
    float             AspectRatio;        // +0x3DC
    float             FOV;                // +0x3E0
    float             NearZ;              // +0x3E4
    float             FarZ;               // +0x3E8
    float             AnimTime;           // +0x3EC
    float             CurAnimTime;        // +0x3F0
    unsigned int      IFLFrame;           // +0x3F4
    nglSceneParamSet  SceneParams;        // +0x3F8
};
static_assert(sizeof(nglScene) == 0x400, "nglScene size mismatch");

// ============================================================================
// ngl_scene.o (data, defined in ngl_scene.cpp)
// ============================================================================
extern nglScene* nglBuildScene;
extern nglScene* nglRootBuildScene;
extern unsigned char* nglListWork;
extern unsigned char* nglListWorkPos;
extern int nglListWorkSize;
extern int nglLastListAllocWarnFrame;
extern void (*nglEndOfRenderCallback)(void* Data);
extern void* nglEndOfRenderData;
extern void (*nglEndOfFrameCallback)(void* Data);
extern void* nglEndOfFrameData;
extern void (*nglEndOfVBlankCallback)(void* Data);
extern void* nglEndOfVBlankData;

// ============================================================================
// ngl_scene.o (functions, defined in ngl_scene.cpp)
// ============================================================================
void nglValidateMatrices(nglScene* Scene);
void nglCalculateMatrices(nglScene* Scene);
void nglSetRenderTarget(nglTexture* Tex);
void nglSetZTarget(nglTexture* Tex);
void nglSetClearStencil(unsigned int Stencil);
void nglSetClearFlags(unsigned int ClearFlags);
void nglSetClearColor(float r, float g, float b, float a);
void nglSetClearZ(float Z);
void nglSetFBWriteMask(unsigned int WriteMask);
void nglSetZWriteEnable(bool Enable);
void nglSetZTestEnable(bool Enable);
void nglSetFBCopy(bool Upload, bool Download);
void nglSetZCopy(bool Upload, bool Download);
void nglSetRenderTiling(int Tiles);
void nglEnableFog(bool Enable);
void nglSetFogColor(float r, float g, float b);
void nglSetFogRange(float Near, float Far, float Min, float Max);
void nglEnableDepthOfField(bool Enable);
void nglSetFocusDepth(float Depth);
void nglSetAnimTime(float Time);
nglSceneParamSet* nglGetSceneParams(nglScene* Scene);
math::Mat44 Perspective(float hs, float vs, float zn, float zf);
math::Mat44 Ortho(float ax, float ay, float zn, float zf);
math::Mat44 Viewport(float x1, float y1, float x2, float y2);
math::Mat44 InvScissor(float sx1, float sy1, float sx2, float sy2);
math::Mat44 ViewportToWorld(nglScene* Scene);
math::Mat44 UI(nglScene* Scene);
void nglSetView(float x1, float y1, float x2, float y2);
void nglSetAspectRatio(float a);
void nglSetPerspectiveMatrix(float fov, float nearz, float farz);
void nglSetOrthoMatrix(float nearz, float farz);
void nglSetWorldToViewMatrix(const math::Mat43& WorldToView);
void nglSetCameraMatrix(const math::Mat43& CameraToWorld);
void nglSetSceneCallBack(nglSceneCallbackType Type, void (*Fn)(void*), void* Data);
void nglSetEndOfRenderCallback(void (*Fn)(void*), void* Data);
void nglSetEndOfFrameCallback(void (*Fn)(void*), void* Data);
void nglSetEndOfVBlankCallback(void (*Fn)(void*), void* Data);
float nglGetRemainingFrameTime();
void nglListEndScene();
nglScene* nglListSelectScene(nglScene* scene);
bool nglHiresScreenShotInProgress();
unsigned int nglHiresScreenShotNumColumns();
unsigned int nglHiresScreenShotNumRows();
void nglBeginHiresScreenShot(int Width, int Height);
bool nglSaveHiresScreenshot();
void nglAdjustViewForHiresScreenshot();
void nglSyncFrameBuffers();
void nglLockTexture(nglTexture* Tex);
void nglSetScissorWH(float x1, float y1, float x2, float y2, float w, float h);
void nglSetScissor(float x1, float y1, float x2, float y2);
void nglSetViewport(float x1, float y1, float x2, float y2);
math::Mat44* nglGetMatrix(math::Mat44* result, nglMatrixType ID, nglScene* Scene);
math::Mat44 nglGetMatrix(nglMatrixType ID, nglScene* Scene);
void nglGetMatrix(math::Mat44& result, nglMatrixType ID, nglScene* Scene);
const math::Mat43& nglGetMatrix_ViewToWorld(nglScene* Scene);
const math::Mat43& nglGetMatrix_WorldToView(nglScene* Scene);
const math::Mat44& nglGetMatrix_ViewToScreen(nglScene* Scene);
const math::Mat44& nglGetMatrix_WorldToScreen(nglScene* Scene);
math::Position3* nglProjectPoint(math::Position3* result, const math::Position3* In,
                                 nglScene* Scene);
void nglProjectPoint(math::Position3& result, const math::Position3& In,
                     nglScene* Scene);
math::Position3* nglUnprojectPoint(math::Position3* result, const math::Position3* In,
                                   nglScene* Scene);
math::Position3 nglUnprojectPoint(const math::Position3& In, nglScene* Scene);
void nglSetDefaultSceneParams();
void nglSetupBeginScene(nglScene* Scene, nglSceneParamType ParamSource);
nglScene* nglListBeginScene(nglSceneParamType ParamSource);
void nglListAddCustomNode(void (*CustomNodeFn)(void*), void* Data, const nglSortInfo* SortInfo);
nglScene* nglListBeginSceneNode(nglSceneParamType ParamSource, nglSortInfo* SortInfo);
void nglSortScene(nglScene* Scene);
void nglPresent();

#endif // COD3_NGL_NGL_SCENE_H

// ============================================================================
// ngl_lighting.h - NGL lighting types + API (ngl_xboxr:ngl_lighting.o).
// Source: src/ngl_lighting.cpp
// Layouts verified against IDA local types.
// ============================================================================
#ifndef COD3_NGL_NGL_LIGHTING_H
#define COD3_NGL_NGL_LIGHTING_H

#include "core/math_types.h"

struct nglTexture;
struct nglMeshNode;
struct nglScene;

// ============================================================================
// nglLightType
// ============================================================================
enum nglLightType {
    NGLLIGHT_POINT = 0,
    NGLLIGHT_DIRECTIONAL = 1,
    NGLLIGHT_PROJECTED_DIRECTIONAL = 2,
    NGLLIGHT_PROJECTED_SPOT = 3,
    NGLLIGHT_PROJECTED_PARALLEL = 4,
    NGLLIGHT_POINTGUN = 5,
    NGLLIGHT_SUN = 6,
    NGLLIGHT_SPECIAL = 7,
    NGLLIGHT_USER_FIRST = 8,
};

// ============================================================================
// nglLightNode - 48 bytes
// ============================================================================
struct nglLightNode {
    nglLightNode* Next[8];   // +0x00
    nglLightNode* LocalNext; // +0x20
    unsigned int  LightCat;  // +0x24
    nglLightType  Type;      // +0x28
    void*         NodeData;  // +0x2C
};
static_assert(sizeof(nglLightNode) == 0x30, "nglLightNode size mismatch");

// ============================================================================
// nglLightContext - 112 bytes
// ============================================================================
struct nglLightContext {
    nglLightNode  Head;      // +0x00
    nglLightNode  ProjHead;  // +0x30
    math::Vector4 Ambient;   // +0x60
};
static_assert(sizeof(nglLightContext) == 0x70, "nglLightContext size mismatch");

// ============================================================================
// nglDirLightInfo - 32 bytes
// ============================================================================
struct nglDirLightInfo {
    math::Dir3     Dir;    // +0x00
    math::Vector4  Color;  // +0x10
};
static_assert(sizeof(nglDirLightInfo) == 0x20, "nglDirLightInfo size mismatch");

// ============================================================================
// nglPointLightInfo - 48 bytes
// ============================================================================
struct nglPointLightInfo {
    math::Position3 Pos;                // +0x00
    math::Vector4   Color;              // +0x10
    float           Near;               // +0x20
    float           Far;                // +0x24
    bool            isVertexPointLight; // +0x28
    uint8_t         _pad[3];            // +0x29
};
static_assert(sizeof(nglPointLightInfo) == 0x30, "nglPointLightInfo size mismatch");

// ============================================================================
// nglFrustum - 96 bytes
// ============================================================================
struct nglFrustum {
    math::Vector4 Planes[6];  // +0x00
};
static_assert(sizeof(nglFrustum) == 0x60, "nglFrustum size mismatch");

// ============================================================================
// nglDicLightInfo - 176 bytes
// ============================================================================
struct nglDicLightInfo {
    math::Vector4 Coeffs[9];  // +0x00
    math::Vector4 SunColor;   // +0x90
    math::Vector4 Ambient;    // +0xA0
};
static_assert(sizeof(nglDicLightInfo) == 0xB0, "nglDicLightInfo size mismatch");

// ============================================================================
// Light-category bits (LightCat)
// ============================================================================
enum {
    NGLLIGHTCAT_0 = 0x1000000,
    NGLLIGHTCAT_1 = 0x2000000,
    NGLLIGHTCAT_2 = 0x4000000,
    NGLLIGHTCAT_3 = 0x8000000,
    NGLLIGHTCAT_4 = 0x10000000,
    NGLLIGHTCAT_5 = 0x20000000,
    NGLLIGHTCAT_6 = 0x40000000,
    NGLLIGHTCAT_7 = 0x80000000,
};

// ============================================================================
// ngl_lighting.o (data, defined in ngl_lighting.cpp)
// ============================================================================
extern nglLightContext* nglDefaultLightContext;   // 0x14D2C0C
extern nglLightContext* nglBuildLightContext;     // 0x14D2C10
extern nglLightContext* nglSendLightContext;      // 0x14D2C14
extern struct nglLightContextParamType nglLightContextParam;  // 0x10E301C
extern unsigned int nglLightContextParamID;       // 0x10E3060

// ============================================================================
// ngl_lighting.o (functions, defined in ngl_lighting.cpp)
// ============================================================================
void nglSetLightContext(nglLightContext* Context, nglScene* Scene);
nglLightContext* nglSelectBuildLightContext(nglLightContext* Context);
void nglSetAmbientLight(float r, float g, float b);
void nglDetermineGunLights(const math::Position3* WorldPos);
bool nglGetFakePointLight(nglDirLightInfo* DirLight, nglPointLightInfo* Light,
                          const math::Position3* Pos);
nglPointLightInfo* nglGetLightAsPointLight(nglPointLightInfo* Out, nglLightNode* Node,
                                           const math::Position3* Pos);
nglDirLightInfo* nglGetLightAsDirLight(nglDirLightInfo* Out, nglLightNode* Node,
                                       const math::Position3* Pos);
nglPointLightInfo* nglGetSinglePointLight(nglPointLightInfo* Out, const math::Position3* Pos);
nglDirLightInfo* nglGetSingleDirLight(nglDirLightInfo* Out, const math::Position3* Pos);
void nglGetDirLightMatrix(nglMeshNode* MeshNode, math::Mat44* Dir, math::Mat44* Color);
nglLightContext* nglCreateLightContext();
void nglListAddLight(nglLightType Type, void* NodeData, int LightCat);
void nglListAddDirLight(unsigned int LightCat, const math::Dir3& Dir, const math::Vector4& Color);
void nglListAddPointLight(unsigned int LightCat, const math::Position3* Pos,
                          float Near, float Far, const math::Vector4* Color,
                          bool isVertexPointLight);
void nglListAddPointLightGun(unsigned int LightCat, const math::Position3* Pos,
                             float Near, float Far, const math::Vector4* Color,
                             bool isVertexPointLight);
void nglDetermineProjLights(nglMeshNode* MeshNode);
unsigned int nglCheckAvailableLights(nglMeshNode* MeshNode);
void nglDetermineLights(nglMeshNode* MeshNode);
void nglListAddProjLightNode(nglLightType Type, void* NodeData, int LightCat);
void nglListAddProjectorLight(void* NodeData, unsigned int LightCat);
void nglListAddDirProjectorLight(unsigned int LightCat, const math::Mat43* PO,
                                 const math::Position3* _Scale, unsigned int BlendMode,
                                 nglTexture* Tex);
void nglListAddDicLight(unsigned int LightCat, nglDicLightInfo* dic);

// Inline COMDATs (ngl_lighting.o).
bool nglIsSphereVisible(const nglFrustum* Frustum, const math::Vector4* Center, float Radius);
void nglBuildFrustum(nglFrustum* frustum, const math::Mat43* m);

#endif // COD3_NGL_NGL_LIGHTING_H

// ============================================================================
// nglScene — render scene (1024 bytes, verified against IDA local type).
// Only the fields used by ported code are defined; the rest are padding.
// ============================================================================
#ifndef COD3_NGL_NGLSCENE_H
#define COD3_NGL_NGLSCENE_H

#include "core/math_types.h"

struct nglRenderNode;
struct nglTexture;

struct nglScene {
    uint8_t           _pad0[0x10];        // +0x00 (ngliScene base)
    math::Mat44       Projection;         // +0x10
    math::Mat44       View;               // +0x50
    math::Mat44       Device;             // +0x90
    math::Mat43       ViewToWorld;        // +0xD0
    math::Mat44       ViewToScreen;       // +0x110
    math::Mat43       WorldToView;        // +0x150
    math::Mat44       WorldToScreen;      // +0x190
    math::Mat44       ViewportToWorld;    // +0x1D0
    math::Mat44       UIToDevice;         // +0x210
    math::Position3   ViewPos;            // +0x250
    math::Dir3        ViewDir;            // +0x260
    math::Vector4     ClipPlanes[6];      // +0x270
    uint8_t           _pad2D0[0x308 - 0x2D0];  // +0x2D0
    nglTexture*       RenderTarget;       // +0x308
    nglTexture*       ZTarget;            // +0x30C
    uint8_t           _pad310[4];         // +0x310 (ProjType)
    nglRenderNode*    OpaqueRenderList;   // +0x314
    nglRenderNode*    TransRenderList;    // +0x318
    unsigned int      OpaqueListCount;    // +0x31C
    unsigned int      TransListCount;     // +0x320
    uint8_t           _pad324[0x3A8 - 0x324];  // +0x324
    float             FogNear;            // +0x3A8
    float             FogFar;             // +0x3AC
    float             FogMin;             // +0x3B0
    float             FogMax;             // +0x3B4
    uint8_t           _pad3B8[0x3DC - 0x3B8];  // +0x3B8
    float             AspectRatio;        // +0x3DC
    float             FOV;                // +0x3E0
    float             NearZ;              // +0x3E4
    float             FarZ;               // +0x3E8
    float             AnimTime;           // +0x3EC
    float             CurAnimTime;        // +0x3F0
    unsigned int      IFLFrame;           // +0x3F4
    uint8_t           _pad3F8[0x400 - 0x3F8];  // +0x3F8
};
static_assert(sizeof(nglScene) == 0x400, "nglScene size mismatch");

#endif // COD3_NGL_NGLSCENE_H

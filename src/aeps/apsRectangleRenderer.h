// ============================================================================
// apsRectangleRenderer — rectangle renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsRectangleRenderer.cpp
// Verified against IDA (aeps_xboxr:apsRectangleRenderer.o):
//   ctor(cArgs) @0x8051F0
//   Init        @0x805230
//   Render      @0x805260
// Derives from apsBillboardRenderer (apsBillboardRenderer.h).
// ============================================================================
#ifndef COD3_AEPS_APSRECTANGLERENDERER_H
#define COD3_AEPS_APSRECTANGLERENDERER_H
#include "apsBillboardRenderer.h"
// ============================================================================
// apsRectangleNode — rectangle render node (192 bytes).
// ============================================================================
class apsRectangleNode : public apsBillboardNode {
public:
    virtual void GetDesc(char* buf);
    virtual void Render() override;  // ea: 0x804EA0
};
static_assert(sizeof(apsRectangleNode) == 0xC0, "apsRectangleNode size mismatch");
// ============================================================================
// apsRectangleRenderer — rectangle particle renderer (128 bytes).
// ============================================================================
class apsRectangleRenderer : public apsBillboardRenderer {
public:
    struct cArgs : public apsBillboardRenderer::cArgs {
    };

    apsRectangleRenderer(const cArgs* args);          // @0x8051F0
    virtual ~apsRectangleRenderer();                  // @0x8051C0 (vtable)
    static void Init();                                    // @0x805230
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo);  // @0x805260
    virtual unsigned int GetId() const;
    virtual float GetVersion() const;
    virtual int IsCameraFacing() const;
    virtual void SetScreenFacingNormal(const math::Dir3& iNormal);
    virtual float GetChanceToRemove() const;
    virtual bool GetMeshRadius(float& oRadius) const;
    APS_DECLARE_RETRIEVE(apsRectangleRenderer, apsBillboardRenderer((APS_VTABLE_RETRIEVING_CTOR)0))};
static_assert(sizeof(apsRectangleRenderer) == 0x80, "apsRectangleRenderer size mismatch");
// ============================================================================
// Shader microcode registration structs (data in apsRectangleRendererVertex.o)
// ============================================================================
struct apsRectangleRender {
    static unsigned int* VS;                  // ?VS@apsRectangleRender@@3PAKA
    static const unsigned int** VShaderTable; // ?VShaderTable@apsRectangleRender@@3PAPBIA
    static void RegisterVShader() { nglDxRegisterVShader(VS, VShaderTable[0]); }   // ea: 0x805200
};
struct apsRectangleRenderPixel {
    static unsigned int** PS;                 // ?PS@apsRectangleRenderPixel@@3PAPAKA
    static const unsigned int** PShaderTable; // ?PShaderTable@apsRectangleRenderPixel@@3PAPBIA
    static void RegisterPShader() { nglDxRegisterPShader(PS, PShaderTable[0]); }   // ea: 0x805180
    static void InitPShader() { nglDxInitPShader(PShaderTable[0]); }              // ea: 0x8051A0
};
#endif // COD3_AEPS_APSRECTANGLERENDERER_H

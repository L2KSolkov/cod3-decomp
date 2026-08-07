// ============================================================================
// apsColorRectangleRenderer — color rectangle renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsColorRectangleRenderer.cpp
// Verified against IDA (aeps_xboxr:apsColorRectangleRenderer.o):
//   ctor(cArgs) @0x804920
//   Init        @0x804960
//   Render      @0x804990
// Derives from apsBillboardRenderer (apsBillboardRenderer.h).
// ============================================================================
#ifndef COD3_AEPS_APSCOLORRECTANGLERENDERER_H
#define COD3_AEPS_APSCOLORRECTANGLERENDERER_H
#include "apsBillboardRenderer.h"
// ============================================================================
// apsColorRectangleNode — color rectangle render node (192 bytes).
// ============================================================================
class apsColorRectangleNode : public apsBillboardNode {
public:
    virtual void GetDesc(char* buf);
    virtual void Render() override;  // ea: 0x8048A0
};
static_assert(sizeof(apsColorRectangleNode) == 0xC0, "apsColorRectangleNode size mismatch");
// ============================================================================
// apsColorRectangleRenderer — color rectangle particle renderer (128 bytes).
// ============================================================================
class apsColorRectangleRenderer : public apsBillboardRenderer {
public:
    struct cArgs : public apsBillboardRenderer::cArgs {
    };

    apsColorRectangleRenderer(const cArgs* args);          // @0x804920
    virtual ~apsColorRectangleRenderer();                  // @0x8048F0 (vtable)
    static void Init();                                    // @0x804960
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo);  // @0x804990
    virtual unsigned int GetId() const;
    virtual float GetVersion() const;
    virtual int IsCameraFacing() const;
    virtual void SetScreenFacingNormal(const math::Dir3& iNormal);
    virtual float GetChanceToRemove() const;
    virtual bool GetMeshRadius(float& oRadius) const;
    APS_DECLARE_RETRIEVE(apsColorRectangleRenderer, apsBillboardRenderer((APS_VTABLE_RETRIEVING_CTOR)0))};
static_assert(sizeof(apsColorRectangleRenderer) == 0x80, "apsColorRectangleRenderer size mismatch");
// ============================================================================
// Shader microcode registration structs (data in apsColorRectangleRendererVertex.o)
// ============================================================================
struct apsColorRectangleRender {
    static unsigned int* VS;                  // ?VS@apsColorRectangleRender@@3PAKA
    static const unsigned int** VShaderTable; // ?VShaderTable@apsColorRectangleRender@@3PAPBIA
    static void RegisterVShader() { nglDxRegisterVShader(VS, VShaderTable[0]); }   // ea: 0x804930
};
struct apsColorRectangleRenderPixel {
    static unsigned int** PS;                 // ?PS@apsColorRectangleRenderPixel@@3PAPAKA
    static const unsigned int** PShaderTable; // ?PShaderTable@apsColorRectangleRenderPixel@@3PAPBIA
    static void RegisterPShader() { nglDxRegisterPShader(PS, PShaderTable[0]); }   // ea: 0x804910
    static void InitPShader() { nglDxInitPShader(PShaderTable[0]); }              // ea: 0x804860
};
#endif // COD3_AEPS_APSCOLORRECTANGLERENDERER_H

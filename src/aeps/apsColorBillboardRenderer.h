// ============================================================================
// apsColorBillboardRenderer — color billboard renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsColorBillboardRenderer.cpp
// Verified against IDA (aeps_xboxr:apsColorBillboardRenderer.o):
//   ctor(cArgs) @0x8056E0
//   Init        @0x805720
//   Render      @0x805750
// Derives from apsBillboardRenderer (apsBillboardRenderer.h).
// ============================================================================
#ifndef COD3_AEPS_APSCOLORBILLBOARDRENDERER_H
#define COD3_AEPS_APSCOLORBILLBOARDRENDERER_H

#include "apsBillboardRenderer.h"

// ============================================================================
// apsColorBillboardNode — color billboard render node (192 bytes).
// ============================================================================
class apsColorBillboardNode : public apsBillboardNode {
public:
    virtual void GetDesc(char* buf);
    virtual void Render() override;  // ea: 0x805670
};
static_assert(sizeof(apsColorBillboardNode) == 0xC0, "apsColorBillboardNode size mismatch");

// ============================================================================
// apsColorBillboardRenderer — color billboard particle renderer (128 bytes).
// ============================================================================
class apsColorBillboardRenderer : public apsBillboardRenderer {
public:
    // Nested cArgs (derives from apsBillboardRenderer::cArgs, same 100 bytes)
    struct cArgs : public apsBillboardRenderer::cArgs {};

    apsColorBillboardRenderer(const cArgs* args);          // @0x8056E0
    virtual ~apsColorBillboardRenderer();                  // @0x805690 (vtable)

    static void Init();                                    // @0x805720
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo);  // @0x805750
    virtual unsigned int GetId() const;
    virtual float GetVersion() const;
    virtual int IsCameraFacing() const;
    virtual void SetScreenFacingNormal(const math::Dir3& iNormal);
    virtual float GetChanceToRemove() const;
    virtual bool GetMeshRadius(float& oRadius) const;
};
static_assert(sizeof(apsColorBillboardRenderer) == 0x80, "apsColorBillboardRenderer size mismatch");

// ============================================================================
// Shader microcode registration structs (data in apsColorBillboardRendererVertex.o)
// ============================================================================
struct apsColorBillboardRender {
    static unsigned int* VS;                  // ?VS@apsColorBillboardRender@@3PAKA
    static const unsigned int** VShaderTable; // ?VShaderTable@apsColorBillboardRender@@3PAPBIA

    static void RegisterVShader() { nglDxRegisterVShader(VS, VShaderTable[0]); }   // ea: 0x8056C0
};

struct apsColorBillboardRenderPixel {
    static unsigned int** PS;                 // ?PS@apsColorBillboardRenderPixel@@3PAPAKA
    static const unsigned int** PShaderTable; // ?PShaderTable@apsColorBillboardRenderPixel@@3PAPBIA

    static void RegisterPShader() { nglDxRegisterPShader(PS, PShaderTable[0]); }   // ea: 0x8056A0
    static void InitPShader() { nglDxInitPShader(PShaderTable[0]); }              // ea: 0x805680
};

#endif // COD3_AEPS_APSCOLORBILLBOARDRENDERER_H

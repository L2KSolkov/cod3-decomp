// ============================================================================
// apsColorUVARenderer — color UVA renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsColorUVARenderer.cpp
// Verified against IDA (aeps_xboxr:apsColorUVARenderer.o):
//   ctor(cArgs) @0x805480
//   Init        @0x8054C0
//   Render      @0x8054F0
// Derives from apsUVARenderer (apsUVARenderer.h).
// ============================================================================
#ifndef COD3_AEPS_APSCOLORUVARENDERER_H
#define COD3_AEPS_APSCOLORUVARENDERER_H
#include "apsUVARenderer.h"
// ============================================================================
// apsColorUVANode — color UVA render node (192 bytes).
// ============================================================================
class apsColorUVANode : public apsUVANode {
public:
    virtual void GetDesc(char* buf);
    virtual void Render() override;  // ea: 0x8053F0
};
static_assert(sizeof(apsColorUVANode) == 0xC0, "apsColorUVANode size mismatch");
// ============================================================================
// apsColorUVARenderer — color UVA particle renderer (144 bytes).
// ============================================================================
class apsColorUVARenderer : public apsUVARenderer {
public:
    struct cArgs : public apsUVARenderer::cArgs {
    };

    apsColorUVARenderer(const cArgs* args);          // @0x805480
    virtual ~apsColorUVARenderer();                  // @0x805450 (vtable)
    static void Init();                                    // @0x8054C0
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo);  // @0x8054F0
    virtual unsigned int GetId() const;
    virtual float GetVersion() const;
    APS_DECLARE_RETRIEVE(apsColorUVARenderer, apsUVARenderer((APS_VTABLE_RETRIEVING_CTOR)0))};
static_assert(sizeof(apsColorUVARenderer) == 0x90, "apsColorUVARenderer size mismatch");
// ============================================================================
// Shader microcode registration structs (data in apsColorUVARendererVertex.o)
// ============================================================================
struct apsColorUVARender {
    static unsigned int* VS;                  // ?VS@apsColorUVARender@@3PAKA
    static const unsigned int** VShaderTable; // ?VShaderTable@apsColorUVARender@@3PAPBIA
    static void RegisterVShader() { nglDxRegisterVShader(reinterpret_cast<unsigned long*>(VS), VShaderTable[0]); }   // ea: 0x805420
};
struct apsColorUVARenderPixel {
    static unsigned int** PS;                 // ?PS@apsColorUVARenderPixel@@3PAPAKA
    static const unsigned int** PShaderTable; // ?PShaderTable@apsColorUVARenderPixel@@3PAPBIA
    static void RegisterPShader() { nglDxRegisterPShader(reinterpret_cast<unsigned long**>(PS), PShaderTable[0]); }   // ea: 0x805400
    static void InitPShader() { nglDxInitPShader(PShaderTable[0]); }              // ea: 0x8053D0
};
#endif // COD3_AEPS_APSCOLORUVARENDERER_H

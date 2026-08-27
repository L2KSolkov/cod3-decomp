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

    APS_DECLARE_RETRIEVE(apsColorBillboardRenderer, apsBillboardRenderer((APS_VTABLE_RETRIEVING_CTOR)0))
};
static_assert(sizeof(apsColorBillboardRenderer) == 0x80, "apsColorBillboardRenderer size mismatch");
// ============================================================================
// Shader microcode registration structs (data in apsColorBillboardRendererVertex.o)
// ============================================================================
namespace apsColorBillboardRender {
    extern unsigned int* VS;
    extern const unsigned int** VShaderTable;
    void RegisterVShader();
}
namespace apsColorBillboardRenderPixel {
    extern unsigned int** PS;
    extern const unsigned int** PShaderTable;
    void RegisterPShader();
    void InitPShader();
}
#endif // COD3_AEPS_APSCOLORBILLBOARDRENDERER_H

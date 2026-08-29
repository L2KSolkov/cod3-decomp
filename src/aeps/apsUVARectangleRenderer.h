// ============================================================================
// apsUVARectangleRenderer — UVA rectangle renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsUVARectangleRenderer.cpp
// Verified against IDA (aeps_xboxr:apsUVARectangleRenderer.o):
//   ctor(cArgs) @0x804F60
//   Init        @0x804FA0
//   Render      @0x804FD0
// Derives from apsUVARenderer (apsUVARenderer.h).
// ============================================================================
#ifndef COD3_AEPS_APSUVARECTANGLERENDERER_H
#define COD3_AEPS_APSUVARECTANGLERENDERER_H
#include "apsUVARenderer.h"
// ============================================================================
// apsUVARectangleNode — UVA rectangle render node (192 bytes).
// ============================================================================
class apsUVARectangleNode : public apsUVANode {
public:
    virtual void GetDesc(char* buf);
    virtual void Render() override;
};
static_assert(sizeof(apsUVARectangleNode) == 0xC0, "apsUVARectangleNode size mismatch");
// ============================================================================
// apsUVARectangleRenderer — UVA rectangle particle renderer (144 bytes).
// ============================================================================
class apsUVARectangleRenderer : public apsUVARenderer {
public:
    struct cArgs : public apsUVARenderer::cArgs {
    };

    apsUVARectangleRenderer(const cArgs* args);          // @0x804F60
    virtual ~apsUVARectangleRenderer();                  // @0x804F30 (vtable)
    static void Init();                                    // @0x804FA0
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo);  // @0x804FD0
    virtual unsigned int GetId() const;
    virtual float GetVersion() const;
    APS_DECLARE_RETRIEVE(apsUVARectangleRenderer, apsUVARenderer((APS_VTABLE_RETRIEVING_CTOR)0))};
static_assert(sizeof(apsUVARectangleRenderer) == 0x90, "apsUVARectangleRenderer size mismatch");
// ============================================================================
// Shader microcode registration structs (data in apsUVARectangleRendererVertex.o)
// ============================================================================
namespace apsUVARectangleRender {
    extern unsigned int* VS;
    extern const unsigned int** VShaderTable;
    unsigned long GetVShader();
    void RegisterVShader();
}
namespace apsUVARectangleRenderPixel {
    extern unsigned int** PS;
    extern const unsigned int** PShaderTable;
    unsigned long* GetPShader();
    void RegisterPShader();
    void InitPShader();
}
#endif // COD3_AEPS_APSUVARECTANGLERENDERER_H

// ============================================================================
// apsColorUVARectangleRenderer — color UVA rectangle renderer (3 funcs).
// Source: c:\cod\code\tl\aeps\source\apsColorUVARectangleRenderer.cpp
// Verified against IDA (aeps_xboxr:apsColorUVARectangleRenderer.o):
//   ctor(cArgs) @0x804C90
//   Init        @0x804CD0
//   Render      @0x804D00
// Derives from apsUVARenderer (apsUVARenderer.h).
// ============================================================================
#ifndef COD3_AEPS_APSCOLORUVARECTANGLERENDERER_H
#define COD3_AEPS_APSCOLORUVARECTANGLERENDERER_H
#include "apsUVARenderer.h"
// ============================================================================
// apsColorUVARectangleNode — color UVA rectangle render node (192 bytes).
// ============================================================================
class apsColorUVARectangleNode : public apsUVANode {
public:
    virtual void GetDesc(char* buf);
    virtual void Render() override;  // ea: 0x804C30
};
static_assert(sizeof(apsColorUVARectangleNode) == 0xC0, "apsColorUVARectangleNode size mismatch");
// ============================================================================
// apsColorUVARectangleRenderer — color UVA rectangle particle renderer (144 bytes).
// ============================================================================
class apsColorUVARectangleRenderer : public apsUVARenderer {
public:
    struct cArgs : public apsUVARenderer::cArgs {
    };

    apsColorUVARectangleRenderer(const cArgs* args);          // @0x804C90
    virtual ~apsColorUVARectangleRenderer();                  // @0x804C60 (vtable)
    static void Init();                                    // @0x804CD0
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo);  // @0x804D00
    virtual unsigned int GetId() const;
    virtual float GetVersion() const;
    APS_DECLARE_RETRIEVE(apsColorUVARectangleRenderer, apsUVARenderer((APS_VTABLE_RETRIEVING_CTOR)0))};
static_assert(sizeof(apsColorUVARectangleRenderer) == 0x90, "apsColorUVARectangleRenderer size mismatch");
// ============================================================================
// Shader microcode registration structs (data in apsColorUVARectangleRendererVertex.o)
// ============================================================================
namespace apsColorUVARectangleRender {
    extern unsigned int* VS;
    extern const unsigned int** VShaderTable;
    unsigned long GetVShader();
    void RegisterVShader();
}
namespace apsColorUVARectangleRenderPixel {
    extern unsigned int** PS;
    extern const unsigned int** PShaderTable;
    unsigned long* GetPShader();
    void RegisterPShader();
    void InitPShader();
}
#endif // COD3_AEPS_APSCOLORUVARECTANGLERENDERER_H

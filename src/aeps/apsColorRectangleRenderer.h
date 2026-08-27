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
    APS_DECLARE_RETRIEVE(apsColorRectangleRenderer, apsBillboardRenderer((APS_VTABLE_RETRIEVING_CTOR)0))};
static_assert(sizeof(apsColorRectangleRenderer) == 0x80, "apsColorRectangleRenderer size mismatch");
// ============================================================================
// Shader microcode registration structs (data in apsColorRectangleRendererVertex.o)
// ============================================================================
namespace apsColorRectangleRender {
    extern unsigned int* VS;
    extern const unsigned int** VShaderTable;
    void RegisterVShader();
}
namespace apsColorRectangleRenderPixel {
    extern unsigned int** PS;
    extern const unsigned int** PShaderTable;
    void RegisterPShader();
    void InitPShader();
}
#endif // COD3_AEPS_APSCOLORRECTANGLERENDERER_H

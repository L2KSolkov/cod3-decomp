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
    apsRectangleNode();
    virtual void GetDesc(char* buf);
    virtual void Render() override;  // ea: 0x804EA0
};
static_assert(sizeof(apsRectangleNode) == 0xC0, "apsRectangleNode size mismatch");
// ============================================================================
// apsRectangleRenderer — rectangle particle renderer (128 bytes).
// ============================================================================
class apsRectangleRenderer : public apsBillboardRenderer {
public:
    class cArgs : public apsBillboardRenderer::cArgs {
    public:
    };

    apsRectangleRenderer(const cArgs& args);          // @0x8051F0
    apsRectangleRenderer(const cArgs* args) : apsRectangleRenderer(*args) {}
    virtual ~apsRectangleRenderer();                  // @0x8051C0 (vtable)
    static void Init();                                    // @0x805230
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo);  // @0x805260
    virtual unsigned int GetId() const;
    virtual float GetVersion() const;
    APS_DECLARE_RETRIEVE(apsRectangleRenderer, apsBillboardRenderer((APS_VTABLE_RETRIEVING_CTOR)0))};
static_assert(sizeof(apsRectangleRenderer) == 0x80, "apsRectangleRenderer size mismatch");
// ============================================================================
// Shader microcode registration structs (data in apsRectangleRendererVertex.o)
// ============================================================================
namespace apsRectangleRender {
    extern unsigned long* VS;                  // ?VS@apsRectangleRender@@3PAKA
    extern const unsigned long** VShaderTable; // ?VShaderTable@apsRectangleRender@@3PAPBIA
    unsigned long GetVShader();                // ea: 0x80DD50
    void RegisterVShader();                    // ea: 0x8052A0
}
namespace apsRectangleRenderPixel {
    extern unsigned long** PS;                 // ?PS@apsRectangleRenderPixel@@3PAPAKA
    extern const unsigned long** PShaderTable; // ?PShaderTable@apsRectangleRenderPixel@@3PAPBIA
    unsigned long* GetPShader();               // ea: 0x80DD60
    void RegisterPShader();                    // ea: 0x8052C0
    void InitPShader();                        // ea: 0x8052E0
}
#endif // COD3_AEPS_APSRECTANGLERENDERER_H

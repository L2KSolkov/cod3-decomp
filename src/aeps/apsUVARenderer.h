// ============================================================================
// apsUVARenderer — UVA sprite renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsUVARenderer.cpp
// Verified against IDA (aeps_xboxr:apsUVARenderer.o):
//   ctor(cArgs) @0x805960
//   Init        @0x805A50
//   Render      @0x805A80
// Derives from apsBillboardRenderer (apsBillboardRenderer.h).
// ============================================================================
#ifndef COD3_AEPS_APSUVARENDERER_H
#define COD3_AEPS_APSUVARENDERER_H
#include "apsBillboardRenderer.h"
#include "apsParam.h"
// ============================================================================
// apsUVANode — UVA render node (192 bytes, apsBillboardNode-derived).
// ============================================================================
class apsUVANode : public apsBillboardNode {
public:
    apsRenderSort::Buffer* GetRenderSortBuffer(); // ea: 0x816370
    virtual void GetDesc(char* buf);
    virtual void Render() override;  // ea: 0x804290
};
static_assert(sizeof(apsUVANode) == 0xC0, "apsUVANode size mismatch");
// ============================================================================
// apsUVARenderer — UVA (UV-animated) particle renderer (144 bytes).
// ============================================================================
class apsUVARenderer : public apsBillboardRenderer {
public:
    // Nested cArgs (apsBillboardRenderer::cArgs + frame counts, 108 bytes)
    struct cArgs : public apsBillboardRenderer::cArgs {
        int mWidthFrames;   // +0x64
        int mHeightFrames;  // +0x68
        void SetWidthFrames(apsParam param);   // ?SetWidthFrames@cArgs@apsUVARenderer@@QAEXVapsParam@@@Z
        void SetHeightFrames(apsParam param);  // ?SetHeightFrames@cArgs@apsUVARenderer@@QAEXVapsParam@@@Z
    };

    float mWidthFrames;     // +0x80
    float mMaxFrame;        // +0x84
    float mInvWidthFrames;  // +0x88
    float mInvHeightFrames; // +0x8C
    float WidthFrames() const;    // ea: 0x816380
    float MaxFrame() const;       // ea: 0x816390
    float InvWidthFrames() const; // ea: 0x8163A0
    float InvHeightFrames() const;// ea: 0x8163B0
    apsUVARenderer(const cArgs* args);          // @0x805960
    virtual ~apsUVARenderer();                  // @0x805900 (vtable)
    static void Init();                                    // @0x805A50
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo);  // @0x805A80
    virtual unsigned int GetId() const;
    virtual float GetVersion() const;
    APS_DECLARE_RETRIEVE(apsUVARenderer, apsBillboardRenderer((APS_VTABLE_RETRIEVING_CTOR)0))};
static_assert(sizeof(apsUVARenderer) == 0x90, "apsUVARenderer size mismatch");
// ============================================================================
// Shader microcode registration structs (data in apsUVARendererVertex.o)
// ============================================================================
namespace apsUVARender {
    extern unsigned int* VS;
    extern const unsigned int** VShaderTable;
    unsigned long GetVShader();
    void RegisterVShader();
}
namespace apsUVARenderPixel {
    extern unsigned int** PS;
    extern const unsigned int** PShaderTable;
    unsigned long* GetPShader();
    void RegisterPShader();
    void InitPShader();
}
#endif // COD3_AEPS_APSUVARENDERER_H

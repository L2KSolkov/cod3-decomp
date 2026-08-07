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
    apsUVARenderer(const cArgs* args);          // @0x805960
    virtual ~apsUVARenderer();                  // @0x805900 (vtable)
    static void Init();                                    // @0x805A50
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo);  // @0x805A80
    virtual unsigned int GetId() const;
    virtual float GetVersion() const;
    virtual int IsCameraFacing() const;
    virtual void SetScreenFacingNormal(const math::Dir3& iNormal);
    virtual float GetChanceToRemove() const;
    virtual bool GetMeshRadius(float& oRadius) const;
    APS_DECLARE_RETRIEVE(apsUVARenderer, apsBillboardRenderer((APS_VTABLE_RETRIEVING_CTOR)0))};
static_assert(sizeof(apsUVARenderer) == 0x90, "apsUVARenderer size mismatch");
// ============================================================================
// Shader microcode registration structs (data in apsUVARendererVertex.o)
// ============================================================================
struct apsUVARender {
    static unsigned int* VS;                  // ?VS@apsUVARender@@3PAKA
    static const unsigned int** VShaderTable; // ?VShaderTable@apsUVARender@@3PAPBIA
    static void RegisterVShader() { nglDxRegisterVShader(VS, VShaderTable[0]); }   // ea: 0x805930
};
struct apsUVARenderPixel {
    static unsigned int** PS;                 // ?PS@apsUVARenderPixel@@3PAPAKA
    static const unsigned int** PShaderTable; // ?PShaderTable@apsUVARenderPixel@@3PAPBIA
    static void RegisterPShader() { nglDxRegisterPShader(PS, PShaderTable[0]); }   // ea: 0x805950
    static void InitPShader() { nglDxInitPShader(PShaderTable[0]); }              // ea: 0x805970
};
#endif // COD3_AEPS_APSUVARENDERER_H

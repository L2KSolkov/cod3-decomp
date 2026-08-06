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
    virtual void Render() override;  // ea: 0x804F00
};
static_assert(sizeof(apsUVARectangleNode) == 0xC0, "apsUVARectangleNode size mismatch");

// ============================================================================
// apsUVARectangleRenderer — UVA rectangle particle renderer (144 bytes).
// ============================================================================
class apsUVARectangleRenderer : public apsUVARenderer {
public:
    struct cArgs : public apsUVARenderer::cArgs {};

    apsUVARectangleRenderer(const cArgs* args);          // @0x804F60
    virtual ~apsUVARectangleRenderer();                  // @0x804F30 (vtable)

    static void Init();                                    // @0x804FA0
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo);  // @0x804FD0
    virtual unsigned int GetId() const;
    virtual float GetVersion() const;
    virtual int IsCameraFacing() const;
    virtual void SetScreenFacingNormal(const math::Dir3& iNormal);
    virtual float GetChanceToRemove() const;
    virtual bool GetMeshRadius(float& oRadius) const;
};
static_assert(sizeof(apsUVARectangleRenderer) == 0x90, "apsUVARectangleRenderer size mismatch");

// ============================================================================
// Shader microcode registration structs (data in apsUVARectangleRendererVertex.o)
// ============================================================================
struct apsUVARectangleRender {
    static unsigned int* VS;                  // ?VS@apsUVARectangleRender@@3PAKA
    static const unsigned int** VShaderTable; // ?VShaderTable@apsUVARectangleRender@@3PAPBIA

    static void RegisterVShader() { nglDxRegisterVShader(VS, VShaderTable[0]); }   // ea: 0x804F70
};

struct apsUVARectangleRenderPixel {
    static unsigned int** PS;                 // ?PS@apsUVARectangleRenderPixel@@3PAPAKA
    static const unsigned int** PShaderTable; // ?PShaderTable@apsUVARectangleRenderPixel@@3PAPBIA

    static void RegisterPShader() { nglDxRegisterPShader(PS, PShaderTable[0]); }   // ea: 0x804F50
    static void InitPShader() { nglDxInitPShader(PShaderTable[0]); }              // ea: 0x804F10
};

#endif // COD3_AEPS_APSUVARECTANGLERENDERER_H

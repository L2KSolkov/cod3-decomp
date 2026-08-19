// ============================================================================
// apsShrimpRenderer — shrimp/sprite particle renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsShrimpRenderer.cpp
// Verified against IDA (aeps_xboxr:apsShrimpRenderer.o):
//   Init            @0x8044F0 (?Init@apsShrimpRenderer@@SAXXZ)
//   ctor(cArgs)     @0x804520 (??0apsShrimpRenderer@@QAE@ABVcArgs@0@@Z)
//   Render          @0x8045E0 (?Render@apsShrimpRenderer@@UAE?AW4eRenderResult@apsRenderer@@ABUapsRendererRenderInfo@@@Z)
// ============================================================================
#ifndef COD3_AEPS_APSSHRIMPRENDERER_H
#define COD3_AEPS_APSSHRIMPRENDERER_H

#include "apsRenderer.h"
#include "apsRetrieveVtable.h"
#include "apsParam.h"
#include "apsRenderNode.h"
#include "apsCommon.h"
#include "ngl/nglScene.h"
#include "ngl/ngl_lighting.h"
#include <intrin.h>

// ============================================================================
// nglLightContext — 112-byte IDA-verified light context.
// ============================================================================
// ============================================================================
// apsEBlendMode — particle blend modes (values from IDA)
// ============================================================================
enum apsEBlendMode {
    apsEBlendMode_Blend = 0,
    apsEBlendMode_Add = 1,
    apsEBlendMode_Subtract = 2,
    apsEBlendMode_BlendWithLighting = 3,
    apsEBlendMode_NumBlendModes = 4,
};

// ============================================================================
// apsShrimpNode — shrimp render node (192 bytes = apsRenderNode + renderer).
// ============================================================================
class apsShrimpNode : public apsRenderNode {
public:
    class apsShrimpRenderer* mRenderer;  // +0xB0

    void SetRenderer(class apsShrimpRenderer* r) { mRenderer = r; }   // ea: 0x802F90
    virtual void GetDesc(char* buf);                                  // ea: 0x802FA0
    virtual void Render() override;                                   // ea: 0x812D90 (apsShrimpNode.o)
};
static_assert(sizeof(apsShrimpNode) == 0xC0, "apsShrimpNode size mismatch");

// ============================================================================
// apsShrimpRenderer — shrimp particle renderer (64 bytes).
// ============================================================================
class apsShrimpRenderer : public apsRenderer {
public:
    struct cArgs {
        struct PackedColor { float x, y, z, w; };  // math::Vector4::Packed

        nglTexture*           mTexture;       // +0x00
        apsEBlendMode         mBlendMode;     // +0x04
        int                   mNumFrames;     // +0x08
        int                   mNumRows;       // +0x0C
        int                   mNumRotations;  // +0x10
        int                   mSpriteWidth;   // +0x14
        int                   mSpriteHeight;  // +0x18
        int                   mTextureWidth;  // +0x1C
        int                   mTextureHeight; // +0x20
        PackedColor           mTintColor;     // +0x24 (16 bytes)

        // cArgs setters (inline COMDATs in apsRegister.o)
        void SetTexture(apsParam param);         // ?SetTexture@cArgs@apsShrimpRenderer@@QAEXVapsParam@@@Z
        void SetBlendMode(apsParam param);       // ?SetBlendMode@cArgs@apsShrimpRenderer@@QAEXVapsParam@@@Z
        void SetNumFrames(apsParam param);       // ?SetNumFrames@cArgs@apsShrimpRenderer@@QAEXVapsParam@@@Z
        void SetNumRows(apsParam param);         // ?SetNumRows@cArgs@apsShrimpRenderer@@QAEXVapsParam@@@Z
        void SetNumRotations(apsParam param);    // ?SetNumRotations@cArgs@apsShrimpRenderer@@QAEXVapsParam@@@Z
        void SetSpriteWidth(apsParam param);     // ?SetSpriteWidth@cArgs@apsShrimpRenderer@@QAEXVapsParam@@@Z
        void SetSpriteHeight(apsParam param);    // ?SetSpriteHeight@cArgs@apsShrimpRenderer@@QAEXVapsParam@@@Z
        void SetTextureWidth(apsParam param);    // ?SetTextureWidth@cArgs@apsShrimpRenderer@@QAEXVapsParam@@@Z
        void SetTextureHeight(apsParam param);   // ?SetTextureHeight@cArgs@apsShrimpRenderer@@QAEXVapsParam@@@Z
        void SetTintColor(apsParam param);       // ?SetTintColor@cArgs@apsShrimpRenderer@@QAEXVapsParam@@@Z
    };

    math::Vector4 mTintColor;      // +0x10
    nglTexture*   mTexture;        // +0x20
    apsEBlendMode mBlendMode;      // +0x24
    int16_t       mNumFrames;      // +0x28
    int16_t       mNumRows;        // +0x2A
    int16_t       mNumRotations;   // +0x2C
    int16_t       mSpriteWidth;    // +0x2E
    int16_t       mSpriteHeight;   // +0x30
    int16_t       mTextureWidth;   // +0x32
    int16_t       mTextureHeight;  // +0x34

    apsShrimpRenderer(const cArgs* args);          // @0x804520
    virtual ~apsShrimpRenderer();                  // @0x8044D0 (vtable)

    static void Init();                                // @0x8044F0
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo);  // @0x8045E0
    virtual unsigned int GetId() const;
    virtual float GetVersion() const;
    virtual int IsCameraFacing() const;
    virtual void SetScreenFacingNormal(const math::Dir3& iNormal);
    virtual float GetChanceToRemove() const;
    virtual bool GetMeshRadius(float& oRadius) const;

    APS_DECLARE_RETRIEVE_LEAF(apsShrimpRenderer)};
static_assert(sizeof(apsShrimpRenderer) == 0x40, "apsShrimpRenderer size mismatch");

// ============================================================================
// externs (ngl/aps)
// ============================================================================
extern void* nglListAlloc(unsigned Bytes, unsigned Alignment);
extern nglScene* nglBuildScene;
extern nglLightContext* nglDefaultLightContext;
extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);
extern void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode);
extern void nglDxInitPShader(const unsigned int* Microcode);

// ============================================================================
// Shader microcode registration structs (data in apsShrimpRendererVertex.o)
// ============================================================================
struct apsShrimpRender {
    static unsigned int* VS;                  // ?VS@apsShrimpRender@@3PAKA
    static const unsigned int** VShaderTable; // ?VShaderTable@apsShrimpRender@@3PAPBIA

    static void RegisterVShader() { nglDxRegisterVShader(reinterpret_cast<unsigned long*>(VS), VShaderTable[0]); }   // ea: 0x804470
};

struct apsShrimpRenderPixel {
    static unsigned int** PS;                 // ?PS@apsShrimpRenderPixel@@3PAPAKA
    static const unsigned int** PShaderTable; // ?PShaderTable@apsShrimpRenderPixel@@3PAPBIA

    static void RegisterPShader() { nglDxRegisterPShader(reinterpret_cast<unsigned long**>(PS), PShaderTable[0]); }   // ea: 0x804490
    static void InitPShader() { nglDxInitPShader(PShaderTable[0]); }              // ea: 0x8044B0
};

// ============================================================================
// apsInternal::GetBlendColor — declared in apsInternal.h (inline COMDAT,
// apsShrimpRenderer.o). ea: 0x804400
// ============================================================================


#endif // COD3_AEPS_APSSHRIMPRENDERER_H

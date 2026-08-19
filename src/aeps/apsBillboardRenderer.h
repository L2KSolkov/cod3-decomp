// ============================================================================
// apsBillboardRenderer — billboard particle renderer (6 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsBillboardRenderer.cpp
// Verified against IDA (aeps_xboxr:apsBillboardRenderer.o):
//   UsesAmbientLighting @0x805C50
//   UsesDiffuseLighting @0x805CD0
//   Init                @0x805D50
//   SetNodeParams       @0x805D80
//   ctor(cArgs)         @0x805F60
//   Render              @0x8060F0
// ============================================================================
#ifndef COD3_AEPS_APSBILLBOARDRENDERER_H
#define COD3_AEPS_APSBILLBOARDRENDERER_H
#include "apsRenderer.h"
#include "apsParam.h"
#include "apsRetrieveVtable.h"
#include "apsRetrieveVtable.h"
#include "apsRenderNode.h"
#include "apsCommon.h"
#include "apsShrimpRenderer.h"  // apsEBlendMode
#include "ngl/nglScene.h"
#include <intrin.h>
// ============================================================================
// apsRenderSort::Buffer — sorted-render scratch buffer (8 bytes).
// ============================================================================
namespace apsRenderSort {
struct Buffer {
    unsigned int  capacity;   // +0x00
    unsigned char** buffer;   // +0x04
};
}
// ============================================================================
// apsBillboardNode — billboard render node (192 bytes).
// ============================================================================
class apsBillboardNode : public apsRenderNode {
public:
    class apsBillboardRenderer* mRenderer;         // +0xB0
    apsRenderSort::Buffer*      mRenderSortBuffer; // +0xB4
    class apsBillboardRenderer* Renderer() { return mRenderer; }  // ea: 0x8048B0
    void SetRenderer(class apsBillboardRenderer* r) { mRenderer = r; }  // ea: 0x804890
    void SetRenderSortBuffer(apsRenderSort::Buffer* b) { mRenderSortBuffer = b; }  // ea: 0x8048D0
    virtual void GetDesc(char* buf);              // ea: 0x8048E0
    virtual void Render() override;               // ea: 0x813C90 (apsBillboardNode.o)
};
static_assert(sizeof(apsBillboardNode) == 0xC0, "apsBillboardNode size mismatch");
// ============================================================================
// apsBillboardRenderer — billboard particle renderer (128 bytes).
// ============================================================================
class apsBillboardRenderer : public apsRenderer {
public:
    struct cArgs {
        struct PackedColor { float x, y, z, w; };  // math::Vector4::Packed
        struct PackedDir { float x, y, z; };       // math::Dir3::Packed
        nglTexture*   mTexture;          // +0x00
        apsEBlendMode mBlendMode;        // +0x04
        PackedDir     mNormal;           // +0x08 (12 bytes)
        PackedColor   mTintColor;        // +0x14
        PackedColor   mAmbientCoeff;     // +0x24
        PackedColor   mDiffuseCoeff;     // +0x34
        int           mVelocityTracked;  // +0x44
        float         mAlphaFadeStart;   // +0x48
        float         mAlphaFadeEnd;     // +0x4C
        float         mZFeatherDistance; // +0x50
        void*         mShaderWorksShader;// +0x54
        int           mIsShimmer;        // +0x58
        int           mUseSortedRendering;  // +0x5C
        float         mChanceToRemove;   // +0x60
        // cArgs setters (inline COMDATs in apsRegister.o)
        void SetTexture(apsParam param);          // ?SetTexture@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
        void SetNormal(apsParam param);           // ?SetNormal@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
        void SetBlendMode(apsParam param);        // ?SetBlendMode@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
        void SetVelocityTracked(apsParam param);  // ?SetVelocityTracked@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
        void SetAlphaFadeStart(apsParam param);   // ?SetAlphaFadeStart@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
        void SetAlphaFadeEnd(apsParam param);     // ?SetAlphaFadeEnd@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
        void SetTintColor(apsParam param);        // ?SetTintColor@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
        void SetAmbientCoeff(apsParam param);     // ?SetAmbientCoeff@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
        void SetDiffuseCoeff(apsParam param);     // ?SetDiffuseCoeff@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
        void SetZFeatherDistance(apsParam param); // ?SetZFeatherDistance@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
        void SetShaderWorksShader(apsParam param);// ?SetShaderWorksShader@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
        void SetIsShimmer(apsParam param);        // ?SetIsShimmer@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
        void SetUseSortedRendering(apsParam param);  // ?SetUseSortedRendering@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
        void SetChanceToRemove(apsParam param);   // ?SetChanceToRemove@cArgs@apsBillboardRenderer@@QAEXVapsParam@@@Z
    };
    math::Dir3     mNormal;              // +0x10
    math::Vector4  mTintColor;           // +0x20
    nglTexture*    mTexture;             // +0x30
    apsEBlendMode  mBlendMode;           // +0x34
    math::Vector4  mAmbientCoeff;        // +0x40
    math::Vector4  mDiffuseCoeff;        // +0x50
    float          mAlphaFadeStart;      // +0x60
    float          mAlphaFadeEnd;        // +0x64
    float          mZFeatherDistance;    // +0x68
    void*          mShaderWorksShader;   // +0x6C
    int            mIsShimmer;           // +0x70
    int            mUseSortedRendering;  // +0x74
    float          mChanceToRemove;      // +0x78
    apsBillboardRenderer(const cArgs* args);          // @0x805F60
    virtual ~apsBillboardRenderer();                  // @0x805CE0 (vtable)
    static void Init();                                    // @0x805D50
    bool UsesAmbientLighting() const;                      // @0x805C50
    bool UsesDiffuseLighting() const;                      // @0x805CD0
    bool SetNodeParams(apsRenderNode* node, const apsRendererRenderInfo& rinfo);  // @0x805D80
    bool UseSortedRendering() const { return mUseSortedRendering != 0; }
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo);  // @0x8060F0
    virtual unsigned int GetId() const;
    virtual float GetVersion() const;
    virtual int IsCameraFacing() const;
    virtual void SetScreenFacingNormal(const math::Dir3& iNormal);
    virtual float GetChanceToRemove() const;
    // Inline template (emitted in this object): DefaultRender<Renderer,Node>
    // ea: 0x8061A0 (apsBillboardRenderer/apsBillboardNode instantiation)
    template <typename RendererT, typename NodeT>
    eRenderResult DefaultRender(const apsRendererRenderInfo& rinfo) {
        NodeT* node = (NodeT*)nglListAlloc(0xC0, 0x10);
        if (node == NULL)
            return RENDERRESULT_NO_PARTICLES;
        node->mFlags = 0;
        node->mRenderSortBuffer = NULL;
        if (this->UseSortedRendering() != 0) {
            apsRenderSort::Buffer* v4 = (apsRenderSort::Buffer*)nglListAlloc(4 * rinfo.numParticles + 8, 0x10);
            if (v4 != NULL) {
                v4->capacity = rinfo.numParticles;
                v4->buffer = (unsigned char**)&v4[1];
                node->mRenderSortBuffer = v4;
            }
        }
        node->mRenderer = (RendererT*)this;
        if (!this->SetNodeParams(node, rinfo))
            return RENDERRESULT_NO_RENDERER;
        nglScene* v6 = apsCommon::mShimmerScene;
        if (node->mRenderer != NULL && node->mRenderer->mIsShimmer != 0 && v6 != NULL) {
            nglScene* v7 = nglListSelectScene(v6);
            nglListAddNode_Translucent(node, rinfo.groupDist2Camera);
            nglListSelectScene(v7);
            return RENDERRESULT_VISIBLE;
        } else {
            this->AddNode(node, rinfo.groupDist2Camera);
            return RENDERRESULT_VISIBLE;
        }
    }
    APS_DECLARE_RETRIEVE_LEAF(apsBillboardRenderer)};
static_assert(sizeof(apsBillboardRenderer) == 0x80, "apsBillboardRenderer size mismatch");
// ============================================================================
// Shader microcode registration structs (data in apsBillboardRendererVertex.o)
// ============================================================================
struct apsBillboardRender {
    static unsigned int* VS;                  // ?VS@apsBillboardRender@@3PAKA
    static const unsigned int** VShaderTable; // ?VShaderTable@apsBillboardRender@@3PAPBIA
    static void RegisterVShader() { nglDxRegisterVShader(reinterpret_cast<unsigned long*>(VS), VShaderTable[0]); }   // ea: 0x805CF0
};
struct apsBillboardRenderPixel {
    static unsigned int** PS;                 // ?PS@apsBillboardRenderPixel@@3PAPAKA
    static const unsigned int** PShaderTable; // ?PShaderTable@apsBillboardRenderPixel@@3PAPBIA
    static void RegisterPShader() { nglDxRegisterPShader(reinterpret_cast<unsigned long**>(PS), PShaderTable[0]); }   // ea: 0x805D10
    static void InitPShader() { nglDxInitPShader(PShaderTable[0]); }              // ea: 0x805D30
};
// ============================================================================
// externs
// ============================================================================
extern void* nglListAlloc(unsigned Bytes, unsigned Alignment);
extern nglScene* nglBuildScene;
extern nglScene* nglListSelectScene(nglScene* scene);
extern void nglListAddNode_Translucent(nglRenderNode* Node, float Dist);
extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);
extern void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode);
extern void nglDxInitPShader(const unsigned int* Microcode);
// apsInternal::GetBlendColor (inline, apsInternal.h)
#include "apsInternal.h"
#endif // COD3_AEPS_APSBILLBOARDRENDERER_H

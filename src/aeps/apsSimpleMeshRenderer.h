// ============================================================================
// apsSimpleMeshRenderer — mesh-based particle renderer (4 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsSimpleMeshRenderer.cpp
// Verified against IDA (aeps_xboxr:apsSimpleMeshRenderer.o):
//   Init            @0x802920 (?Init@apsSimpleMeshRenderer@@SAXXZ)
//   InitShader      @0x802950 (?InitShader@apsSimpleMeshRenderer@@SAXXZ)
//   ctor(cArgs)     @0x802990 (??0apsSimpleMeshRenderer@@QAE@ABVcArgs@0@@Z)
//   Render          @0x8029C0 (?Render@apsSimpleMeshRenderer@@UAE?AW4eRenderResult@apsRenderer@@ABUapsRendererRenderInfo@@@Z)
// ============================================================================
#ifndef COD3_AEPS_APSSIMPLEMESHRENDERER_H
#define COD3_AEPS_APSSIMPLEMESHRENDERER_H

#include "apsRenderer.h"
#include "apsCommon.h"
#include "ngl/nglScene.h"

#include <intrin.h>

struct nglMesh;
struct nglLightContext;
class nglMeshNode;
struct nglMeshSection;
struct nglMaterial;

// Forward declaration (apsSimpleMeshNode references the renderer)
class apsSimpleMeshRenderer;

// ============================================================================
// nglShader — base shader (16 bytes, verified against IDA: tlInitList + fields)
// ============================================================================
struct nglShader : tlInitList {
    bool Disabled;  // +0x08
    int  ID;        // +0x0C
};

// ============================================================================
// apsSimpleMeshShader — simple-mesh shader (16 bytes, nglShader-derived)
// ============================================================================
class apsSimpleMeshShader : public nglShader {
public:
    apsSimpleMeshShader();   // ea: 0x802860
    virtual tlFixedString GetName();   // ea: 0x802890
    virtual void AddNode(nglMeshNode* node, nglMeshSection* section, nglMaterial* material);  // ea: 0x8028B0
};
static_assert(sizeof(apsSimpleMeshShader) == 0x10, "apsSimpleMeshShader size mismatch");

// ============================================================================
// apsSimpleMeshNode — mesh render node (192 bytes).
// ============================================================================
class apsSimpleMeshNode : public apsRenderNode {
public:
    apsSimpleMeshRenderer* mRenderer;      // +0xB0
    nglLightContext*       mLightContext;  // +0xB4
    int                    mZBuffer;       // +0xB8

    void SetRenderer(apsSimpleMeshRenderer* r) { mRenderer = r; }       // ea: 0x8028C0
    void SetLightContext(nglLightContext* lc) { mLightContext = lc; }   // ea: 0x8028D0
    virtual void GetDesc(char* buf);                                    // ea: 0x8028E0
};
static_assert(sizeof(apsSimpleMeshNode) == 0xC0, "apsSimpleMeshNode size mismatch");

// ============================================================================
// apsSimpleMeshRenderer — mesh particle renderer (24 bytes).
// ============================================================================
class apsSimpleMeshRenderer : public apsRenderer {
public:
    struct cArgs {
        nglMesh*    mMesh;
        nglTexture* mTexture;
    };

    nglMesh*    mMesh;      // +0x10
    nglTexture* mTexture;   // +0x14

    apsSimpleMeshRenderer(const cArgs* args);          // @0x802990
    virtual ~apsSimpleMeshRenderer();                  // @0x8029A0 (vtable)

    static void Init();                                // @0x802920
    static void InitShader();                          // @0x802950 (?InitShader@apsSimpleMeshRenderer@@SAXXZ)
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo);  // @0x8029C0
    virtual unsigned int GetId() const;
    virtual float GetVersion() const;
    virtual int IsCameraFacing() const;
    virtual void SetScreenFacingNormal(const math::Dir3& iNormal);
    virtual float GetChanceToRemove() const;
    virtual bool GetMeshRadius(float& oRadius) const;
};
static_assert(sizeof(apsSimpleMeshRenderer) == 0x18, "apsSimpleMeshRenderer size mismatch");


// ============================================================================
// ngl mesh/shader externs
// ============================================================================
extern void* nglListAlloc(unsigned Bytes, unsigned Alignment);
extern nglScene* nglBuildScene;
extern nglLightContext* nglDefaultLightContext;
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);
extern void nglDxInitPShader(const unsigned int* Microcode);

// ============================================================================
// Shader microcode registration structs (inline COMDATs + data in
// apsSimpleMeshRendererVertex.o)
// ============================================================================
struct apsSimpleMeshRender {
    static unsigned int* VS;                  // ?VS@apsSimpleMeshRender@@3PAKA
    static const unsigned int** VShaderTable; // ?VShaderTable@apsSimpleMeshRender@@3PAPBIA

    static void RegisterVShader() { nglDxRegisterVShader(VS, VShaderTable[0]); }   // ea: 0x8028F0
};

struct apsSimpleMeshRenderPixel {
    static unsigned int** PS;                 // ?PS@apsSimpleMeshRenderPixel@@3PAPAKA
    static const unsigned int** PShaderTable; // ?PShaderTable@apsSimpleMeshRenderPixel@@3PAPBIA

    static void RegisterPShader() { nglDxRegisterPShader(PS, PShaderTable[0]); }   // ea: 0x802910
    static void InitPShader() { nglDxInitPShader(PShaderTable[0]); }              // ea: 0x8029A0
};


#endif // COD3_AEPS_APSSIMPLEMESHRENDERER_H

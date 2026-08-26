// ============================================================================
// apsSimpleMeshRenderer.cpp — mesh-based particle renderer (4 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsSimpleMeshRenderer.cpp
// Verified against IDA (aeps_xboxr:apsSimpleMeshRenderer.o):
//   Init            @0x802920 (?Init@apsSimpleMeshRenderer@@SAXXZ)
//   InitShader      @0x802950 (?InitShader@apsSimpleMeshRenderer@@SAXXZ)
//   ctor(cArgs)     @0x802990 (??0apsSimpleMeshRenderer@@QAE@ABVcArgs@0@@Z)
//   Render          @0x8029C0 (?Render@apsSimpleMeshRenderer@@UAE?AW4eRenderResult@apsRenderer@@ABUapsRendererRenderInfo@@@Z)
// ============================================================================
#include "apsSimpleMeshRenderer.h"
#include "ngl/ngl_dx_gpu.h"

#include <new>

// APS shader static data definitions (aeps_xboxr)
unsigned long* apsSimpleMeshRender::VS = nullptr;
const unsigned long** apsSimpleMeshRender::VShaderTable = nullptr;
unsigned long** apsSimpleMeshRenderPixel::PS = nullptr;
const unsigned long** apsSimpleMeshRenderPixel::PShaderTable = nullptr;

// tl_system.o (tl_xboxr, ported)
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);


// ============================================================================
// apsSimpleMeshRenderer::Init — register the vertex/pixel shaders.
// ea: 0x802920
// ============================================================================
void apsSimpleMeshRenderer::Init() {
    if (apsSimpleMeshRender::VShaderTable != NULL)
        nglDxRegisterVShader(apsSimpleMeshRender::VS,
                             reinterpret_cast<const unsigned int*>(apsSimpleMeshRender::VShaderTable[0]));
    if (apsSimpleMeshRenderPixel::PShaderTable != NULL)
        nglDxRegisterPShader(apsSimpleMeshRenderPixel::PS,
                             reinterpret_cast<const unsigned int*>(apsSimpleMeshRenderPixel::PShaderTable[0]));
}

// ============================================================================
// apsSimpleMeshRenderer::InitShader — allocate a apsSimpleMeshShader and link
// it into the init list.
// ea: 0x802950
// ============================================================================
void apsSimpleMeshRenderer::InitShader() {
    apsAllocator* Allocator = apsCommon::GetAllocator();
    void* result = Allocator->MemAlign(16, 4);
    if (result != 0) {
        new (result) apsSimpleMeshShader();
    }
}

// ============================================================================
// apsSimpleMeshRenderer::apsSimpleMeshRenderer — construct with cArgs.
// ea: 0x802990
// ============================================================================
apsSimpleMeshRenderer::apsSimpleMeshRenderer(const apsSimpleMeshRenderer::cArgs* args) {
    mFields = 0;
    mPriority = 0;
    mIsDynamicallyLit = 0;
    mMesh = args->mMesh;
    mTexture = args->mTexture;
    mFields = 49;
}

// ============================================================================
// apsSimpleMeshRenderer::GetId - ea: 0x7F36E0 (apsRegister.o COMDAT)
// ============================================================================
unsigned int apsSimpleMeshRenderer::GetId() const {
    return 1397584744;
}

// ============================================================================
// apsSimpleMeshRenderer::GetVersion - ea: 0x7F36F0 (apsRegister.o COMDAT)
// ============================================================================
float apsSimpleMeshRenderer::GetVersion() const {
    return 1.0f;
}

// ============================================================================
// apsSimpleMeshRenderer::IsCameraFacing - ea: 0x7F3700 (apsRegister.o COMDAT)
// ============================================================================
int apsSimpleMeshRenderer::IsCameraFacing() const {
    return 0;
}

// ============================================================================
// apsSimpleMeshRenderer::SetScreenFacingNormal - ea: 0x7F3710
// ============================================================================
void apsSimpleMeshRenderer::SetScreenFacingNormal(const math::Dir3& iNormal) {
    (void)iNormal;
}

// ============================================================================
// apsSimpleMeshRenderer::GetMeshRadius - ea: 0x7F3720
// ============================================================================
bool apsSimpleMeshRenderer::GetMeshRadius(float& oRadius) const {
    if (mMesh == nullptr)
        return false;
    oRadius = mMesh->Sphere.v.m128_f32[3];
    return true;
}

// ============================================================================
// apsSimpleMeshRenderer::Render — allocate a mesh node, populate it from the
// render info, and insert it into the transparent render list.
// ea: 0x8029C0
// ============================================================================
apsRenderer::eRenderResult apsSimpleMeshRenderer::Render(const apsRendererRenderInfo& rinfo) {
    if (this->mMesh == NULL)
        return RENDERRESULT_NO_PARTICLES;

    apsSimpleMeshNode* node = (apsSimpleMeshNode*)nglListAlloc(0xC0, 0x10);
    if (node == NULL)
        return RENDERRESULT_NO_PARTICLES;

    new (node) apsSimpleMeshNode();
    node->mFlags = 0;
    node->mRenderer = this;
    node->SetParticles(rinfo.numParticles, rinfo.particles, rinfo.pfd->mStride);
    node->mSphere = rinfo.sphere;
    node->SetMatrix(rinfo.localToWorld);

    nglLightContext* lightContext = rinfo.lightContext;
    if (lightContext == NULL)
        lightContext = nglDefaultLightContext;
    node->mLightContext = lightContext;

    int v6;
    if ((this->mFields & 0x4000) != 0) {
        if ((rinfo.pfd->mFields & 0x4000) == 0 &&
            _tlAssert("source/apsSimpleMeshRenderer.cpp", 108,
                      "rinfo.pfd->HasField(apsPFDField_Velocity)",
                      "velocity not present in particle")) {
            __debugbreak();
        }
        v6 = 2;
    } else {
        v6 = this->IsCameraFacing() != 0;
    }

    float Dist = node->GetDist(nglBuildScene);
    if (_mm_shuffle_ps(rinfo.sphere.mSphere.v, rinfo.sphere.mSphere.v, 255).m128_f32[0] + Dist
        > nglBuildScene->FogNear) {
        v6 |= 8;
    }

    unsigned int mFlags = node->mFlags;
    node->SortDist = Dist;
    node->mFlags = v6 | mFlags;
    node->Next = nglBuildScene->TransRenderList;
    nglBuildScene->TransRenderList = node;
    ++nglBuildScene->TransListCount;
    return RENDERRESULT_VISIBLE;
}

// ============================================================================
// Inline COMDATs (apsRenderNode methods, emitted in this object)
// ============================================================================
// apsRenderNode::GetDist — distance from camera to sphere center (view space).
// ea: 0x802630
float apsRenderNode::GetDist(nglScene* Scene) {
    __m128 v4;
    v4.m128_f32[0] = mSphere.mSphere.v.m128_f32[0];
    v4.m128_f32[1] = 0.0f;
    v4.m128_f32[2] = mSphere.mSphere.v.m128_f32[2];
    v4.m128_f32[3] = 0.0f;

    __m128 v2 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v4, v4, 0), Scene->WorldToView.x.v),
            _mm_mul_ps(_mm_shuffle_ps(v4, v4, 85), Scene->WorldToView.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v4, v4, 170), Scene->WorldToView.z.v),
            Scene->WorldToView.w.v));
    return _mm_shuffle_ps(v2, v2, 170).m128_f32[0];
}

// apsRenderNode::SetMatrix — set the local-to-world transform.
// ea: 0x802700
void apsRenderNode::SetMatrix(const math::Mat43* matrix) {
    if (matrix != NULL)
        mLocalToWorld = *matrix;
    else
        apsMath::SetIdentityMatrix(mLocalToWorld);
}

// apsRenderNode::SetParticles — set the particle buffer reference.
// ea: 0x8027D0
void apsRenderNode::SetParticles(int num, unsigned char* ref, int stride) {
    if (num != 0 && ref != NULL && stride != 0) {
        mNumParticles = num;
        mParticles = ref;
        mStride = stride;
    } else {
        mNumParticles = 0;
        mParticles = NULL;
        mStride = 0;
    }
}

// apsRenderNode::~apsRenderNode — ea: 0x802C20
apsRenderNode::~apsRenderNode() {
}

// apsSimpleMeshShader::apsSimpleMeshShader — link into the init list.
// ea: 0x802860
apsSimpleMeshShader::apsSimpleMeshShader() {
    next = tlInitList::head;
    tlInitList::head = this;
    Disabled = false;
}

// apsSimpleMeshShader::GetName — ea: 0x802890
tlFixedString apsSimpleMeshShader::GetName() {
    return tlFixedString("aeps_simple");
}

// apsSimpleMeshShader::AddNode — ea: 0x8028B0
void apsSimpleMeshShader::AddNode(nglMeshNode* node, nglMeshSection* section, nglMaterial* material) {
}

// apsSimpleMeshNode::GetDesc — ea: 0x8028E0
void apsSimpleMeshNode::GetDesc(char* buf) {
}

// ============================================================================
// apsSimpleMeshRenderer::~apsSimpleMeshRenderer - apsRegister.o COMDAT (sets base vtable)
// ============================================================================
apsSimpleMeshRenderer::~apsSimpleMeshRenderer() {
    *(unsigned int*)this = 0x00D384E0;  // apsVirtualBase vtable
}

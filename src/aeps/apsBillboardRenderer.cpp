// ============================================================================
// apsBillboardRenderer.cpp — billboard particle renderer (6 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsBillboardRenderer.cpp
// Verified against IDA (aeps_xboxr:apsBillboardRenderer.o):
//   UsesAmbientLighting @0x805C50
//   UsesDiffuseLighting @0x805CD0
//   Init                @0x805D50
//   SetNodeParams       @0x805D80
//   ctor(cArgs)         @0x805F60
//   Render              @0x8060F0
// ============================================================================
#include "apsBillboardRenderer.h"
#include "apsShrimpRenderer.h"

// APS shader static data definitions (aeps_xboxr)
unsigned int* apsBillboardRender::VS = nullptr;
const unsigned int** apsBillboardRender::VShaderTable = nullptr;
unsigned int** apsBillboardRenderPixel::PS = nullptr;
const unsigned int** apsBillboardRenderPixel::PShaderTable = nullptr;

// tl_system.o (tl_xboxr, ported)
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

namespace apsRenderSort {

// ea: 0x00518800
ParticleIterator::~ParticleIterator()
{
}

// ea: 0x00518840
SortedParticleIterator::~SortedParticleIterator()
{
}

// ea: 0x00518850
UnsortedParticleIterator::~UnsortedParticleIterator()
{
}

// ea: 0x00518860
ParticleIterator::ParticleIterator()
{
}

// ea: 0x00518870
UnsortedParticleIterator::UnsortedParticleIterator()
{
}

// ea: 0x00518880
unsigned char* UnsortedParticleIterator::GetNextParticle()
{
    unsigned char* result = m_currentParticle;
    if (result >= m_particleEnd)
        return nullptr;
    m_currentParticle = result + m_stride;
    return result;
}

}

// ============================================================================
// apsBillboardRenderer::UsesAmbientLighting — any positive ambient coefficient?
// ea: 0x805C50
// ============================================================================
bool apsBillboardRenderer::UsesAmbientLighting() const {
    return mAmbientCoeff.v.m128_f32[0] > 0.0f
        || _mm_shuffle_ps(mAmbientCoeff.v, mAmbientCoeff.v, 85).m128_f32[0] > 0.0f
        || _mm_shuffle_ps(mAmbientCoeff.v, mAmbientCoeff.v, 170).m128_f32[0] > 0.0f;
}

// ============================================================================
// apsBillboardRenderer::UsesDiffuseLighting — any positive diffuse coefficient?
// ea: 0x805CD0
// ============================================================================
bool apsBillboardRenderer::UsesDiffuseLighting() const {
    return mDiffuseCoeff.v.m128_f32[0] > 0.0f
        || _mm_shuffle_ps(mDiffuseCoeff.v, mDiffuseCoeff.v, 85).m128_f32[0] > 0.0f
        || _mm_shuffle_ps(mDiffuseCoeff.v, mDiffuseCoeff.v, 170).m128_f32[0] > 0.0f;
}

// ============================================================================
// apsBillboardRenderer::Init — register the vertex/pixel shaders.
// ea: 0x805D50
// ============================================================================
void apsBillboardRenderer::Init() {
    if (apsBillboardRender::VShaderTable != NULL)
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(apsBillboardRender::VS), apsBillboardRender::VShaderTable[0]);
    if (apsBillboardRenderPixel::PShaderTable != NULL)
        nglDxRegisterPShader(reinterpret_cast<unsigned long**>(apsBillboardRenderPixel::PS), apsBillboardRenderPixel::PShaderTable[0]);
}

// ============================================================================
// apsBillboardRenderer::SetNodeParams — populate a billboard node from the
// render info (particles, matrix, blend color, flags).
// ea: 0x805D80
// ============================================================================
bool apsBillboardRenderer::SetNodeParams(apsRenderNode* node, const apsRendererRenderInfo& rinfo) {
    int mStride = rinfo.pfd->mStride;
    int numParticles = rinfo.numParticles;
    unsigned char* particles = rinfo.particles;
    if (numParticles != 0 && particles != NULL && mStride != 0) {
        node->mNumParticles = numParticles;
        node->mStride = mStride;
    } else {
        numParticles = 0;
        node->mNumParticles = 0;
        node->mStride = 0;
    }
    node->mParticles = (numParticles != 0 && particles != NULL && mStride != 0) ? particles : NULL;
    node->mSphere = rinfo.sphere;
    node->SetMatrix(rinfo.localToWorld);

    math::Vector4 v20;
    if (rinfo.lightContext != NULL && this->mBlendMode == apsEBlendMode_BlendWithLighting) {
        v20 = apsInternal::GetBlendColor(rinfo.lightContext);
        v20.v = _mm_mul_ps(v20.v, this->mTintColor.v);
    } else {
        v20 = this->mTintColor;
    }

    if (this->UsesAmbientLighting())
        v20.v = _mm_mul_ps(v20.v, _mm_mul_ps(this->mAmbientCoeff.v, rinfo.lightInfo->m_ambientColor.v));

    node->mBlendColor = v20;

    unsigned int v21 = 0;
    if ((this->mFields & 0x4000) != 0) {
        if ((rinfo.pfd->mFields & 0x4000) == 0 &&
            _tlAssert("source/apsBillboardRenderer.cpp", 82,
                      "rinfo.pfd->HasField(apsPFDField_Velocity)",
                      "velocity not present in particle")) {
            __debugbreak();
        }
        v21 = 2;
    } else {
        v21 = this->IsCameraFacing() != 0;
    }

    float Dist = node->GetDist(nglBuildScene);
    if (this->mBlendMode <= apsEBlendMode_Blend || this->mBlendMode > apsEBlendMode_Subtract) {
        float radius = _mm_shuffle_ps(rinfo.sphere.mSphere.v, rinfo.sphere.mSphere.v, 255).m128_f32[0];
        if (radius + Dist > nglBuildScene->FogNear)
            v21 |= 8;
    }

    node->mFlags |= v21;
    node->mLightInfo = *rinfo.lightInfo;
    return true;
}

// ============================================================================
// apsBillboardRenderer::apsBillboardRenderer — construct with cArgs.
// ea: 0x805F60
// ============================================================================
apsBillboardRenderer::apsBillboardRenderer(const apsBillboardRenderer::cArgs* args) {
    mFields = 0;
    mPriority = 0;
    mIsDynamicallyLit = 0;
    mTintColor.v = _mm_setr_ps(args->mTintColor.x, args->mTintColor.y,
                               args->mTintColor.z, args->mTintColor.w);
    mTexture = args->mTexture;
    mBlendMode = args->mBlendMode;
    mAmbientCoeff.v = _mm_setr_ps(args->mAmbientCoeff.x, args->mAmbientCoeff.y,
                                  args->mAmbientCoeff.z, args->mAmbientCoeff.w);
    mDiffuseCoeff.v = _mm_setr_ps(args->mDiffuseCoeff.x, args->mDiffuseCoeff.y,
                                  args->mDiffuseCoeff.z, args->mDiffuseCoeff.w);
    mAlphaFadeStart = args->mAlphaFadeStart;
    mAlphaFadeEnd = args->mAlphaFadeEnd;
    mZFeatherDistance = args->mZFeatherDistance;
    mShaderWorksShader = args->mShaderWorksShader;
    mIsShimmer = args->mIsShimmer;
    mUseSortedRendering = args->mUseSortedRendering;
    mChanceToRemove = args->mChanceToRemove;
    mNormal.v = _mm_setr_ps(args->mNormal.x, args->mNormal.y, args->mNormal.z, 0.0f);

    mIsDynamicallyLit = this->UsesAmbientLighting() || this->UsesDiffuseLighting();
    mFields = (args->mVelocityTracked != 0) ? 16467 : 83;
}

// ============================================================================
// apsBillboardRenderer::Render — color field is not supported; dispatch to the
// DefaultRender template.
// ea: 0x8060F0
// ============================================================================
apsRenderer::eRenderResult apsBillboardRenderer::Render(const apsRendererRenderInfo& rinfo) {
    if ((rinfo.pfd->mFields & 8) != 0 &&
        _tlAssert("source/apsBillboardRenderer.cpp", 226,
                  "!rinfo.pfd->HasField(apsPFDField_Color)",
                  "apsBillboardRenderer can't handle color")) {
        __debugbreak();
    }
    return this->DefaultRender<apsBillboardRenderer, apsBillboardNode>(rinfo);
}

// ============================================================================
// apsBillboardRenderer::GetId - ea: 0x7F30D0 (apsRegister.o COMDAT)
// ============================================================================
unsigned int apsBillboardRenderer::GetId() const {
    return 1114399343;
}

// ============================================================================
// apsBillboardRenderer::GetVersion - ea: 0x7F30E0 (apsRegister.o COMDAT)
// ============================================================================
float apsBillboardRenderer::GetVersion() const {
    return 1.0f;
}

// ============================================================================
// apsBillboardRenderer::IsCameraFacing - ea: 0x7F30F0 (apsRegister.o COMDAT)
// ============================================================================
int apsBillboardRenderer::IsCameraFacing() const {
    return mNormal.v.m128_f32[0] == 0.0f
        && _mm_shuffle_ps(mNormal.v, mNormal.v, 85).m128_f32[0] == 0.0f
        && _mm_shuffle_ps(mNormal.v, mNormal.v, 170).m128_f32[0] == 0.0f;
}

// ============================================================================
// apsBillboardRenderer::SetScreenFacingNormal - ea: 0x7F3170
// ============================================================================
void apsBillboardRenderer::SetScreenFacingNormal(const math::Dir3& iNormal) {
    mNormal.v = iNormal.v;
}

// ============================================================================
// apsBillboardRenderer::GetChanceToRemove - ea: 0x7F31B0
// ============================================================================
float apsBillboardRenderer::GetChanceToRemove() const {
    return mChanceToRemove;
}

// ============================================================================
// apsBillboardNode::GetDesc — ea: 0x8048E0 (inline COMDAT)
// ============================================================================
void apsBillboardNode::GetDesc(char* buf) {
}

// ============================================================================
// apsBillboardRenderer::~apsBillboardRenderer - apsRegister.o COMDAT (sets base vtable)
// ============================================================================
apsBillboardRenderer::~apsBillboardRenderer() {
    *(unsigned int*)this = 0x00D384E0;  // apsVirtualBase vtable
}

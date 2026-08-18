// ============================================================================
// apsShrimpRenderer.cpp — shrimp/sprite particle renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsShrimpRenderer.cpp
// Verified against IDA (aeps_xboxr:apsShrimpRenderer.o):
//   Init            @0x8044F0 (?Init@apsShrimpRenderer@@SAXXZ)
//   ctor(cArgs)     @0x804520 (??0apsShrimpRenderer@@QAE@ABVcArgs@0@@Z)
//   Render          @0x8045E0 (?Render@apsShrimpRenderer@@UAE?AW4eRenderResult@apsRenderer@@ABUapsRendererRenderInfo@@@Z)
// ============================================================================
#include "apsShrimpRenderer.h"
#include "apsInternal.h"

// APS shader static data definitions (aeps_xboxr)
unsigned int* apsShrimpRender::VS = nullptr;
const unsigned int** apsShrimpRender::VShaderTable = nullptr;
unsigned int** apsShrimpRenderPixel::PS = nullptr;
const unsigned int** apsShrimpRenderPixel::PShaderTable = nullptr;

// tl_system.o (tl_xboxr, ported)
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);


// ============================================================================
// apsShrimpRenderer::Init — register the vertex/pixel shaders.
// ea: 0x8044F0
// ============================================================================
void apsShrimpRenderer::Init() {
    if (apsShrimpRender::VShaderTable != NULL)
        nglDxRegisterVShader(apsShrimpRender::VS, apsShrimpRender::VShaderTable[0]);
    if (apsShrimpRenderPixel::PShaderTable != NULL)
        nglDxRegisterPShader(apsShrimpRenderPixel::PS, apsShrimpRenderPixel::PShaderTable[0]);
}

// ============================================================================
// apsShrimpRenderer::apsShrimpRenderer — construct with cArgs.
// ea: 0x804520
// ============================================================================
apsShrimpRenderer::apsShrimpRenderer(const apsShrimpRenderer::cArgs* args) {
    mFields = 0;
    mPriority = 0;
    mIsDynamicallyLit = 0;
    mTintColor.v = _mm_setr_ps(args->mTintColor.x, args->mTintColor.y,
                               args->mTintColor.z, args->mTintColor.w);
    mTexture = args->mTexture;
    mBlendMode = args->mBlendMode;
    mNumFrames = (int16_t)args->mNumFrames;
    mNumRows = (int16_t)args->mNumRows;
    mNumRotations = (int16_t)args->mNumRotations;
    mSpriteWidth = (int16_t)args->mSpriteWidth;
    mSpriteHeight = (int16_t)args->mSpriteHeight;
    mTextureWidth = (int16_t)args->mTextureWidth;
    mTextureHeight = (int16_t)args->mTextureHeight;
    mFields = 469;
}

// ============================================================================
// apsShrimpRenderer::GetId - ea: 0x7F3820 (apsRegister.o COMDAT)
// ============================================================================
unsigned int apsShrimpRenderer::GetId() const {
    return 1399352688;
}

// ============================================================================
// apsShrimpRenderer::GetVersion - ea: 0x7F3830 (apsRegister.o COMDAT)
// ============================================================================
float apsShrimpRenderer::GetVersion() const {
    return 1.0f;
}

// ============================================================================
// apsShrimpRenderer::IsCameraFacing - ea: 0x7F3840 (apsRegister.o COMDAT)
// ============================================================================
int apsShrimpRenderer::IsCameraFacing() const {
    return 0;
}

// ============================================================================
// apsShrimpRenderer::SetScreenFacingNormal - ea: 0x7F3850
// ============================================================================
void apsShrimpRenderer::SetScreenFacingNormal(const math::Dir3& iNormal) {
    (void)iNormal;
}

// ============================================================================
// apsShrimpRenderer::Render — allocate a shrimp node, populate it from the
// render info, and insert it into the opaque render list.
// ea: 0x8045E0
// ============================================================================
apsRenderer::eRenderResult apsShrimpRenderer::Render(const apsRendererRenderInfo& rinfo) {
    const apsPFD* pfd = rinfo.pfd;

    if ((pfd->mFields & 0x80) == 0 &&
        _tlAssert("source/apsShrimpRenderer.cpp", 90,
                  "rinfo.pfd->HasField(apsPFDField_Height)", "apsShrimp needs Height")) {
        __debugbreak();
    }
    if ((rinfo.pfd->mFields & 0x100) == 0 &&
        _tlAssert("source/apsShrimpRenderer.cpp", 91,
                  "rinfo.pfd->HasField(apsPFDField_UVAFrame)", "apsShrimp needs UVAFrame")) {
        __debugbreak();
    }

    apsShrimpNode* node = (apsShrimpNode*)nglListAlloc(0xC0, 0x10);
    if (node == NULL)
        return RENDERRESULT_NO_PARTICLES;

    node->mFlags = 0;
    node->mRenderer = this;

    int mStride = rinfo.pfd->mStride;
    int numParticles = rinfo.numParticles;
    unsigned char* particles = rinfo.particles;
    if (numParticles != 0 && particles != NULL && mStride != 0) {
        node->mParticles = particles;
        node->mStride = mStride;
    } else {
        numParticles = 0;
        node->mParticles = NULL;
        node->mStride = 0;
    }
    node->mNumParticles = numParticles;
    node->mSphere = rinfo.sphere;
    node->SetMatrix(rinfo.localToWorld);

    if (rinfo.lightContext != NULL && this->mBlendMode == apsEBlendMode_BlendWithLighting) {
        math::Vector4 BlendColor;
        BlendColor = apsInternal::GetBlendColor(rinfo.lightContext);
        BlendColor.v = _mm_mul_ps(BlendColor.v, this->mTintColor.v);
        node->mBlendColor = BlendColor;
    } else {
        node->mBlendColor = this->mTintColor;
    }

    int v15;
    if ((this->mFields & 0x4000) != 0) {
        if ((rinfo.pfd->mFields & 0x4000) == 0 &&
            _tlAssert("source/apsShrimpRenderer.cpp", 119,
                      "rinfo.pfd->HasField(apsPFDField_Velocity)",
                      "velocity not present in particle")) {
            __debugbreak();
        }
        v15 = 2;
    } else {
        v15 = this->IsCameraFacing() != 0;
    }

    float v19 = _mm_shuffle_ps(rinfo.sphere.mSphere.v, rinfo.sphere.mSphere.v, 255).m128_f32[0];
    float Dist = node->GetDist(nglBuildScene);
    if (Dist + v19 > nglBuildScene->FogNear)
        v15 |= 8;

    unsigned int v17 = node->mFlags;
    node->SortDist = NAN;
    node->mFlags = v15 | v17;
    node->Next = nglBuildScene->OpaqueRenderList;
    nglBuildScene->OpaqueRenderList = node;
    ++nglBuildScene->OpaqueListCount;
    return RENDERRESULT_VISIBLE;
}

// ============================================================================
// apsShrimpNode::GetDesc — ea: 0x802FA0 (inline COMDAT)
// ============================================================================
void apsShrimpNode::GetDesc(char* buf) {
}

// ============================================================================
// apsShrimpRenderer::~apsShrimpRenderer - apsRegister.o COMDAT (sets base vtable)
// ============================================================================
apsShrimpRenderer::~apsShrimpRenderer() {
    *(unsigned int*)this = 0x00D384E0;  // apsVirtualBase vtable
}

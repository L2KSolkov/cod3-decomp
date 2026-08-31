// ============================================================================
// cdFlagShader.cpp — flag shader (7 non-inline funcs).
// Source: source/cdFlagShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdFlagShader.o):
//   cdFlagShaderMat::ctor @0x7C9FF0
//   InitCDFlagShader  @0x7CA080
//   ToggleCDFlagShader @0x7CA160
//   cdFlagShader::Register @0x7CA3C0
//   cdFlagShader::AddNode @0x7CA400
// ============================================================================
#include "cdFlagShader.h"

#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_state.h"
#include "render/ShaderCommon.h"

#include <intrin.h>
#include <string.h>

// Shader global pointer definitions
cdFlagShader* gCDFlagShader = nullptr;  // ?gCDFlagShader@@3PAVcdFlagShader@@A
extern unsigned int gShaderSwitchingFlags;
extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_BC2CFC;
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2CF8;
extern unsigned int dword_BC2D04;
extern unsigned int D3D__DirtyFlags;
extern unsigned int D3D__TextureState[4][32];
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;
extern unsigned int cdFlagRandomSeedID;

static __m128 FlagZeroW(__m128 value)
{
    const __m128 zero = _mm_setzero_ps();
    return _mm_shuffle_ps(value, _mm_shuffle_ps(zero, value, 160), 52);
}

static void FlagStoreVector(unsigned char* dst, unsigned int offset, __m128 value)
{
    *reinterpret_cast<__m128*>(dst + offset) = value;
}

// ea: 0x007CA470
void cdFlagShaderNode::Render()
{
    const unsigned int cullMode = this->mMaterial->mCullMode != 1 ? 0u : 0x900u;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, cullMode) == 0)
        D3DDevice_SetRenderState_CullMode(cullMode);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 0);
        dword_BC2CFC = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 1);
        dword_BC2D00 = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAFUNC, 0x204u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4033C, 0x204u);
        dword_BC2CF8 = 0x204u;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF, 0x80u) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40340, 0x80u);
        dword_BC2D04 = 0x80u;
    }

    nglDxSetTexture(0, this->mMaterial->mTexture, 1u, 3u);
    if (nglDxTexCache.Prev[0].WrapU != 1) {
        nglDxTexCache.Prev[0].WrapU = 1;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSU, 1u) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSU] = 1;
        }
    }
    if (nglDxTexCache.Prev[0].WrapV != 1) {
        nglDxTexCache.Prev[0].WrapV = 1;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSV, 1u) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSV] = 1;
        }
    }

    math::Mat43 flagToLocal;
    CalculateFlagMatrix(flagToLocal, this->Section, 5.0f);

    float time = ShaderCommon::gTime;
    const unsigned int seedId = cdFlagRandomSeedID;
    if ((this->MeshNode->ShaderParams.Array[seedId >> 5] &
         (1u << (seedId & 0x1Fu))) != 0) {
        const unsigned int seed = this->MeshNode->ShaderParams.Array[seedId + 2] & 0xFFFu;
        time += static_cast<float>(seed) * 0.00024420026f * 100.0f;
    }

    const __m128 f0 = flagToLocal.x.v;
    const __m128 f1 = flagToLocal.y.v;
    const __m128 f2 = flagToLocal.z.v;
    const __m128 f3 = flagToLocal.w.v;
    const __m128 v6 = _mm_shuffle_ps(f1, f2, 68);
    const __m128 v7 = _mm_shuffle_ps(v6, f0, 136);
    const __m128 v8 = _mm_shuffle_ps(v7, v7, 170);
    const __m128 v9 = _mm_shuffle_ps(v6, f0, 221);
    const __m128 v10 = _mm_shuffle_ps(_mm_shuffle_ps(f1, f2, 238), f0, 168);
    const __m128 v11 = _mm_setr_ps(v10.m128_f32[1], v10.m128_f32[2],
                                   v10.m128_f32[0], 0.0f);
    const __m128 v12 = FlagZeroW(v10);
    const __m128 v13 = FlagZeroW(v9);
    const __m128 v14 = _mm_mul_ps(v13, v11);
    const __m128 v15 = _mm_sub_ps(FlagZeroW(v14), _mm_mul_ps(v11, v12));
    const __m128 v16 = FlagZeroW(v7);
    const __m128 v17 = _mm_mul_ps(v11, v16);
    const __m128 v18 = _mm_mul_ps(v16, v11);
    const __m128 v19 = _mm_setr_ps(v7.m128_f32[1], v8.m128_f32[0],
                                   v7.m128_f32[0], 0.0f);
    const __m128 v61 = _mm_mul_ps(v12, v19);
    const __m128 v20 = _mm_mul_ps(_mm_setr_ps(v8.m128_f32[0],
                                               v7.m128_f32[0],
                                               v7.m128_f32[1], 0.0f), v15);
    const float v20Sum = v20.m128_f32[0] + v20.m128_f32[1] + v20.m128_f32[2] + v20.m128_f32[3];
    const __m128 inv = _mm_set1_ps(1.0f / v20Sum);
    const __m128 v23 = _mm_mul_ps(v15, inv);
    const __m128 v55 = _mm_mul_ps(_mm_sub_ps(FlagZeroW(v61), v17), inv);
    const __m128 v24 = _mm_mul_ps(_mm_sub_ps(FlagZeroW(v18), _mm_mul_ps(v19, v13)), inv);
    const __m128 v25 = _mm_xor_ps(_mm_set1_ps(-0.0f),
        _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(f3, f3, 0), v23),
                              _mm_mul_ps(_mm_shuffle_ps(f3, f3, 85), v55)),
                   _mm_mul_ps(_mm_shuffle_ps(f3, f3, 170), v24)));
    const __m128 v26 = _mm_shuffle_ps(v24, v25, 68);
    const __m128 v27 = _mm_shuffle_ps(v23, v55, 68);
    const __m128 v64 = _mm_shuffle_ps(v27, v26, 136);
    const __m128 v65 = _mm_shuffle_ps(v27, v26, 221);
    const __m128 v66 = _mm_shuffle_ps(_mm_shuffle_ps(v23, v55, 238),
                                      _mm_shuffle_ps(v24, v25, 238), 136);

    const math::Mat44& localToScreen = this->MeshNode->LocalToScreen;
    const __m128 x = localToScreen.x.v;
    const __m128 y = localToScreen.y.v;
    const __m128 z = localToScreen.z.v;
    const __m128 w = localToScreen.w.v;
    const __m128 v31 = _mm_shuffle_ps(x, y, 68);
    const __m128 v32 = _mm_shuffle_ps(z, w, 68);
    const __m128 v33 = _mm_shuffle_ps(z, w, 238);
    const __m128 v34 = v31;
    const __m128 v64b = _mm_shuffle_ps(v31, v32, 221);
    const __m128 v35 = FlagZeroW(f0);
    const __m128 v36 = _mm_shuffle_ps(v34, v32, 136);
    const __m128 v37 = _mm_shuffle_ps(x, y, 238);
    const __m128 v38 = _mm_shuffle_ps(v37, v33, 221);
    const __m128 v39 = _mm_shuffle_ps(v37, v33, 136);
    const __m128 v40 = v38;
    const __m128 v41 = FlagZeroW(f1);
    const __m128 v42 = FlagZeroW(f2);
    const __m128 v43 = _mm_shuffle_ps(f3, _mm_shuffle_ps(_mm_set1_ps(1.0f), f3, 160), 52);
    const auto combine = [&](const __m128& q) {
        return _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(q, q, 0), v36),
                                     _mm_mul_ps(_mm_shuffle_ps(q, q, 85), v64b)),
                          _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(q, q, 170), v39),
                                     _mm_mul_ps(_mm_shuffle_ps(q, q, 255), v40)));
    };
    const __m128 v44 = combine(v35);
    const __m128 v45 = combine(v41);
    const __m128 v46 = combine(v42);
    const __m128 v47 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v43, v43, 170), v39),
                                  _mm_mul_ps(_mm_shuffle_ps(v43, v43, 255), v40));
    const __m128 v48 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v43, v43, 0), v36),
                                  _mm_mul_ps(_mm_shuffle_ps(v43, v43, 85), v64b));
    const __m128 v49 = _mm_shuffle_ps(v44, v45, 68);
    const __m128 v50 = _mm_shuffle_ps(v44, v45, 238);
    const __m128 v51 = _mm_add_ps(v48, v47);
    const __m128 v52 = _mm_shuffle_ps(v46, v51, 68);
    const __m128 v63 = _mm_shuffle_ps(v49, v52, 136);
    const __m128 v64c = _mm_shuffle_ps(v49, v52, 221);
    const __m128 v65b = _mm_shuffle_ps(v50, _mm_shuffle_ps(v46, v51, 238), 136);
    const __m128 v66b = _mm_shuffle_ps(v50, _mm_shuffle_ps(v46, v51, 238), 221);

    alignas(16) unsigned char params[0x4D0] = {};
    FlagStoreVector(params, 0, v64);
    reinterpret_cast<float*>(params + 16)[0] = v65.m128_f32[0];
    reinterpret_cast<float*>(params + 24)[0] = v65.m128_f32[2];
    FlagStoreVector(params, 32, v66);
    FlagStoreVector(params, 48, _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f));
    FlagStoreVector(params, 64, v63);
    FlagStoreVector(params, 80, v64c);
    FlagStoreVector(params, 96, v65b);
    FlagStoreVector(params, 112, v66b);
    *reinterpret_cast<float*>(params + 128) = 0.25f;
    *reinterpret_cast<float*>(params + 132) = 1.5f;
    *reinterpret_cast<float*>(params + 136) = time;
    *reinterpret_cast<float*>(params + 140) = time * 0.25f;
    *reinterpret_cast<float*>(params + 144) = 42.0f;
    memcpy(params + 160, FlagSinTable, sizeof(FlagSinTable));
    *reinterpret_cast<unsigned int*>(params + 1184) = 1045220557u;
    *reinterpret_cast<unsigned int*>(params + 1188) = 1051931443u;
    *reinterpret_cast<unsigned int*>(params + 1192) = 1065353216u;
    *reinterpret_cast<unsigned int*>(params + 1196) = 0x40000000u;
    *reinterpret_cast<float*>(params + 1200) = 1.0f;
    *reinterpret_cast<float*>(params + 1204) = 0.6f;
    *reinterpret_cast<float*>(params + 1208) = 0.30000001f;
    *reinterpret_cast<float*>(params + 1212) = 0.0f;
    *reinterpret_cast<float*>(params + 1216) = 0.08f;
    *reinterpret_cast<float*>(params + 1220) = 0.08f;
    D3DDevice_SetVertexShaderConstantNotInlineFast(6, params, 0x134u);
    nglDxInitShaders(false);
    const unsigned int vertexShader = static_cast<unsigned int>(cdFlagVertex::VS[0]);
    if (vertexShader != gpuHashVertexShader) {
        gpuHashVertexShader = vertexShader;
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(static_cast<uintptr_t>(vertexShader)), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    const _D3DPixelShaderDef* pixelShader =
        reinterpret_cast<const _D3DPixelShaderDef*>(cdFlagPixel::PS[0]);
    if (pixelShader != reinterpret_cast<const _D3DPixelShaderDef*>(
                             static_cast<uintptr_t>(gpuHashPixelShader))) {
        gpuHashPixelShader = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pixelShader));
        D3DDevice_SetPixelShaderProgram(pixelShader);
    }
    nglGpuDrawSection(this->Section);
    nglDxState.PrevBM = static_cast<unsigned int>(-1);
}

// ea: 0x007CB840
cdFlagShader::cdFlagShader() {
    this->next = tlInitList::head;
    tlInitList::head = this;
    this->Disabled = false;
    *((unsigned char*)&gShaderSwitchingFlags) &= 0xF7;
}

// ea: 0x007CBA90
cdFlagShader::~cdFlagShader() = default;

// ea: 0x007CB8B0
void cdFlagVertex::RegisterVShader()
{
    nglDxRegisterVShader(cdFlagVertex::VS,
                         reinterpret_cast<const unsigned int*>(cdFlagVertex::VShaderTable[0]));
    cdFlagVertex::Shader = cdFlagVertex::VS[0];
}

// ea: 0x007CB8D0
unsigned long cdFlagVertex::GetVShader()
{
    return static_cast<unsigned int>(cdFlagVertex::VS[0]);
}

// ea: 0x007CB8E0
void cdFlagPixel::RegisterShader()
{
    nglDxRegisterPShader(cdFlagPixel::PS,
                         reinterpret_cast<const unsigned int*>(cdFlagPixel::PShaderTable[0]));
    cdFlagPixel::Shader = cdFlagPixel::PS[0];
}

// ea: 0x007CB900
void cdFlagPixel::RegisterPShader()
{
    cdFlagPixel::RegisterShader();
}

// ea: 0x007CB920
unsigned long* cdFlagPixel::GetPShader()
{
    return cdFlagPixel::PS[0];
}

namespace AeAssert {
    enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10 };
    extern ECoderId gCurrentAuthor;
    extern const char* gCurrentFile;
    extern int   gCurrentLine;
    extern const char* gCurrentExpr;
    bool IsIgnored();
    bool Assert(const char* msg, ...);
}

// ============================================================================
// Globals
// ============================================================================
math::Vector4 FlagSinTable[64];       // ?FlagSinTable@@3PAVVector4@math@@A
unsigned int gShaderSwitchingFlags;   // @0x10DDB14

// ============================================================================
// cdFlagShaderMat::cdFlagShaderMat — bind texture + shader.
// ea: 0x7C9FF0
// ============================================================================
cdFlagShaderMat::cdFlagShaderMat(nglTexture* iTexture) {
    this->mTexture = iTexture;
    cdFlagShader* v3 = gCDFlagShader;
    if (gCDFlagShader != NULL) {
        this->Shader = v3;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdFlagShader.cpp";
    AeAssert::gCurrentLine = 21;
    AeAssert::gCurrentExpr = "gCDFlagShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDFlagShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly"))
        __debugbreak();
    this->Shader = gCDFlagShader;
}

// ============================================================================
// InitCDFlagShader — allocate the shader, link the init list, build the
// flag sine table (64 entries of 2 components each).
// ea: 0x7CA080
// ============================================================================
void InitCDFlagShader() {
    cdFlagShader* v0 = (cdFlagShader*)mem_heap_malloc(0x10);
    if (v0 != NULL) {
        ::new (v0) cdFlagShader;
    } else {
        v0 = NULL;
    }
    gCDFlagShader = v0;

    math::Vector4 v3;
    v3.v.m128_f32[2] = 0.0f;
    v3.v.m128_f32[3] = 0.0f;
    int v1 = 0;
    math::Vector4* result = FlagSinTable;
    do {
        ++v1;
        ++result;
        v3.v.m128_f32[0] = (float)(sin(4.0 * ((v1 - 1) * 0.098174773)) * 0.2 +
                                    sin((v1 - 1) * 0.098174773));
        v3.v.m128_f32[1] = (float)(sin(4.0 * (v1 * 0.098174773)) * 0.2 +
                                    sin(v1 * 0.098174773));
        result[-1] = v3;
    } while (result < &FlagSinTable[64]);
}

// ============================================================================
// ToggleCDFlagShader — toggle the flag-shader enable bit (bit 3).
// ea: 0x7CA160
// ============================================================================
void ToggleCDFlagShader() {
    unsigned char byte = *((unsigned char*)&gShaderSwitchingFlags);
    byte = (unsigned char)(((byte ^ (8 * ~(byte >> 3))) & 8) ^ byte);
    *((unsigned char*)&gShaderSwitchingFlags) = byte;
}

// ea: 0x007CA180
void CalculateFlagMatrix(math::Mat43& Matrix, nglMeshSection* Section,
                         float Intensity) {
    math::Dir3 Up;
    Up.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    const __m128 sphere = Section->Sphere.v;
    const float radius = sphere.m128_f32[3];
    const __m128 cross = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(Up.v, Up.v, 9),
                   _mm_shuffle_ps(sphere, sphere, 18)),
        _mm_mul_ps(_mm_shuffle_ps(Up.v, Up.v, 18),
                   _mm_shuffle_ps(sphere, sphere, 9)));
    const __m128 crossSquared = _mm_mul_ps(cross, cross);
    const float halfWidth = sqrtf(radius * radius -
        (crossSquared.m128_f32[0] +
         _mm_shuffle_ps(crossSquared, crossSquared, 85).m128_f32[0] +
         _mm_shuffle_ps(crossSquared, crossSquared, 170).m128_f32[0]));
    const __m128 upDot = _mm_mul_ps(sphere, Up.v);
    const float side = upDot.m128_f32[0] +
        _mm_shuffle_ps(upDot, upDot, 85).m128_f32[0] +
        _mm_shuffle_ps(upDot, upDot, 170).m128_f32[0] - halfWidth;
    const __m128 origin = _mm_mul_ps(Up.v, _mm_set1_ps(side));
    const __m128 center = _mm_sub_ps(
        _mm_sub_ps(sphere, origin), _mm_mul_ps(Up.v, _mm_set1_ps(halfWidth)));
    const __m128 width = _mm_mul_ps(center, _mm_set1_ps(2.0f));
    const __m128 height = _mm_mul_ps(Up.v, _mm_set1_ps(halfWidth + halfWidth));
    const __m128 tangent = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(height, height, 9),
                   _mm_shuffle_ps(width, width, 18)),
        _mm_mul_ps(_mm_shuffle_ps(height, height, 18),
                   _mm_shuffle_ps(width, width, 9)));
    const __m128 tangentSquared = _mm_mul_ps(tangent, tangent);
    const float tangentLength = sqrtf(tangentSquared.m128_f32[0] +
        _mm_shuffle_ps(tangentSquared, tangentSquared, 85).m128_f32[0] +
        _mm_shuffle_ps(tangentSquared, tangentSquared, 170).m128_f32[0]);
    Matrix.x.v = width;
    Matrix.y.v = height;
    Matrix.z.v = _mm_mul_ps(_mm_div_ps(tangent, _mm_set1_ps(tangentLength)),
                            _mm_set1_ps(Intensity));
    Matrix.w.v = origin;
}

// ea: 0x007CB9F0
cdFlagShaderNode::cdFlagShaderNode(nglMeshNode* iMeshNode,
                                   nglMeshSection* iSection,
                                   cdFlagShaderMat* iMaterial) {
    this->MeshNode = iMeshNode;
    this->Section = iSection;
    this->mMaterial = iMaterial;
}

// ea: 0x007CBA50
cdFlagShaderNode::~cdFlagShaderNode() = default;

// ea: 0x007CB870
tlFixedString cdFlagShader::GetName() { return tlFixedString("cdFlag"); }

// ============================================================================
// cdFlagShader::Register — register the flag vertex/pixel shaders.
// ea: 0x7CA3C0
// ============================================================================
void cdFlagShader::Register() {
    nglShader::Register();
    nglDxRegisterVShader(cdFlagVertex::VS,
                         reinterpret_cast<const unsigned int*>(cdFlagVertex::VShaderTable[0]));
    cdFlagVertex::Shader = cdFlagVertex::VS[0];
    nglDxRegisterPShader(cdFlagPixel::PS,
                         reinterpret_cast<const unsigned int*>(cdFlagPixel::PShaderTable[0]));
    cdFlagPixel::Shader = cdFlagPixel::PS[0];
}

// ============================================================================
// cdFlagShader::AddNode — add a flag node to the opaque list.
// ea: 0x7CA400
// ============================================================================
void cdFlagShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                           nglMaterial* iMat) {
    if ((gShaderSwitchingFlags & 8) == 0) {
        cdFlagShaderNode* node = (cdFlagShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdFlagShaderNode
            node->mMaterial = (cdFlagShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDFlagShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

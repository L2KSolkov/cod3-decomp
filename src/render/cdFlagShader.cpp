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

#include <intrin.h>

// Shader global pointer definitions
cdFlagShader* gCDFlagShader = nullptr;  // ?gCDFlagShader@@3PAVcdFlagShader@@A
extern unsigned int gShaderSwitchingFlags;

// ea: 0x007CB840
cdFlagShader::cdFlagShader() {
    this->next = tlInitList::head;
    tlInitList::head = this;
    this->Disabled = false;
    *((unsigned char*)&gShaderSwitchingFlags) &= 0xF7;
}

// ea: 0x007CB8B0
void cdFlagVertex::RegisterVShader()
{
    nglDxRegisterVShader(cdFlagVertex::VS,
                         reinterpret_cast<const unsigned int*>(cdFlagVertex::VShaderTable[0]));
    cdFlagVertex::Shader = cdFlagVertex::VS[0];
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

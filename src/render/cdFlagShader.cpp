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

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdFlagVertex {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
    unsigned long Shader = 0;
}
namespace cdFlagPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
    unsigned long* Shader = nullptr;
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
        v0->next = tlInitList::head;
        tlInitList::head = v0;
        v0->Disabled = false;
        // vftable = cdFlagShader
        *((unsigned char*)&gShaderSwitchingFlags) &= 0xF7;
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

// ============================================================================
// cdFlagShader::Register — register the flag vertex/pixel shaders.
// ea: 0x7CA3C0
// ============================================================================
tlFixedString cdFlagShader::GetName() { return tlFixedString("cdFlag"); }

void cdFlagShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdFlagVertex::VS, cdFlagVertex::VShaderTable, 0);
    cdFlagVertex::Shader = cdFlagVertex::VS != nullptr ? cdFlagVertex::VS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdFlagPixel::PS, cdFlagPixel::PShaderTable, 0);
    cdFlagPixel::Shader = cdFlagPixel::PS != nullptr ? cdFlagPixel::PS[0] : 0;
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

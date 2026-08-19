// ============================================================================
// cdWheelMarkShader.cpp — wheel mark shader (7 non-inline funcs).
// Source: source/cdWheelMarkShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWheelMarkShader.o):
//   cdWheelMarkShaderMat::ctor @0x7C92B0
//   InitCDWheelMarkShader  @0x7C9330
//   ToggleCDWheelMarkShader @0x7C9380 (empty)
//   InitCDWheelMarkVertexDefBuilder @0x7C9390
//   cdWheelMarkShader::Register @0x7C93D0
//   cdWheelMarkShader::AddNode @0x7C9410
// ============================================================================
#include "cdWheelMarkShader.h"

const _D3DVERTEXSHADERINPUT cdWheelMarkVertexElements[3] = {
    {0, 0, 50, 0, 0},
    {0, 12, 68, 0, 0},
    {0, 0, 2, 0, 0},
};  // ?cdWheelMarkVertexElements (render_xboxr @ 0xE3C7A0)
gpuVertexFormat cdWheelMarkVertexFormat;  // ?cdWheelMarkVertexFormat@@3UgpuVertexFormat@@A (render_xboxr)

#include <intrin.h>

// Shader global pointer definitions
cdWheelMarkShader* gCDWheelMarkShader = nullptr;  // ?gCDWheelMarkShader@@3PAVcdWheelMarkShader@@A
unsigned int cdWheelMarkShaderDataID;  // ?cdWheelMarkShaderDataID@@3IA @ 0x14CD57C

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdWheelMarkShaderVertex {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
    unsigned long Shader = 0;
}
namespace cdWheelMarkShaderPixel {
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
// cdWheelMarkShaderMat::cdWheelMarkShaderMat — default material, bind shader.
// ea: 0x7C92B0
// ============================================================================
cdWheelMarkShaderMat::cdWheelMarkShaderMat() {
    this->mTexture = NULL;
    cdWheelMarkShader* v2 = gCDWheelMarkShader;
    if (gCDWheelMarkShader != NULL) {
        this->Shader = v2;
        return;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "cdWheelMarkShader.cpp";
    AeAssert::gCurrentLine = 20;
    AeAssert::gCurrentExpr = "gCDWheelMarkShader";
    if (AeAssert::IsIgnored()) {
        this->Shader = gCDWheelMarkShader;
        return;
    }
    if (AeAssert::Assert("Material is being created before the shader; the pointers won't be set up properly")) {
        __debugbreak();
        this->Shader = gCDWheelMarkShader;
        return;
    }
    this->Shader = gCDWheelMarkShader;
}

// ============================================================================
// InitCDWheelMarkShader — allocate the shader and link into the init list.
// ea: 0x7C9330
// ============================================================================
void InitCDWheelMarkShader() {
    cdWheelMarkShader* result = (cdWheelMarkShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdWheelMarkShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWheelMarkShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~2;
        gCDWheelMarkShader = result;
    } else {
        gCDWheelMarkShader = NULL;

    }

}

// ============================================================================
// ToggleCDWheelMarkShader — no-op toggle.
// ea: 0x7C9380
// ============================================================================
void ToggleCDWheelMarkShader() {

}

// ============================================================================
// InitCDWheelMarkVertexDefBuilder — build the wheel-mark vertex format.
// ea: 0x7C9390
// ============================================================================
void InitCDWheelMarkVertexDefBuilder() {
    gpuVertexFormat v2;
    gpuVertexFormat* v0 = gpuCreateVertexFormat(&v2, 0x10, cdWheelMarkVertexElements);
    cdWheelMarkVertexFormat.VertexSize = v0->VertexSize;
    cdWheelMarkVertexFormat.Elements = v0->Elements;
    cdWheelMarkVertexFormat.VertexDeclaration = v0->VertexDeclaration;
}

// ============================================================================
// cdWheelMarkShader::Register — register the wheel-mark vertex/pixel shaders.
// ea: 0x7C93D0
// ============================================================================
tlFixedString cdWheelMarkShader::GetName() { return tlFixedString("cdWheelMark"); }

void cdWheelMarkShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdWheelMarkShaderVertex::VS, cdWheelMarkShaderVertex::VShaderTable, 0);
    cdWheelMarkShaderVertex::Shader = cdWheelMarkShaderVertex::VS != nullptr ? cdWheelMarkShaderVertex::VS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdWheelMarkShaderPixel::PS, cdWheelMarkShaderPixel::PShaderTable, 0);
    cdWheelMarkShaderPixel::Shader = cdWheelMarkShaderPixel::PS != nullptr ? cdWheelMarkShaderPixel::PS[0] : 0;
}

// ============================================================================
// cdWheelMarkShader::AddNode — add a wheel-mark node to the opaque list.
// ea: 0x7C9410
// ============================================================================
void cdWheelMarkShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                nglMaterial* iMat) {
    cdWheelMarkShaderNode* node = (cdWheelMarkShaderNode*)nglListAlloc(0x1C, 0x10);
    if (node != NULL) {
        node->MeshNode = iMeshNode;
        node->Section = iSection;
        // vftable = cdWheelMarkShaderNode
        node->mMaterial = (cdWheelMarkShaderMat*)iMat;
    } else {
        node = NULL;
    }
    node->SortHash = gCDWheelMarkShader->ID | 0x80000000;
    node->Next = nglBuildScene->OpaqueRenderList;
    nglBuildScene->OpaqueRenderList = node;
    ++nglBuildScene->OpaqueListCount;
}

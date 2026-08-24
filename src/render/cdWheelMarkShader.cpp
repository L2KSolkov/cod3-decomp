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
#include "ngl/ngl_dx_quad.h"

#include <cstdint>

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

extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_40344;
extern unsigned int dword_40348;
extern unsigned int dword_4034C;
extern unsigned int dword_40350;
extern unsigned int dword_4035C;
extern unsigned int dword_BC2CF8;
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2D04;
extern unsigned int dword_BC2D08;
extern unsigned int dword_BC2D0C;
extern unsigned int dword_BC2D38;
extern unsigned int dword_BC2D3C;
extern unsigned int dword_BC2CFC;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;

namespace {

struct WheelMarkShaderParams {
    math::Mat44 mLocalToScreen;
    math::Vector4 cFadeScale;
    math::Vector4 cUVScale;
};
static_assert(sizeof(WheelMarkShaderParams) == 0x60,
              "IDA wheel-mark shader parameter layout");

static const unsigned int kPrimitiveVertexCount[11][2] = {
    {0, 0}, {1, 0}, {2, 0}, {1, 1}, {1, 1}, {3, 0},
    {1, 2}, {1, 2}, {4, 0}, {2, 2}, {0, 0}
};
static const unsigned int kPrimitiveVertexBase[26] = {
    0, 1, 0, 2, 0, 1, 1, 1, 1, 3, 0, 1, 2,
    1, 2, 4, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0
};

static unsigned gpuGetPrimCountExact(unsigned primitive, unsigned count) {
    switch (primitive) {
    case D3DPT_POINTLIST: return count;
    case D3DPT_LINELIST: return count >> 1;
    case D3DPT_LINESTRIP: return count - 1;
    case D3DPT_TRIANGLELIST: return count / 3;
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN: return count - 2;
    case D3DPT_QUADLIST: return count >> 2;
    default: return 0;
    }
}

static void gpuSetAlphaTestExact(unsigned cmp, unsigned reference) {
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 1);
        dword_BC2D00 = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAFUNC, cmp) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4033C, cmp);
        dword_BC2CF8 = cmp;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF, reference) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40340, reference);
        dword_BC2D04 = reference;
    }
}

static void gpuSetAlphaExact(unsigned source, unsigned destination,
                             unsigned constant, unsigned operation) {
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 1);
        dword_BC2CFC = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SRCBLEND, source) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40344, source);
        dword_BC2D08 = source;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_DESTBLEND, destination) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40348, destination);
        dword_BC2D0C = destination;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_BLENDCOLOR, constant) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4034C, constant);
        dword_BC2D3C = constant;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_BLENDOP, operation) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40350, operation);
        dword_BC2D38 = operation;
    }
}

} // namespace

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
        ::new (node) cdWheelMarkShaderNode;
        node->mMaterial = (cdWheelMarkShaderMat*)iMat;
    } else {
        node = NULL;
    }
    node->SortHash = gCDWheelMarkShader->ID | 0x80000000;
    node->Next = nglBuildScene->OpaqueRenderList;
    nglBuildScene->OpaqueRenderList = node;
    ++nglBuildScene->OpaqueListCount;
}

// ============================================================================
// cdWheelMarkShaderNode::Render — release range-split wheel-mark draw.
// ea: 0x7C9480
// ============================================================================
void cdWheelMarkShaderNode::Render() {
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, 0);
        dword_BC2D10 = 0;
    }
    gpuSetAlphaTestExact(0x204u, 0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);
    gpuSetAlphaExact(0, 0x300u, 0, 0x8006u);

    nglDxSetTexture(0, this->mMaterial->mTexture, 1u, 3u);
    if (nglDxTexCache.Prev[0].WrapU != 1) {
        nglDxTexCache.Prev[0].WrapU = 1;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSU, 1) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSU] = 1;
        }
    }
    if (nglDxTexCache.Prev[0].WrapV != 1) {
        nglDxTexCache.Prev[0].WrapV = 1;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSV, 1) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSV] = 1;
        }
    }

    if (cdWheelMarkShaderVertex::VS != nullptr &&
        cdWheelMarkShaderVertex::VS[0] != gpuHashVertexShader) {
        gpuHashVertexShader = static_cast<unsigned int>(cdWheelMarkShaderVertex::VS[0]);
        D3DDevice_LoadVertexShaderProgram(
            reinterpret_cast<const unsigned int*>(
                static_cast<uintptr_t>(cdWheelMarkShaderVertex::VS[0])), 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    if (cdWheelMarkShaderPixel::PS != nullptr &&
        cdWheelMarkShaderPixel::PS[0] != nullptr &&
        static_cast<unsigned int>(reinterpret_cast<uintptr_t>(cdWheelMarkShaderPixel::PS[0])) !=
            gpuHashPixelShader) {
        gpuHashPixelShader = static_cast<unsigned int>(
            reinterpret_cast<uintptr_t>(cdWheelMarkShaderPixel::PS[0]));
        D3DDevice_SetPixelShaderProgram(
            reinterpret_cast<const _D3DPixelShaderDef*>(cdWheelMarkShaderPixel::PS[0]));
    }

    nglMeshSection* section = this->Section;
    unsigned int vertexCount = static_cast<unsigned int>(section->NVertices);
    unsigned int firstVertex = 0;
    unsigned int secondCount = 0;
    unsigned int* sectionVertexCount = reinterpret_cast<unsigned int*>(&section->NVertices);
    nglShaderParamSet* params = &this->MeshNode->ShaderParams;
    if (params->Array != nullptr &&
        ((1u << (cdWheelMarkShaderDataID & 0x1Fu)) &
         params->Array[cdWheelMarkShaderDataID >> 5]) != 0) {
        auto* range = reinterpret_cast<cdWheelMarkShaderStruct*>(
            params->Array + cdWheelMarkShaderDataID + 2u);
        firstVertex = range->FirstVertex;
        vertexCount = range->VertexCount;
        const unsigned int tail = vertexCount - *sectionVertexCount;
        const unsigned int split = firstVertex + tail;
        if (split > 0) {
            secondCount = split;
            vertexCount -= split;
        }
    }

    WheelMarkShaderParams shaderParams{};
    const nglTexture* texture = this->mMaterial->mTexture;
    shaderParams.cUVScale.v = _mm_setr_ps(
        (static_cast<float>(texture->Height) * 255.0f) /
            static_cast<float>(texture->Width),
        20.0f, 0.0f, 0.0f);
    shaderParams.mLocalToScreen = this->MeshNode->LocalToScreen;

    auto drawRange = [&](unsigned int rangeFirst, unsigned int rangeCount,
                         float fade) {
        if (rangeCount <= 2)
            return;
        shaderParams.cFadeScale.v = _mm_setr_ps(1.0f, fade, 0.0f, 0.0f);
        D3DDevice_SetVertexShaderConstantNotInlineFast(6, &shaderParams, 0x18u);
        gpuSetVertexBuffer(section->VertexBuffer, section->VertexFormat,
                           rangeFirst * section->VertexFormat->VertexSize, 0);
        const unsigned int primitive = static_cast<unsigned int>(section->PrimitiveType);
        const unsigned int primCount = gpuGetPrimCountExact(primitive, rangeCount);
        const unsigned int vertexDrawCount =
            kPrimitiveVertexBase[primitive * 2u] +
            kPrimitiveVertexCount[primitive][0] * primCount;
        D3DDevice_DrawVertices(static_cast<_D3DPRIMITIVETYPE>(primitive), 0,
                               vertexDrawCount);
    };

    if (*sectionVertexCount != 0) {
        if (vertexCount > 2) {
            const float firstFade =
                (static_cast<float>(firstVertex + vertexCount + secondCount) /
                 static_cast<float>(*sectionVertexCount)) - 1.0f;
            drawRange(firstVertex, vertexCount, firstFade);
        }
        if (secondCount > 2) {
            const float secondFade = static_cast<float>(secondCount) /
                                     static_cast<float>(*sectionVertexCount) - 1.0f;
            drawRange(0, secondCount, secondFade);
        }
    }

    nglDxState.PrevBM = static_cast<unsigned int>(-1);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0x900u) == 0)
        D3DDevice_SetRenderState_CullMode(0x900u);
    const unsigned int zWrite = nglBuildScene->ZWriteEnable;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, zWrite) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, zWrite);
        dword_BC2D10 = zWrite;
    }
}

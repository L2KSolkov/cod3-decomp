// ============================================================================
// cdWaterShader.cpp — water shader (6 non-inline funcs).
// Source: source/cdWaterShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWaterShader.o):
//   cdWaterShaderMat::ctor @0x7D95E0
//   InitCDWaterShader  @0x7D9670
//   ToggleCDWaterShader @0x7D96C0
//   cdWaterShader::Register @0x7D96E0
//   cdWaterShader::AddNode @0x7D9720
// ============================================================================
#include "cdWaterShader.h"

extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);

#include <intrin.h>

// Shader global pointer definitions
cdWaterShader* gCDWaterShader = nullptr;  // ?gCDWaterShader@@3PAVcdWaterShader@@A

// Shader static data definitions are restored in cdWaterShaderData.cpp.
// ea: 0x007D9C30
void cdWaterRender::RegisterVShader()
{
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(cdWaterRender::VS),
                         reinterpret_cast<const unsigned int*>(cdWaterRender::VShaderTable[0]));
}
// ============================================================================
// InitCDWaterShader — allocate the shader and link into the init list.
// ea: 0x7D9670
// ============================================================================
void InitCDWaterShader() {
    cdWaterShader* result = (cdWaterShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdWaterShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWaterShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~0x80;
        gCDWaterShader = result;
    } else {
        gCDWaterShader = NULL;
    }
}

// ============================================================================
// ToggleCDWaterShader — toggle water-shader enable (high bit).
// ea: 0x7D96C0
// ============================================================================
void ToggleCDWaterShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[0];
    ShaderCommon::ShaderSwitching.__s0[0] =
        (unsigned char)(~byte ^ ((byte ^ ~byte) & 0x7F));
}

tlFixedString cdWaterShader::GetName() { return tlFixedString("cdWater"); }

// ============================================================================
// cdWaterShader::Register — register the water vertex/pixel shaders.
// ea: 0x7D96E0
// ============================================================================
void cdWaterShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdWaterRender::VS, cdWaterRender::VShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdWaterPixel::PS, cdWaterPixel::PShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdWaterFullbrightPixel::PS, cdWaterFullbrightPixel::PShaderTable, 0);
}

// ============================================================================
// cdWaterShader::AddNode — add a water node to the opaque list.
// ea: 0x7D9720
// ============================================================================
void cdWaterShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                            nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[0] & 0x80) == 0) {
        cdWaterShaderNode* node = (cdWaterShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdWaterShaderNode
            node->mMaterial = (cdWaterShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = -1;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

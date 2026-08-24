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

#include <intrin.h>

// Shader global pointer definitions
cdWaterShader* gCDWaterShader = nullptr;  // ?gCDWaterShader@@3PAVcdWaterShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdWaterRender {
    static const unsigned int VShaderMicrocode[49] = {
        0x000c2078, 0x00000000, 0x0061601a, 0x0800146a, 0xfeb00000,
        0x00000000, 0x00e1201b, 0x08373800, 0x20a01800, 0x00000000,
        0x02a0041a, 0xb4356854, 0xa8b0c854, 0x00000000, 0x06e0c01b,
        0x0836dbff, 0x10a88800, 0x00000000, 0x08e0e01b, 0x0836f802,
        0xd0944800, 0x00000000, 0x00e1001b, 0x08371800, 0x20902800,
        0x00000000, 0x00814055, 0x14016d56, 0xb8000000, 0x00000000,
        0x00618215, 0x18001057, 0x30b0c848, 0x00000000, 0x00414000,
        0x05555800, 0x28000000, 0x00000000, 0x00814000, 0x05ff5802,
        0xb8000000, 0x00000000, 0x006020aa, 0x1c001400, 0x10b0f828,
        0x00000000, 0x0040001a, 0xc4002800, 0x20b0e801,
    };
    static const unsigned int* VShaderTableStorage[1] = { VShaderMicrocode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
    unsigned long Shader = 0;
}
namespace cdWaterPixel {
    static const unsigned int PShaderMicrocode[60] = {
        0xd8301010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c90000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011101, 0x00000021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int* PShaderTableStorage[1] = { PShaderMicrocode };
    static unsigned long* PShaderHandle = nullptr;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
    unsigned long* Shader = nullptr;
}
namespace cdWaterFullbrightPixel {
    static const unsigned int PShaderMicrocode[60] = {
        0xd8301010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8200000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011101, 0x00000001,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int* PShaderTableStorage[1] = { PShaderMicrocode };
    static unsigned long* PShaderHandle = nullptr;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
    unsigned long* Shader = nullptr;
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

// ============================================================================
// cdWaterShader::Register — register the water vertex/pixel shaders.
// ea: 0x7D96E0
// ============================================================================
tlFixedString cdWaterShader::GetName() { return tlFixedString("cdWater"); }

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

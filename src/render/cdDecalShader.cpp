// ============================================================================
// cdDecalShader.cpp — decal shader (6 non-inline funcs).
// Source: source/cdDecalShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdDecalShader.o):
//   cdDecalShaderMat::ctor @0x7D1720
//   InitCDDecalShader  @0x7D17B0
//   ToggleCDDecalShader @0x7D1800
//   cdDecalShader::Register @0x7D1820
//   cdDecalShader::AddNode @0x7D1860
// ============================================================================
#include "cdDecalShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdDecalShader* gCDDecalShader = nullptr;  // ?gCDDecalShader@@3PAVcdDecalShader@@A
cdDecalShader* g_cdDecalShader = nullptr;  // ?g_cdDecalShader@@3PAVcdDecalShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdDecalRender {
    static const unsigned int VShaderMicrocode[49] = {
        0x000c2078, 0x00000000, 0x0062601a, 0x08001468, 0xfeb00000,
        0x00000000, 0x00e1201b, 0x08373800, 0x20a01800, 0x00000000,
        0x02a0021a, 0xb4356854, 0x68b0c84c, 0x00000000, 0x06e0c01b,
        0x0836dbff, 0x10a88800, 0x00000000, 0x08e0e01b, 0x0836f802,
        0xd0a44800, 0x00000000, 0x00e1001b, 0x08371800, 0x20a02800,
        0x00000000, 0x00824055, 0x14016d54, 0xb8000000, 0x00000000,
        0x02000400, 0x0800106c, 0xa0b0f81c, 0x00000000, 0x00424000,
        0x05545800, 0x28000000, 0x00000000, 0x00824000, 0x05fe5800,
        0xb8000000, 0x00000000, 0x006020aa, 0x1c001400, 0x10b0f828,
        0x00000000, 0x0040001a, 0xc4002800, 0x20b0e801,
    };
    static const unsigned int* VShaderTableStorage[1] = { VShaderMicrocode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
    unsigned long Shader = 0;
}
namespace cdDecalPixel {
    static const unsigned int PShaderMicrocode[60] = {
        0xd8d41010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c40000,
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
namespace cdDecalFullbrightPixel {
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
// InitCDDecalShader — allocate the shader and link into the init list.
// ea: 0x7D17B0
// ============================================================================
void InitCDDecalShader() {
    cdDecalShader* result = (cdDecalShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdDecalShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdDecalShader
        ShaderCommon::ShaderSwitching.__s0[2] &= ~0x20;
        gCDDecalShader = result;
    } else {
        gCDDecalShader = NULL;

    }

}

// ============================================================================
// ToggleCDDecalShader — toggle the decal-shader enable bit (bit 5).
// ea: 0x7D1800
// ============================================================================
void ToggleCDDecalShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    byte = (unsigned char)(((byte ^ (32 * ~(byte >> 5))) & 0x20) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[2] = byte;

}

// ============================================================================
// cdDecalShader::Register — register the decal vertex/pixel shaders.
// ea: 0x7D1820
// ============================================================================
tlFixedString cdDecalShader::GetName() { return tlFixedString("cdDecal"); }

void cdDecalShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdDecalRender::VS, cdDecalRender::VShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdDecalPixel::PS, cdDecalPixel::PShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdDecalFullbrightPixel::PS, cdDecalFullbrightPixel::PShaderTable, 0);
}

// ============================================================================
// cdDecalShader::AddNode — add a decal node to the opaque list.
// ea: 0x7D1860
// ============================================================================
void cdDecalShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                            nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[2] & 0x20) == 0) {
        cdDecalShaderNode* node = (cdDecalShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdDecalShaderNode
            node->mMaterial = (cdDecalShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDDecalShader->ID | 0x80000000;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

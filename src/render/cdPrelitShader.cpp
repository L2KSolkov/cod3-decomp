// ============================================================================
// cdPrelitShader.cpp — prelit shader (5 non-inline funcs).
// Source: source/cdPrelitShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdPrelitShader.o):
//   InitCDPrelitShader  @0x7D3590
//   ToggleCDPrelitShader @0x7D35E0
//   cdPrelitShader::Register @0x7D3600
//   cdPrelitShader::AddNode @0x7D3B70
// ============================================================================
#include "cdPrelitShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdPrelitShader* gCDPrelitShader = nullptr;  // ?gCDPrelitShader@@3PAVcdPrelitShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdPrelitRender {
    static const unsigned int VShaderMicrocode[49] = {
        0x000c2078, 0x00000000, 0x0061601a, 0x0800146a, 0xfeb00000,
        0x00000000, 0x00e1201b, 0x08373800, 0x20a01800, 0x00000000,
        0x02a0041a, 0xb4356854, 0xa8b0c84c, 0x00000000, 0x06e0c01b,
        0x0836dbff, 0x10a88800, 0x00000000, 0x08e0e01b, 0x0836f802,
        0xd0944800, 0x00000000, 0x00e1001b, 0x08371800, 0x20902800,
        0x00000000, 0x00814055, 0x14016d56, 0xb8000000, 0x00000000,
        0x02000600, 0x08001054, 0xe0b0c854, 0x00000000, 0x02414200,
        0x0555586c, 0x6800f81c, 0x00000000, 0x0040001a, 0xc4002800,
        0x20b0e800, 0x00000000, 0x00814000, 0x05ff5802, 0xb8000000,
        0x00000000, 0x006020aa, 0x1c001400, 0x10b0f829,
    };
    static const unsigned int* VShaderTableStorage[1] = { VShaderMicrocode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
}
namespace cdPrelitPixel {
    static const unsigned int PShaderMicrocode[60] = {
        0x00000000, 0xd8301010, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x000000c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c40000,
        0xccd90000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011102, 0x00000021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int* PShaderTableStorage[1] = { PShaderMicrocode };
    static unsigned long* PShaderHandle = nullptr;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
}
namespace cdPrelitFullbrightPixel {
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
}
namespace cdPrelitSolidColorPixel {
    static const unsigned int PShaderMicrocode[60] = {
        0xd1301010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc1200000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011101, 0x00000000,
        0x00000000, 0x00000000, 0xfffffff0, 0xffffffff, 0x000001ff,
    };
    static const unsigned int* PShaderTableStorage[1] = { PShaderMicrocode };
    static unsigned long* PShaderHandle = nullptr;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
}
// ============================================================================
// InitCDPrelitShader — allocate the shader and link into the init list.
// ea: 0x7D3590
// ============================================================================
void InitCDPrelitShader() {
    cdPrelitShader* result = (cdPrelitShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdPrelitShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdPrelitShader
        ShaderCommon::ShaderSwitching.__s0[3] &= ~4;
        gCDPrelitShader = result;
    } else {
        gCDPrelitShader = NULL;

    }

}

// ============================================================================
// ToggleCDPrelitShader — toggle the prelit-shader enable bit (bit 2).
// ea: 0x7D35E0
// ============================================================================
void ToggleCDPrelitShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[3];
    byte = (unsigned char)(((byte ^ (4 * ~(byte >> 2))) & 4) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[3] = byte;

}

// ============================================================================
// cdPrelitShader::Register — register the prelit vertex/pixel shaders.
// ea: 0x7D3600
// ============================================================================
tlFixedString cdPrelitShader::GetName() { return tlFixedString("cdPrelit"); }

void cdPrelitShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdPrelitRender::VS, cdPrelitRender::VShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdPrelitPixel::PS, cdPrelitPixel::PShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdPrelitFullbrightPixel::PS, cdPrelitFullbrightPixel::PShaderTable, 0);
    nglDxRegisterPShaderSafe((unsigned int**)cdPrelitSolidColorPixel::PS, cdPrelitSolidColorPixel::PShaderTable, 0);
}

// ============================================================================
// cdPrelitShader::AddNode — add a prelit node to the opaque list.
// ea: 0x7D3B70
// ============================================================================
void cdPrelitShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                             nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[3] & 4) == 0) {
        cdPrelitShaderNode* node = (cdPrelitShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdPrelitShaderNode
            node->mMaterial = (cdPrelitShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDPrelitShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

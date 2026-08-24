// ============================================================================
// cdSkyShader.cpp — sky shader (4 non-inline funcs).
// Source: source/cdSkyShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSkyShader.o):
//   InitCDSkyShader  @0x7E0EA0
//   ToggleCDSkyShader @0x7E0EF0
//   cdSkyShader::Register @0x7E0F10
// ============================================================================
#include "cdSkyShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdSkyShader* gCDSkyShader = nullptr;  // ?gCDSkyShader@@3PAVcdSkyShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdSkyShaderRender {
    static const unsigned int VShaderMicrocode[29] = {
        0x00072078, 0x00000000, 0x00e1201b, 0x08373800, 0x20b01800,
        0x00000000, 0x00e0c01b, 0x0836d800, 0x20b08800, 0x00000000,
        0x06e0e01b, 0x0836fbff, 0x10b84800, 0x00000000, 0x00e1001b,
        0x08371800, 0x20b02800, 0x00000000, 0x00614215, 0x18001056,
        0xb0b0c848, 0x00000000, 0x02000400, 0x0800106c, 0xa0b0f81c,
        0x00000000, 0x0040001a, 0xc4002800, 0x20b0e801,
    };
    static const unsigned int* VShaderTableStorage[1] = { VShaderMicrocode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
    unsigned long Shader = 0;
}
namespace cdSkyShaderPixel {
    static const unsigned int PShaderMicrocode[60] = {
        0xd8d41010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x200c2000, 0x00001c80,
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
// ============================================================================
// InitCDSkyShader — allocate the shader and link into the init list.
// ea: 0x7E0EA0
// ============================================================================
void InitCDSkyShader() {
    cdSkyShader* result = (cdSkyShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdSkyShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdSkyShader
        ShaderCommon::ShaderSwitching.__s0[2] &= ~0x10;
        gCDSkyShader = result;
    } else {
        gCDSkyShader = NULL;
    }
}

// ============================================================================
// ToggleCDSkyShader — toggle sky-shader enable bit (bit 4).
// ea: 0x7E0EF0
// ============================================================================
void ToggleCDSkyShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[2];
    byte = (unsigned char)(((byte ^ (16 * ~(byte >> 4))) & 0x10) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[2] = byte;
}

// ============================================================================
// cdSkyShader::Register — register the sky vertex/pixel shaders.
// ea: 0x7E0F10
// ============================================================================
tlFixedString cdSkyShader::GetName() { return tlFixedString("cdSky"); }

void cdSkyShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdSkyShaderRender::VS, cdSkyShaderRender::VShaderTable, 0);
    cdSkyShaderRender::Shader = cdSkyShaderRender::VS != nullptr ? cdSkyShaderRender::VS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdSkyShaderPixel::PS, cdSkyShaderPixel::PShaderTable, 0);
    cdSkyShaderPixel::Shader = cdSkyShaderPixel::PS != nullptr ? cdSkyShaderPixel::PS[0] : 0;
}

void cdSkyShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                          nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[2] & 0x10) == 0) {
        cdSkyShaderNode* node = (cdSkyShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            node->mMaterial = (cdSkyShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortDist = 1000.0f - (float)((cdSkyShaderMat*)iMat)->mDrawOrder;
        node->Next = nglBuildScene->TransRenderList;
        nglBuildScene->TransRenderList = node;
        ++nglBuildScene->TransListCount;
    }
}

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

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdSkyShaderRender {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
    unsigned long Shader = 0;
}
namespace cdSkyShaderPixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
    unsigned long* Shader = nullptr;
}

// ============================================================================
// InitCDSkyShader — allocate the shader and link into the init list.
// ea: 0x7E0EA0
// ============================================================================
void InitCDSkyShader() {
    cdSkyShader* result = (cdSkyShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
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
void cdSkyShader::Register() {
    nglShader::Register();
    nglDxRegisterVShaderSafe((unsigned int*)cdSkyShaderRender::VS, cdSkyShaderRender::VShaderTable, 0);
    cdSkyShaderRender::Shader = cdSkyShaderRender::VS != nullptr ? cdSkyShaderRender::VS[0] : 0;
    nglDxRegisterPShaderSafe((unsigned int**)cdSkyShaderPixel::PS, cdSkyShaderPixel::PShaderTable, 0);
    cdSkyShaderPixel::Shader = cdSkyShaderPixel::PS != nullptr ? cdSkyShaderPixel::PS[0] : 0;
}

// ============================================================================
// cdSkyShaderData.cpp — exact release shader microcode and storage.
// Source: render_xboxr:cdSkyShader.o
// ============================================================================
#include "cdSkyShader.h"

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



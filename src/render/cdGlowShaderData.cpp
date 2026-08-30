// cdGlowShaderData.cpp — release glow shader microcode and registration storage.
// Source: render_xboxr:cdGlowShaderVertex.o
#include "cdGlowShader.h"

namespace cdGlowRender4 {
    static const unsigned int Microcode[25] = {
        0x00062078, 0x00000000, 0x062000ff, 0x080013fc, 0x20b81800,
        0x00000000, 0x02000200, 0x0800106c, 0x60b0f84c, 0x00000000,
        0x02000400, 0x0800106c, 0xa0b0f854, 0x00000000, 0x02000600,
        0x0800106c, 0xe0b0f85c, 0x00000000, 0x02000800, 0x0800106d,
        0x20b0f864, 0x00000000, 0x0040001a, 0x08002800, 0x20b0e801,
    };
    static const unsigned int* VShaderTableStorage[1] = { Microcode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
    unsigned long Shader = 0;
}

namespace cdGlowRender1 {
    static const unsigned int Microcode[13] = {
        0x00032078, 0x00000000, 0x062000ff, 0x080013fc, 0x20b81800,
        0x00000000, 0x02000400, 0x0800106c, 0xa0b0f84c, 0x00000000,
        0x0040001a, 0x08002800, 0x20b0e801,
    };
    static const unsigned int* VShaderTableStorage[1] = { Microcode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
    unsigned long Shader = 0;
}

namespace cdGlowShrink {
    static const unsigned int Microcode[60] = {
        0xd830d930, 0xda30db30, 0xdcdcdddd, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x200c2000, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00030c00, 0x00030d00, 0x00030c00, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc820c920,
        0xca20cb20, 0xccdccddd, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00030c00, 0x00030d00, 0x00030c00, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011103, 0x00008421,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int* PShaderTableStorage[1] = { Microcode };
    static unsigned long* PShaderHandle = 0;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
    unsigned long* Shader = 0;
}

namespace cdGlowBlur {
    static const unsigned int Microcode[60] = {
        0xd1d8d1d9, 0xd1dad1db, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x110c0d00, 0x00002080,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000c00, 0x00000d00, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xd1c8d1c9,
        0xd1cad1cb, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x80808080, 0x00000000,
        0x00000c00, 0x00000d00, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011102, 0x00008421,
        0x00000000, 0x00000000, 0xffffff11, 0xffffffff, 0x000001f0,
    };
    static const unsigned int* PShaderTableStorage[1] = { Microcode };
    static unsigned long* PShaderHandle = 0;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
    unsigned long* Shader = 0;
}

namespace cdGlowApply {
    static const unsigned int Microcode[60] = {
        0x00e3d3c8, 0x00000000, 0xd1d81010, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x200c2000, 0x00001c80, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x000100c0, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0xd1c80000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x000100c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00011101, 0x00000001, 0x00000000, 0x00000000, 0xfffffff1,
    };
    static const unsigned int* PShaderTableStorage[1] = { Microcode };
    static unsigned long* PShaderHandle = 0;
    unsigned long** PS = &PShaderHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
    unsigned long* Shader = 0;
}

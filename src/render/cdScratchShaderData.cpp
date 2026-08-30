// cdScratchShaderData.cpp — exact release shader microcode and storage.
// Source: render_xboxr:cdScratchShader.o
#include "cdScratchShader.h"

namespace cdScratchShaderVertex {
    static const unsigned int Microcode[49] = {
+        0x000c2078, 0x00000000, 0x0061601a, 0x0800146a, 0xfeb00000,
        0x00000000, 0x00e1201b, 0x08373800, 0x20a01800, 0x00000000,
        0x02a0021a, 0xb4356854, 0x68b0c84c, 0x00000000, 0x06e0c01b,
        0x0836dbff, 0x10a88800, 0x00000000, 0x08e0e01b, 0x0836f802,
        0xd0944800, 0x00000000, 0x00e1001b, 0x08371800, 0x20902800,
        0x00000000, 0x00814055, 0x14016d56, 0xb8000000, 0x00000000,
        0x02000400, 0x0800106c, 0xa0b0f81c, 0x00000000, 0x00414000,
        0x05555800, 0x28000000, 0x00000000, 0x00814000, 0x05ff5802,
        0xb8000000, 0x00000000, 0x006020aa, 0x1c001400, 0x10b0f828,
        0x00000000, 0x0040001a, 0xc4002800, 0x20b0e801,
    };
    static const unsigned int* VShaderTableStorage[1] = { Microcode };
    static unsigned long ScratchVertexHandle = 0;
    unsigned long* VS = &ScratchVertexHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
}

namespace cdScratchShaderPixel {
    static const unsigned int Microcode[60] = {
+        0xd8d41010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
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
    static const unsigned int* PShaderTableStorage[1] = { Microcode };
    static unsigned long* ScratchPixelHandle = 0;
    unsigned long** PS = &ScratchPixelHandle;
    unsigned int const** PShaderTable = PShaderTableStorage;
}

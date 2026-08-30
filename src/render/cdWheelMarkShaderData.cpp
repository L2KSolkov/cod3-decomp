// cdWheelMarkShaderData.cpp — exact release shader microcode and storage.
// Source: render_xboxr:cdWheelMarkShader.o
#include "cdWheelMarkShader.h"

namespace cdWheelMarkShaderVertex {
    static const unsigned int Microcode[41] = {
+        0x000a2078, 0x00000000, 0x006142aa, 0x18001556, 0xb8000000,
        0x00000000, 0x00e1201b, 0x08373800, 0x20b01800, 0x00000000,
        0x00400200, 0x05fe3000, 0x28000000, 0x00000000, 0x06e0c01b,
        0x0836dbff, 0x10b88800, 0x00000000, 0x00e0e01b, 0x0836f800,
        0x20a04800, 0x00000000, 0x00e1001b, 0x08371800, 0x20a02800,
        0x00000000, 0x00416215, 0x182b7800, 0x20b0c848, 0x00000000,
        0x02000000, 0x08001000, 0x10b0f854, 0x00000000, 0x006020aa,
        0x1c001400, 0x10b0f858, 0x00000000, 0x0040001a, 0xc4002800,
        0x20b0e801,
    };
    static const unsigned int* VShaderTableStorage[1] = { Microcode };
    static unsigned long WheelVertexHandle = 0;
    unsigned long* VS = &WheelVertexHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
    unsigned long Shader = 0;
}

namespace cdWheelMarkShaderPixel {
    static const unsigned int Microcode[60] = {
+        0xd8d91010, 0xdc30da30, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x200c2000, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000c00, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c90000,
        0xcc20ca20, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000c00, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011102, 0x00001081,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int* PShaderTableStorage[1] = { Microcode };
    static unsigned long* WheelPixelHandle = 0;
    unsigned long** PS = &WheelPixelHandle;
    unsigned long* Shader = 0;
    unsigned int const** PShaderTable = PShaderTableStorage;
}

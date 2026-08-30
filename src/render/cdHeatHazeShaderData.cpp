// cdHeatHazeShaderData.cpp — exact release shader microcode and storage.
// Source: render_xboxr:cdHeatHazeShader.o
#include "cdHeatHazeShader.h"

namespace cdHeatHazeRender {
    static const unsigned int Microcode[33] = {
        0x00082078, 0x00000000, 0x01a00400, 0x28001000, 0x20000000,
        0x00000000, 0x0242c015, 0x082adaac, 0x2c003804, 0x00000000,
        0x0042c201, 0x1802d800, 0x23000000, 0x00000000, 0x006c0015,
        0x04001054, 0x30b0c802, 0x00000000, 0x0062c0bf, 0x040012fd,
        0xbc000000, 0x00000000, 0x002020aa, 0x1c001000, 0x22000000,
        0x00000000, 0x02000000, 0x08001068, 0x10b0f84c, 0x00000000,
        0x00400015, 0xc57e1800, 0x20b0c801,
    };
    static const unsigned int* VShaderTableStorage[1] = { Microcode };
    static unsigned long VShaderHandle = 0;
    unsigned long* VS = &VShaderHandle;
    unsigned int const** VShaderTable = VShaderTableStorage;
    unsigned long Shader = 0;
}

namespace cdHeatHazePixel {
    static const unsigned int Microcode[60] = {
        0xd8301010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x200c2000, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0xc8200000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000000c0,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00011101, 0x00000001, 0x00000000, 0x00000000, 0xffffffff,
        0xffffffff, 0x000001ff,
    };
    static const unsigned int* PShaderTableStorage[1] = { Microcode };
    static unsigned long* HeatHazePixelHandle = 0;
    unsigned long** PS = &HeatHazePixelHandle;
    unsigned long* Shader = 0;
    unsigned int const** PShaderTable = PShaderTableStorage;
}

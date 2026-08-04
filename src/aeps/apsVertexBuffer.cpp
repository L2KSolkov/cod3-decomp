// ============================================================================
// apsVertexBuffer.cpp — sprite vertex/index buffer pool (4 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsVertexBuffer.cpp
// Verified against IDA (aeps_xboxr:apsVertexBuffer.o):
//   Init            @0x802CD0 (?Init@apsVertexBuffer@@YAXI@Z)
//   GetBufferPtr    @0x802D90 (?GetBufferPtr@apsVertexBuffer@@YAPAUSpriteVertex@1@I@Z)
//   ReleaseAndDraw  @0x802DD0 (?ReleaseAndDraw@apsVertexBuffer@@YAXXZ)
//   Swap            @0x802E50 (?Swap@apsVertexBuffer@@YAPAUSpriteVertex@1@XZ)
// ============================================================================
#include "apsVertexBuffer.h"

namespace apsVertexBuffer {

// ============================================================================
// Data globals owned by apsVertexBuffer.o
// ============================================================================
gpuVertexFormat   apsVertexFormat;
SpriteVertex**    sBufferPointers = 0;
D3DVertexBuffer** sVertexBuffers = 0;
D3DIndexBuffer*   sIndexBuffer = 0;
unsigned int      sCurVertexBuffer = 0;
unsigned int      sCurNumParticles = 0;
unsigned int      sMaxParticles = 0;
unsigned int      sCurIndex = 0;
SpriteVertex*     sCurVertexPointer = 0;
unsigned int      sHighWaterMark = 0;
unsigned int      sBufferOverflow = 0;
unsigned int      sCurParticle = 0;

// File-local static vertex declaration (elem_1 @0xE49800). Initialized data:
//   Input[0] = { 0, 0x00, Format=0x32 }   position
//   Input[1] = { 0, 0x0C, Format=0x40 }   offset 12 (diffuse)
//   Input[2] = { 0, 0x10, Format=0x22 }   offset 16 (uv)
//   Input[3+] = { 0, 0, Format=D3DVSDT_END(2), 0, 0 }
static _D3DVERTEXATTRIBUTEFORMAT elem_1 = {
    {
        { 0, 0, 0x32, 0, 0 },  // Input[0]
        { 0, 12, 0x40, 0, 0 }, // Input[1]
        { 0, 16, 0x22, 0, 0 }, // Input[2]
        { 0, 0, 2, 0, 0 },     // Input[3] — END
        { 0, 0, 2, 0, 0 },     // Input[4] — END
        { 0, 0, 2, 0, 0 },     // Input[5] — END
        { 0, 0, 2, 0, 0 },     // Input[6] — END
        { 0, 0, 2, 0, 0 },     // Input[7] — END
        { 0, 0, 2, 0, 0 },     // Input[8] — END
        { 0, 0, 2, 0, 0 },     // Input[9] — END
        { 0, 0, 2, 0, 0 },     // Input[10] — END
        { 0, 0, 2, 0, 0 },     // Input[11] — END
        { 0, 0, 2, 0, 0 },     // Input[12] — END
        { 0, 0, 2, 0, 0 },     // Input[13] — END
        { 0, 0, 2, 0, 0 },     // Input[14] — END
        { 0, 0, 2, 0, 0 },     // Input[15] — END
    }
};

// ============================================================================
// Init — create the two sprite vertex buffers, index buffer, and fill the
// quad-triangle index table.
// ea: 0x802CD0
// ============================================================================
void Init(unsigned int iMaxParticles) {
    unsigned int v1 = iMaxParticles;

    apsVertexFormat.VertexSize = 24;
    apsVertexFormat.VertexDeclaration = &elem_1;
    sMaxParticles = iMaxParticles;

    unsigned int v3 = 0;
    for (;;) {
        D3DVertexBuffer* VertexBuffer2 = D3DDevice_CreateVertexBuffer2(4 * iMaxParticles * 24);
        sVertexBuffers[v3] = VertexBuffer2;
        sBufferPointers[v3] = (SpriteVertex*)D3DVertexBuffer_Lock2(VertexBuffer2, 0x20);
        ++v3;
        if (v3 >= 2)
            break;
    }

    sCurVertexPointer = sBufferPointers[sCurVertexBuffer];
    sIndexBuffer = D3DDevice_CreateIndexBuffer2(24 * iMaxParticles);

    unsigned short* Data = (unsigned short*)sIndexBuffer->Data;
    if (iMaxParticles != 0) {
        int v6 = 0;
        do {
            *Data = (unsigned short)v6;
            unsigned short* v7 = Data + 1;
            *v7++ = (unsigned short)(v6 + 1);
            *v7++ = (unsigned short)(v6 + 2);
            *v7++ = (unsigned short)(v6 + 2);
            *v7++ = (unsigned short)(v6 + 1);
            *v7 = (unsigned short)(v6 + 3);
            Data = v7 + 1;
            v6 += 4;
            --v1;
        } while (v1 != 0);
    }
}

// ============================================================================
// GetBufferPtr — reserve particle slots; returns the current vertex pointer,
// or NULL on overflow.
// ea: 0x802D90
// ============================================================================
SpriteVertex* GetBufferPtr(unsigned int iNumParticles) {
    SpriteVertex* result = 0;
    if (iNumParticles + sCurParticle > sMaxParticles) {
        sBufferOverflow = 1;
    } else {
        sCurNumParticles = iNumParticles;
        return sCurVertexPointer;
    }
    return result;
}

// ============================================================================
// ReleaseAndDraw — bind the vertex buffer, draw the accumulated sprites, and
// advance the cursor.
// ea: 0x802DD0
// ============================================================================
void ReleaseAndDraw() {
    D3DIndexBuffer* v0 = sIndexBuffer;
    unsigned int v1 = 2 * sCurIndex;
    unsigned int v2 = 6 * sCurNumParticles;

    gpuSetVertexBuffer(sVertexBuffers[sCurVertexBuffer], &apsVertexFormat, 0, 0);
    D3DDevice_DrawIndexedVertices(D3DPT_TRIANGLELIST, v2,
                                  (const unsigned short*)(v1 + v0->Data));

    sCurParticle += sCurNumParticles;
    SpriteVertex* result = &sCurVertexPointer[4 * sCurNumParticles];
    sCurIndex += 6 * sCurNumParticles;
    sCurVertexPointer = result;
}

// ============================================================================
// Swap — switch to the other vertex buffer and reset cursors.
// ea: 0x802E50
// ============================================================================
SpriteVertex* Swap() {
    unsigned int v0 = ++sCurVertexBuffer;
    if (sCurVertexBuffer >= 2) {
        v0 = 0;
        sCurVertexBuffer = 0;
    }
    SpriteVertex* result = sBufferPointers[v0];
    sCurVertexPointer = result;
    sCurIndex = 0;
    sCurParticle = 0;
    return result;
}

} // namespace apsVertexBuffer

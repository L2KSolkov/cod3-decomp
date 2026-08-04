// ============================================================================
// apsVertexBuffer — sprite vertex/index buffer management (4 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsVertexBuffer.cpp
// Verified against IDA (aeps_xboxr:apsVertexBuffer.o):
//   Init            @0x802CD0 (?Init@apsVertexBuffer@@YAXI@Z)
//   GetBufferPtr    @0x802D90 (?GetBufferPtr@apsVertexBuffer@@YAPAUSpriteVertex@1@I@Z)
//   ReleaseAndDraw  @0x802DD0 (?ReleaseAndDraw@apsVertexBuffer@@YAXXZ)
//   Swap            @0x802E50 (?Swap@apsVertexBuffer@@YAPAUSpriteVertex@1@XZ)
// ============================================================================
#ifndef COD3_AEPS_APSVERTEXBUFFER_H
#define COD3_AEPS_APSVERTEXBUFFER_H

#include "ngl/ngl_dx_gpu.h"

// ============================================================================
// apsVertexBuffer — double-buffered sprite vertex pool (namespace-scope API).
// ============================================================================
namespace apsVertexBuffer {

// 24 bytes: x, y, z, diffuse, tu, tv (verified against IDA)
struct SpriteVertex {
    float      x;       // +0x00
    float      y;       // +0x04
    float      z;       // +0x08
    unsigned int diffuse;  // +0x0C
    float      tu;      // +0x10
    float      tv;      // +0x14
};
static_assert(sizeof(SpriteVertex) == 0x18, "SpriteVertex size mismatch");

void  Init(unsigned int iMaxParticles);                        // @0x802CD0
SpriteVertex* GetBufferPtr(unsigned int iNumParticles);        // @0x802D90
void  ReleaseAndDraw();                                        // @0x802DD0
SpriteVertex* Swap();                                          // @0x802E50

// ---- static data (apsVertexBuffer.o) ----
extern gpuVertexFormat    apsVertexFormat;       // @0x14CEE94
extern SpriteVertex**     sBufferPointers;       // @0x14CEEA0
extern D3DVertexBuffer**  sVertexBuffers;        // @0x14CEEA8
extern D3DIndexBuffer*    sIndexBuffer;          // @0x14CEEB0
extern unsigned int       sCurVertexBuffer;      // @0x14CEEB4
extern unsigned int       sCurNumParticles;      // @0x14CEEB8
extern unsigned int       sMaxParticles;         // @0x14CEEBC
extern unsigned int       sCurIndex;             // @0x14CEEC0
extern SpriteVertex*      sCurVertexPointer;     // @0x14CEEC4
extern unsigned int       sHighWaterMark;        // @0x14CEEC8
extern unsigned int       sBufferOverflow;       // @0x14CEECC
extern unsigned int       sCurParticle;          // @0x14CEED0

} // namespace apsVertexBuffer

#endif // COD3_AEPS_APSVERTEXBUFFER_H

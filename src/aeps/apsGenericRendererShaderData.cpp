// apsGenericRendererShaderData.cpp — exact shared release APS shader microcode.
// IDA confirmed identical 57-dword vertex and 60-dword pixel payloads across
// the remaining APS renderer vertex objects.
#include "apsShrimpRenderer.h"
#include "apsUVARenderer.h"
#include "apsUVARectangleRenderer.h"
#include "apsColorUVARectangleRenderer.h"
#include "apsBillboardRenderer.h"
#include "apsSimpleMeshRenderer.h"

namespace apsGenericShaderData {
const unsigned int VertexMicrocode[57] = {
        0x000e2078, 0x00000000, 0x00e1201b, 0x08373800, 0x21001800,
        0x00000000, 0x00e0c01b, 0x0836d800, 0x28000000, 0x00000000,
        0x008260ff, 0x04007954, 0xf8b00000, 0x00000000, 0x008280ff,
        0x04009955, 0x34b00000, 0x00000000, 0x06e0e01b, 0x0836fbff,
        0x14080000, 0x00000000, 0x00e1001b, 0x08371800, 0x22000000,
        0x00000000, 0x01402000, 0xb4003800, 0x28b00000, 0x00000000,
        0x03402055, 0xb400386c, 0x14b0e804, 0x00000000, 0x01202000,
        0xb5543800, 0x28200000, 0x00000000, 0x0042c21b, 0x1836d800,
        0x2130e818, 0x00000000, 0x03202055, 0xb5543800, 0x98b0f82c,
        0x00000000, 0x0042e41b, 0x2836f800, 0x20a0f848, 0x00000000,
        0x004000ff, 0x34016800, 0x20b01818, 0x00000000, 0x0040001a,
        0xc4002800, 0x20b0e801,
};
const unsigned int PixelMicrocode[60] = {
        0xd8d41010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
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
const unsigned int* VertexTable[1] = { VertexMicrocode };
const unsigned int* PixelTable[1] = { PixelMicrocode };
}

namespace apsShrimpRender { static unsigned int Handle=0; unsigned int* VS=&Handle; const unsigned int** VShaderTable=apsGenericShaderData::VertexTable; }
namespace apsShrimpRenderPixel { static unsigned int* Handle=0; unsigned int** PS=&Handle; const unsigned int** PShaderTable=apsGenericShaderData::PixelTable; }
namespace apsUVARender { static unsigned int Handle=0; unsigned int* VS=&Handle; const unsigned int** VShaderTable=apsGenericShaderData::VertexTable; }
namespace apsUVARenderPixel { static unsigned int* Handle=0; unsigned int** PS=&Handle; const unsigned int** PShaderTable=apsGenericShaderData::PixelTable; }
namespace apsUVARectangleRender { static unsigned int Handle=0; unsigned int* VS=&Handle; const unsigned int** VShaderTable=apsGenericShaderData::VertexTable; }
namespace apsUVARectangleRenderPixel { static unsigned int* Handle=0; unsigned int** PS=&Handle; const unsigned int** PShaderTable=apsGenericShaderData::PixelTable; }
namespace apsColorUVARectangleRender { static unsigned int Handle=0; unsigned int* VS=&Handle; const unsigned int** VShaderTable=apsGenericShaderData::VertexTable; }
namespace apsColorUVARectangleRenderPixel { static unsigned int* Handle=0; unsigned int** PS=&Handle; const unsigned int** PShaderTable=apsGenericShaderData::PixelTable; }
namespace apsBillboardRender { static unsigned int Handle=0; unsigned int* VS=&Handle; const unsigned int** VShaderTable=apsGenericShaderData::VertexTable; }
namespace apsBillboardRenderPixel { static unsigned int* Handle=0; unsigned int** PS=&Handle; const unsigned int** PShaderTable=apsGenericShaderData::PixelTable; }
namespace apsSimpleMeshRender { unsigned long VS[1]={0}; const unsigned long** VShaderTable=(const unsigned long**)apsGenericShaderData::VertexTable; }
namespace apsSimpleMeshRenderPixel { unsigned long* PS[1]={0}; const unsigned long** PShaderTable=(const unsigned long**)apsGenericShaderData::PixelTable; }


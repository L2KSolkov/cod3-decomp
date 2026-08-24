// IDA ABI: nglMeshNode uses the class tag in render_xboxr exports.
// ============================================================================
// cdWorldBlendShader.cpp — world blend shader (5 non-inline funcs).
// Source: source/cdWorldBlendShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldBlendShader.o):
//   InitCDWorldBlendShader  @0x7DD2A0
//   ToggleCDWorldBlendShader @0x7DD2F0
//   cdWorldBlendShader::Register @0x7DD310
//   cdWorldBlendShader::AddNode @0x7DD350
// ============================================================================
#include "cdWorldBlendShader.h"

#include <intrin.h>

// Shader global pointer definitions
cdWorldBlendShader* gCDWorldBlendShader = nullptr;  // ?gCDWorldBlendShader@@3PAVcdWorldBlendShader@@A

// Shader static data definitions (render_xboxr cd*Shader.o).
namespace cdWorldBlendRender {
    static const unsigned int VShader0[54] = {
        0x000d2078, 0x00000000, 0x0062201a, 0x08001468, 0x7eb00000,
        0x00000000, 0x00e1401b, 0x08375800, 0x20a01800, 0x00000000,
        0x02a0021a, 0xb4356854, 0x68b0c84c, 0x00000000, 0x06e0e01b,
        0x0836fbff, 0x10a88800, 0x00000000, 0x08e1001b, 0x08371802,
        0xd0a44800, 0x00000000, 0x00e1201b, 0x08373800, 0x20a02800,
        0x00000000, 0x00820055, 0x14016d54, 0x38000000, 0x00000000,
        0x02000600, 0x08001054, 0xe0b0c854, 0x00000000, 0x02420400,
        0x05541854, 0xa800c85c, 0x00000000, 0x02000800, 0x0800106d,
        0x20b0f81c, 0x00000000, 0x00820000, 0x05fe1800, 0x38000000,
        0x00000000, 0x006020aa, 0x1c001400, 0x10b0f828, 0x00000000,
        0x0040001a, 0xc4002800, 0x20b0e801, 0x00000000,
    };
    static const unsigned int VShader1[81] = {
        0x00142078, 0x00000000, 0x0062201a, 0x08001468, 0x7eb00000,
        0x00000000, 0x0020001a, 0x08001000, 0x2e300000, 0x00000000,
        0x02a0021a, 0xb4356854, 0x68b0c84c, 0x00000000, 0x00e1401b,
        0x08375800, 0x20a01800, 0x00000000, 0x08e0e01b, 0x0836f802,
        0xd0a88800, 0x00000000, 0x06e1001b, 0x08371bff, 0x10a44800,
        0x00000000, 0x00820000, 0x14016d54, 0x38000000, 0x00000000,
        0x00e1201b, 0x08373800, 0x20b02800, 0x00000000, 0x02420600,
        0x05541854, 0xe800c854, 0x00000000, 0x0041eaaa, 0x5801f800,
        0x28200000, 0x00000000, 0x00820000, 0x05fe1800, 0x38000000,
        0x00000000, 0x006020aa, 0x1c001400, 0x10b0f828, 0x00000000,
        0x02000400, 0x08001054, 0xa0b0c85c, 0x00000000, 0x02000800,
        0x0800106d, 0x20a0f81c, 0x00000000, 0x01402000, 0x24003800,
        0x20a02820, 0x00000000, 0x00c1601b, 0x34377800, 0x20b08860,
        0x00000000, 0x00c1801b, 0x34379800, 0x20b04860, 0x00000000,
        0x00c1a01b, 0x3437b800, 0x20b02860, 0x00000000, 0x00c1c01b,
        0x3437d800, 0x20b01860, 0x00000000, 0x0040001a, 0xc4aa2800,
        0x20b0e801,
    };
    unsigned long VS[2] = {};
    unsigned int const* VShaderTable[2] = { VShader0, VShader1 };
}
namespace cdWorldBlendProjectedRender {
    static const unsigned int VShader0[90] = {
        0x00162078, 0x00000000, 0x0062401a, 0x2c001468, 0x2e200000,
        0x00000000, 0x044240ff, 0x2dfe5bfc, 0xb8410000, 0x00000000,
        0x00a00a1a, 0x2434b000, 0x28300000, 0x00000000, 0x00e1401b,
        0x08375800, 0x20a01800, 0x00000000, 0x00824b00, 0x3434b068,
        0xbeb00000, 0x00000000, 0x00600058, 0x08001562, 0xd7400000,
        0x00000000, 0x06400000, 0x34006bff, 0x14b80000, 0x00000000,
        0x01802000, 0x34003800, 0x28b00000, 0x00000000, 0x00802100,
        0x35fe2aa8, 0x78600000, 0x00000000, 0x00600000, 0x44001556,
        0xd8400000, 0x00000000, 0x00400000, 0x64016800, 0x28600000,
        0x00000000, 0x00a00c1a, 0x69b48800, 0x28500000, 0x00000000,
        0x00a00e1a, 0x79b48800, 0x24500000, 0x00000000, 0x09202000,
        0x65543801, 0x18640000, 0x00000000, 0x00e0e01b, 0x0836f800,
        0x20b08800, 0x00000000, 0x00400015, 0x54aa2800, 0x2c500000,
        0x00000000, 0x01402000, 0x64003800, 0x28600000, 0x00000000,
        0x00e1001b, 0x08371800, 0x20b04800, 0x00000000, 0x00e1201b,
        0x08373800, 0x20b02800, 0x00000000, 0x0080c015, 0x5400d801,
        0xb0b0c848, 0x00000000, 0x02000000, 0x08001001, 0x90b0f81c,
        0x00000000, 0x0040001a, 0xc4002800, 0x20b0e801, 0x00000000,
    };
    static const unsigned int VShader1[149] = {
        0x00252078, 0x00000000, 0x0062401a, 0x2c001468, 0x2e200000,
        0x00000000, 0x00626058, 0x3c001560, 0x27b00000, 0x00000000,
        0x04a24a1a, 0x2434b3fc, 0xb8310000, 0x00000000, 0x04a26ada,
        0xb434b3fc, 0xf4380000, 0x00000000, 0x00824b00, 0x3434b068,
        0xbeb00000, 0x00000000, 0x00826b55, 0x34b0b160, 0xf7a00000,
        0x00000000, 0x0060001a, 0x0800146a, 0xde400000, 0x00000000,
        0x004240ff, 0x2dfe5800, 0x21400000, 0x00000000, 0x00400000,
        0x34006800, 0x24b00000, 0x00000000, 0x01802000, 0x34003800,
        0x28b00000, 0x00000000, 0x00802100, 0x35fe2aa8, 0x78600000,
        0x00000000, 0x00600058, 0x0800156e, 0x97a00000, 0x00000000,
        0x004260ff, 0x3dfe7800, 0x22b00000, 0x00000000, 0x00400055,
        0x34aa6800, 0x21b00000, 0x00000000, 0x006000ff, 0x44001556,
        0xd1400000, 0x00000000, 0x00400000, 0x64016800, 0x28600000,
        0x00000000, 0x006000aa, 0xb40017fe, 0xd8b00000, 0x00000000,
        0x00802155, 0x34002aa8, 0x74600000, 0x00000000, 0x08a00c1a,
        0x68348802, 0xd8580000, 0x00000000, 0x00a00e1a, 0x78348800,
        0x24500000, 0x00000000, 0x09202000, 0x65543bfd, 0x18640000,
        0x00000000, 0x00a00c1a, 0x69b54800, 0x22500000, 0x00000000,
        0x00a00e1a, 0x79b54800, 0x21500000, 0x00000000, 0x01202055,
        0x65543800, 0x24600000, 0x00000000, 0x00e1401b, 0x08375800,
        0x20b01800, 0x00000000, 0x00400015, 0x54aa2800, 0x2c500000,
        0x00000000, 0x07402000, 0x64003bff, 0x18640000, 0x00000000,
        0x004000ab, 0x54002800, 0x23500000, 0x00000000, 0x01402055,
        0x64003800, 0x24600000, 0x00000000, 0x00e0e01b, 0x0836f800,
        0x20b08800, 0x00000000, 0x00e1001b, 0x08371800, 0x20b04800,
        0x00000000, 0x00e1201b, 0x08373800, 0x20b02800, 0x00000000,
        0x0080c015, 0x5400d801, 0xb0b0c848, 0x00000000, 0x02000000,
        0x08001001, 0x90b0f81c, 0x00000000, 0x0080c0bf, 0x5400d801,
        0xb0b0c850, 0x00000000, 0x02000000, 0x08001155, 0x90b0f824,
        0x00000000, 0x0040001a, 0xc4aa2800, 0x20b0e801,
    };
    unsigned long VS[2] = {};
    unsigned int const* VShaderTable[2] = { VShader0, VShader1 };
}
namespace cdWorldBlendPixel {
    static const unsigned int PShader0[60] = {
        0x00000000, 0xd8301010, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x000000c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x14c9c834,
        0xccc40000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000c00, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011102, 0x00000021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader1[60] = {
        0x00000000, 0x00000000, 0xdbc51010, 0xd8301010, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x000000d0, 0x000000c0,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x14c9c834,
        0xccc40000, 0x00000000, 0xcc3d0000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000c00, 0x000000c0, 0x00000000, 0x000000c0, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011104, 0x00008021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader2[60] = {
        0x00000000, 0x00000000, 0xd8301010, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x000000c0, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x14c9c834,
        0xccc40000, 0xccca0000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000c00, 0x000000c0, 0x000000c0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011103, 0x00000421,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    static const unsigned int PShader3[60] = {
        0x00000000, 0x00000000, 0x00000000, 0xdbc51010, 0xd8301010,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000000d0,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0x14c9c834,
        0xccc40000, 0xccca0000, 0x00000000, 0xcc3d0000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000c00, 0x000000c0, 0x000000c0, 0x00000000, 0x000000c0,
        0x00000000, 0x00000000, 0x00000000, 0x00011105, 0x00008421,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    unsigned long* PS[2][2] = {};
    unsigned int const* PShaderTable[2][2] = { { PShader0, PShader1 }, { PShader2, PShader3 } };
}
namespace cdWorldBlendProjectedPixel {
    static const unsigned int PShader0[60] = {
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
    static const unsigned int PShader1[60] = {
        0xd8d41010, 0xd9d530dc, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x200c2000, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000c00, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc8c40000,
        0xc9c520cc, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000c00, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011102, 0x00000021,
        0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x000001ff,
    };
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = { PShader0, PShader1 };
}
namespace cdWorldBlendSolidColorPixel {
    static const unsigned int PShader0[60] = {
        0xd1301010, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x130c0300, 0x00001c80,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x000000c0, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc1200000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x000000c0, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00011101, 0x00000001,
        0x00000000, 0x00000000, 0xfffffff0, 0xffffffff, 0x000001ff,
    };
    unsigned long* PS[2] = {};
    unsigned int const* PShaderTable[2] = { PShader0, nullptr };
    unsigned long* Shader = nullptr;
}
// ============================================================================
// InitCDWorldBlendShader — allocate the shader and link into the init list.
// ea: 0x7DD2A0
// ============================================================================
void InitCDWorldBlendShader() {
    cdWorldBlendShader* result = (cdWorldBlendShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        ::new (result) cdWorldBlendShader;
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdWorldBlendShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~8;
        gCDWorldBlendShader = result;
    } else {
        gCDWorldBlendShader = NULL;
    }
}

// ============================================================================
// ToggleCDWorldBlendShader — toggle world-blend enable bit (bit 3).
// ea: 0x7DD2F0
// ============================================================================
void ToggleCDWorldBlendShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[0];
    byte = (unsigned char)(((byte ^ (8 * ~(byte >> 3))) & 8) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[0] = byte;
}

// ============================================================================
// cdWorldBlendShader::Register — register the world-blend shaders.
// ea: 0x7DD310
// ============================================================================
tlFixedString cdWorldBlendShader::GetName() { return tlFixedString("cdWorldBlend"); }

void cdWorldBlendShader::Register() {
    nglShader::Register();
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShaderSafe((unsigned int*)&cdWorldBlendRender::VS[v0], cdWorldBlendRender::VShaderTable, v0);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterVShaderSafe((unsigned int*)&cdWorldBlendProjectedRender::VS[v0], cdWorldBlendProjectedRender::VShaderTable, v0);
    }
    for (int v0 = 0, i = 4; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdWorldBlendPixel::PS[0][v0], cdWorldBlendPixel::PShaderTable[0], v0);
    }
    for (int v0 = 0, i = 2; i != 0; --i, ++v0) {
        nglDxRegisterPShaderSafe((unsigned int**)&cdWorldBlendProjectedPixel::PS[v0], cdWorldBlendProjectedPixel::PShaderTable, v0);
    }
    nglDxRegisterPShaderSafe((unsigned int**)cdWorldBlendSolidColorPixel::PS, cdWorldBlendSolidColorPixel::PShaderTable, 0);
    cdWorldBlendSolidColorPixel::Shader = cdWorldBlendSolidColorPixel::PS != nullptr ? cdWorldBlendSolidColorPixel::PS[0] : 0;
}

// ============================================================================
// cdWorldBlendShader::AddNode — add a clipped node to the opaque list.
// ea: 0x7DD350
// ============================================================================
void cdWorldBlendShader::AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection,
                                 nglMaterial* iMat) {
    if ((ShaderCommon::ShaderSwitching.__s0[0] & 8) == 0 &&
        cdGetClipResult(iSection, iMeshNode, nglBuildScene) != -1) {
        cdWorldBlendShaderNode* node = (cdWorldBlendShaderNode*)nglListAlloc(0x18, 0x10);
        if (node != NULL) {
            node->MeshNode = iMeshNode;
            node->Section = iSection;
            // vftable = cdWorldBlendShaderNode
            node->mMaterial = (cdWorldBlendShaderMat*)iMat;
        } else {
            node = NULL;
        }
        node->SortHash = gCDWorldBlendShader->ID;
        node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = node;
        ++nglBuildScene->OpaqueListCount;
    }
}

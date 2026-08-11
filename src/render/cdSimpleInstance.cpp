// ============================================================================
// cdSimpleInstance.cpp — simple instance (7 non-inline funcs).
// Source: source/cdSimpleInstance.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleInstance.o):
//   InitCDSimpleInstanceShader @0x7C4C40
//   ToggleCDSimpleInstanceShader @0x7C4C90
//   cdSimpleInstance::Create @0x7C4CB0
//   cdSimpleInstance::Destroy @0x7C4D00
// ============================================================================
#include "cdSimpleInstance.h"

cdSimpleInstanceShader* gCDSimpleInstanceShader = nullptr;  // ?gCDSimpleInstanceShader (render_xboxr @ 0x10DE008)

#include <intrin.h>

// ============================================================================
// InitCDSimpleInstanceShader — allocate the shader and link into the init list.
// ea: 0x7C4C40
// ============================================================================
void InitCDSimpleInstanceShader() {
    cdSimpleInstanceShader* result = (cdSimpleInstanceShader*)mem_heap_malloc(0x10);
    if (result != NULL) {
        result->next = tlInitList::head;
        tlInitList::head = result;
        result->Disabled = false;
        // vftable = cdSimpleInstanceShader
        ShaderCommon::ShaderSwitching.__s0[1] &= ~0x10;
        gCDSimpleInstanceShader = result;
    } else {
        gCDSimpleInstanceShader = NULL;
    }
}

// ============================================================================
// ToggleCDSimpleInstanceShader — toggle instance enable bit (bit 4).
// ea: 0x7C4C90
// ============================================================================
void ToggleCDSimpleInstanceShader() {
    unsigned char byte = ShaderCommon::ShaderSwitching.__s0[1];
    byte = (unsigned char)(((byte ^ (16 * ~(byte >> 4))) & 0x10) ^ byte);
    ShaderCommon::ShaderSwitching.__s0[1] = byte;
}

// ============================================================================
// cdSimpleInstance::Create — allocate the instance buffers.
// ea: 0x7C4CB0
// ============================================================================
void cdSimpleInstance::Create(nglMesh* mesh, nglMeshSection* section, int numInstances) {
    this->Mesh = mesh;
    this->Section = section;
    this->Material = NULL;
    this->Insts = (XForm*)tlMemAlloc(0xD800, 0x20, 0);
    this->cells = (int*)tlMemAlloc(0x600, 0x20, 0);
    this->NInstances = 0;
}

// ============================================================================
// cdSimpleInstance::Destroy — free the instance buffers.
// ea: 0x7C4D00
// ============================================================================
void cdSimpleInstance::Destroy() {
    tlMemFree(this->Insts);
    tlMemFree(this->cells);
}

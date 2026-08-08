// ============================================================================
// nglRenderNode - base render list node + render list helpers.
// Source: c:\cod\code\tl\ngl\include\nglRenderNode.h
// Layout verified against IDA types: nglRenderNode_Vtbl (4B), nglRenderNode
// (12B = vftable + Next + SortHash/SortDist union), nglSortInfo (8B).
// Inline COMDATs verified: nglListAlloc ea 0x660140 (game.o),
// nglListAddNode_Opaque/Translucent ea 0x7C5B40/0x7C5B70 + 0x7C5BA0/0x7C5BE0
// (cdScratchShader.o), nglRenderNode ctor/GetDesc/GetSortInfo (cg.o).
// ============================================================================

#ifndef COD3_NGL_NGL_RENDER_NODE_H
#define COD3_NGL_NGL_RENDER_NODE_H

#include "ngl/nglScene.h"
#include <stdint.h>

// ngl_scene.o (data, not yet ported - temporarily defined in ngl_font.cpp)
extern class nglScene* nglBuildScene;
extern unsigned char* nglListWork;
extern unsigned char* nglListWorkPos;
extern int nglListWorkSize;
extern int nglLastListAllocWarnFrame;

// ngl_scene.o (function, not yet ported)
extern void nglValidateMatrices(nglScene* Scene);

// ngl_internal.o (ported)
extern int nglFrame;

// tl_system.o (ported)
extern void tlFatal(const char* fmt, ...);

// ============================================================================
// nglSortInfo - 8 bytes
// ============================================================================
struct nglSortInfo {
    enum Type : int {
        NGLSORT_OPAQUE = 0x0,
        NGLSORT_TRANSLUCENT = 0x1,
    };
    Type           Type;    // +0x00
    union {
        float        Dist;   // +0x04
        unsigned int Hash;   // +0x04
    };
};
static_assert(sizeof(nglSortInfo) == 8, "nglSortInfo size mismatch");

// ============================================================================
// nglRenderNode_Vtbl - vtable holder (4 bytes)
// ============================================================================
class nglRenderNode_Vtbl {
public:
    virtual ~nglRenderNode_Vtbl() {}
    virtual void Dummy() {}
};

// ============================================================================
// nglRenderNode - base render list node (12 bytes)
// ============================================================================
class nglRenderNode : public nglRenderNode_Vtbl {
public:
    nglRenderNode* Next;                       // +0x04
    union {
        unsigned int SortHash;                 // +0x08
        float        SortDist;                 // +0x08
    };

    nglRenderNode() {}

    virtual void Render() = 0;
    virtual void GetDesc(char* Desc) { if (Desc) *Desc = 0; }
    virtual void GetSortInfo(nglSortInfo& Info) { (void)Info; }
};
static_assert(sizeof(nglRenderNode) == 0xC, "nglRenderNode size mismatch");

// ============================================================================
// Render list allocation - ea: 0x660140
// ============================================================================
inline void* nglListAlloc(unsigned int Bytes, unsigned int Alignment) {
    unsigned char* result = (unsigned char*)(~(Alignment - 1) & ((uintptr_t)nglListWorkPos + Alignment - 1));
    if (result + Bytes <= nglListWork + nglListWorkSize) {
        nglListWorkPos = result + Bytes;
        return result;
    }
    if (nglLastListAllocWarnFrame != nglFrame) {
        tlFatal("Render list allocation overflow. Reserved = %d Requested = %d Free = %d.\n",
                nglListWorkSize, Bytes, (int)(nglListWork + nglListWorkSize - result));
        nglLastListAllocWarnFrame = nglFrame;
    }
    return NULL;
}

// ============================================================================
// List insertion - ea: 0x7C5B40 / 0x7C5B70 / 0x7C5BA0 / 0x7C5BE0
// ============================================================================
inline void nglListAddNode_Opaque(nglRenderNode* Node) {
    Node->Next = nglBuildScene->OpaqueRenderList;
    nglBuildScene->OpaqueRenderList = Node;
    ++nglBuildScene->OpaqueListCount;
}

inline void nglListAddNode_Translucent(nglRenderNode* Node) {
    Node->Next = nglBuildScene->TransRenderList;
    nglBuildScene->TransRenderList = Node;
    ++nglBuildScene->TransListCount;
}

inline void nglListAddNode_Opaque(nglRenderNode* Node, unsigned int Hash) {
    Node->SortHash = Hash;
    Node->Next = nglBuildScene->OpaqueRenderList;
    nglBuildScene->OpaqueRenderList = Node;
    ++nglBuildScene->OpaqueListCount;
}

inline void nglListAddNode_Translucent(nglRenderNode* Node, float Dist) {
    Node->SortDist = Dist;
    Node->Next = nglBuildScene->TransRenderList;
    nglBuildScene->TransRenderList = Node;
    ++nglBuildScene->TransListCount;
}

#endif // COD3_NGL_NGL_RENDER_NODE_H

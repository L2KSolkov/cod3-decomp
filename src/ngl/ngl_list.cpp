// ============================================================================
// ngl_list.cpp - render-list insertion helpers (ngl_xboxr:ngl_scene.o).
// Dedicated TU: nglRenderNode.h and ngl_scene.h cannot both be included with
// the aeps render-node headers, so these helpers live here.
// ============================================================================

#include "ngl/ngl_scene.h"
#include "ngl/nglRenderNode.h"

// ea: 0x7C5B40
void nglListAddNode_Opaque(nglRenderNode* Node)
{
    Node->Next = nglBuildScene->OpaqueRenderList;
    nglBuildScene->OpaqueRenderList = Node;
    ++nglBuildScene->OpaqueListCount;
}

// ea: 0x7C5B70
void nglListAddNode_Translucent(nglRenderNode* Node)
{
    Node->Next = nglBuildScene->TransRenderList;
    nglBuildScene->TransRenderList = Node;
    ++nglBuildScene->TransListCount;
}

// ea: 0x7C5BA0
void nglListAddNode_Opaque(nglRenderNode* Node, unsigned int Hash)
{
    Node->SortHash = Hash;
    Node->Next = nglBuildScene->OpaqueRenderList;
    nglBuildScene->OpaqueRenderList = Node;
    ++nglBuildScene->OpaqueListCount;
}

// ea: 0x7C5BE0
void nglListAddNode_Translucent(nglRenderNode* Node, float Dist)
{
    Node->SortDist = Dist;
    Node->Next = nglBuildScene->TransRenderList;
    nglBuildScene->TransRenderList = Node;
    ++nglBuildScene->TransListCount;
}

// ea: 0x7C6810
void nglListAddNode(nglRenderNode* Node)
{
    nglSortInfo sortInfo;
    Node->GetSortInfo(sortInfo);
    Node->SortHash = sortInfo.Hash;
    if (sortInfo.Type == nglSortInfo::NGLSORT_TRANSLUCENT) {
        Node->Next = nglBuildScene->TransRenderList;
        nglBuildScene->TransRenderList = Node;
        ++nglBuildScene->TransListCount;
    } else {
        Node->Next = nglBuildScene->OpaqueRenderList;
        nglBuildScene->OpaqueRenderList = Node;
        ++nglBuildScene->OpaqueListCount;
    }
}

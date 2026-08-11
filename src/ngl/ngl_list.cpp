// ============================================================================
// ngl_list.cpp - render-list insertion helpers (ngl_xboxr:ngl_scene.o).
// Dedicated TU: nglRenderNode.h and ngl_scene.h cannot both be included with
// the aeps render-node headers, so these helpers live here.
// ============================================================================

#include "ngl/ngl_scene.h"
#include "ngl/nglRenderNode.h"

// ea: 0x7C5B70
void nglListAddNode_Translucent(nglRenderNode* Node, float Dist)
{
    Node->SortDist = Dist;
    Node->Next = nglBuildScene->TransRenderList;
    nglBuildScene->TransRenderList = Node;
    ++nglBuildScene->TransListCount;
}

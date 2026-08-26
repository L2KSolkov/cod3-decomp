// ============================================================================
// apsNodeRenderer.h — particle-node renderer template.
// The per-node Render functions are thin forwarders that build a
// cNodeRenderer<T,N> on the stack, point mNode at the node, and call
// Render().  The template's Render body is inline COMDAT in each
// aps*Node.o (large D3D core) — left unresolved here (resolved by
// /FORCE:UNRESOLVED until the D3D core is ported).
// ============================================================================
#ifndef COD3_AEPS_APSNODERENDERER_H
#define COD3_AEPS_APSNODERENDERER_H

// ============================================================================
// cNodeRenderer — particle renderer bound to a render node (4 bytes: mNode)
// ============================================================================
template<class Particle, class Node>
struct cNodeRenderer {
    Node* mNode;  // +0x00

    // The reference emits one COMDAT body for each particle/node pair.  Keep
    // a concrete definition here so the node vtables can be emitted while
    // those pair-specific bodies are ported.
    void Render() {}
};

#endif // COD3_AEPS_APSNODERENDERER_H

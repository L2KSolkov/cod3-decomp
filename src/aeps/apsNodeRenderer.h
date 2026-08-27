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

#include "core/math_types.h"

struct BillboardParticle;
struct ColorBillboardParticle;
struct RectangleParticle;
struct ColorRectangleParticle;
struct UVAParticle;
struct ColorUVAParticle;
struct ColorUVARectangleParticle;
struct UVARectangleParticle;
class apsBillboardNode;
class apsColorBillboardNode;
class apsRectangleNode;
class apsColorRectangleNode;
class apsUVANode;
class apsColorUVANode;
class apsColorUVARectangleNode;
class apsUVARectangleNode;

namespace apsRenderSort {
template <typename Particle>
int fncompare(const void* elem1, const void* elem2);

template <typename Particle>
void SortPointers(Buffer* sortBuffer, unsigned char* firstParticle,
                  unsigned int stride, unsigned int numParticles);
}

// ============================================================================
// cNodeRenderer — particle renderer bound to a render node (4 bytes: mNode)
// ============================================================================
template<class Particle, class Node>
struct cNodeRenderer {
    Node* mNode;  // +0x00

    cNodeRenderer() : mNode(nullptr) {}
    explicit cNodeRenderer(Node* node) : mNode(node) {}

    void SetupDefaultShaders(const math::Mat43&) {}
    void SetupShaders() {}

    // The reference emits one COMDAT body for each particle/node pair.  Keep
    // a concrete definition here so the node vtables can be emitted while
    // those pair-specific bodies are ported.
    void Render() {}
};

template <>
cNodeRenderer<BillboardParticle, apsBillboardNode>::cNodeRenderer(
    apsBillboardNode* node);
template <>
void cNodeRenderer<BillboardParticle, apsBillboardNode>::SetupDefaultShaders(
    const math::Mat43& localMatrix);
template <>
void cNodeRenderer<BillboardParticle, apsBillboardNode>::SetupShaders();
template <>
void cNodeRenderer<BillboardParticle, apsBillboardNode>::Render();

template <>
cNodeRenderer<ColorBillboardParticle, apsColorBillboardNode>::cNodeRenderer(
    apsColorBillboardNode* node);
template <>
void cNodeRenderer<ColorBillboardParticle, apsColorBillboardNode>::SetupDefaultShaders(
    const math::Mat43& localMatrix);
template <>
void cNodeRenderer<ColorBillboardParticle, apsColorBillboardNode>::SetupShaders();
template <>
void cNodeRenderer<ColorBillboardParticle, apsColorBillboardNode>::Render();

template <>
cNodeRenderer<RectangleParticle, apsRectangleNode>::cNodeRenderer(
    apsRectangleNode* node);
template <>
void cNodeRenderer<RectangleParticle, apsRectangleNode>::SetupDefaultShaders(
    const math::Mat43& localMatrix);
template <>
void cNodeRenderer<RectangleParticle, apsRectangleNode>::SetupShaders();

template <>
cNodeRenderer<UVAParticle, apsUVANode>::cNodeRenderer(apsUVANode* node);
template <>
void cNodeRenderer<UVAParticle, apsUVANode>::SetupDefaultShaders(
    const math::Mat43& localMatrix);
template <>
void cNodeRenderer<UVAParticle, apsUVANode>::SetupShaders();
template <>
void cNodeRenderer<UVAParticle, apsUVANode>::Render();

template <>
cNodeRenderer<ColorUVAParticle, apsColorUVANode>::cNodeRenderer(
    apsColorUVANode* node);
template <>
void cNodeRenderer<ColorUVAParticle, apsColorUVANode>::SetupDefaultShaders(
    const math::Mat43& localMatrix);
template <>
void cNodeRenderer<ColorUVAParticle, apsColorUVANode>::SetupShaders();
template <>
void cNodeRenderer<ColorUVAParticle, apsColorUVANode>::Render();

template <>
cNodeRenderer<ColorUVARectangleParticle, apsColorUVARectangleNode>::cNodeRenderer(
    apsColorUVARectangleNode* node);
template <>
void cNodeRenderer<ColorUVARectangleParticle, apsColorUVARectangleNode>::SetupDefaultShaders(
    const math::Mat43& localMatrix);
template <>
void cNodeRenderer<ColorUVARectangleParticle, apsColorUVARectangleNode>::SetupShaders();

template <>
cNodeRenderer<UVARectangleParticle, apsUVARectangleNode>::cNodeRenderer(
    apsUVARectangleNode* node);
template <>
void cNodeRenderer<UVARectangleParticle, apsUVARectangleNode>::SetupDefaultShaders(
    const math::Mat43& localMatrix);
template <>
void cNodeRenderer<UVARectangleParticle, apsUVARectangleNode>::SetupShaders();

template <>
cNodeRenderer<ColorRectangleParticle, apsColorRectangleNode>::cNodeRenderer(
    apsColorRectangleNode* node);
template <>
void cNodeRenderer<ColorRectangleParticle, apsColorRectangleNode>::SetupDefaultShaders(
    const math::Mat43& localMatrix);
template <>
void cNodeRenderer<ColorRectangleParticle, apsColorRectangleNode>::SetupShaders();

#endif // COD3_AEPS_APSNODERENDERER_H

// ============================================================================
// apsParticleTypes.h — particle types used as cNodeRenderer template args.
// Layout is only consumed by the unported cNodeRenderer<T,N>::Render D3D
// cores; here they are opaque placeholders so the node forwarders compile.
// ============================================================================
#ifndef COD3_AEPS_APSPARTICLETYPES_H
#define COD3_AEPS_APSPARTICLETYPES_H

struct BillboardParticle {};
struct RectangleParticle {};
struct ColorBillboardParticle {};
struct ColorRectangleParticle {};
struct UVAParticle {};
struct UVARectangleParticle {};
struct ColorUVAParticle {};
struct ColorUVARectangleParticle {};

#endif // COD3_AEPS_APSPARTICLETYPES_H

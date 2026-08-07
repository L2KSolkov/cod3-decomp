// ============================================================================
// apsRenderer — particle renderer base class + fixup (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsRenderer.cpp
// Verified against IDA (aeps_xboxr:apsRenderer.o):
//   AddNode         @0x8120e0  (?AddNode@apsRenderer@@QAEXPAVapsRenderNode@@M@Z)
//   SphereIsVisible @0x812160  (?SphereIsVisible@apsRenderer@@QAEIABUapsSphere@@PAUnglScene@@@Z)
//   Fixup           @0x8121f0  (?Fixup@apsRenderer@@QAEXABUapsFixupParams@@@Z)
// Class layout (16 bytes, from IDA local type apsRenderer):
//   __vftable @0x00 (implicit), mFields @0x04, mPriority @0x08, mIsDynamicallyLit @0x0C
// ============================================================================
#ifndef COD3_AEPS_APSRENDERER_H
#define COD3_AEPS_APSRENDERER_H

#include "core/math_types.h"

#include "apsPFD.h"
#include "apsRenderNode.h"

struct nglScene;
struct nglLightContext;
namespace apsLight { struct LightInfo; }

struct apsRendererRenderInfo {
    unsigned char*             particles;          // +0x00
    int                        numParticles;       // +0x04
    const apsPFD*              pfd;                // +0x08
    apsSphere                  sphere;             // +0x10
    const math::Mat43*         localToWorld;       // +0x20
    nglLightContext*           lightContext;       // +0x24
    float                      groupDist2Camera;   // +0x28
    const apsLight::LightInfo* lightInfo;          // +0x2C
    float                      maxParticleRadius;  // +0x30

    apsRendererRenderInfo() {
        particles = 0;
        numParticles = 0;
        pfd = 0;
        localToWorld = 0;
        lightContext = 0;
        lightInfo = 0;
    }
};

struct apsFixupParams {
    struct Lookup {
        unsigned int id;
        unsigned int vtbl;
    };

    void*  basePtr;        // +0x000
    int    numDomains;     // +0x004
    Lookup domains[64];    // +0x008
    int    numRenderers;   // +0x208
    Lookup renderers[64];  // +0x20C
    int    numActions;     // +0x40C
    Lookup actions[128];   // +0x410
};

class apsRenderer {
public:
    enum eRenderResult {
        RENDERRESULT_NOT_VISIBLE = 0,
        RENDERRESULT_VISIBLE = 1,
        RENDERRESULT_NO_PARTICLES = 2,
        RENDERRESULT_NO_RENDERER = 3,
    };

    virtual ~apsRenderer();                                             // slot 0
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo) = 0;  // slot 1
    virtual unsigned int GetId() const = 0;                             // slot 2
    virtual float GetVersion() const = 0;                               // slot 3
    virtual int IsCameraFacing() const = 0;                             // slot 4
    virtual void SetScreenFacingNormal(const math::Dir3& iNormal) = 0;  // slot 5
    virtual float GetChanceToRemove() const = 0;                        // slot 6
    virtual bool GetMeshRadius(float& oRadius) const = 0;               // slot 7

    // apsRenderer.o (non-inline):
    void AddNode(apsRenderNode* iNode, float iDist);
    unsigned int SphereIsVisible(const apsSphere& iSphere, nglScene* iScene);
    void Fixup(const apsFixupParams& iFixupParams);

    unsigned int mFields;            // +0x04
    int          mPriority;          // +0x08
    unsigned int mIsDynamicallyLit;  // +0x0C

    unsigned int GetRequiredParticleFields() const { return mFields; }  // ?GetRequiredParticleFields@apsRenderer@@QBEIXZ (inline COMDAT)
};

#endif // COD3_AEPS_APSRENDERER_H

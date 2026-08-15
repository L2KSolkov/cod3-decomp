// ============================================================================
// tr_tiny.cpp - render.o small accessors/ctors/template stubs (1-12 bytes)
// Every body verified against IDA (codmp_xboxr.xbe) disasm/decompile; the
// mangled name expected by the linker map is annotated per definition.
// ============================================================================

#include "core/math_types.h"
#include "core/color.h"

#include <stdint.h>
#include <stddef.h>
#include <new>

// Param-ID globals (assigned at register time; -1 = unassigned sentinel).
unsigned int nglTintParamID = 0xFFFFFFFF;          // ?nglTintParamID@@3IA @ 0x10E305C
unsigned int nglTextureFrameParamID = 0xFFFFFFFF;  // ?nglTextureFrameParamID@@3IA @ 0x10E3040
unsigned int TextureMatrixParamID = 0xFFFFFFFF;    // ?TextureMatrixParamID@@3IA @ 0x10DE048
unsigned int isRotatingTextureParamID = 0xFFFFFFFF; // ?isRotatingTextureParamID@@3IA @ 0x10DE04C
unsigned int cdSimpleAlphaAlphaParamID = 0xFFFFFFFF; // ?cdSimpleAlphaAlphaParamID@@3IA @ 0x10DE05C
unsigned int cdFlagRandomSeedID = 0xFFFFFFFF;      // ?cdFlagRandomSeedID@@3IA @ 0x10DE098
extern unsigned int nglLightContextParamID;        // ngl_lighting.cpp
extern unsigned int cdWheelMarkShaderDataID;       // cdWheelMarkShader.cpp

// ============================================================================
// Color / math::Packed accessors
// ============================================================================
float Color::get_red() const { return r; }    // 0x6E5A30
float Color::get_green() const { return g; }  // 0x6E5A40
float Color::get_blue() const { return b; }   // 0x6E5A50
float Color::get_alpha() const { return a; }  // 0x6E5A60

float math::Dir3::Packed::GetX() const { return x; }      // 0x6E5EC0
float math::Dir3::Packed::GetY() const { return y; }      // 0x6E5ED0
float math::Dir3::Packed::GetZ() const { return z; }      // 0x6E5EE0
float math::Vector4::Packed::GetX() const { return x; }   // 0x6E6020
float math::Vector4::Packed::GetY() const { return y; }   // 0x6E6030
float math::Vector4::Packed::GetZ() const { return z; }   // 0x6E6040
float math::Vector4::Packed::GetW() const { return w; }   // 0x6E6050

// ============================================================================
// ngl param-type GetID statics + functors
// ============================================================================
struct nglLightContext;
struct nglTintParamType {
    math::Vector4* Value;            // +0x00
    static unsigned int GetID();     // ?GetID@nglTintParamType@@SAIXZ @ 0x6E76C0
    nglTintParamType operator()(math::Vector4* value);  // ??RnglTintParamType@@QAE?AU0@PAVVector4@math@@@Z @ 0x6E76D0
};
struct nglTextureFrameParamType {
    int Value;                       // +0x00
    static unsigned int GetID();     // ?GetID@nglTextureFrameParamType@@SAIXZ @ 0x6E76F0
    nglTextureFrameParamType operator()(int value);  // ??RnglTextureFrameParamType@@QAE?AU0@H@Z @ 0x6E7700
};
struct nglLightContextParamType {
    nglLightContext* Value;          // +0x00
    static unsigned int GetID();     // ?GetID@nglLightContextParamType@@SAIXZ @ 0x6E7720
    nglLightContextParamType operator()(nglLightContext* value);  // ??RnglLightContextParamType@@QAE?AU0@PAUnglLightContext@@@Z @ 0x6E7730
};
class TextureMatrixParamType {
public:
    static unsigned int GetID();  // ?GetID@TextureMatrixParamType@@SAIXZ @ 0x6E7F20
};
class isRotatingTextureParamType {
public:
    static unsigned int GetID();  // ?GetID@isRotatingTextureParamType@@SAIXZ @ 0x6E7F30
};
class cdSimpleAlphaAlphaParamType {
public:
    static unsigned int GetID();  // ?GetID@cdSimpleAlphaAlphaParamType@@SAIXZ @ 0x6E7F40
};
class cdWheelMarkShaderDataType {
public:
    static unsigned int GetID();  // ?GetID@cdWheelMarkShaderDataType@@SAIXZ @ 0x6E84D0
};
class cdFlagRandomSeedType {
public:
    static unsigned int GetID();  // ?GetID@cdFlagRandomSeedType@@SAIXZ @ 0x6E85A0
};

unsigned int nglTintParamType::GetID() { return nglTintParamID; }
unsigned int nglTextureFrameParamType::GetID() { return nglTextureFrameParamID; }
unsigned int nglLightContextParamType::GetID() { return nglLightContextParamID; }
unsigned int TextureMatrixParamType::GetID() { return TextureMatrixParamID; }
unsigned int isRotatingTextureParamType::GetID() { return isRotatingTextureParamID; }
unsigned int cdSimpleAlphaAlphaParamType::GetID() { return cdSimpleAlphaAlphaParamID; }
unsigned int cdWheelMarkShaderDataType::GetID() { return cdWheelMarkShaderDataID; }
unsigned int cdFlagRandomSeedType::GetID() { return cdFlagRandomSeedID; }

nglTintParamType nglTintParamType::operator()(math::Vector4* value)
{
    nglTintParamType r;
    r.Value = value;
    return r;
}
nglTextureFrameParamType nglTextureFrameParamType::operator()(int value)
{
    nglTextureFrameParamType r;
    r.Value = value;
    return r;
}
nglLightContextParamType nglLightContextParamType::operator()(nglLightContext* value)
{
    nglLightContextParamType r;
    r.Value = value;
    return r;
}

// ============================================================================
// nglDxRenderState
// ============================================================================
extern unsigned int dword_BC2D10;  // ZWRITEENABLE cache
extern unsigned int dword_BC2CF4;  // ZFUNC cache ("Value")
class nglDxRenderState {
public:
    bool GetZWrite();          // ?GetZWrite@nglDxRenderState@@QAE_NXZ @ 0x6E7CA0
    unsigned int GetZFunc();   // ?GetZFunc@nglDxRenderState@@QAEIXZ @ 0x6E7CF0
    void SetZWrite(bool v);    // ?SetZWrite@nglDxRenderState@@QAEX_N@Z @ 0x6E7C60
    void SetZFunc(unsigned int v);  // ?SetZFunc@nglDxRenderState@@QAEXI@Z @ 0x6E7CB0
    void SetColorWrite(unsigned int v);  // ?SetColorWrite@nglDxRenderState@@QAEXI@Z @ 0x6E7D00
};
bool nglDxRenderState::GetZWrite() { return dword_BC2D10 != 0; }
unsigned int nglDxRenderState::GetZFunc() { return dword_BC2CF4; }

// ============================================================================
// nglShader virtual no-ops
// ============================================================================
struct nglMaterial;
class nglShader {
public:
    virtual void ReleaseMaterial(nglMaterial* mat);   // ?ReleaseMaterial@nglShader@@UAEXPAUnglMaterial@@@Z @ 0x6E7DF0
    virtual void RebaseMaterial(nglMaterial* mat, unsigned int id);  // ?RebaseMaterial@nglShader@@UAEXPAUnglMaterial@@I@Z @ 0x6E7E00
};
void nglShader::ReleaseMaterial(nglMaterial*) {}
void nglShader::RebaseMaterial(nglMaterial*, unsigned int) {}

// ============================================================================
// nglMeshParams
// ============================================================================
class nglMeshParams {
public:
    unsigned int Flags;  // +0x00
    nglMeshParams();     // ??0nglMeshParams@@QAE@XZ @ 0x6E7AD0
    nglMeshParams(unsigned int Flags);  // ??0nglMeshParams@@QAE@I@Z @ 0x6E7AE0
};
nglMeshParams::nglMeshParams() { Flags = 0; }

// ============================================================================
// cdAepsShader virtual stubs (derives from tlInitList)
// ============================================================================
class nglMeshNode;
struct nglMeshSection;
struct nglMaterial;
extern void mem_heap_free(void* ptr);
class tlInitList {
public:
private:
    tlInitList* next;                // +0x04
    static tlInitList* head;         // ?head@tlInitList@@0PAV1@A
    friend class cdAepsShader;
public:
    virtual ~tlInitList() {}
};
tlInitList* tlInitList::head = nullptr;
class tlFixedString {
public:
    tlFixedString(const char* str);  // ??0tlFixedString@@QAE@PBD@Z
};
class ShaderCommon {
public:
    union ShaderSwitching_t {
        unsigned char bytes[8];
    };
    static ShaderSwitching_t ShaderSwitching;  // ?ShaderSwitching@ShaderCommon@@3TShaderSwitching_t@1@A
};
ShaderCommon::ShaderSwitching_t ShaderCommon::ShaderSwitching;
class cdAepsShader : public tlInitList {
public:
    bool Disabled;                   // +0x08
    cdAepsShader();                  // ??0cdAepsShader@@QAE@XZ @ 0x6E87F0
    virtual ~cdAepsShader();   // ??1cdAepsShader@@UAE@XZ @ 0x6EBF60
    virtual void AddNode(nglMeshNode* node, nglMeshSection* section,
                         nglMaterial* mat);  // ?AddNode@cdAepsShader@@UAEXPAVnglMeshNode@@PAUnglMeshSection@@PAUnglMaterial@@@Z @ 0x6E8850
    virtual tlFixedString GetName();  // ?GetName@cdAepsShader@@UAE?AVtlFixedString@@XZ @ 0x6E8830
    static void operator delete(void* ptr);  // mem_heap_free
};
cdAepsShader::cdAepsShader()
{
    next = tlInitList::head;
    tlInitList::head = this;
    Disabled = false;
    ShaderCommon::ShaderSwitching.bytes[4] &= (unsigned char)~1u;
}
cdAepsShader::~cdAepsShader() {}
void cdAepsShader::AddNode(nglMeshNode*, nglMeshSection*, nglMaterial*) {}
tlFixedString cdAepsShader::GetName()
{
    return tlFixedString("Particles");
}
void cdAepsShader::operator delete(void* ptr) { mem_heap_free(ptr); }

// ============================================================================
// apsClient
// ============================================================================
class apsClient {
public:
    apsClient();                     // ??0apsClient@@QAE@XZ @ 0x6E8720
    virtual ~apsClient() {}          // vtable for ??_EapsClient
    virtual void UpdateAndRender(float dt);  // ?UpdateAndRender@apsClient@@UAEXM@Z @ 0x6E86C0
};
apsClient::apsClient() {}
void apsClient::UpdateAndRender(float) {}

// ============================================================================
// apsEffect / apsEffectTemplate accessors
// ============================================================================
template <class T>
class apsArray {
public:
    uint8_t _pad[6];
    short mSize;                     // +0x06
    int size() const;                // ?size@?$apsArray@UElement@apsEffectTemplate@@@@QBEHXZ @ 0x6E9990
};
template <class T>
int apsArray<T>::size() const
{
    return mSize;
}
class apsEffectTemplate;
class apsEffect {
public:
    union SortKey {
        unsigned int fields;         // +0x00
        unsigned int _32;            // +0x00
        unsigned int As_u32();       // ?As_u32@SortKey@apsEffect@@QAEIXZ @ 0x6E8650
    };
    struct CollisionData;
    uint8_t _pad0[0x48];
    apsEffectTemplate* mTemplate;    // +0x48
    uint8_t _pad1[0x58 - 0x4C];
    SortKey mSortKey;                // +0x58
    uint8_t _pad2[0x6C - 0x5C];
    CollisionData* mCollisionData;   // +0x6C
    const math::Mat43& GetLocalToWorldTransform() const;  // ?GetLocalToWorldTransform@apsEffect@@QBEABVMat43@math@@XZ @ 0x6E8660
    CollisionData* GetCollisionData();   // ?GetCollisionData@apsEffect@@QAEPAUCollisionData@1@XZ @ 0x6E8670
    apsEffectTemplate* TemplatePtr();    // ?TemplatePtr@apsEffect@@QAEPAVapsEffectTemplate@@XZ @ 0x6E8680
    SortKey GetSortKey() const;          // ?GetSortKey@apsEffect@@QBE?ATSortKey@1@XZ @ 0x6E8690
};

class apsEffectTemplate {
public:
    struct Element {
        uint8_t _pad[8];
        float mEndTime;              // +0x08
        float EndTime() const;       // ?EndTime@Element@apsEffectTemplate@@QBEMXZ @ 0x6E8020
    };
    uint8_t _pad[0x50];
    apsArray<Element> mElements;     // +0x50 (apsArray; mSize short at +0x56)
    uint8_t _pad2[0x6C - 0x58];
    int mPriority;                   // +0x6C
    int GetPriority() const;         // ?GetPriority@apsEffectTemplate@@QBEHXZ @ 0x6E8030
    int GetNumElements() const;      // ?GetNumElements@apsEffectTemplate@@QBEHXZ @ 0x6EBDF0
    const Element& GetElement(int iIndex) const;  // ?GetElement@apsEffectTemplate@@QBEABUElement@1@H@Z
    float GetEndTime(int iElementNum) const;     // ?GetEndTime@apsEffectTemplate@@QBEMH@Z @ 0x6EBE90
};

unsigned int apsEffect::SortKey::As_u32() { return _32; }
const math::Mat43& apsEffect::GetLocalToWorldTransform() const
{
    return *(const math::Mat43*)this;
}
apsEffect::CollisionData* apsEffect::GetCollisionData() { return mCollisionData; }
apsEffectTemplate* apsEffect::TemplatePtr() { return mTemplate; }

float apsEffectTemplate::Element::EndTime() const { return mEndTime; }
int apsEffectTemplate::GetPriority() const { return mPriority; }
int apsEffectTemplate::GetNumElements() const { return mElements.size(); }

template class apsArray<apsEffectTemplate::Element>;

// ============================================================================
// apsError
// ============================================================================
class apsError {
public:
    enum eErrorType { eErrorType_0 = 0 };
    class ErrorMessage {
    public:
        eErrorType mType;            // +0x00
        const char* mMessage;        // +0x04
        eErrorType GetType();        // ?GetType@ErrorMessage@apsError@@QAE?AW4eErrorType@2@XZ @ 0x6E85B0
        const char* GetMessage();    // ?GetMessage@ErrorMessage@apsError@@QAEPBDXZ @ 0x6E85C0
    };
};
apsError::eErrorType apsError::ErrorMessage::GetType() { return mType; }
const char* apsError::ErrorMessage::GetMessage() { return mMessage; }

// ============================================================================
// ParticleEffect / WheelMark / WheelMarkMgr / CtrlIcon
// ============================================================================
class ParticleEffect {
public:
    struct RaycastData;
    uint8_t _pad[0x14];
    int culled;                      // +0x14
    int IsCulled() const;            // ?IsCulled@ParticleEffect@@QBEHXZ @ 0x6E6660
    void SetCulled(int value);       // ?SetCulled@ParticleEffect@@QAEXH@Z @ 0x6E6620
};
int ParticleEffect::IsCulled() const { return culled; }
void ParticleEffect::SetCulled(int value) { culled = value; }

struct cdWheelMarkVertex;
struct cdWheelMarkVertex {
    uint8_t _pad[0x10];
};
class Entity;
enum wheel_e { wheel_e_0 = 0 };
class WheelMark {
public:
    uint8_t _pad[4];
    unsigned int TimeStamp;          // +0x04
    void* Owner;                     // +0x08
    int Wheel;                       // +0x0C
    uint8_t _pad3[0x50 - 0x10];
    int NumVerts;                    // +0x50
    int LastVert;                    // +0x54
    uint8_t _pad4[0x64 - 0x58];
    cdWheelMarkVertex* VertexBuffer; // +0x64
    unsigned int GetTimeStamp() const;  // ?GetTimeStamp@WheelMark@@QBEIXZ @ 0x6E8510
    bool Matches(Entity* owner, wheel_e wheel) const;  // ?Matches@WheelMark@@QBE_NPAVEntity@@W4wheel_e@@@Z @ 0x6E84E0
protected:
    unsigned char GetId(cdWheelMarkVertex* V);  // ?GetId@WheelMark@@IAEEPAUcdWheelMarkVertex@@@Z @ 0x6E8550
    cdWheelMarkVertex* AddVertex();  // ?AddVertex@WheelMark@@IAEPAUcdWheelMarkVertex@@XZ @ 0x6E8520
};
unsigned int WheelMark::GetTimeStamp() const { return TimeStamp; }
bool WheelMark::Matches(Entity* owner, wheel_e wheel) const
{
    return Owner == owner && Wheel == wheel;
}
cdWheelMarkVertex* WheelMark::AddVertex()
{
    cdWheelMarkVertex* result = &VertexBuffer[LastVert];
    LastVert = (LastVert + 1) & 0x3FF;
    ++NumVerts;
    return result;
}

class cdWheelMarkShaderMat;
class WheelMarkMgr {
protected:
    static cdWheelMarkShaderMat* Material;  // ?Material@WheelMarkMgr@@1PAVcdWheelMarkShaderMat@@A
public:
    static cdWheelMarkShaderMat* GetMaterial();  // ?GetMaterial@WheelMarkMgr@@SAPAVcdWheelMarkShaderMat@@XZ @ 0x6E8570
};
cdWheelMarkShaderMat* WheelMarkMgr::GetMaterial() { return Material; }

class CtrlIcon {
public:
    static CtrlIcon* sInst;          // ?sInst@CtrlIcon@@2PAV1@A
    static CtrlIcon* Inst();         // ?Inst@CtrlIcon@@SAPAV1@XZ @ 0x6E84A0
};
CtrlIcon* CtrlIcon::Inst() { return sInst; }

// ============================================================================
// dpvs_plane_t
// ============================================================================
class PoolAllocator;
class dpvs_plane_t {
public:
    static PoolAllocator* sAllocator;   // ?sAllocator@dpvs_plane_t@@2PAVPoolAllocator@@A
    dpvs_plane_t();                     // ??0dpvs_plane_t@@QAE@XZ @ 0x6E8090
    static PoolAllocator* GetAllocator();  // ?GetAllocator@dpvs_plane_t@@SAPAVPoolAllocator@@XZ @ 0x6E5E10
    static void SetAllocator(PoolAllocator* allocator);  // ?SetAllocator@dpvs_plane_t@@SAXPAVPoolAllocator@@@Z @ 0x6E5E20
};
dpvs_plane_t::dpvs_plane_t() {}
PoolAllocator* dpvs_plane_t::GetAllocator() { return sAllocator; }
void dpvs_plane_t::SetAllocator(PoolAllocator* allocator) { sAllocator = allocator; }

// ============================================================================
// SceneManager / XModelParts / PakManager / apsBounds
// ============================================================================
class WorldSpawn;
class SceneManager {
public:
    uint8_t _pad[0x1A0];
    const WorldSpawn* mWorldSpawn;   // +0x1A0
    const WorldSpawn* GetWorldSpawn();  // ?GetWorldSpawn@SceneManager@@QAEPBVWorldSpawn@@XZ @ 0x6E8010
};
const WorldSpawn* SceneManager::GetWorldSpawn() { return mWorldSpawn; }

class XModelParts {
public:
    uint8_t _pad[0x3C];
    const char* mName;               // +0x3C (InplaceString::mStr)
    const char* GetName();           // ?GetName@XModelParts@@QAEPBDXZ @ 0x6E5E00
};
const char* XModelParts::GetName() { return mName; }

enum TPakId { TPAKID_0 = 0 };
class PakManager {
public:
    uint8_t _pad[0x3C];
    TPakId mDebugPakId;              // +0x3C
    TPakId GetDebugPakId() const;    // ?GetDebugPakId@PakManager@@QBE?AW4TPakId@@XZ @ 0x6E5DB0
};
TPakId PakManager::GetDebugPakId() const { return mDebugPakId; }

class apsBounds {
public:
    math::Dir3 mMin;                 // +0x00
    math::Dir3 mMax;                 // +0x10
    void Init();                     // ?Init@apsBounds@@QAEXXZ
    const math::Dir3& Min() const;   // ?Min@apsBounds@@QBEABVDir3@math@@XZ @ 0x6E5DC0
    const math::Dir3& Max() const;   // ?Max@apsBounds@@QBEABVDir3@math@@XZ @ 0x6E5DD0
};
const math::Dir3& apsBounds::Min() const { return mMin; }
const math::Dir3& apsBounds::Max() const { return mMax; }

// ============================================================================
// apsCommon / TlSystemCallbacks / ScopeDisableWarnings
// ============================================================================
struct nglScene;
class apsCommon {
public:
    static nglScene* mBuildScene;    // ?mBuildScene@apsCommon@@2PAUnglScene@@A
    static void InvalidateBuildScenePtr();  // ?InvalidateBuildScenePtr@apsCommon@@SAXXZ @ 0x6E5DF0
    static void SaveBuildScenePtr(nglScene* sc);  // ?SaveBuildScenePtr@apsCommon@@SAXPAUnglScene@@@Z @ 0x6E5DE0
};
void apsCommon::InvalidateBuildScenePtr() { mBuildScene = nullptr; }
void apsCommon::SaveBuildScenePtr(nglScene* sc) { mBuildScene = sc; }

class TlSystemCallbacks {
public:
    static bool sWarningsEnabled;    // ?sWarningsEnabled@TlSystemCallbacks@@2_NA
    static void EnableWarningPrint();   // ?EnableWarningPrint@TlSystemCallbacks@@SAXXZ @ 0x6E8050
    static void DisableWarningPrint();  // ?DisableWarningPrint@TlSystemCallbacks@@SAXXZ @ 0x6E8060
};
void TlSystemCallbacks::EnableWarningPrint() { sWarningsEnabled = true; }
void TlSystemCallbacks::DisableWarningPrint() { sWarningsEnabled = false; }

class ScopeDisableWarnings {
public:
    ScopeDisableWarnings();          // ??0ScopeDisableWarnings@@QAE@XZ @ 0x6E8070
    ~ScopeDisableWarnings();         // ??1ScopeDisableWarnings@@QAE@XZ @ 0x6E8080
};
ScopeDisableWarnings::ScopeDisableWarnings()
{
    TlSystemCallbacks::sWarningsEnabled = false;
}
ScopeDisableWarnings::~ScopeDisableWarnings()
{
    TlSystemCallbacks::sWarningsEnabled = true;
}

// ============================================================================
// DObj / cdl_proftimer / AnimationPlayer
// ============================================================================
class DObj {
public:
    uint8_t _pad[0xD8];
    int mLOD;                        // +0xD8
    int mLODOverride;                // +0xDC
    int mLODAnim;                    // +0xE0
    int GetLOD() const;              // ?GetLOD@DObj@@QBEHXZ @ 0x6E7770
    void ClearLODOverride();         // ?ClearLODOverride@DObj@@QAEXXZ @ 0x6E77A0
    void ClearLODAnim();             // ?ClearLODAnim@DObj@@QAEXXZ @ 0x6E77B0
};
int DObj::GetLOD() const
{
    int result = mLODOverride;
    if (result < 0)
    {
        result = mLODAnim;
        if (result < 0)
            return mLOD;
    }
    return result;
}
void DObj::ClearLODOverride() { mLODOverride = -1; }
void DObj::ClearLODAnim() { mLODAnim = -1; }

class cdl_proftimer {
public:
    float value;                     // +0x00
    void start();                    // ?start@cdl_proftimer@@QAEXXZ
    void stop();                     // ?stop@cdl_proftimer@@QAEXXZ
    void reset();                    // ?reset@cdl_proftimer@@QAEXXZ @ 0x6E7EA0
};
void cdl_proftimer::reset() { value = 0; }

extern void tlMemFree(void* ptr);
extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
class AnimationPlayer {
public:
    static void* operator new(unsigned int sz);  // ??2AnimationPlayer@@SAPAXI@Z @ 0x6E7E70
    static void operator delete(void* ptr);  // ??3AnimationPlayer@@SAXPAX@Z @ 0x6E7E90
    ~AnimationPlayer();      // ??1AnimationPlayer@@QAE@XZ (defined elsewhere)
};
void AnimationPlayer::operator delete(void* ptr) { tlMemFree(ptr); }

// ============================================================================
// trRefEntity / trModelCellRef_t / dpvs_t / render view ctors
// ============================================================================
class trRefEntity {
public:
    void* get_dlist_node();          // ?get_dlist_node@trRefEntity@@QAEPAXXZ @ 0x6E5E60
    static int get_dlist_node_offset();  // ?get_dlist_node_offset@trRefEntity@@SAHXZ @ 0x6E5E70
};
void* trRefEntity::get_dlist_node() { return this; }
int trRefEntity::get_dlist_node_offset() { return 0; }

class trModelCellRef_t {
public:
    trModelCellRef_t();              // ??0trModelCellRef_t@@QAE@XZ @ 0x6E80A0
};
trModelCellRef_t::trModelCellRef_t() {}

class dpvs_t {
public:
    struct __unnamed {
        __unnamed();                 // ??0__unnamed@dpvs_t@@QAE@XZ @ 0x6E80B0
    };
};
dpvs_t::__unnamed::__unnamed() {}

class viewParms_t {
public:
    viewParms_t();                   // ??0viewParms_t@@QAE@XZ @ 0x6E8580
};
viewParms_t::viewParms_t() {}

class viewModelInfo_t {
public:
    viewModelInfo_t();               // ??0viewModelInfo_t@@QAE@XZ @ 0x6E8590
};
viewModelInfo_t::viewModelInfo_t() {}

struct BspTreeView2;
struct world_t {
    uint8_t _pad[0x100];
    BspTreeView2* bspTree;           // +0x100
};
struct trGlobals_t {
    uint8_t _pad[0x290];
    world_t* world;                  // +0x290
    trGlobals_t();                   // ??0trGlobals_t@@QAE@XZ @ 0x6EBEC0
};
trGlobals_t::trGlobals_t() {}
extern trGlobals_t tr;               // ?tr@@3UtrGlobals_t@@A (tr_main.cpp)

// ============================================================================
// DebugLine / DebugSphere / DebugTri default ctors
// ============================================================================
class DebugLine {
public:
    DebugLine();                     // ??0DebugLine@@QAE@XZ @ 0x6E7260
};
DebugLine::DebugLine() {}

class DebugSphere {
public:
    DebugSphere();                   // ??0DebugSphere@@QAE@XZ @ 0x6E7390
};
DebugSphere::DebugSphere() {}

class DebugTri {
public:
    DebugTri();                      // ??0DebugTri@@QAE@XZ @ 0x6E7460
};
DebugTri::DebugTri() {}

// ============================================================================
// ApsGameClient debug primitives
// ============================================================================
class ApsGameClient {
public:
    struct ApsDebugSphere {
        uint8_t _pad[0x24];
        float age;                   // +0x24
        ApsDebugSphere();            // ??0ApsDebugSphere@ApsGameClient@@QAE@XZ @ 0x6E8700
    };
    struct ApsDebugLine {
        uint8_t _pad[0x30];
        float age;                   // +0x30
        ApsDebugLine();              // ??0ApsDebugLine@ApsGameClient@@QAE@XZ @ 0x6E8710
    };
};
ApsGameClient::ApsDebugSphere::ApsDebugSphere() { age = 0.0f; }
ApsGameClient::ApsDebugLine::ApsDebugLine() { age = 0.0f; }

// ============================================================================
// codListBeginScene / codListEndScene (tail wrappers)
// ============================================================================
enum nglSceneParamType { NGLSCENE_PARENT = 0 };
extern nglScene* nglListBeginScene(nglSceneParamType ParamSource);
extern void nglListEndScene();
void codListBeginScene(nglSceneParamType paramSource)  // ?codListBeginScene@@YAXW4nglSceneParamType@@@Z @ 0x6EBB40
{
    nglListBeginScene(paramSource);
}
void codListEndScene()  // ?codListEndScene@@YAXXZ @ 0x6EBB50
{
    nglListEndScene();
}

// ============================================================================
// ReadUnaligned / toDir3-family templates
// ============================================================================
template <class T>
T ReadUnaligned(const void* iMem)  // ??$ReadUnaligned@F@@YAFPBX@Z @ 0x6EA9A0
{
    return *(const T*)iMem;
}
template short ReadUnaligned<short>(const void* iMem);

template <class T>
const math::Dir3& toUnitDir3(const T& v)  // ??$toUnitDir3@VDir3@math@@@@YAABVDir3@math@@ABV01@@Z @ 0x6EA950
{
    return (const math::Dir3&)v;
}
template const math::Dir3& toUnitDir3<math::Dir3>(const math::Dir3& v);

template <class T>
const math::Position3& toPosition3(const T& v)  // ??$toPosition3@VDir3@math@@@@YAABVPosition3@math@@ABVDir3@1@@Z @ 0x6EA960
{
    return (const math::Position3&)v;
}
template const math::Position3& toPosition3<math::Dir3>(const math::Dir3& v);

template <class T>
const math::Dir3& toDir3(const T& v)  // ??$toDir3@VDir3@math@@@@YAABVDir3@math@@ABV01@@Z @ 0x6EA970
{
    return (const math::Dir3&)v;
}
template const math::Dir3& toDir3<math::Dir3>(const math::Dir3& v);

// ============================================================================
// jqPtr<void>
// ============================================================================
template <class T>
class jqPtr {
public:
    void* Value;                     // +0x00
    jqPtr();                         // ??0?$jqPtr@X@@QAE@XZ @ 0x6E8900
    void*& Ptr();                    // ?Ptr@?$jqPtr@X@@QAEAAPAXXZ @ 0x6E9F60
    operator void*();                // ??B?$jqPtr@X@@QAEPAXXZ @ 0x6EBF90
    const jqPtr<T>& operator=(void* v);  // ??4?$jqPtr@X@@QAEABV0@PAX@Z @ 0x6EBF70
};
template <class T>
jqPtr<T>::jqPtr() {}
template <class T>
void*& jqPtr<T>::Ptr() { return Value; }
template <class T>
jqPtr<T>::operator void*() { return Value; }
template class jqPtr<void>;

// ============================================================================
// InplaceVector size
// ============================================================================
template <class T>
class InplaceVector {
public:
    unsigned int mSize;              // +0x00
    T* mList;                        // +0x04
    unsigned int size() const;       // ?size@?$InplaceVector@...@@QBEIXZ
};
template <class T>
unsigned int InplaceVector<T>::size() const
{
    return mSize;
}
struct nglMesh;
class XModelCollTri;
class XModelCollSurf;
class XModelParts;
class XModel;
template unsigned int InplaceVector<nglMesh*>::size() const;          // @ 0x6E8AB0
template unsigned int InplaceVector<const XModelCollTri*>::size() const;   // @ 0x6E8AC0
template unsigned int InplaceVector<const XModelCollSurf*>::size() const;  // @ 0x6E8B50
template unsigned int InplaceVector<const XModelParts*>::size() const;     // @ 0x6E9F70
template unsigned int InplaceVector<const XModel*>::size() const;          // @ 0x6EA000

// ============================================================================
// InplaceAssetBank Size
// ============================================================================
class InplaceString;
template <class Key, class Value>
class InplaceTree {
public:
    void* mRoot;                     // +0x00
    int mCount;                      // +0x04
    template <class T>
    unsigned int* Find(T const& key) const;  // ?Find@...?$InplaceTree@VInplaceString@@I@@QBEPAIABQ...@@Z
};
template <class T, class Tree>
class InplaceAssetBank {
public:
    uint8_t _pad[0x08];
    Tree mTree;                      // +0x08
    unsigned int mSize;              // +0x10 (mPtrs.mSize)
    unsigned int Size() const;       // ?Size@?$InplaceAssetBank@...@@QBEIXZ
    template <class K>
    bool FindIndex(K const& key, unsigned int* out) const;  // ??$FindIndex@...@?$InplaceAssetBank@...@@QBE_NABQ...@@PAI@Z
};
template <class T, class Tree>
unsigned int InplaceAssetBank<T, Tree>::Size() const
{
    return mSize;
}
template unsigned int
InplaceAssetBank<XModelParts, InplaceTree<InplaceString, unsigned int>>::Size() const;  // @ 0x6EC050
template unsigned int
InplaceAssetBank<XModel, InplaceTree<InplaceString, unsigned int>>::Size() const;      // @ 0x6EC160

// ============================================================================
// ae_pair / ae_sized_array_base / ae_sized_array
// ============================================================================
template <class A, class B>
class ae_pair {
public:
    ae_pair();                       // ??0?$ae_pair@FF@@QAE@XZ @ 0x6E8DD0
    A first;
    B second;
};
template <class A, class B>
ae_pair<A, B>::ae_pair() {}

template <class T, int N>
class ae_sized_array_base {
public:
    ae_sized_array_base();           // ??0?$ae_sized_array_base@V?$ae_pair@FF@@$0BAA@@@QAE@XZ @ 0x6E9D90
    T m_elements[N];
};
template <class T, int N>
ae_sized_array_base<T, N>::ae_sized_array_base() {}

struct MultiApk;
class LightGrid {
public:
    struct LightIndex {
        unsigned short mIndex;
        unsigned short mAttenuationInt;
    };
};

template <class T, int N>
class ae_sized_array : public ae_sized_array_base<T, N> {
public:
    int m_size;                      // +sizeof(T)*N
    ae_sized_array() : m_size(0) {}  // ??0?$ae_sized_array@PAUMultiApk@@$0CA@@@QAE@XZ @ 0x6E9860
    class iterator {
    private:
        friend class ae_sized_array;
        iterator(T* ptr);            // ??0iterator@?$ae_sized_array@P6AXXZ$0CA@@@AAE@PAP6AXXZ@Z @ 0x6EA420
    public:
        T* m_ptr;                    // +0x00
        T& operator*() const;        // ??Diterator@?$ae_sized_array@P6AXXZ$0CA@@@QBEAAP6AXXZXZ @ 0x6E9420
        iterator& operator++();      // ??Eiterator@?$ae_sized_array@P6AXXZ$0CA@@@QAEAAV01@XZ @ 0x6E9430
        bool operator!=(iterator rhs) const;  // ??9iterator@?$ae_sized_array@P6AXXZ$0CA@@@QBE_NV01@@Z @ 0x6E9440
    };
    int size() const;                // ?size@?$ae_sized_array@...@@QBEHXZ
    iterator begin();                // ?begin@?$ae_sized_array@P6AXXZ$0CA@@@QAE?AViterator@1@XZ @ 0x6EBFA0
    iterator end();                  // ?end@?$ae_sized_array@P6AXXZ$0CA@@@QAE?AViterator@1@XZ @ 0x6EBFB0
    bool empty() const;              // ?empty@?$ae_sized_array@V?$ae_pair@FF@@$0BAA@@@QBE_NXZ @ 0x6E8E20
    void set_size(int s);            // ?set_size@?$ae_sized_array@V?$ae_pair@FF@@$0BAA@@@QAEXH@Z @ 0x6EA1F0
    T& back();                       // ?back@?$ae_sized_array@ULightIndex@LightGrid@@$0M@@@QAEAAULightIndex@LightGrid@@XZ @ 0x6EC900
    T& pop_back();                   // ?pop_back@?$ae_sized_array@V?$ae_pair@FF@@$0BAA@@@QAEAAV?$ae_pair@FF@@XZ @ 0x6E8EE0
    void clear();                    // ?clear@?$ae_sized_array@V?$ae_pair@FF@@$0BAA@@@QAEXXZ @ 0x6EC6A0
};
template <class T, int N>
T& ae_sized_array<T, N>::iterator::operator*() const
{
    return *m_ptr;
}
template <class T, int N>
typename ae_sized_array<T, N>::iterator& ae_sized_array<T, N>::iterator::operator++()
{
    ++m_ptr;
    return *this;
}
template <class T, int N>
ae_sized_array<T, N>::iterator::iterator(T* ptr)
    : m_ptr(ptr)
{
}
template <class T, int N>
int ae_sized_array<T, N>::size() const
{
    return m_size;
}
template <class T, int N>
typename ae_sized_array<T, N>::iterator ae_sized_array<T, N>::begin()
{
    return iterator(m_elements);
}
template <class T, int N>
void ae_sized_array<T, N>::clear()
{
    m_size = 0;
}
template class ae_sized_array_base<ae_pair<short, short>, 256>;
template class ae_sized_array<void (__cdecl*)(void), 32>;
template class ae_sized_array<LightGrid::LightIndex, 12>;
template class ae_sized_array<MultiApk*, 32>;
template class ae_sized_array<ae_pair<short, short>, 256>;
template class ae_pair<short, short>;

// ============================================================================
// ae_vector
// ============================================================================
class DebugTexturedQuad2D {
public:
    uint8_t _pad[0x28];
};
class LightEffect;
class ParticleEffect;

template <class T>
class ae_vector {
public:
    T* mElements;                    // +0x00
    int mCapacity;                   // +0x04
    int mSize;                       // +0x08
    ae_vector();                     // ??0?$ae_vector@...@@QAE@XZ
    ~ae_vector();                    // ??1?$ae_vector@...@@QAE@XZ
    T* begin();                      // ?begin@?$ae_vector@...@@QAEP...@@XZ
    T* end();                        // ?end@?$ae_vector@...@@QAEP...@@XZ
    int size() const;                // ?size@?$ae_vector@PAVParticleEffect@@@@QBEHXZ @ 0x6E8F20
    T* find(T const& iFindVal);      // declared; specialized for ae_vector<LightEffect*>
    void resize(int n) { mSize = n; }
    void clear() { resize(0); }      // ?clear@?$ae_vector@...@@QAEXXZ
private:
    T* construct_array(int iCapacity, int iSize);  // ?construct_array@?$ae_vector@...@@AAEP...@@HH@Z
    T* construct_array(int iNumber);              // ...AAEP...@@H@Z
    void destroy_all();              // ?destroy_all@?$ae_vector@...@@AAEXXZ
};
template <class T>
ae_vector<T>::~ae_vector()
{
    destroy_all();
}
template <class T>
T* ae_vector<T>::construct_array(int iCapacity, int iSize)
{
    (void)iSize;
    return (T*)tlMemAlloc((unsigned int)(sizeof(T) * iCapacity), 8u, 0);
}
template <class T>
T* ae_vector<T>::construct_array(int iNumber)
{
    return (T*)tlMemAlloc((unsigned int)(sizeof(T) * iNumber), 8u, 0);
}
template <class T>
void ae_vector<T>::destroy_all()
{
    if (mElements != nullptr)
    {
        tlMemFree(mElements);
        mElements = nullptr;
        mCapacity = 0;
    }
}
template <>
LightEffect** ae_vector<LightEffect*>::find(LightEffect* const& iFindVal)  // @ 0x6E9220
{
    LightEffect** result = mElements;
    for (LightEffect** i = &mElements[mSize]; result != i; ++result)
    {
        if (*result == iFindVal)
            break;
    }
    return result;
}
template <class T>
ae_vector<T>::ae_vector()
    : mElements(nullptr), mCapacity(0), mSize(0)
{
}
template <class T>
T* ae_vector<T>::begin()
{
    return mElements;
}
template <class T>
T* ae_vector<T>::end()
{
    return &mElements[mSize];
}
template <class T>
int ae_vector<T>::size() const
{
    return mSize;
}
template class ae_vector<LightEffect*>;
template class ae_vector<ParticleEffect*>;
template class ae_vector<DebugTexturedQuad2D>;

// ============================================================================
// reserved_dlist<trRefEntity>
// ============================================================================
template <class T>
class reserved_dlist {
public:
    struct dlist_node {
        dlist_node* m_next;
        dlist_node* m_prev;
    };
    int m_size;                      // +0x00
    dlist_node* m_head;              // +0x04
    dlist_node* m_end;               // +0x08
    dlist_node* m_tail;              // +0x0C
    reserved_dlist();                // ??0?$reserved_dlist@VtrRefEntity@@@@QAE@XZ @ 0x6EC570
    class iterator {
    public:
        dlist_node* m_node;          // +0x00
        dlist_node* m_next;          // +0x04
        iterator(dlist_node* cur, dlist_node* next);  // ??0iterator@?$reserved_dlist@VtrRefEntity@@@@QAE@PAUdlist_node@1@0@Z @ 0x6EA480
        T* operator*();              // ??Diterator@?$reserved_dlist@VtrRefEntity@@@@QAEPAVtrRefEntity@@XZ @ 0x6ED060
        iterator& operator++();      // ??Eiterator@?$reserved_dlist@VtrRefEntity@@@@QAEAAV01@XZ @ 0x6E9AE0
        bool compare(const iterator& rhs) const;  // ?compare@iterator@?$reserved_dlist@VtrRefEntity@@@@QBE_NABV12@@Z @ 0x6EA4A0
        bool operator!=(const iterator& rhs) const;  // ??9iterator@?$reserved_dlist@VtrRefEntity@@@@QBE_NABV01@@Z @ 0x6ED070
    };
    void validate() const;           // ?validate@?$reserved_dlist@VtrRefEntity@@@@QBEXXZ @ 0x6EA1A0
    dlist_node* get_head();          // ?get_head@?$reserved_dlist@VtrRefEntity@@@@QAEPAUdlist_node@1@XZ @ 0x6EA630
    bool empty() const;              // ?empty@?$reserved_dlist@VtrRefEntity@@@@QBE_NXZ @ 0x6EA640
    void clear();                    // ?clear@?$reserved_dlist@VtrRefEntity@@@@QAEXXZ @ 0x6E8CA0
    iterator end();                  // ?end@?$reserved_dlist@VtrRefEntity@@@@QAE?AViterator@1@XZ @ 0x6EC5C0
    void push_back(T* obj);          // ?push_back@?$reserved_dlist@VtrRefEntity@@@@QAEXPAVtrRefEntity@@@Z @ 0x6EC590
    static T* node_to_object(dlist_node* node);  // ?node_to_object@?$reserved_dlist@VtrRefEntity@@@@SAPAVtrRefEntity@@PAUdlist_node@1@@Z @ 0x6EA190
};
template <class T>
reserved_dlist<T>::reserved_dlist()
{
    m_size = 0;
    m_head = (dlist_node*)&m_end;
    m_end = nullptr;
    m_tail = (dlist_node*)&m_head;
}
template <class T>
void reserved_dlist<T>::push_back(T* obj)
{
    dlist_node* node = (dlist_node*)obj;
    node->m_next = (dlist_node*)&m_end;
    node->m_prev = m_tail;
    m_tail->m_next = node;
    m_tail = node;
    ++m_size;
}
template <class T>
T* reserved_dlist<T>::iterator::operator*()
{
    return (T*)m_node;
}
template <class T>
void reserved_dlist<T>::validate() const {}
template <class T>
typename reserved_dlist<T>::dlist_node* reserved_dlist<T>::get_head()
{
    return m_head;
}
template <class T>
T* reserved_dlist<T>::node_to_object(dlist_node* node)
{
    return (T*)node;
}
template class reserved_dlist<trRefEntity>;

// ============================================================================
// phys_static_array
// ============================================================================
template <class T, int N>
class phys_static_array {
public:
    uint8_t m_buffer[N * sizeof(T)]; // +0x00
    T* m_slot_array;                 // +N*sizeof(T)
    int m_alloc_count;               // +N*sizeof(T)+4
    phys_static_array();             // ??0?$phys_static_array@...@@QAE@XZ
    ~phys_static_array() {}          // ??1?$phys_static_array@...@@QAE@XZ @ 0x6EC5E0/0x6ED040/0x6EF050
    int get_count() const;           // ?get_count@?$phys_static_array@PAVtrRefEntity@@$0EA@@@QBE?BHXZ @ 0x6E9AD0
    void remove_all();               // ?remove_all@?$phys_static_array@...@@QAEXXZ
private:
    void call_destructors() {}       // ?call_destructors@?$phys_static_array@...@@AAEXXZ
    void reset_buffer();             // ?reset_buffer@?$phys_static_array@...@@AAEXXZ
};
template <class T, int N>
int phys_static_array<T, N>::get_count() const
{
    return m_alloc_count;
}
template <class T, int N>
void phys_static_array<T, N>::remove_all()
{
    m_alloc_count = 0;
}
template <class T, int N>
void phys_static_array<T, N>::reset_buffer()
{
    m_alloc_count = 0;
}
template class phys_static_array<math::Vector4, 20>;
template class phys_static_array<trRefEntity*, 64>;
template class phys_static_array<phys_static_array<math::Vector4, 20>, 64>;

// ============================================================================
// std:: allocator / vector / _Vector_val / destroy helpers
// ============================================================================
namespace std {

struct _Nonscalar_ptr_iterator_tag {};

template <class T>
class allocator {
public:
    allocator() {}                                // ??0?$allocator@...@@std@@QAE@XZ
    allocator(const allocator&) {}                // ...QAE@ABV01@@Z
    T* allocate(unsigned int _Count);             // ?allocate@...@@std@@QAEPAU...@@I@Z
    void deallocate(T* _Ptr, unsigned int);       // ?deallocate@...@@std@@QAEXPAU...@@I@Z
    void construct(T* _Ptr, const T& _Val);       // ?construct@...@@std@@QAEXPAU...@@ABU34@@Z
    void destroy(T*) {}                           // ?destroy@...@@std@@QAEXPAU...@@Z
    unsigned int max_size() const                 // ?max_size@...@@std@@QBEIXZ
    {
        return (unsigned int)(((size_t)-1) / sizeof(T));
    }
};

template <class T, class A>
class _Vector_val {
protected:
    _Vector_val(A _Al) {}                         // ??0?$_Vector_val@...@@std@@IAE@V?$allocator@...@@1@@Z
};

template <class T, class A>
class vector {
public:
    A _Alval;                                     // allocator storage
    T* _Myfirst;                                  // +0x00
    T* _Mylast;                                   // +0x04
    T* _Myend;                                    // +0x08
    vector();                                     // ??0?$vector@...@@std@@QAE@XZ
    ~vector();                                    // ??1?$vector@...@@std@@QAE@XZ
    class const_iterator {
    public:
        T* _Myptr;                                // +0x00
        const_iterator(T* _Ptr);                  // ??0const_iterator@?$vector@...@@std@@QAE@PAU...@@@Z
        const T& operator*() const { return *_Myptr; }  // ??Dconst_iterator@...@@std@@QBEABU...@@XZ
        bool operator==(const const_iterator& _Right) const;  // ??8const_iterator@...@@std@@QBE_NABV012@@Z
        bool operator!=(const const_iterator& _Right) const;  // ??9const_iterator@...@@std@@QBE_NABV012@@Z
        int operator-(const const_iterator& _Right) const;    // ??Gconst_iterator@...@@std@@QBEHABV012@@Z
    };
    class iterator {
    public:
        T* _Myptr;                                // +0x00
        iterator(T* _Ptr);                        // ??0iterator@?$vector@...@@std@@QAE@PAU...@@@Z
        T& operator*() const { return *_Myptr; }  // ??Diterator@?$vector@...@@std@@QBEAAU...@@XZ
        iterator& operator++() { ++_Myptr; return *this; }  // ??Eiterator@...@@QAEAAV012@XZ
        bool operator!=(const iterator& rhs) const { return _Myptr != rhs._Myptr; }
        T* operator->() const { return _Myptr; }  // ??Citerator@...@@QBEPAU...@@XZ
        iterator operator+(int _Off) const;       // ??Hiterator@...@@QBE?AV012@H@Z
        iterator& operator+=(int _Off);           // ??Yiterator@...@@QAEAAV012@H@Z
        int operator-(const const_iterator& _Right) const;  // ??Giterator@...@@QBEHABVconst_iterator@12@@Z
    };
    unsigned int size() const;                    // ?size@?$vector@...@@std@@QBEIXZ
    unsigned int capacity() const;                // ?capacity@?$vector@...@@std@@QBEIXZ
    iterator begin();                             // ?begin@?$vector@...@@std@@QAE?AViterator@12@XZ
    iterator end();                               // ?end@?$vector@...@@std@@QAE?AViterator@12@XZ
    void clear();                                 // ?clear@?$vector@...@@std@@QAEXXZ
    void resize(unsigned int _Newsize);           // ?resize@?$vector@...@@std@@QAEXI@Z
    void resize(unsigned int _Newsize, T _Val);   // 3-arg form (defined elsewhere)
    unsigned int max_size() const                 // ?max_size@?$vector@...@@std@@QBEIXZ
    {
        return (unsigned int)(((size_t)-1) / sizeof(T));
    }
protected:
    template <class It>
    It _Ucopy(It _First, It _Last, It _Dest);     // ??$_Ucopy@...@?$vector@...@@std@@IAE...@@Z
    T* _Ufill(T* _Ptr, unsigned int _Count, const T& _Val);  // ?_Ufill@?$vector@...@@std@@IAEPAU...@@PAU34@IABU34@@Z
    void _Tidy();                                 // ?_Tidy@?$vector@...@@std@@IAEXXZ
    void _Destroy(T*, T*) {}                      // ?_Destroy@?$vector@...@@std@@IAEXPAU...@@0@Z
};

template <class T>
void _Destroy(T*) {}                              // ??$_Destroy@U...@@std@@YAXPAU...@@@Z

template <class T, class A>
void _Destroy_range(T*, T*, A&) {}                // 4-arg form

template <class T, class A>
void _Destroy_range(T*, T*, A&, _Nonscalar_ptr_iterator_tag) {}  // 5-arg form

template <class T1, class T2>
_Nonscalar_ptr_iterator_tag _Ptr_cat(T1&, T2&)    // ??$_Ptr_cat@...@@std@@YA?AU_Nonscalar_ptr_iterator_tag@0@AAPAU...@@0@Z
{
    return _Nonscalar_ptr_iterator_tag();
}

template <class T>
T* _Allocate(unsigned int _Count, T*);            // ??$_Allocate@U...@@std@@YAPAU...@@IPAU12@@Z

template <class T, class U>
void _Construct(T* _Ptr, const U& _Val);          // ??$_Construct@U...@@std@@YAXPAU...@@ABU12@@Z

template <class It, class Pred>
It find_if(It _First, It _Last, Pred);            // ??$find_if@...@@std@@YA...@@Z
template <class It, class Out, class Pred>
Out remove_copy_if(It _First, It _Last, Out _Dest, Pred);  // ??$remove_copy_if@...@@std@@YA...@@Z
template <class It, class T>
It find(It _First, It _Last, T const& _Val);     // ??$find@...@@std@@YA...@@Z
template <class It, class T>
void fill(It _First, It _Last, const T& _Val);    // ??$fill@...@@std@@YAX...@@Z
template <class It, class Out>
Out copy(It _First, It _Last, Out _Dest);         // ??$copy@...@@std@@YA...@@Z
template <class It, class Out>
Out copy_backward(It _First, It _Last, Out _Dest);  // ??$copy_backward@...@@std@@YA...@@Z
template <class It, class Out>
Out _Copy_opt(It _First, It _Last, Out _Dest, _Nonscalar_ptr_iterator_tag);  // ??$_Copy_opt@...@@std@@YA...@@Z
template <class It, class Out>
Out _Copy_backward_opt(It _First, It _Last, Out _Dest, _Nonscalar_ptr_iterator_tag);  // ??$_Copy_backward_opt@...@@std@@YA...@@Z
template <class It, class Out, class A>
Out _Uninit_copy(It _First, It _Last, Out _Dest, A& _Al, _Nonscalar_ptr_iterator_tag);  // ??$_Uninit_copy@...@@std@@YA...@@Z
template <class It, class Out, class A>
Out _Uninitialized_copy(It _First, It _Last, Out _Dest, A& _Al);  // ??$_Uninitialized_copy@...@@std@@YA...@@Z
template <class It, class Count, class T, class A>
void _Uninit_fill_n(It _First, Count _Count, const T& _Val, A& _Al, _Nonscalar_ptr_iterator_tag);  // ??$_Uninit_fill_n@...@@std@@YA...@@Z
template <class It, class Count, class T, class A>
void _Uninitialized_fill_n(It _First, Count _Count, const T& _Val, A& _Al);  // ??$_Uninitialized_fill_n@...@@std@@YA...@@Z

}  // namespace std

struct nglTexture;
class DynamicDecalMgr {
public:
    struct DecalSet {
        void* mTexture;    // nglTexture*
        void* mDecalSet;   // DynamicDecalSet*
        bool operator==(nglTexture* texture) const;  // ??8DecalSet@DynamicDecalMgr@@QBE_NPAUnglTexture@@@Z @ 0x6E84B0
    };
};

template class std::allocator<DynamicDecalMgr::DecalSet>;
template class std::allocator<ApsGameClient::ApsDebugSphere>;
template class std::allocator<ApsGameClient::ApsDebugLine>;
template class std::_Vector_val<DynamicDecalMgr::DecalSet,
                                std::allocator<DynamicDecalMgr::DecalSet>>;
template class std::_Vector_val<ApsGameClient::ApsDebugSphere,
                                std::allocator<ApsGameClient::ApsDebugSphere>>;
template class std::_Vector_val<ApsGameClient::ApsDebugLine,
                                std::allocator<ApsGameClient::ApsDebugLine>>;
template class std::vector<DynamicDecalMgr::DecalSet,
                           std::allocator<DynamicDecalMgr::DecalSet>>;
template class std::vector<ApsGameClient::ApsDebugSphere,
                           std::allocator<ApsGameClient::ApsDebugSphere>>;
template class std::vector<ApsGameClient::ApsDebugLine,
                           std::allocator<ApsGameClient::ApsDebugLine>>;

template void std::_Destroy<DynamicDecalMgr::DecalSet>(DynamicDecalMgr::DecalSet*);
template void std::_Destroy<ApsGameClient::ApsDebugSphere>(ApsGameClient::ApsDebugSphere*);
template void std::_Destroy<ApsGameClient::ApsDebugLine>(ApsGameClient::ApsDebugLine*);

template void std::_Destroy_range<ApsGameClient::ApsDebugSphere,
                                  std::allocator<ApsGameClient::ApsDebugSphere>>(
    ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*,
    std::allocator<ApsGameClient::ApsDebugSphere>&);
template void std::_Destroy_range<ApsGameClient::ApsDebugLine,
                                  std::allocator<ApsGameClient::ApsDebugLine>>(
    ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*,
    std::allocator<ApsGameClient::ApsDebugLine>&);
template void std::_Destroy_range<DynamicDecalMgr::DecalSet,
                                  std::allocator<DynamicDecalMgr::DecalSet>>(
    DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*,
    std::allocator<DynamicDecalMgr::DecalSet>&);

template void std::_Destroy_range<ApsGameClient::ApsDebugSphere,
                                  std::allocator<ApsGameClient::ApsDebugSphere>>(
    ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*,
    std::allocator<ApsGameClient::ApsDebugSphere>&, std::_Nonscalar_ptr_iterator_tag);
template void std::_Destroy_range<ApsGameClient::ApsDebugLine,
                                  std::allocator<ApsGameClient::ApsDebugLine>>(
    ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*,
    std::allocator<ApsGameClient::ApsDebugLine>&, std::_Nonscalar_ptr_iterator_tag);
template void std::_Destroy_range<DynamicDecalMgr::DecalSet,
                                  std::allocator<DynamicDecalMgr::DecalSet>>(
    DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*,
    std::allocator<DynamicDecalMgr::DecalSet>&, std::_Nonscalar_ptr_iterator_tag);

template std::_Nonscalar_ptr_iterator_tag
std::_Ptr_cat<ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*>(
    ApsGameClient::ApsDebugSphere*&, ApsGameClient::ApsDebugSphere*&);
template std::_Nonscalar_ptr_iterator_tag
std::_Ptr_cat<ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*>(
    ApsGameClient::ApsDebugLine*&, ApsGameClient::ApsDebugLine*&);
template std::_Nonscalar_ptr_iterator_tag
std::_Ptr_cat<DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*>(
    DynamicDecalMgr::DecalSet*&, DynamicDecalMgr::DecalSet*&);

// ============================================================================
// batch 88 - 13..25 byte render.o symbols
// ============================================================================
nglMeshParams::nglMeshParams(unsigned int Flags) { this->Flags = Flags; }  // 0x6E7AE0

class cdl_profcounter {
public:
    unsigned __int64 value;  // +0x00
    void reset();            // ?reset@cdl_profcounter@@QAEXXZ @ 0x6E7F00
};
void cdl_profcounter::reset() { value = 0; }

// DbLinkedHandle<DObjHandleDb, DObj>
class DObjHandleDb;
class Handle {
public:
    unsigned int mVal;   // +0x00
};
template <class Db, class T>
class DbLinkedHandle {
public:
    unsigned int mVal;   // +0x00
    explicit DbLinkedHandle(int v);       // ??0?$DbLinkedHandle@VDObjHandleDb@@VDObj@@@@QAE@H@Z @ 0x6E8DE0
    DbLinkedHandle& operator=(Handle rhs);  // ??4?$DbLinkedHandle@VDObjHandleDb@@VDObj@@@@QAEAAV0@VHandle@@@Z @ 0x6E8E00
    T* operator*() const;                  // ??D?$DbLinkedHandle@VDObjHandleDb@@VDObj@@@@QBEPAVDObj@@XZ @ 0x6EE720
};
template <class Db, class T>
DbLinkedHandle<Db, T>::DbLinkedHandle(int v)
{
    mVal = v;
}
template <class Db, class T>
DbLinkedHandle<Db, T>& DbLinkedHandle<Db, T>::operator=(Handle rhs)
{
    mVal = rhs.mVal;
    return *this;
}
template class DbLinkedHandle<DObjHandleDb, DObj>;

// ae_sized_array additions (empty/set_size/end/back/iterator)
template <class T, int N>
bool ae_sized_array<T, N>::empty() const
{
    return m_size == 0;
}
template <class T, int N>
void ae_sized_array<T, N>::set_size(int s)
{
    m_size = s;
}
template <class T, int N>
typename ae_sized_array<T, N>::iterator ae_sized_array<T, N>::end()
{
    return iterator(m_elements + m_size);
}
template <class T, int N>
T& ae_sized_array<T, N>::back()
{
    int i = m_size - 1;
    if (i <= 0)
        i = 0;
    return m_elements[i];
}
template <class T, int N>
bool ae_sized_array<T, N>::iterator::operator!=(iterator rhs) const
{
    return m_ptr != rhs.m_ptr;
}

// std::vector additions
template <class T>
T* std::allocator<T>::allocate(unsigned int _Count)
{
    return (T*)mem_heap_malloc(sizeof(T) * _Count);
}
template <class T>
void std::allocator<T>::deallocate(T* _Ptr, unsigned int)
{
    mem_heap_free(_Ptr);
}
template <class T, class A>
std::vector<T, A>::vector()
    : _Myfirst(nullptr), _Mylast(nullptr), _Myend(nullptr)
{
}
template <class T, class A>
unsigned int std::vector<T, A>::size() const
{
    return _Myfirst != nullptr ? (unsigned int)(_Mylast - _Myfirst) : 0;
}
template <class T, class A>
unsigned int std::vector<T, A>::capacity() const
{
    return _Myfirst != nullptr ? (unsigned int)(_Myend - _Myfirst) : 0;
}
template <class T, class A>
typename std::vector<T, A>::iterator std::vector<T, A>::begin()
{
    return iterator(_Myfirst);
}
template <class T, class A>
typename std::vector<T, A>::iterator std::vector<T, A>::end()
{
    return iterator(_Mylast);
}
template <class T, class A>
void std::vector<T, A>::resize(unsigned int _Newsize)
{
    resize(_Newsize, T());
}
template <class T, class A>
std::vector<T, A>::iterator::iterator(T* _Ptr)
    : _Myptr(_Ptr)
{
}
template <class T, class A>
std::vector<T, A>::const_iterator::const_iterator(T* _Ptr)
    : _Myptr(_Ptr)
{
}
template <class T, class A>
typename std::vector<T, A>::iterator std::vector<T, A>::iterator::operator+(int _Off) const
{
    return iterator(&_Myptr[_Off]);
}
template <class T, class A>
typename std::vector<T, A>::iterator& std::vector<T, A>::iterator::operator+=(int _Off)
{
    _Myptr += _Off;
    return *this;
}
template <class T, class A>
bool std::vector<T, A>::const_iterator::operator==(const const_iterator& _Right) const
{
    return _Myptr == _Right._Myptr;
}
template <class T, class A>
bool std::vector<T, A>::const_iterator::operator!=(const const_iterator& _Right) const
{
    return _Myptr != _Right._Myptr;
}

// ngl param functors / misc accessors
class nglParamSet {
protected:
    unsigned int* Array;              // +0x00
    unsigned int& ArrayEntry(unsigned int Idx);  // ?ArrayEntry@nglParamSet@@IAEAAII@Z @ 0x6E7660
};
unsigned int& nglParamSet::ArrayEntry(unsigned int Idx)
{
    return Array[Idx + 2];
}

struct jqBatch;
class jqModule {
public:
    void (__cdecl* Code)(jqBatch*);  // +0x00
    const char* Name;                 // +0x04
    jqModule(void (__cdecl* Code)(jqBatch*), const char* Name);  // ??0jqModule@@QAE@P6AXPAUjqBatch@@@ZPBD@Z @ 0x6E7750
};
jqModule::jqModule(void (__cdecl* code)(jqBatch*), const char* name)
{
    Code = code;
    Name = name;
}

class ParticleEraserPred {
public:
    bool operator()(const ParticleEffect* effect);  // ??RParticleEraserPred@@QAE_NPBVParticleEffect@@@Z @ 0x6E8730
};
bool ParticleEraserPred::operator()(const ParticleEffect* effect)
{
    return effect == nullptr;
}

class LightEraserPred {
public:
    bool operator()(const LightEffect* effect);  // ??RLightEraserPred@@QAE_NPBVLightEffect@@@Z @ 0x6E8860
};
bool LightEraserPred::operator()(const LightEffect* effect)
{
    return effect == nullptr;
}

// IVPointer<XModelParts>
template <class T>
class IVPointer {
public:
    T* mValue;                         // +0x00
    TPakId mPakId;                     // +0x04
    IVPointer();                       // ??0?$IVPointer@VXModelParts@@@@QAE@XZ @ 0x6EAC30
    IVPointer(TPakId pakId, T* value); // ...QAE@W4TPakId@@PAVXModelParts@@@Z @ 0x6EAC10
    T* operator*();                    // ??D?$IVPointer@VXModelParts@@@@QAEPAVXModelParts@@XZ @ 0x6ED020
private:
    T* Deref() const;                  // ?Deref@?$IVPointer@VXModelParts@@@@ABEPAVXModelParts@@XZ @ 0x6EA440
};
extern void ValidatePakId(TPakId pakId);  // ?ValidatePakId@@YAXW4TPakId@@@Z
template <class T>
IVPointer<T>::IVPointer()
    : mValue(nullptr), mPakId((TPakId)0xFFFFFFFF)
{
}
template <class T>
IVPointer<T>::IVPointer(TPakId pakId, T* value)
    : mValue(value), mPakId(pakId)
{
}
template <class T>
T* IVPointer<T>::Deref() const
{
    ValidatePakId(mPakId);
    return mValue;
}
template <class T>
T* IVPointer<T>::operator*()
{
    ValidatePakId(mPakId);
    return mValue;
}
template class IVPointer<XModelParts>;

// nglMeshIterator<cdDynamicDecalVertex, unsigned short>
struct cdDynamicDecalVertex {
    float Position_x;                // +0x00
    float Position_y;                // +0x04
    float Position_z;                // +0x08
    float TexCoord_x;                // +0x0C
    float TexCoord_y;                // +0x10
    unsigned int Color;                // +0x14
};
template <class T, class I>
class nglMeshIterator {
public:
    void* section;                   // +0x00
    int cur_index;                   // +0x04
    unsigned short* index;           // +0x08
    T* vertex;                         // +0x0C
    const nglMeshIterator<T, I>& operator++();  // ??E?$nglMeshIterator@UcdDynamicDecalVertex@@G@@QAEABV0@XZ @ 0x6E9C90
    void BeginStrip(int strip);      // ?BeginStrip@?$nglMeshIterator@UcdDynamicDecalVertex@@G@@QAEXH@Z @ 0x6E9BC0
    void WritePosition(float x, float y, float z);  // ?WritePosition@...@@QAEXMMM@Z @ 0x6E9C00
    void WriteTexCoord(float u, float v);           // ?WriteTexCoord@...@@QAEXMM@Z @ 0x6E9C60
    void WriteColor(unsigned int color);   // ?WriteColor@?$nglMeshIterator@UcdDynamicDecalVertex@@G@@QAEXI@Z @ 0x6E9C40
    void WriteFlush();                     // ?WriteFlush@?$nglMeshIterator@UcdDynamicDecalVertex@@G@@QAEXXZ @ 0x6E9CC0
};
template <class T, class I>
const nglMeshIterator<T, I>& nglMeshIterator<T, I>::operator++()
{
    *index = (unsigned short)cur_index;
    ++index;
    ++cur_index;
    vertex = vertex + 1;
    return *this;
}
template <class T, class I>
void nglMeshIterator<T, I>::BeginStrip(int)
{
    if (cur_index > 0)
    {
        *index = (unsigned short)(cur_index - 1);
        ++index;
        *index = (unsigned short)cur_index;
        ++index;
    }
}
template <class T, class I>
void nglMeshIterator<T, I>::WritePosition(float x, float y, float z)
{
    vertex->Position_x = x;
    vertex->Position_y = y;
    vertex->Position_z = z;
}
template <class T, class I>
void nglMeshIterator<T, I>::WriteTexCoord(float u, float v)
{
    vertex->TexCoord_x = u;
    vertex->TexCoord_y = v;
}
template <class T, class I>
void nglMeshIterator<T, I>::WriteColor(unsigned int color)
{
    vertex->Color = color;
}
template <class T, class I>
void nglMeshIterator<T, I>::WriteFlush()
{
    // Two no-op section callbacks in the original (j_nullsub_67/j_nullsub_27).
}
template class nglMeshIterator<cdDynamicDecalVertex, unsigned short>;

// reserved_dlist additions
template <class T>
bool reserved_dlist<T>::empty() const
{
    return m_head == (dlist_node*)&m_end;
}
template <class T>
void reserved_dlist<T>::clear()
{
    m_size = 0;
    m_head = (dlist_node*)&m_end;
    m_end = nullptr;
    m_tail = (dlist_node*)&m_head;
}
template <class T>
typename reserved_dlist<T>::iterator reserved_dlist<T>::end()
{
    return iterator((dlist_node*)&m_end, nullptr);
}
template <class T>
reserved_dlist<T>::iterator::iterator(dlist_node* cur, dlist_node* next)
    : m_node(cur), m_next(next)
{
}
template <class T>
typename reserved_dlist<T>::iterator& reserved_dlist<T>::iterator::operator++()
{
    dlist_node* n = m_next;
    if (n != nullptr)
    {
        m_node = n;
        m_next = n->m_next;
    }
    return *this;
}
template <class T>
bool reserved_dlist<T>::iterator::compare(const iterator& rhs) const
{
    return m_next == rhs.m_next;
}
template <class T>
bool reserved_dlist<T>::iterator::operator!=(const iterator& rhs) const
{
    return m_next != rhs.m_next;
}

// phys_static_array ctor (slot array points at the embedded buffer)
template <class T, int N>
phys_static_array<T, N>::phys_static_array()
    : m_slot_array((T*)this), m_alloc_count(0)
{
}

// ParticleEffect::RaycastData
class apsBounds;
class proximity_data_t {
public:
    proximity_data_t();                // ??0proximity_data_t@@QAE@XZ (g.o)
};
struct ParticleEffect::RaycastData {
    apsBounds mBounds;                 // +0x00
    proximity_data_t mProximityData;   // +0x20
    RaycastData();                     // ??0RaycastData@ParticleEffect@@QAE@XZ @ 0x6EF030
};
ParticleEffect::RaycastData::RaycastData()
{
    mBounds.Init();
    new (&mProximityData) proximity_data_t();
}

// DiagMat33 assign
const math::DiagMat33& math::DiagMat33::operator=(const math::DiagMat33& m)
{
    v = m.v;
    return *this;
}

// math::Vector4::Packed setters
void math::Vector4::Packed::SetX(float _x) { x = _x; }  // 0x6E5FA0
void math::Vector4::Packed::SetY(float _y) { y = _y; }  // 0x6E5FC0
void math::Vector4::Packed::SetZ(float _z) { z = _z; }  // 0x6E5FE0
void math::Vector4::Packed::SetW(float _w) { w = _w; }  // 0x6E6000

// apsEffectTemplate / apsEffect additions
float apsEffectTemplate::GetEndTime(int iElementNum) const  // 0x6EBE90
{
    return GetElement(iElementNum).mEndTime;
}
apsEffect::SortKey apsEffect::GetSortKey() const  // 0x6E8690
{
    return mSortKey;
}

// tl_min
template <class T, class U>
T tl_min(const T& a, const U& b)  // ??$tl_min@HH@@YAHABH0@Z / ??$tl_min@II@@YAIABI0@Z
{
    return (a < b) ? a : b;
}
template int tl_min<int, int>(const int&, const int&);
template unsigned int tl_min<unsigned int, unsigned int>(const unsigned int&,
                                                         const unsigned int&);

// std::_Allocate / _Construct
template <class T>
T* std::_Allocate(unsigned int _Count, T*)  // ??$_Allocate@U...@@std@@YAPAU...@@IPAU12@@Z
{
    return (T*)mem_heap_malloc(sizeof(T) * _Count);
}
template <class T, class U>
void std::_Construct(T* _Ptr, const U& _Val)  // ??$_Construct@U...@@std@@YAXPAU...@@ABU12@@Z
{
    if (_Ptr != nullptr)
        *_Ptr = (T)_Val;
}

// PoolAllocator / allocator-backed operators
class PoolAllocator {
public:
    void* Allocate(unsigned int size, bool forceHeapAlloc);  // ?Allocate@PoolAllocator@@QAEPAXI_N@Z
    void Release(void* ptr);                                 // ?Release@PoolAllocator@@QAEXPAX@Z
};
class LightEffect {
private:
    static PoolAllocator* sAllocator;  // ?sAllocator@LightEffect@@0PAVPoolAllocator@@A
public:
    ~LightEffect();                  // ??1LightEffect@@QAE@XZ (defined elsewhere)
    static void* operator new(unsigned int size);   // ??2LightEffect@@SAPAXI@Z @ 0x6E66A0
    static void operator delete(void* ptr);         // ??3LightEffect@@SAXPAX@Z @ 0x6E66C0
};
PoolAllocator* LightEffect::sAllocator = nullptr;
void* LightEffect::operator new(unsigned int size)
{
    return sAllocator->Allocate(size, false);
}
void LightEffect::operator delete(void* ptr)
{
    sAllocator->Release(ptr);
}

class trStaticModelList_t {
public:
    static PoolAllocator* sAllocator;  // ?sAllocator@trStaticModelList_t@@2PAVPoolAllocator@@A
    static void* operator new(unsigned int size, bool forceHeapAlloc,
                              const char* file, int line);  // ??2trStaticModelList_t@@SAPAXI_NPBDH@Z @ 0x6E5E80
    static void operator delete(void* ptr);                 // ??3trStaticModelList_t@@SAXPAX@Z @ 0x6E5EA0
};
PoolAllocator* trStaticModelList_t::sAllocator = nullptr;
void* trStaticModelList_t::operator new(unsigned int size, bool forceHeapAlloc,
                                        const char*, int)
{
    return sAllocator->Allocate(size, forceHeapAlloc);
}
void trStaticModelList_t::operator delete(void* ptr)
{
    sAllocator->Release(ptr);
}

// AnimationPlayer::operator new
extern void* tlMemAlloc(unsigned int size, unsigned int align, unsigned int flags);
void* AnimationPlayer::operator new(unsigned int sz)  // ??2AnimationPlayer@@SAPAXI@Z @ 0x6E7E70
{
    return tlMemAlloc(sz, 8u, 0);
}

// WheelMark::GetId
unsigned char WheelMark::GetId(cdWheelMarkVertex* V)  // ?GetId@WheelMark@@IAEEPAUcdWheelMarkVertex@@@Z @ 0x6E8550
{
    return (unsigned char)((255 * (V - VertexBuffer)) >> 10);
}

// jqPtr::operator=
template <class T>
const jqPtr<T>& jqPtr<T>::operator=(void* v)  // ??4?$jqPtr@X@@QAEABV0@PAX@Z @ 0x6EBF70
{
    Value = v;
    return *this;
}

// D3DDevice::GetRenderState
enum _D3DRENDERSTATETYPE { D3DRS_FIRST = 0 };
extern "C" unsigned int D3D__RenderState[256];  // Xbox render-state array (Win32 shim)
class D3DDevice {
public:
    static long __stdcall GetRenderState(_D3DRENDERSTATETYPE State,
                                         unsigned long* pValue);  // ?GetRenderState@D3DDevice@@SGJW4_D3DRENDERSTATETYPE@@PAK@Z @ 0x6E5A10
};
long D3DDevice::GetRenderState(_D3DRENDERSTATETYPE State, unsigned long* pValue)
{
    *pValue = D3D__RenderState[State];
    return 0;
}

// DynamicDecalMgr::DecalSet comparison
struct nglTexture;
bool DynamicDecalMgr::DecalSet::operator==(nglTexture* texture) const  // ??8DecalSet@DynamicDecalMgr@@QBE_NPAUnglTexture@@@Z @ 0x6E84B0
{
    return mTexture == texture;
}

// explicit instantiations for batch 88 templates
template ApsGameClient::ApsDebugLine*
std::_Allocate<ApsGameClient::ApsDebugLine>(unsigned int, ApsGameClient::ApsDebugLine*);
template ApsGameClient::ApsDebugSphere*
std::_Allocate<ApsGameClient::ApsDebugSphere>(unsigned int, ApsGameClient::ApsDebugSphere*);
template DynamicDecalMgr::DecalSet*
std::_Allocate<DynamicDecalMgr::DecalSet>(unsigned int, DynamicDecalMgr::DecalSet*);
template void std::_Construct<DynamicDecalMgr::DecalSet, DynamicDecalMgr::DecalSet>(
    DynamicDecalMgr::DecalSet*, const DynamicDecalMgr::DecalSet&);

// ============================================================================
// batch 89 - 26..45 byte render.o symbols
// ============================================================================
template <class T, int N>
T& ae_sized_array<T, N>::pop_back()  // @ 0x6E8EE0
{
    if (m_size != 0)
        --m_size;
    return m_elements[m_size];
}

void math::Mat44::SetX(const math::Vector4& _x) { x = _x; }  // 0x6E65F0
void math::Mat44::SetY(const math::Vector4& _y) { y = _y; }  // 0x6E6630
void math::Mat44::SetZ(const math::Vector4& _z) { z = _z; }  // 0x6E6670

class BspPlane {
public:
    math::Dir3 mPlane;               // +0x00
    math::Dir3 GetNormal() const;    // ?GetNormal@BspPlane@@QBE?AVDir3@math@@XZ @ 0x6E5E30
};
math::Dir3 BspPlane::GetNormal() const
{
    return mPlane;
}

// cFreeList<T>::Free
struct DSkel;
struct DSkelMax;
struct DSkel4;
template <class T>
class cFreeList {
public:
    T* mpFree;                       // +0x00
    int mUsed;                       // +0x04
    int mFree;                       // +0x08
    void Free(T* ptr);               // ?Free@?$cFreeList@...@@QAEXPA...@@@Z
};
template <class T>
void cFreeList<T>::Free(T* ptr)
{
    if (ptr != nullptr)
    {
        --mUsed;
        ++mFree;
        *(void**)ptr = mpFree;
        mpFree = ptr;
    }
}
template class cFreeList<DObj>;
template class cFreeList<DSkel>;
template class cFreeList<DSkelMax>;
template class cFreeList<DSkel4>;

// std::allocator::construct
template <class T>
void std::allocator<T>::construct(T* _Ptr, const T& _Val)
{
    if (_Ptr != nullptr)
        *_Ptr = _Val;
}

// std::vector additions
template <class T, class A>
std::vector<T, A>::~vector()
{
    _Tidy();
}
template <class T, class A>
void std::vector<T, A>::clear()
{
    _Tidy();
}
template <class T, class A>
void std::vector<T, A>::_Tidy()
{
    if (_Myfirst != nullptr)
        mem_heap_free(_Myfirst);
    _Myfirst = nullptr;
    _Mylast = nullptr;
    _Myend = nullptr;
}
template <class T, class A>
template <class It>
It std::vector<T, A>::_Ucopy(It _First, It _Last, It _Dest)
{
    return std::_Uninit_copy(_First, _Last, _Dest, this->_Alval,
                             std::_Nonscalar_ptr_iterator_tag());
}
template <class T, class A>
T* std::vector<T, A>::_Ufill(T* _Ptr, unsigned int _Count, const T& _Val)
{
    std::_Uninit_fill_n(_Ptr, _Count, _Val, this->_Alval,
                        std::_Nonscalar_ptr_iterator_tag());
    return &_Ptr[_Count];
}
template <class T, class A>
int std::vector<T, A>::iterator::operator-(const const_iterator& _Right) const
{
    return (int)(_Myptr - _Right._Myptr);
}
template <class T, class A>
int std::vector<T, A>::const_iterator::operator-(const const_iterator& _Right) const
{
    return (int)(_Myptr - _Right._Myptr);
}

// std algorithm templates
template <class It, class Pred>
It std::find_if(It _First, It _Last, Pred _Pred)
{
    for (; _First != _Last; ++_First)
    {
        if (_Pred(*_First))
            break;
    }
    return _First;
}
template <class It, class Out, class Pred>
Out std::remove_copy_if(It _First, It _Last, Out _Dest, Pred _Pred)
{
    for (; _First != _Last; ++_First)
    {
        if (!_Pred(*_First))
            *_Dest++ = *_First;
    }
    return _Dest;
}
template <class It, class T>
It std::find(It _First, It _Last, T const& _Val)
{
    for (; _First != _Last; ++_First)
    {
        if ((*_First).mTexture == _Val)
            break;
    }
    return _First;
}
template <class It, class T>
void std::fill(It _First, It _Last, const T& _Val)
{
    for (; _First != _Last; ++_First)
        *_First = _Val;
}
template <class It, class Out>
Out std::_Copy_opt(It _First, It _Last, Out _Dest, std::_Nonscalar_ptr_iterator_tag)
{
    for (; _First != _Last; ++_First, ++_Dest)
        *_Dest = *_First;
    return _Dest;
}
template <class It, class Out>
Out std::copy(It _First, It _Last, Out _Dest)
{
    return std::_Copy_opt(_First, _Last, _Dest, std::_Nonscalar_ptr_iterator_tag());
}
template <class It, class Out>
Out std::_Copy_backward_opt(It _First, It _Last, Out _Dest, std::_Nonscalar_ptr_iterator_tag)
{
    while (_First != _Last)
        *--_Dest = *--_Last;
    return _Dest;
}
template <class It, class Out>
Out std::copy_backward(It _First, It _Last, Out _Dest)
{
    return std::_Copy_backward_opt(_First, _Last, _Dest, std::_Nonscalar_ptr_iterator_tag());
}
template <class It, class Out, class A>
Out std::_Uninit_copy(It _First, It _Last, Out _Dest, A& _Al,
                      std::_Nonscalar_ptr_iterator_tag)
{
    (void)_Al;
    for (; _First != _Last; ++_First, ++_Dest)
    {
        if (_Dest != nullptr)
            *_Dest = *_First;
    }
    return _Dest;
}
template <class It, class Out, class A>
Out std::_Uninitialized_copy(It _First, It _Last, Out _Dest, A& _Al)
{
    return std::_Uninit_copy(_First, _Last, _Dest, _Al,
                             std::_Nonscalar_ptr_iterator_tag());
}
template <class It, class Count, class T, class A>
void std::_Uninit_fill_n(It _First, Count _Count, const T& _Val, A& _Al,
                         std::_Nonscalar_ptr_iterator_tag)
{
    (void)_Al;
    while (_Count != 0)
    {
        if (_First != nullptr)
            *_First = _Val;
        ++_First;
        --_Count;
    }
}
template <class It, class Count, class T, class A>
void std::_Uninitialized_fill_n(It _First, Count _Count, const T& _Val, A& _Al)
{
    std::_Uninit_fill_n(_First, _Count, _Val, _Al,
                        std::_Nonscalar_ptr_iterator_tag());
}

// explicit instantiations for batch 89 std templates
template void std::_Construct<ApsGameClient::ApsDebugSphere, ApsGameClient::ApsDebugSphere>(
    ApsGameClient::ApsDebugSphere*, const ApsGameClient::ApsDebugSphere&);
template void std::_Construct<ApsGameClient::ApsDebugLine, ApsGameClient::ApsDebugLine>(
    ApsGameClient::ApsDebugLine*, const ApsGameClient::ApsDebugLine&);
template std::vector<DynamicDecalMgr::DecalSet,
                     std::allocator<DynamicDecalMgr::DecalSet>>::iterator
std::find<std::vector<DynamicDecalMgr::DecalSet,
                      std::allocator<DynamicDecalMgr::DecalSet>>::iterator,
          nglTexture*>(
    std::vector<DynamicDecalMgr::DecalSet,
                std::allocator<DynamicDecalMgr::DecalSet>>::iterator,
    std::vector<DynamicDecalMgr::DecalSet,
                std::allocator<DynamicDecalMgr::DecalSet>>::iterator,
    nglTexture* const&);
template ParticleEffect**
std::find_if<ParticleEffect**, ParticleEraserPred>(
    ParticleEffect**, ParticleEffect**, ParticleEraserPred);
template LightEffect**
std::find_if<LightEffect**, LightEraserPred>(
    LightEffect**, LightEffect**, LightEraserPred);
template ParticleEffect**
std::remove_copy_if<ParticleEffect**, ParticleEffect**, ParticleEraserPred>(
    ParticleEffect**, ParticleEffect**, ParticleEffect**, ParticleEraserPred);
template LightEffect**
std::remove_copy_if<LightEffect**, LightEffect**, LightEraserPred>(
    LightEffect**, LightEffect**, LightEffect**, LightEraserPred);
template void std::fill<DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet>(
    DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*,
    const DynamicDecalMgr::DecalSet&);
template void std::fill<ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere>(
    ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*,
    const ApsGameClient::ApsDebugSphere&);
template void std::fill<ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine>(
    ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*,
    const ApsGameClient::ApsDebugLine&);
template DynamicDecalMgr::DecalSet*
std::copy<DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*>(
    DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*);
template ApsGameClient::ApsDebugSphere*
std::copy<ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*>(
    ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*);
template ApsGameClient::ApsDebugLine*
std::copy<ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*>(
    ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*);
template DynamicDecalMgr::DecalSet*
std::copy_backward<DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*>(
    DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*);
template ApsGameClient::ApsDebugSphere*
std::copy_backward<ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*>(
    ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*);
template ApsGameClient::ApsDebugLine*
std::copy_backward<ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*>(
    ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*);
template DynamicDecalMgr::DecalSet*
std::_Copy_opt<DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*>(
    DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*,
    std::_Nonscalar_ptr_iterator_tag);
template ApsGameClient::ApsDebugSphere*
std::_Copy_opt<ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*>(
    ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*,
    std::_Nonscalar_ptr_iterator_tag);
template ApsGameClient::ApsDebugLine*
std::_Copy_opt<ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*>(
    ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*,
    std::_Nonscalar_ptr_iterator_tag);
template DynamicDecalMgr::DecalSet*
std::_Copy_backward_opt<DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*>(
    DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*,
    std::_Nonscalar_ptr_iterator_tag);
template ApsGameClient::ApsDebugSphere*
std::_Copy_backward_opt<ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*>(
    ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*,
    std::_Nonscalar_ptr_iterator_tag);
template ApsGameClient::ApsDebugLine*
std::_Copy_backward_opt<ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*>(
    ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*,
    std::_Nonscalar_ptr_iterator_tag);
template DynamicDecalMgr::DecalSet*
std::_Uninitialized_copy<DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*,
                         std::allocator<DynamicDecalMgr::DecalSet>>(
    DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*,
    std::allocator<DynamicDecalMgr::DecalSet>&);
template ApsGameClient::ApsDebugSphere*
std::_Uninitialized_copy<ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*,
                         std::allocator<ApsGameClient::ApsDebugSphere>>(
    ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*,
    std::allocator<ApsGameClient::ApsDebugSphere>&);
template ApsGameClient::ApsDebugLine*
std::_Uninitialized_copy<ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*,
                         std::allocator<ApsGameClient::ApsDebugLine>>(
    ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*,
    std::allocator<ApsGameClient::ApsDebugLine>&);
template DynamicDecalMgr::DecalSet*
std::_Uninit_copy<DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*,
                  std::allocator<DynamicDecalMgr::DecalSet>>(
    DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*,
    std::allocator<DynamicDecalMgr::DecalSet>&, std::_Nonscalar_ptr_iterator_tag);
template void std::_Uninitialized_fill_n<ApsGameClient::ApsDebugSphere*, unsigned int,
                                         ApsGameClient::ApsDebugSphere,
                                         std::allocator<ApsGameClient::ApsDebugSphere>>(
    ApsGameClient::ApsDebugSphere*, unsigned int, const ApsGameClient::ApsDebugSphere&,
    std::allocator<ApsGameClient::ApsDebugSphere>&);
template void std::_Uninitialized_fill_n<ApsGameClient::ApsDebugLine*, unsigned int,
                                         ApsGameClient::ApsDebugLine,
                                         std::allocator<ApsGameClient::ApsDebugLine>>(
    ApsGameClient::ApsDebugLine*, unsigned int, const ApsGameClient::ApsDebugLine&,
    std::allocator<ApsGameClient::ApsDebugLine>&);
template void std::_Uninitialized_fill_n<DynamicDecalMgr::DecalSet*, unsigned int,
                                         DynamicDecalMgr::DecalSet,
                                         std::allocator<DynamicDecalMgr::DecalSet>>(
    DynamicDecalMgr::DecalSet*, unsigned int, const DynamicDecalMgr::DecalSet&,
    std::allocator<DynamicDecalMgr::DecalSet>&);
template void std::_Uninit_fill_n<DynamicDecalMgr::DecalSet*, unsigned int,
                                  DynamicDecalMgr::DecalSet,
                                  std::allocator<DynamicDecalMgr::DecalSet>>(
    DynamicDecalMgr::DecalSet*, unsigned int, const DynamicDecalMgr::DecalSet&,
    std::allocator<DynamicDecalMgr::DecalSet>&, std::_Nonscalar_ptr_iterator_tag);

template DynamicDecalMgr::DecalSet*
std::vector<DynamicDecalMgr::DecalSet,
            std::allocator<DynamicDecalMgr::DecalSet>>::_Ucopy<DynamicDecalMgr::DecalSet*>(
    DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*, DynamicDecalMgr::DecalSet*);
template ApsGameClient::ApsDebugSphere*
std::vector<ApsGameClient::ApsDebugSphere,
            std::allocator<ApsGameClient::ApsDebugSphere>>::_Ucopy<ApsGameClient::ApsDebugSphere*>(
    ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*, ApsGameClient::ApsDebugSphere*);
template ApsGameClient::ApsDebugLine*
std::vector<ApsGameClient::ApsDebugLine,
            std::allocator<ApsGameClient::ApsDebugLine>>::_Ucopy<ApsGameClient::ApsDebugLine*>(
    ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*, ApsGameClient::ApsDebugLine*);

// ae_pair equality
template <class A, class B>
bool operator==(const ae_pair<A, B>& lhs, const ae_pair<A, B>& rhs)  // ??$?8FF@@YA_NABV?$ae_pair@FF@@0@Z @ 0x6EA9B0
{
    return lhs.first == rhs.first && lhs.second == rhs.second;
}
template bool operator==<short, short>(const ae_pair<short, short>&,
                                       const ae_pair<short, short>&);

// DbLinkedHandle::operator*
class DObjHandleDb {
public:
    struct DbElement {
        void* mObject;               // +0x00
        int mKey;                    // +0x04
    };
    uint8_t _pad[0xA8];
    DbElement mElements[0x540];      // +0xA8
private:
    template <class Db, class T>
    friend class DbLinkedHandle;
    static DObjHandleDb sInst;       // ?sInst@DObjHandleDb@@0V1@A
};
template <class Db, class T>
T* DbLinkedHandle<Db, T>::operator*() const  // ??D?$DbLinkedHandle@VDObjHandleDb@@VDObj@@@@QBEPAVDObj@@XZ @ 0x6EE720
{
    unsigned int mVal = this->mVal;
    unsigned int idx = mVal & 0xFFF;
    if (idx < 0x540
        && mVal >> 12 == (unsigned int)DObjHandleDb::sInst.mElements[idx].mKey)
        return (T*)DObjHandleDb::sInst.mElements[idx].mObject;
    return nullptr;
}
// InplaceTree::Find + InplaceAssetBank::FindIndex
template <class T, class Tree>
template <class K>
bool InplaceAssetBank<T, Tree>::FindIndex(K const& key, unsigned int* out) const  // ??$FindIndex@...@?$InplaceAssetBank@...@@QBE_NABQ...@@PAI@Z
{
    unsigned int* found = mTree.Find(key);
    if (found == nullptr)
        return false;
    *out = *found;
    return true;
}
template bool InplaceAssetBank<XModel, InplaceTree<InplaceString, unsigned int>>::
    FindIndex<char*>(char* const&, unsigned int*) const;
template bool InplaceAssetBank<XModel, InplaceTree<InplaceString, unsigned int>>::
    FindIndex<char const*>(char const* const&, unsigned int*) const;
template bool InplaceAssetBank<XModelParts, InplaceTree<InplaceString, unsigned int>>::
    FindIndex<char const*>(char const* const&, unsigned int*) const;

// jqBatch
class jqBatch {
public:
    uint8_t _pad[0x10];
    int InputSize;                   // +0x10
    int OutputSize;                  // +0x14
    int ScratchSize;                 // +0x18
    int StaticSize;                  // +0x1C
    uint8_t _pad2[0x28 - 0x20];
    int Priority;                    // +0x28
    int GroupID;                     // +0x2C
    jqBatch();                       // ??0jqBatch@@QAE@XZ @ 0x6EBC20
};
jqBatch::jqBatch()
{
    InputSize = 0;
    OutputSize = 0;
    ScratchSize = 0;
    StaticSize = 0;
    Priority = 1;
    GroupID = -1;
}

// codListAddMesh
class nglMeshNode;
struct nglMesh;
class nglMeshParams;
struct nglShaderParamSet;
extern nglMeshNode* _codListAddMesh(nglMesh* mesh, const math::Mat43& localToWorld,
                                    nglMeshParams* meshParams,
                                    nglShaderParamSet* shaderParams,
                                    void (__cdecl* fn)(nglMeshNode*));
nglMeshNode* codListAddMesh(nglMesh* Mesh, const math::Mat43& localToWorld,
                            nglMeshParams* meshParams,
                            nglShaderParamSet* shaderParams)  // ?codListAddMesh@@YAPAVnglMeshNode@@PAUnglMesh@@ABVMat43@math@@PAVnglMeshParams@@PAUnglShaderParamSet@@@Z @ 0x6EBB60
{
    return _codListAddMesh(Mesh, localToWorld, meshParams, shaderParams, nullptr);
}

// nglIsSphereVisible(Scene) wrapper
extern bool nglIsSphereVisible(const math::Position3& Center, float Radius,
                               const math::Vector4* Clip);  // ?nglIsSphereVisible@@YA_NABVPosition3@math@@MPBVVector4@2@@Z
struct nglScene {
    uint8_t _pad[0x270];
};
bool nglIsSphereVisible(const math::Position3& Center, float Radius,
                        nglScene* Scene)  // ?nglIsSphereVisible@@YA_NABVPosition3@math@@MPAUnglScene@@@Z @ 0x6E7C30
{
    return nglIsSphereVisible(Center, Radius,
                              (const math::Vector4*)((char*)Scene + 0x270));
}

// nglDxRenderState setters
extern unsigned int dword_40354;  // ZFUNC state slot
extern unsigned int dword_40358;  // COLORWRITE state slot
extern unsigned int dword_4035C;  // ZWRITE state slot
extern unsigned int dword_BC2D1C; // COLORWRITE cache
extern "C" int __stdcall D3DDevice_SetRenderState_ParameterCheck(unsigned int State,
                                                                 unsigned int Value);
extern "C" void __fastcall D3DDevice_SetRenderState_Simple(unsigned int Method,
                                                           unsigned int Value);
void nglDxRenderState::SetZWrite(bool v)  // ?SetZWrite@nglDxRenderState@@QAEX_N@Z @ 0x6E7C60
{
    if (D3DDevice_SetRenderState_ParameterCheck(0x40, v) == 0)
    {
        D3DDevice_SetRenderState_Simple((unsigned int)&dword_4035C, v);
        dword_BC2D10 = v;
    }
}
void nglDxRenderState::SetZFunc(unsigned int v)  // ?SetZFunc@nglDxRenderState@@QAEXI@Z @ 0x6E7CB0
{
    if (D3DDevice_SetRenderState_ParameterCheck(0x39, v) == 0)
    {
        D3DDevice_SetRenderState_Simple((unsigned int)&dword_40354, v);
        dword_BC2CF4 = v;
    }
}
void nglDxRenderState::SetColorWrite(unsigned int v)  // ?SetColorWrite@nglDxRenderState@@QAEXI@Z @ 0x6E7D00
{
    if (D3DDevice_SetRenderState_ParameterCheck(0x43, v) == 0)
    {
        D3DDevice_SetRenderState_Simple((unsigned int)&dword_40358, v);
        dword_BC2D1C = v;
    }
}

// R_GenerateDrawSurfs / FX_RenderFX wrappers
extern void R_SetupProjection();  // ?R_SetupProjection@@YAXXZ
extern int gRenderEntities;       // ?gRenderEntities@@3HA
extern void R_AddWorldSurfacesDPVS();  // ?R_AddWorldSurfacesDPVS@@YAXXZ (unported)
void R_GenerateDrawSurfs()  // ?R_GenerateDrawSurfs@@YAXXZ @ 0x6DA090
{
    extern void R_AddEntitySurfaces();  // ?R_AddEntitySurfaces@@YAXXZ (tr_main.cpp)
    R_SetupProjection();
    if (tr.world != nullptr && tr.world->bspTree != nullptr)
        R_AddWorldSurfacesDPVS();
    if (gRenderEntities != 0)
        R_AddEntitySurfaces();
}

extern int gParticleBatchGroup;                    // ?gParticleBatchGroup@@3HA
extern cdl_proftimer cdl_proftimer_fx_render;      // ?cdl_proftimer_fx_render@@3Ucdl_proftimer@@A
extern void RenderEffectsInternal();               // ?RenderEffectsInternal@@YAXXZ
extern void FX_BuildSortedParticleEffectList();    // ?FX_BuildSortedParticleEffectList@@YAXXZ
void FX_RenderFX()  // ?FX_RenderFX@@YAXXZ @ 0x6DA3C0
{
    (void)gParticleBatchGroup;  // passed to a no-op callback in the original
    cdl_proftimer_fx_render.start();
    RenderEffectsInternal();
    cdl_proftimer_fx_render.stop();
    FX_BuildSortedParticleEffectList();
}

// deleting-destructor emission triggers (??_G / ??_E compiler stubs)
class AnimationPlayer;
void force_emit_LightEffect_delete(LightEffect* p) { delete p; }
void force_emit_AnimationPlayer_delete(AnimationPlayer* p) { delete p; }
void force_emit_cdAepsShader_delete(cdAepsShader* p) { delete p; }
void force_emit_apsClient_delete(apsClient* p) { delete[] p; }
void force_emit_physvec_delete(phys_static_array<math::Vector4, 20>* p) { delete p; }

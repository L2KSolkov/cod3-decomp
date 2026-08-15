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
class tlInitList {
public:
    virtual ~tlInitList() {}
};
class cdAepsShader : public tlInitList {
public:
    virtual ~cdAepsShader();   // ??1cdAepsShader@@UAE@XZ @ 0x6EBF60
    virtual void AddNode(nglMeshNode* node, nglMeshSection* section,
                         nglMaterial* mat);  // ?AddNode@cdAepsShader@@UAEXPAVnglMeshNode@@PAUnglMeshSection@@PAUnglMaterial@@@Z @ 0x6E8850
};
cdAepsShader::~cdAepsShader() {}
void cdAepsShader::AddNode(nglMeshNode*, nglMeshSection*, nglMaterial*) {}

// ============================================================================
// apsClient
// ============================================================================
class apsClient {
public:
    apsClient();                     // ??0apsClient@@QAE@XZ @ 0x6E8720
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
class WheelMark {
public:
    uint8_t _pad[4];
    unsigned int TimeStamp;          // +0x04
    uint8_t _pad2[0x64 - 0x08];
    cdWheelMarkVertex* VertexBuffer; // +0x64
    unsigned int GetTimeStamp() const;  // ?GetTimeStamp@WheelMark@@QBEIXZ @ 0x6E8510
protected:
    unsigned char GetId(cdWheelMarkVertex* V);  // ?GetId@WheelMark@@IAEEPAUcdWheelMarkVertex@@@Z @ 0x6E8550
};
unsigned int WheelMark::GetTimeStamp() const { return TimeStamp; }

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
    uint8_t _pad[0xDC];
    int mLODOverride;                // +0xDC
    int mLODAnim;                    // +0xE0
    void ClearLODOverride();         // ?ClearLODOverride@DObj@@QAEXXZ @ 0x6E77A0
    void ClearLODAnim();             // ?ClearLODAnim@DObj@@QAEXXZ @ 0x6E77B0
};
void DObj::ClearLODOverride() { mLODOverride = -1; }
void DObj::ClearLODAnim() { mLODAnim = -1; }

class cdl_proftimer {
public:
    float value;                     // +0x00
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

class trGlobals_t {
public:
    trGlobals_t();                   // ??0trGlobals_t@@QAE@XZ @ 0x6EBEC0
};
trGlobals_t::trGlobals_t() {}

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
class InplaceTree;
template <class T, class Tree>
class InplaceAssetBank {
public:
    uint8_t _pad[0x10];
    unsigned int mSize;              // +0x10 (mPtrs.mSize)
    unsigned int Size() const;       // ?Size@?$InplaceAssetBank@...@@QBEIXZ
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
    T* begin();                      // ?begin@?$ae_vector@...@@QAEP...@@XZ
    T* end();                        // ?end@?$ae_vector@...@@QAEP...@@XZ
    int size() const;                // ?size@?$ae_vector@PAVParticleEffect@@@@QBEHXZ @ 0x6E8F20
    void resize(int n) { mSize = n; }
    void clear() { resize(0); }      // ?clear@?$ae_vector@...@@QAEXXZ
};
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
    dlist_node** m_tail;             // +0x0C
    class iterator {
    public:
        dlist_node* m_node;          // +0x00
        dlist_node* m_next;          // +0x04
        iterator(dlist_node* cur, dlist_node* next);  // ??0iterator@?$reserved_dlist@VtrRefEntity@@@@QAE@PAUdlist_node@1@0@Z @ 0x6EA480
        T* operator*();              // ??Diterator@?$reserved_dlist@VtrRefEntity@@@@QAEPAVtrRefEntity@@XZ @ 0x6ED060
        iterator& operator++();      // ??Eiterator@?$reserved_dlist@VtrRefEntity@@@@QAEAAV01@XZ @ 0x6E9AE0
        bool compare(const iterator& rhs) const;  // ?compare@iterator@?$reserved_dlist@VtrRefEntity@@@@QBE_NABV12@@Z @ 0x6EA4A0
    };
    void validate() const;           // ?validate@?$reserved_dlist@VtrRefEntity@@@@QBEXXZ @ 0x6EA1A0
    dlist_node* get_head();          // ?get_head@?$reserved_dlist@VtrRefEntity@@@@QAEPAUdlist_node@1@XZ @ 0x6EA630
    bool empty() const;              // ?empty@?$reserved_dlist@VtrRefEntity@@@@QBE_NXZ @ 0x6EA640
    void clear();                    // ?clear@?$reserved_dlist@VtrRefEntity@@@@QAEXXZ @ 0x6E8CA0
    iterator end();                  // ?end@?$reserved_dlist@VtrRefEntity@@@@QAE?AViterator@1@XZ @ 0x6EC5C0
    static T* node_to_object(dlist_node* node);  // ?node_to_object@?$reserved_dlist@VtrRefEntity@@@@SAPAVtrRefEntity@@PAUdlist_node@1@@Z @ 0x6EA190
};
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
    T* _Myfirst;                                  // +0x00
    T* _Mylast;                                   // +0x04
    T* _Myend;                                    // +0x08
    vector();                                     // ??0?$vector@...@@std@@QAE@XZ
    class iterator {
    public:
        T* _Myptr;                                // +0x00
        iterator(T* _Ptr);                        // ??0iterator@?$vector@...@@std@@QAE@PAU...@@@Z
        T& operator*() const { return *_Myptr; }  // ??Diterator@?$vector@...@@std@@QBEAAU...@@XZ
        iterator& operator++() { ++_Myptr; return *this; }  // ??Eiterator@...@@QAEAAV012@XZ
        T* operator->() const { return _Myptr; }  // ??Citerator@...@@QBEPAU...@@XZ
        iterator operator+(int _Off) const;       // ??Hiterator@...@@QBE?AV012@H@Z
        iterator& operator+=(int _Off);           // ??Yiterator@...@@QAEAAV012@H@Z
    };
    class const_iterator {
    public:
        T* _Myptr;                                // +0x00
        const_iterator(T* _Ptr);                  // ??0const_iterator@?$vector@...@@std@@QAE@PAU...@@@Z
        const T& operator*() const { return *_Myptr; }  // ??Dconst_iterator@...@@std@@QBEABU...@@XZ
        bool operator==(const const_iterator& _Right) const;  // ??8const_iterator@...@@std@@QBE_NABV012@@Z
        bool operator!=(const const_iterator& _Right) const;  // ??9const_iterator@...@@std@@QBE_NABV012@@Z
    };
    unsigned int size() const;                    // ?size@?$vector@...@@std@@QBEIXZ
    unsigned int capacity() const;                // ?capacity@?$vector@...@@std@@QBEIXZ
    iterator begin();                             // ?begin@?$vector@...@@std@@QAE?AViterator@12@XZ
    iterator end();                               // ?end@?$vector@...@@std@@QAE?AViterator@12@XZ
    void resize(unsigned int _Newsize);           // ?resize@?$vector@...@@std@@QAEXI@Z
    void resize(unsigned int _Newsize, T _Val);   // 3-arg form (defined elsewhere)
    unsigned int max_size() const                 // ?max_size@?$vector@...@@std@@QBEIXZ
    {
        return (unsigned int)(((size_t)-1) / sizeof(T));
    }
protected:
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
    uint8_t _pad[0x14];
    unsigned int Color;                // +0x14
};
template <class T, class I>
class nglMeshIterator {
public:
    uint8_t _pad[0x0C];
    T* vertex;                         // +0x0C
    void WriteColor(unsigned int color);   // ?WriteColor@?$nglMeshIterator@UcdDynamicDecalVertex@@G@@QAEXI@Z @ 0x6E9C40
    void WriteFlush();                     // ?WriteFlush@?$nglMeshIterator@UcdDynamicDecalVertex@@G@@QAEXXZ @ 0x6E9CC0
};
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
    m_tail = &m_head;
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
struct cdWheelMarkVertex {
    uint8_t _pad[0x10];
};
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

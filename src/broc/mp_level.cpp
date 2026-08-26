// ============================================================================
// mp_level — multiplayer level loading and BrocSys script initialization
// From mp_level.xboxd:mp_level_wad.o (20 funcs)
// ea: 0xC8F720-0xC95CC0 (MP_LEVEL segment, rwx data)
// ============================================================================

#include "engine/broc_types.h"
#include "game/AeThreadFunctor.h"
#include "mp_util_wad.h"

#include <cstring>
#include <new>
#include <unordered_map>

#include "mp_level_ee_resolver.inc"

// mp_level's public entry-point ABI uses the global game-side BrocAPI and
// BrocExports declarations.  The complete IDA-derived layouts live in the
// Broc namespace, so InitScript uses those views when installing callbacks.
struct BrocAPI;
struct BrocExports;

extern BrocAPI* gpBrocAPI;
extern BrocExports gBrocExports;

namespace BrocHelper {
void Init();
void RegisterBroFunc(char* name, unsigned int (__cdecl* func)(void*));
}

namespace BrocSys {
void ValidateApiSize(int sizeofBrocAPI, int sizeofBrocExports);
}

namespace Broc {

// ea: 0x00925400. IDA forwards the functor entity and creation request to the
// runtime's internal thread-create callback.
unsigned int thread_create(bool createHandle, const char* file, int line,
                           const char* func, AeThreadFunctor* functor)
{
    const unsigned int ent = functor->GetEnt();
    return gBrocAPI.mThreadCreateInternal(file, line, func, ent, functor,
                                          createHandle);
}

} // namespace Broc

namespace _mp_loadout {
void main();
void* SpecialClassAudio__functor(Broc::entity player);
}

namespace _mp_audio {
void* ThreadLineSound__functor(Broc::entity self);
void* ThreadStaticSound__functor(Broc::entity self);
void* sound_repeat__functor(Broc::entity self);
void* interior_triggering_device__functor(Broc::entity trigger,
                                           Broc::entity other);
}

namespace _mp_hq {
void* TriggerRadio__functor(Broc::entity self);
}

namespace _mp_ctf {
void* PickupFlag__functor(Broc::entity self);
void* Goal__functor(Broc::entity self);
}

namespace _mp_scf {
void* Goal__functor(Broc::entity self, Broc::entity triggerer);
void* PickupFlag__functor(Broc::entity self, Broc::entity triggerer);
}

namespace _mp_tankdrive {
void* death__functor(Broc::entity self, Broc::entity attacker);
void* inactivity_blowup__functor(Broc::entity self);
void* fire__functor(Broc::entity self);
void* damage__functor(Broc::entity self, Broc::bint damage,
                      Broc::entity attacker, Broc::bint mod);
}

namespace _mp_war {
void* WAR_TouchFlag__functor(Broc::entity self);
}

namespace _mp_common {
void LaunchGametype();
}

namespace BrocSys {
unsigned int GetLevel();
}

namespace mp_level_wad {

// IDA types: mp_level_wad::Level derives from mp_util_wad::Level and adds
// one byte of level-local flags; mp_level_wad::Anim wraps mp_util_wad::Anim.
struct Level : mp_util_wad::Level {
    unsigned char flags;
};

struct Anim : mp_util_wad::Anim {
};

static_assert(sizeof(Level) == 0x234, "mp_level_wad::Level size mismatch");
static_assert(sizeof(Anim) == 0x08, "mp_level_wad::Anim size mismatch");

Level* pLevel = nullptr;
Anim* pAnim = nullptr;

} // namespace mp_level_wad

// ============================================================================
// Animation & hash string registry
// ============================================================================

namespace mp_level_wad {

// ea: 0xC8F720
void RegisterHashStrings()
{
    // The release entry point is a thin forwarder into mp_util_wad.  That
    // routine registers both the multiplayer strings and animation hashes.
    mp_util_wad::RegisterHashStrings();
}

// ea: 0xC8F740
unsigned int ResolveAnim(unsigned int treename, unsigned int animname,
                         unsigned int* getVal, unsigned int setVal)
{
    return static_cast<unsigned int>(mp_util_wad::ResolveAnim(
        treename, animname, getVal, setVal));
}

// ea: 0xC8F790
const char* ResolveAnimName(unsigned int anim)
{
    return mp_util_wad::ResolveAnimName(anim);
}

// ea: 0xC8F7D0
bool ValidateAnimationIndices()
{
    // The release wrapper performs this validation twice; the first call's
    // return value is intentionally discarded before returning the second.
    mp_util_wad::ValidateAnimationIndices();
    return mp_util_wad::ValidateAnimationIndices();
}

// ea: 0xC8F830
unsigned int GetBroAnim(unsigned int treename, unsigned int animname)
{
    return mp_util_wad::GetBroAnim(treename, animname);
}

// ea: 0xC8F880
bool GetNextAnimTree(int index, char* treename, int bufSize)
{
    const char* animtrees[1] = { "generic_human" };
    return index < 1 && std::strncpy(treename, animtrees[index], bufSize) != nullptr;
}

} // namespace mp_level_wad

// ============================================================================
// Script thread management
// ============================================================================

namespace mp_level_wad {

unsigned int SpawnScriptThread(unsigned int fcn, bool createHandle,
                               Broc::entity self, Broc::entity ent1,
                               Broc::entity ent2, float f1, float f2) { // ea: 0xC8F8D0
    (void)ent2;

    // The generated release dispatcher uses a fixed source path and the
    // generated line number for each functor.  The line is diagnostic only,
    // but retaining the release offsets keeps script error reports useful.
    const char* const file =
        "c:\\cod\\code\\script\\gen\\mp_level_wad.cpp";
    const auto create = [&](void* raw, int line, const char* name) {
        const unsigned int handle = Broc::thread_create(
            createHandle, file, line, name,
            reinterpret_cast<AeThreadFunctor*>(raw));
        return createHandle ? handle : static_cast<unsigned int>(-1);
    };

    switch (fcn) {
    case 0x886ABA4Au:
        return create(_mp_audio::ThreadLineSound__functor(self), 5,
                      "_mp_audio::ThreadLineSound");
    case 0xD1D46C1Du:
        return create(_mp_tankdrive::death__functor(self, ent1), 38,
                      "_mp_tankdrive::death");
    case 0xBFA4248Bu:
        return create(_mp_hq::TriggerRadio__functor(self), 23,
                      "_mp_hq::TriggerRadio");
    case 0xCF389A90u:
        return create(_mp_loadout::SpecialClassAudio__functor(self), 26,
                      "_mp_loadout::SpecialClassAudio");
    case 0xD19F242Du:
        return create(_mp_audio::interior_triggering_device__functor(self, ent1),
                      11, "_mp_audio::interior_triggering_device");
    case 0xD84CF81Fu:
        return create(_mp_war::WAR_TouchFlag__functor(self), 47,
                      "_mp_war::WAR_TouchFlag");
    case 0xE2069A52u:
        return create(_mp_ctf::PickupFlag__functor(self), 20,
                      "_mp_ctf::PickupFlag");
    case 0xEFA1CD4Au:
        return create(_mp_audio::ThreadStaticSound__functor(self), 8,
                      "_mp_audio::ThreadStaticSound");
    case 0x60037BCAu:
        return create(_mp_audio::sound_repeat__functor(self), 14,
                      "_mp_audio::sound_repeat");
    case 0x75A5F0EFu:
        return create(_mp_ctf::Goal__functor(self), 17, "_mp_ctf::Goal");
    case 0x80A56673u:
        return create(_mp_tankdrive::inactivity_blowup__functor(self), 44,
                      "_mp_tankdrive::inactivity_blowup");
    case 0x446C74FDu:
        return create(_mp_tankdrive::fire__functor(self), 41,
                      "_mp_tankdrive::fire");
    case 0x0C1FD1B6u: {
        const Broc::bint damage(static_cast<int>(f1));
        const Broc::bint mod(static_cast<int>(f2));
        return create(_mp_tankdrive::damage__functor(self, damage, ent1, mod),
                      35, "_mp_tankdrive::damage");
    }
    case 0x0D0DDE0Eu:
        return create(_mp_scf::Goal__functor(self, ent1), 29,
                      "_mp_scf::Goal");
    case 0x2739A2B1u:
        return create(_mp_scf::PickupFlag__functor(self, ent1), 32,
                      "_mp_scf::PickupFlag");
    default:
        return 0;
    }
}

} // namespace mp_level_wad

namespace mp_level_wad {

// ea: 0xC901E0
bool ScriptThreadExists(unsigned int fcn)
{
    bool result = false;
    if (fcn > 0x886ABA4Au) {
        if (fcn > 0xD1D46C1Du) {
            if (fcn == 0xD84CF81Fu || fcn == 0xE2069A52u || fcn == 0xEFA1CD4Au)
                return true;
        } else if (fcn == 0xD1D46C1Du || fcn == 0xBFA4248Bu ||
                   fcn == 0xCF389A90u || fcn == 0xD19F242Du) {
            return true;
        }
    } else {
        if (fcn == 0x886ABA4Au)
            return true;
        if (fcn <= 0x446C74FDu) {
            if (fcn != 0x446C74FDu && fcn != 0x0C1FD1B6u &&
                fcn != 0x0D0DDE0Eu && fcn != 0x2739A2B1u)
                return result;
            return true;
        }
        if (fcn == 0x60037BCAu || fcn == 0x75A5F0EFu || fcn == 0x80A56673u)
            return true;
    }
    return result;
}

} // namespace mp_level_wad

// ============================================================================
// Level lifecycle
// ============================================================================

void mp_level_main() {}           // ea: 0xC94D50
void mp_level_InternalMain() {}   // ea: 0xC95160
void mp_level_Shutdown() {}       // ea: 0xC94FA0

namespace mp_level {
mp_level_wad::Level* level = nullptr;
mp_level_wad::Anim* anim = nullptr;
static int sNumInsts = 0;

// The release keeps one NodeFieldManager singleton for each node-handle/value
// pair.  The manager is simply a packed (node,key) -> value map; the generated
// script callbacks below expose the same behavior to BrocExports.
template <typename T> class NodeFieldManager {
public:
    void SetField(int node, unsigned int key, const T& value)
    {
        if (node == -1) {
            if (Broc::gBrocAPI.mWarning(
                    "c:\\cod\\code\\script\\include\\nodefieldmanager.h",
                    43, "Setting node field on invalid node!")) {
                __debugbreak();
            }
            return;
        }
        mNodeFields[Pack(node, key)] = value;
    }

    T* GetField(T* result, int node, unsigned int key) const
    {
        if (node == -1) {
            *result = T();
            return result;
        }
        const auto it = mNodeFields.find(Pack(node, key));
        *result = it == mNodeFields.end() ? T() : it->second;
        return result;
    }

private:
    static unsigned long long Pack(int node, unsigned int key)
    {
        return (static_cast<unsigned long long>(key) << 32) |
               static_cast<unsigned int>(node);
    }

    std::unordered_map<unsigned long long, T> mNodeFields;
};

template <>
Broc::string* NodeFieldManager<Broc::string>::GetField(
    Broc::string* result, int node, unsigned int key) const
{
    if (node == -1) {
        ::new (result) Broc::string();
        return result;
    }
    const auto it = mNodeFields.find(Pack(node, key));
    if (it == mNodeFields.end())
        ::new (result) Broc::string();
    else
        ::new (result) Broc::string(it->second);
    return result;
}

template <typename Handle, typename T>
NodeFieldManager<T>* GetNfmInst(bool shutdown)
{
    static NodeFieldManager<T>* instance = nullptr;
    if (instance == nullptr && !shutdown) {
        instance = new (std::nothrow) NodeFieldManager<T>();
        if (instance != nullptr)
            ++sNumInsts;
    }
    if (shutdown && instance != nullptr) {
        delete instance;
        instance = nullptr;
        --sNumInsts;
    }
    return instance;
}

// ea: 0xC94D50
void main()
{
    _mp_loadout::main();
    _mp_common::LaunchGametype();
}

// ea: 0xC94F80
void MainThreadHook(Broc::entity)
{
    main();
}

// ea: 0xC95160
void InternalMain()
{
    level = new (std::nothrow) mp_level_wad::Level();
    if (level == nullptr) {
        if (Broc::gBrocAPI.mAssert(
                "c:\\cod\\code\\script\\include\\entrypoint.inl", 0,
                "level's memory was not allocated")) {
            __debugbreak();
        }
        return;
    }

    std::memset(&level->flags, 0, sizeof(level->flags));
    mp_level_wad::pLevel = level;
    mp_anim_wad::pLevel = &level->_base;
    mp_util_wad::pLevel = static_cast<mp_util_wad::Level*>(level);

    anim = new (std::nothrow) mp_level_wad::Anim();
    if (anim == nullptr) {
        if (Broc::gBrocAPI.mAssert(
                "c:\\cod\\code\\script\\include\\entrypoint.inl", 0,
                "anim's memory was not allocated")) {
            __debugbreak();
        }
        return;
    }

    mp_level_wad::pAnim = anim;
    mp_anim_wad::pAnim = &anim->_base;
    mp_util_wad::pAnim = static_cast<mp_util_wad::Anim*>(anim);
    level->_base.entity = Broc::entity(BrocSys::GetLevel());

    void* storage = AeThreadFunctor::operator new(
        sizeof(AeThreadFunctor1<Broc::entity>));
    AeThreadFunctor* functor = storage != nullptr
        ? ::new (storage) AeThreadFunctor1<Broc::entity>(
              mp_level::MainThreadHook, level->_base.entity)
        : nullptr;
    const unsigned int handle = Broc::gBrocAPI.mThreadCreateInternal(
        "c:\\cod\\code\\script\\include\\entrypoint.inl", 0,
        "InternalMain", level->_base.entity.GetHandle(), functor, true);
    Broc::gBrocAPI.mBrocExports.mMainThreadHandle = handle;
}

void SetPNodeFieldString(unsigned int node, unsigned int key,
                         Broc::string value)
{
    GetNfmInst<Broc::TPathnodeHandle, Broc::string>(false)->SetField(
        static_cast<int>(node), key, value);
}

void SetPNodeFieldInt(unsigned int node, unsigned int key, int value)
{
    GetNfmInst<Broc::TPathnodeHandle, Broc::bint>(false)->SetField(
        static_cast<int>(node), key, Broc::bint(value));
}

void SetPNodeFieldFloat(unsigned int node, unsigned int key, float value)
{
    GetNfmInst<Broc::TPathnodeHandle, Broc::bfloat>(false)->SetField(
        static_cast<int>(node), key, Broc::bfloat(value));
}

void SetPNodeFieldPathnode(unsigned int node, unsigned int key,
                           Broc::pathnode value)
{
    GetNfmInst<Broc::TPathnodeHandle, Broc::pathnode>(false)->SetField(
        static_cast<int>(node), key, value);
}

Broc::string* GetPNodeFieldString(Broc::string* result, unsigned int node,
                                  unsigned int key)
{
    return GetNfmInst<Broc::TPathnodeHandle, Broc::string>(false)->GetField(
        result, static_cast<int>(node), key);
}

int GetPNodeFieldInt(unsigned int node, unsigned int key)
{
    Broc::bint result;
    GetNfmInst<Broc::TPathnodeHandle, Broc::bint>(false)->GetField(
        &result, static_cast<int>(node), key);
    return result.mVal;
}

float GetPNodeFieldFloat(unsigned int node, unsigned int key)
{
    Broc::bfloat result;
    GetNfmInst<Broc::TPathnodeHandle, Broc::bfloat>(false)->GetField(
        &result, static_cast<int>(node), key);
    return result.mVal;
}

Broc::pathnode* GetPNodeFieldPathnode(Broc::pathnode* result,
                                      unsigned int node, unsigned int key)
{
    return GetNfmInst<Broc::TPathnodeHandle, Broc::pathnode>(false)->GetField(
        result, static_cast<int>(node), key);
}

void SetVNodeFieldString(unsigned int node, unsigned int key,
                         Broc::string value)
{
    GetNfmInst<Broc::TVehiclenodeHandle, Broc::string>(false)->SetField(
        static_cast<int>(node), key, value);
}

void SetVNodeFieldInt(unsigned int node, unsigned int key, int value)
{
    GetNfmInst<Broc::TVehiclenodeHandle, Broc::bint>(false)->SetField(
        static_cast<int>(node), key, Broc::bint(value));
}

void SetVNodeFieldFloat(unsigned int node, unsigned int key, float value)
{
    GetNfmInst<Broc::TVehiclenodeHandle, Broc::bfloat>(false)->SetField(
        static_cast<int>(node), key, Broc::bfloat(value));
}

void SetVNodeFieldVehiclenode(unsigned int node, unsigned int key,
                              Broc::vehiclenode value)
{
    GetNfmInst<Broc::TVehiclenodeHandle, Broc::vehiclenode>(false)->SetField(
        static_cast<int>(node), key, value);
}

Broc::string* GetVNodeFieldString(Broc::string* result, unsigned int node,
                                  unsigned int key)
{
    return GetNfmInst<Broc::TVehiclenodeHandle, Broc::string>(false)->GetField(
        result, static_cast<int>(node), key);
}

int GetVNodeFieldInt(unsigned int node, unsigned int key)
{
    Broc::bint result;
    GetNfmInst<Broc::TVehiclenodeHandle, Broc::bint>(false)->GetField(
        &result, static_cast<int>(node), key);
    return result.mVal;
}

float GetVNodeFieldFloat(unsigned int node, unsigned int key)
{
    Broc::bfloat result;
    GetNfmInst<Broc::TVehiclenodeHandle, Broc::bfloat>(false)->GetField(
        &result, static_cast<int>(node), key);
    return result.mVal;
}

Broc::vehiclenode* GetVNodeFieldVehiclenode(Broc::vehiclenode* result,
                                            unsigned int node,
                                            unsigned int key)
{
    return GetNfmInst<Broc::TVehiclenodeHandle, Broc::vehiclenode>(false)
        ->GetField(result, static_cast<int>(node), key);
}

void Shutdown() {}                // ea: 0xC94FA0
void hack_ps2_InitScript(Broc::BrocExports& exports) // ea: 0xC94D70
{
    exports.mSetPNodeField_string = SetPNodeFieldString;
    exports.mGetPNodeField_string = GetPNodeFieldString;
    exports.mSetPNodeField_int = SetPNodeFieldInt;
    exports.mGetPNodeField_int = GetPNodeFieldInt;
    exports.mSetPNodeField_float = SetPNodeFieldFloat;
    exports.mGetPNodeField_float = GetPNodeFieldFloat;
    exports.mSetPNodeField_pathnode = SetPNodeFieldPathnode;
    exports.mGetPNodeField_pathnode = GetPNodeFieldPathnode;
    exports.mSetVNodeField_string = SetVNodeFieldString;
    exports.mGetVNodeField_string = GetVNodeFieldString;
    exports.mSetVNodeField_int = SetVNodeFieldInt;
    exports.mGetVNodeField_int = GetVNodeFieldInt;
    exports.mSetVNodeField_float = SetVNodeFieldFloat;
    exports.mGetVNodeField_float = GetVNodeFieldFloat;
    exports.mSetVNodeField_vehiclenode = SetVNodeFieldVehiclenode;
    exports.mGetVNodeField_vehiclenode = GetVNodeFieldVehiclenode;
}
}

void hack_ps2_InitScript(Broc::BrocExports& exports)
{
    mp_level::hack_ps2_InitScript(exports);
}
void AnimNamespaceVariableResolver(int, int, int, int) {} // ea: 0xC94E50

void BrocAnimInitialize() {}      // ea: 0xC94E90
void BrocAnimCleanup() {}         // ea: 0xC94EB0
unsigned BrocAnimResolver(const char*, const char*) { return 0; } // ea: 0xC94ED0
void BrocAnimDebug(Broc::entity) {} // ea: 0xC94F60
void MainThreadHook(Broc::entity) {} // ea: 0xC94F80

namespace mp_level {

// ea: 0xC94E50
void AnimNamespaceVariableResolver(int treename, int tree_index,
                                   int animname, int index)
{
    mp_level_wad::ResolveAnim(static_cast<unsigned int>(treename),
                              static_cast<unsigned int>(animname),
                              nullptr,
                              static_cast<unsigned int>(index + (tree_index << 16)));
}

// ea: 0xC94E90, 0xC94EB0, 0xC94F60
void BrocAnimInitialize() {}
void BrocAnimCleanup() {}
void BrocAnimDebug(Broc::entity) {}

// ea: 0xC94ED0
unsigned int BrocAnimResolver(const char* treename, const char* animname)
{
    HashStr tree;
    HashStr anim;
    unsigned int val = 0;
    Broc::string_hash(&tree, treename);
    Broc::string_hash(&anim, animname);
    if (mp_level_wad::ResolveAnim(tree.mVal, anim.mVal, &val, 0) != 0)
        return val;
    return 0;
}

} // namespace mp_level

// ============================================================================
// InitScript — entry point called by the engine at level load
// ============================================================================
typedef void (*InitFunc)();

namespace mp_level {

InitFunc InitScript(::BrocAPI** gamesAPIptr, ::BrocExports& exports) { // ea: 0xC95CC0
    Broc::BrocExports& exportsView =
        *reinterpret_cast<Broc::BrocExports*>(&exports);

    exportsView.mInit();
    *gamesAPIptr = reinterpret_cast<::BrocAPI*>(&Broc::gBrocAPI);
    Broc::gBrocAPI.mKillThread = false;
    exportsView.mValidateApiSize(4924, 456);
    Broc::ExtendedEntity::sGetFunctions =
        Broc::EEHelper::mp_level_wad_EEGetFunctions;
    Broc::ExtendedEntity::InitScript(exportsView);
    hack_ps2_InitScript(exportsView);
    exportsView.mRegisterDebugStrings = mp_level_wad::RegisterHashStrings;
    exportsView.mAnimIndexResolver = AnimNamespaceVariableResolver;
    exportsView.mAnimIndexValidate = mp_level_wad::ValidateAnimationIndices;
    exportsView.mGetNextAnimtree = mp_level_wad::GetNextAnimTree;
    exportsView.mAnimGetBroValue = mp_level_wad::GetBroAnim;
    exportsView.mAnimInitialize = BrocAnimInitialize;
    exportsView.mAnimCleanup = BrocAnimCleanup;
    exportsView.mAnimResolver = BrocAnimResolver;
    exportsView.mAnimNameResolver = mp_level_wad::ResolveAnimName;
    exportsView.mAnimDebug = BrocAnimDebug;
    exportsView.mShutdown = Shutdown;
    exportsView.mSpawnScriptThread =
        reinterpret_cast<decltype(exportsView.mSpawnScriptThread)>(
            mp_level_wad::SpawnScriptThread);
    exportsView.mScriptThreadExists = mp_level_wad::ScriptThreadExists;
    exportsView.mThreadExecute = Broc::ThreadExecute;
    return InternalMain;
}

} // namespace mp_level

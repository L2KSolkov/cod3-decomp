// ============================================================================
// mp_level — multiplayer level loading and BrocSys script initialization
// From mp_level.xboxd:mp_level_wad.o (20 funcs)
// ea: 0xC8F720-0xC95CC0 (MP_LEVEL segment, rwx data)
// ============================================================================

#include "engine/broc_types.h"
#include "game/AeThreadFunctor.h"

#include <cstring>

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

// ============================================================================
// Animation & hash string registry
// ============================================================================

namespace mp_level_wad {

// ea: 0xC8F720
void RegisterHashStrings()
{
    // The callee belongs to the mp_util_wad family; keep this object-local
    // entry point until that family is linked into the port.
}

// ea: 0xC8F740
unsigned int ResolveAnim(unsigned int treename, unsigned int animname,
                         unsigned int* getVal, unsigned int setVal)
{
    (void)treename;
    (void)animname;
    (void)getVal;
    (void)setVal;
    return false;
}

// ea: 0xC8F790
const char* ResolveAnimName(unsigned int anim)
{
    (void)anim;
    return nullptr;
}

// ea: 0xC8F7D0
bool ValidateAnimationIndices()
{
    return true;
}

// ea: 0xC8F830
unsigned int GetBroAnim(unsigned int treename, unsigned int animname)
{
    (void)treename;
    (void)animname;
    return 0;
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
    (void)fcn;
    (void)createHandle;
    (void)self;
    (void)ent1;
    (void)ent2;
    (void)f1;
    (void)f2;
    return 0;
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
void InternalMain() {}            // ea: 0xC95160
void Shutdown() {}                // ea: 0xC94FA0
void hack_ps2_InitScript(Broc::BrocExports&) {} // ea: 0xC94D70
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

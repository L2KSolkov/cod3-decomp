// ============================================================================
// mp_level — multiplayer level loading and BrocSys script initialization
// From mp_level.xboxd:mp_level_wad.o (20 funcs)
// ea: 0xC8F720-0xC95CC0 (MP_LEVEL segment, rwx data)
// ============================================================================

#include <cstdint>

// Forward types
namespace Broc {
    struct entity { uint32_t handle; };
    struct vector { float x, y, z; };
    struct BrocAPI {};
    struct BrocExports {};
}

// ============================================================================
// Animation & hash string registry
// ============================================================================

void RegisterHashStrings() {} // ea: 0xC8F720

unsigned ResolveAnim(unsigned type, unsigned index1, unsigned* out, unsigned index2) { // ea: 0xC8F740
    return 0;
}

const char* ResolveAnimName(unsigned index) { // ea: 0xC8F790
    return nullptr;
}

bool ValidateAnimationIndices() { // ea: 0xC8F7D0
    return true;
}

unsigned GetBroAnim(unsigned index1, unsigned index2) { // ea: 0xC8F830
    return 0;
}

bool GetNextAnimTree(int handle, char* buf, int size) { // ea: 0xC8F880
    return false;
}

// ============================================================================
// Script thread management
// ============================================================================

unsigned SpawnScriptThread(unsigned id, bool flag, Broc::entity ent,
                           const Broc::vector* v1, const Broc::vector* v2,
                           float a, float b, float c, Broc::vector* out) { // ea: 0xC8F8D0
    return 0;
}

bool ScriptThreadExists(unsigned id) { // ea: 0xC901E0
    return false;
}

// ============================================================================
// Level lifecycle
// ============================================================================

void mp_level_main() {}           // ea: 0xC94D50
void mp_level_InternalMain() {}   // ea: 0xC95160
void mp_level_Shutdown() {}       // ea: 0xC94FA0
void hack_ps2_InitScript(Broc::BrocExports&) {} // ea: 0xC94D70
void AnimNamespaceVariableResolver(int, int, int, int) {} // ea: 0xC94E50

void BrocAnimInitialize() {}      // ea: 0xC94E90
void BrocAnimCleanup() {}         // ea: 0xC94EB0
unsigned BrocAnimResolver(const char*, const char*) { return 0; } // ea: 0xC94ED0
void BrocAnimDebug(Broc::entity) {} // ea: 0xC94F60
void MainThreadHook(Broc::entity) {} // ea: 0xC94F80

// ============================================================================
// InitScript — entry point called by the engine at level load
// ============================================================================
typedef void (*InitFunc)();

InitFunc InitScript(Broc::BrocAPI**, Broc::BrocExports&) { // ea: 0xC95CC0
    return mp_level_main;
}

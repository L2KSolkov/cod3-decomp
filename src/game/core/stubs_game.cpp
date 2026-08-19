// AUTO-GENERATED STUBS — Game core systems (core.o + CallFunctor.o)
// 0 non-inline functions to port
// When ported, functions move from here to their real .cpp files.

#include <stdio.h>

enum TPakId { kPakTypeLevel = 0, kPakTypeNone = -1 };

#define COD3_UNIMPLEMENTED(lib) \
    fprintf(stderr, "COD3 UNIMPLEMENTED: %s\n", lib)

void __cod3_stub_game_core(void) {
    COD3_UNIMPLEMENTED("game_core");
}

// Singleton CreateInst/DeleteInst stubs (real symbols are static members in
// core.o: ?CreateInst@<Class>@@SAXXZ / ?DeleteInst@<Class>@@SAXXZ). Ported
// into the owning class files as each singleton is reconstructed.
class BankManager { public: static void CreateInst(); static void DeleteInst(); };
class LightGridMgr { public: static void DeleteInst(); };
class XModelManager { public: static void DeleteInst(); };
class XModelPartsManager { public: static void DeleteInst(); };
class DestructibleBankManager { public: static void CreateInst(); static void DeleteInst(); };
class PhysDataBankManager { public: static void CreateInst(); static void DeleteInst(); };
class AITypeManager { public: static void DeleteInst(); };
class SoundDevice { public: static void CreateInst(); static void DeleteInst(); };
class AudioBankMgr { public: static void CreateInst(); static void DeleteInst(); };
class SoundMediaMgr { public: static void CreateInst(); static void DeleteInst(); };
struct MusicMgr { public: static void CreateInst(); static void DeleteInst(); };
class StreamZoneManager { public: static void CreateInst(); static void DeleteInst(); };
class DbTablesetMgr { public: static void CreateInst(); static void DeleteInst(); };
class EffectEventSys { public: static void CreateInst(); static void DeleteInst(); };
class GdbFileManager { public: static void CreateInst(); static void DeleteInst(); };
class DialogueManager { public: static void CreateInst(); static void DeleteInst(); };
class EntityManager { public: static EntityManager* CreateInst(); static void DeleteInst(); };
class SceneManager { public: static void DeleteInst(); };
class ConfigStringManager { public: static void CreateInst(); static void DeleteInst(); };
class PathNodeMgr { public: static void CreateInst(); static void DeleteInst(); };
class STBManager { public: static void CreateInst(); static void DeleteInst(); };
class CtrlIcon { public: static void CreateInst(); static void DeleteInst(); };
class MultiplayerMgr { public: static void CreateInst(); static void DeleteInst(); };
class CheckpointMgr { public: static void CreateInst(); static void DeleteInst(); };
class SplineMgr { public: static void CreateInst(); static void DeleteInst(); };
class SmokeGrenadeMgr { public: static void CreateInst(); };
class CGBankManager { public: static void DeleteInst(); };
class DCGBankManager { public: static void DeleteInst(); };
struct AnimBank;
class AnimBankManager {
public:
    static void CreateInst();
    AnimBank* GetBank(TPakId pak_id);  // ?GetBank@AnimBankManager@@QAEPAUAnimBank@@W4TPakId@@@Z
};
class RumbleManager { public: static void CreateInst(); static void DeleteInst(); };
class InteractionController { public: static void CreateInst(); static void DeleteInst(); };
class BinFileManager { public: static void CreateInst(); static void DeleteInst(); };
class CurveManager { public: static void CreateInst(); static void DeleteInst(); };
class PlayerAnimMgr { public: static void CreateInst(); static void DeleteInst(); };
class DynamicDecalMgr { public: static void CreateInst(); static void DeleteInst(); };

void LightGridMgr::DeleteInst() {}
void XModelManager::DeleteInst() {}
void XModelPartsManager::DeleteInst() {}
void DestructibleBankManager::CreateInst() {}
void DestructibleBankManager::DeleteInst() {}
void PhysDataBankManager::CreateInst() {}
void PhysDataBankManager::DeleteInst() {}
void AITypeManager::DeleteInst() {}
void SoundDevice::DeleteInst() {}
void AudioBankMgr::DeleteInst() {}
void SoundMediaMgr::CreateInst() {}
void SoundMediaMgr::DeleteInst() {}
void MusicMgr::CreateInst() {}
void MusicMgr::DeleteInst() {}
void DbTablesetMgr::CreateInst() {}
void DbTablesetMgr::DeleteInst() {}
void EffectEventSys::CreateInst() {}
void EffectEventSys::DeleteInst() {}
void GdbFileManager::CreateInst() {}
void GdbFileManager::DeleteInst() {}
void DialogueManager::CreateInst() {}
void DialogueManager::DeleteInst() {}
void EntityManager::DeleteInst() {}
void SceneManager::DeleteInst() {}
void ConfigStringManager::CreateInst() {}
void ConfigStringManager::DeleteInst() {}
void PathNodeMgr::CreateInst() {}
void PathNodeMgr::DeleteInst() {}
void CtrlIcon::CreateInst() {}
void CtrlIcon::DeleteInst() {}
void MultiplayerMgr::DeleteInst() {}
void CheckpointMgr::CreateInst() {}
void CheckpointMgr::DeleteInst() {}
void SplineMgr::CreateInst() {}
void SplineMgr::DeleteInst() {}
void SmokeGrenadeMgr::CreateInst() {}
void CGBankManager::DeleteInst() {}
void DCGBankManager::DeleteInst() {}
void AnimBankManager::CreateInst() {}
AnimBank* AnimBankManager::GetBank(TPakId pak_id)
{
    (void)pak_id;
    return nullptr;
}
void* AnimBankManager_GetBank(void* self, TPakId pak_id)
{
    (void)self; (void)pak_id;
    return nullptr;
}
void* AnimBankManager_GetBank(void* self, int pak_id)
{
    (void)self; (void)pak_id;
    return nullptr;
}
void AnimBankManager_UnloadAll(void* self)
{
    (void)self;
}
void AnimBankManager_UnloadAll()
{
    // stub
}
void RumbleManager::CreateInst() {}
void RumbleManager::DeleteInst() {}
void InteractionController::CreateInst() {}
void InteractionController::DeleteInst() {}
void BinFileManager::CreateInst() {}
void BinFileManager::DeleteInst() {}
void CurveManager::CreateInst() {}
void CurveManager::DeleteInst() {}
void PlayerAnimMgr::CreateInst() {}
void PlayerAnimMgr::DeleteInst() {}
void DynamicDecalMgr::CreateInst() {}
void DynamicDecalMgr::DeleteInst() {}

// j_nullsub_N no-op stubs. IDA-invented names for linker thunks that jump to
// nullsubs (do-nothing functions); binary semantics are exactly no-ops.
class Entity;
class DObj;
struct actor_s;
struct weaponParms;
struct nglMeshSection;
struct ai_orient_t;
namespace math { class Position3; }

void j_nullsub_15(actor_s* a, Entity* b) { (void)a; (void)b; }
void j_nullsub_17(Entity* e, int a, int b, math::Position3* p, float f)
{
    (void)e; (void)a; (void)b; (void)p; (void)f;
}
void j_nullsub_20() {}
void j_nullsub_27(nglMeshSection* s) { (void)s; }
void j_nullsub_30(DObj* o, int* p) { (void)o; (void)p; }
void j_nullsub_33(const char* s) { (void)s; }
void j_nullsub_35() {}
void j_nullsub_37(actor_s* a, weaponParms* w) { (void)a; (void)w; }
void j_nullsub_47(weaponParms* w, const float* a, float* b)
{
    (void)w; (void)a; (void)b;
}
void j_nullsub_54(weaponParms* w, const float* a, float* b)
{
    (void)w; (void)a; (void)b;
}
void j_nullsub_57(actor_s* a) { (void)a; }
void j_nullsub_58(void* self, bool use) { (void)self; (void)use; }
void j_nullsub_60(actor_s* a) { (void)a; }
void j_nullsub_64(Entity* a, Entity* b) { (void)a; (void)b; }
void j_nullsub_67(nglMeshSection* s) { (void)s; }
void j_nullsub_72(int a, int b, float f) { (void)a; (void)b; (void)f; }
void j_nullsub_74(Entity* e, int a) { (void)e; (void)a; }
void j_nullsub_77(Entity* e) { (void)e; }
void j_nullsub_82(DObj* o, int* p) { (void)o; (void)p; }
void j_nullsub_82(void* o, int* p) { (void)o; (void)p; }
void j_nullsub_83(ai_orient_t* a, float f) { (void)a; (void)f; }
void j_nullsub_84(Entity* e, int a, int b, const float* c, const float* d,
                  float f)
{
    (void)e; (void)a; (void)b; (void)c; (void)d; (void)f;
}
void j_nullsub_86(int a) { (void)a; }
void j_nullsub_89(DObj* o, float f) { (void)o; (void)f; }
void j_nullsub_89(void* o, float f) { (void)o; (void)f; }
void j_nullsub_93() {}
void j_nullsub_117(unsigned int a, int b, const float* c, float d, float* e,
                   float* f, float* g)
{
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g;
}
void j_nullsub_118(actor_s* a) { (void)a; }
void j_nullsub_120(Entity* e) { (void)e; }
void j_nullsub_121(int a, int b, float f, int c)
{
    (void)a; (void)b; (void)f; (void)c;
}
void* SoundMediaMgr_j_nullsub_91(void* p)
{
    (void)p;
    return nullptr;
}

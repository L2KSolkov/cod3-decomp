// ============================================================================
// g_debugthread.cpp - DebugThread message helpers + Task/HealthRegenTask
// (game2.o). Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"
#include "game/logic/g_inspector.h"
#include "core/tlFixedString.h"
#include "core/ae_fixed_string.h"
#include "core/ae_array.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// DebugThread::Render support types (game2.o)
// ============================================================================
struct ai_funcs_s {
    void* pfnStart;          // +0x00
    void* pfnFinish;         // +0x04
    void* pfnSuspend;        // +0x08
    void* pfnResume;         // +0x0C
    void* pfnThink;          // +0x10
    void* pfnInterruptPoint; // +0x14
    void* pfnTouch;          // +0x18
    void* pfnPain;           // +0x1C
    void* pfnNodeClaimRevoked;   // +0x20
    void* pfnMoveAwayRequested;  // +0x24
    char  debugName[16];     // +0x28
};
const ai_funcs_s AIFuncTable[32] = {};  // ?AIFuncTable@@3QBUai_funcs_s@@B (mp_actors.o)

struct apsStats {
    int numActiveEffects;         // +0x00
    int maxRequestedBlockSize;    // +0x04
    int numActiveParticles;       // +0x08
    int maxActiveParticles;       // +0x0C
};
extern void apsGetStats(apsStats& stats);  // ?apsGetStats@@YAXAAUapsStats@@@Z (render.o)
extern bool apsGetPoolInfo(int nPool, int& size, int& capacity, int& used,
                           int& peak);  // ?apsGetPoolInfo@@YA_NHAAH000@Z (render.o)
extern void FX_ReportFX();               // ?FX_ReportFX@@YAXXZ (render.o)

template <typename A, typename B>
struct ae_pair {
    A first;   // +0x00
    B second;  // +0x04
};
struct ParticleEffect {
    static ae_sized_array<ae_pair<short, short>, 256> sArray;  // ?sArray@ParticleEffect@@2V?$ae_sized_array@V?$ae_pair@FF@@$0BAA@@@A (render.o)
};
ae_sized_array<ae_pair<short, short>, 256> ParticleEffect::sArray;  // render.o @ 0x13646A8

// AeThread / AeThreadManager list walk (sv_stubs.h owns AeThreadManager)
struct AeThread {
    void* mPrev;         // +0x00 (dlist node)
    void* mNext;         // +0x04
    unsigned int mOwner; // +0x08 (DbLinkedHandle mVal)
    void* mFunctor;      // +0x0C
    unsigned int mFlags; // +0x10 (Bitmask mVal)
    unsigned int mHandle;// +0x14
    unsigned char _pad18[0x44 - 0x18];
    const char* mFuncName;  // +0x44
    const char* mFile;      // +0x48
    int mLine;              // +0x4C

    void GetCondText(ae_fixed_string<64, unsigned char>& str);  // ?GetCondText@AeThread@@QAEXAAV?$ae_fixed_string@$0EA@E@@@Z
};

// AeThread::GetCondText (game2.o; stub)
void AeThread::GetCondText(ae_fixed_string<64, unsigned char>& str)
{
    (void)str;
}

extern void DisplayPoolTotals(PoolAllocator* pool);   // g_game2_misc.cpp
extern PoolAllocator* gCommonPoolAllocator;           // ?gCommonPoolAllocator@@3PAVPoolAllocator@@A
extern PoolAllocator* gAeThreadBackupStackAllocator;  // g_local.h
DbLinkedHandle<EntityHandleDb, Entity> g_renderUniqueIndex;  // ?g_renderUniqueIndex@@3V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@A (game2.o)
extern vmCvar_t memory_reportBrocPool;             // ?memory_reportBrocPool@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t memory_reportBrocBackupStackPool;  // ?memory_reportBrocBackupStackPool@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t memory_reportCommonPool;           // ?memory_reportCommonPool@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t memory_reportAepsStats;            // ?memory_reportAepsStats@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t memory_displayAepsStats;           // ?memory_displayAepsStats@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t memory_showStatistics;             // ?memory_showStatistics@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t g_debugProneCheck;                 // ?g_debugProneCheck@@3UvmCvar_t@@A (g.o)
extern vmCvar_t g_debugProneCheckDepthCheck;       // ?g_debugProneCheckDepthCheck@@3UvmCvar_t@@A (g.o)
int Actor_IsSuppressed(actor_s* pSelf)  // ?Actor_IsSuppressed@@YIHPAUactor_s@@@Z (mp_actors.o 0x77C6A0)
{
    (void)pSelf;
    return 0;
}
extern void Path_DrawDebugNode(const PathNodes::PathNode* pNode);  // ?Path_DrawDebugNode@@YAXPBUPathNode@PathNodes@@@Z (mp_actors.o)
float scaleScalar;  // ?scaleScalar@@3MA (render.o)
// ?RE_Text_Paint@@YAXMMHMQBMPBDMHH@Z (render.o; text renderer not ported yet)
void RE_Text_Paint(float x, float y, int font, float scale,
                   const float* color, const char* text, float a7,
                   int a8, int a9)
{
    (void)x; (void)y; (void)font; (void)scale; (void)color;
    (void)text; (void)a7; (void)a8; (void)a9;
}
extern int mem_get_high_used_bytes(mem_heap_type heap_name);  // ?mem_get_high_used_bytes@@YAHW4mem_heap_type@@@Z (mem_heap)

// ============================================================================
// nalGeneric local surface (animation/nal.cpp; used by AnimationPlayer)
// ============================================================================
struct nalPositionOrientationLocal {
    math::Position3 pos;    // +0x00
    math::Dir3 orient;      // +0x10
};
struct nalMatrix4x4Local {
    float m[4][4];          // +0x00
};

namespace nalGeneric {
class nalGenericBoneHandle {
public:
    unsigned int index;   // +0x00
    void* skeleton;       // +0x04
};
class nalGenericPose {
public:
    unsigned char* m_data;  // +0x00
    unsigned int m_size;    // +0x04
    nalGenericPose& operator=(const nalGenericPose& other);
    nalPositionOrientationLocal GetModelPositionOrientation(
        const nalGenericBoneHandle& handle) const;  // ?GetModelPositionOrientation@nalGenericPose@nalGeneric@@QBE?BVnalPositionOrientation@@ABVnalGenericBoneHandle@2@@Z
};
class nalGenericSkeleton {
public:
    void GetBoneHandle(nalGenericBoneHandle& handle,
                       const tlFixedString& boneName);  // ?GetBoneHandle@nalGenericSkeleton@nalGeneric@@QBEXAAVnalGenericBoneHandle@2@ABVtlFixedString@@@Z
};
void Blend(nalGenericPose& out, float blend, const nalGenericPose& a,
           const nalGenericPose& b);  // ?Blend@nalGeneric@@YAXAAVnalGenericPose@1@MABV21@1@Z
void BlendTorso(nalGenericPose& out, float blend, const nalGenericPose& a,
                const nalGenericPose& b);  // ?BlendTorso@nalGeneric@@YAXAAVnalGenericPose@1@MABV21@1@Z
}  // namespace nalGeneric

// nalGeneric stubs (animation/nal.cpp; port later)
namespace nalGeneric {
nalGenericPose& nalGenericPose::operator=(const nalGenericPose& other)
{
    (void)other;
    return *this;
}
nalPositionOrientationLocal nalGenericPose::GetModelPositionOrientation(
    const nalGenericBoneHandle& handle) const
{
    (void)handle;
    nalPositionOrientationLocal r = {};
    return r;
}
void nalGenericSkeleton::GetBoneHandle(nalGenericBoneHandle& handle,
                                       const tlFixedString& boneName)
{
    (void)handle; (void)boneName;
}
void Blend(nalGenericPose& out, float blend, const nalGenericPose& a,
           const nalGenericPose& b)
{
    (void)out; (void)blend; (void)a; (void)b;
}
void BlendTorso(nalGenericPose& out, float blend, const nalGenericPose& a,
                const nalGenericPose& b)
{
    (void)out; (void)blend; (void)a; (void)b;
}
}

extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);  // ?_tlAssert@@YA_NPBDH00@Z (tl_system.o)

// ============================================================================
// DebugThread message globals (game2.o data)
// ============================================================================
float gDebugThread_MessageRGB[3];
float gDebugThread_MessageScale;
float gDebugThread_MessageXpos;
char* gDebugThread_Message;  // ?gDebugThread_Message@@3PADA (cl.o)
int gDebugThread_MessageTicks;
float gDebugThread_MessageYpos;
int gDebugThread_MessageAlphaMin;

// SoundDevice::Sound (full view in sv_stubs.h, 0x3C) - IsFinished ported at
// ea 0x602940 in effect_events.cpp.

DbLinkedHandle<EntityHandleDb, Entity> g_SoundOnlyPlay;  // ?g_SoundOnlyPlay@@3V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@A (game2.o @ 0xDEB5B4)
extern vmCvar_t sound_disableAllOtherSounds;   // ?sound_disableAllOtherSounds@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t sound_showSoundStatForEntity;  // ?sound_showSoundStatForEntity@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t g_debugProneCheck;             // ?g_debugProneCheck@@3UvmCvar_t@@A (g.o)
extern vmCvar_t g_debugProneCheckDepthCheck;   // ?g_debugProneCheckDepthCheck@@3UvmCvar_t@@A (g.o)
extern const char* nslGetSourceName(nslSourceID sid);   // ?nslGetSourceName@@YAPBDW4nslSourceID@@@Z (nslSource.o)
extern float nslGetSourceParam(nslSourceID sid, int index, float defaultValue);  // ?nslGetSourceParam@@YAMW4nslSourceID@@HM@Z
extern void nslGetSourcePosition(nslSourceID sid, float* position);  // ?nslGetSourcePosition@@YAXW4nslSourceID@@QAM@Z

#define NSL_SOURCE_ID_INVALID ((nslSourceID)-1)

// ============================================================================
// DebugThread::DisplayEntitySound - ea: 0x4F8AB0
// ============================================================================
void DebugThread::DisplayEntitySound(const math::Position3* entityPos,
                                     int xpos, int ypos, int yinc, float scale)
{
    if (sound_disableAllOtherSounds.integer == 1)
        g_SoundOnlyPlay.mHandle.mVal = m_entityHandle.mHandle.mVal;
    else
        g_SoundOnlyPlay.mHandle.mVal = (unsigned int)-1;
    if (sound_showSoundStatForEntity.integer != 1)
        return;
    char tmpstr[128];
    for (unsigned int i = 0; i < 512; ++i)
    {
        SoundDevice::Sound* v11 =
            (SoundDevice::Sound*)((char*)SoundDevice::sInst + i * 0x3C);
        if (v11->mEntHandle.mVal != m_entityHandle.mHandle.mVal
            || v11->mSource == NSL_SOURCE_ID_INVALID
            || v11->IsFinished())
        {
            continue;
        }
        float minVal = v11->mMinRange;
        nslSourceID id = (nslSourceID)v11->mSource;
        float maxVal = v11->mMaxRange;
        const char* SourceName = nslGetSourceName(id);
        int v13 = yinc + ypos;
        sprintf(tmpstr, "Sound Name: %s ", SourceName);
        g_inspectorManager.m_currentRgba[0] = 0.0f;
        g_inspectorManager.m_currentRgba[1] = 0.8f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, xpos, v13, scale);
        double SourceParam = nslGetSourceParam(id, 0, -1.0f);
        int v15 = yinc + v13;
        sprintf(tmpstr, "Sound Vol: %f ", SourceParam);
        g_inspectorManager.m_currentRgba[0] = 0.0f;
        g_inspectorManager.m_currentRgba[1] = 0.8f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, xpos, v15, scale);
        int v16 = yinc + v15;
        sprintf(tmpstr, "Min: %f  Max: %f", minVal, maxVal);
        g_inspectorManager.m_currentRgba[0] = 0.0f;
        g_inspectorManager.m_currentRgba[1] = 0.8f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, xpos, v16, scale);
        Entity* Player = EntityManager::sInst->GetPlayer(currCl);
        float camPos = Player->r.currentOrigin.v.m128_f32[0];
        float v23 = Player->r.currentOrigin.v.m128_f32[1];
        float v24 = Player->r.currentOrigin.v.m128_f32[2];
        float soundPos[3];
        nslGetSourcePosition(id, soundPos);
        float vDelta = camPos - soundPos[0];
        float v29 = v23 - soundPos[1];
        float v30 = v24 - soundPos[2];
        int v17 = yinc + v16;
        sprintf(tmpstr,
                "Distance from source (2d-top down view): %f ",
                sqrtf(v30 * v30 + v29 * v29 + vDelta * vDelta));
        g_inspectorManager.m_currentRgba[0] = 0.0f;
        g_inspectorManager.m_currentRgba[1] = 0.8f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, xpos, v17, scale);
        ypos = yinc + v17;
        sprintf(tmpstr,
                "Distance from source (3d): %f ",
                sqrtf((v23 - soundPos[1]) * (v23 - soundPos[1])
                      + (camPos - soundPos[0]) * (camPos - soundPos[0])));
        g_inspectorManager.m_currentRgba[0] = 0.0f;
        g_inspectorManager.m_currentRgba[1] = 0.8f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, xpos, ypos, scale);
        float vUp[3] = { 0.0f, 0.0f, 1.0f };
        float srcZ = soundPos[2];
        soundPos[2] = srcZ + 25.0f;
        if (g_debugProneCheck.integer != 0)
        {
            G_DebugCircle2Ex(soundPos, minVal, vUp, colorGreen,
                             g_debugProneCheckDepthCheck.integer, 1);
        }
        soundPos[2] = srcZ + 30.0f;
        if (g_debugProneCheck.integer != 0)
        {
            G_DebugCircle2Ex(soundPos, maxVal, vUp, colorRed,
                             g_debugProneCheckDepthCheck.integer, 1);
        }
    }
}

// ============================================================================
// RenderUniqueIndex - ea: 0x503FD0
// ============================================================================
void RenderUniqueIndex(DbLinkedHandle<EntityHandleDb, Entity> index)
{
    unsigned int mVal = index.mHandle.mVal;
    unsigned int idx = mVal & 0xFFF;
    if (idx >= 0x540
        || mVal >> 12 != EntityHandleDb::sInst.mElements[idx].mKey
        || EntityHandleDb::sInst.mElements[idx].mObject == nullptr)
        return;
    char tmpstr[128];
    int y = 50;
    for (unsigned int i = 0; i < 0x540; ++i)
    {
        Entity* Object = EntityHandleDb::sInst.mElements[i].mObject;
        if (Object == nullptr)
            continue;
        if (Object->mHandle.mHandle.mVal != mVal || Object->actor == nullptr)
            continue;
        const char* name = "<no name>";
        if (Object->targetname.mBlock != nullptr
            && Object->targetname.mBlock->mLength != 0)
            name = Object->targetname.mBlock->mBuff;
        sprintf(tmpstr, "%s(%d)   ID:%d", name, mVal, mVal);
        g_inspectorManager.m_currentRgba[0] = 0.0f;
        g_inspectorManager.m_currentRgba[1] = 1.0f;
        g_inspectorManager.m_currentRgba[2] = 0.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, 100, y, 0.25f);
        y += 15;
        sprintf(tmpstr, "POS: %5.1f, %5.1f, %5.1f   YAW: %3.1f",
                Object->r.currentOrigin.v.m128_f32[0],
                Object->r.currentOrigin.v.m128_f32[1],
                Object->r.currentOrigin.v.m128_f32[2],
                Object->r.currentAngles.v.m128_f32[1]);
        g_inspectorManager.m_currentRgba[0] = 0.0f;
        g_inspectorManager.m_currentRgba[1] = 1.0f;
        g_inspectorManager.m_currentRgba[2] = 0.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, 100, y, 0.25f);
        y += 15;
        const char* stateName = AIFuncTable[
            Object->actor->eState[Object->actor->iStateLevel]].debugName;
        sprintf(tmpstr, "%s(%s)", stateName, (const char*)((char*)Object->actor + 0xB0C));
        g_inspectorManager.m_currentRgba[0] = 1.0f;
        g_inspectorManager.m_currentRgba[1] = 1.0f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, 100, y, 0.25f);
        y += 15;
        switch (Object->actor->eAnimMode)
        {
        case 0: strcpy(tmpstr, "Ai Anim Move Along Path"); break;
        case 1: strcpy(tmpstr, "Ai Anim Use Pos Deltas"); break;
        case 2: strcpy(tmpstr, "Ai Anim Use Angle Deltas"); break;
        case 3: strcpy(tmpstr, "Ai Anim Use Both Deltas"); break;
        case 4: strcpy(tmpstr, "Ai Anim Use Both Deltas Noclip"); break;
        case 5: strcpy(tmpstr, "Ai Anim Use Both Deltas NoGravity"); break;
        case 6: strcpy(tmpstr, "Ai Anim Stationary"); break;
        case 7: strcpy(tmpstr, "Ai Anim No Physics"); break;
        default: strcpy(tmpstr, "Ai Anim Unknown"); break;
        }
        g_inspectorManager.m_currentRgba[0] = 1.0f;
        g_inspectorManager.m_currentRgba[1] = 1.0f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, 100, y, 0.25f);
        y += 15;
        if (Object->IsVisible() != 0)
            strcpy(tmpstr, "Visible");
        else
            strcpy(tmpstr, "Not Visible");
        g_inspectorManager.m_currentRgba[0] = 1.0f;
        g_inspectorManager.m_currentRgba[1] = 1.0f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, 100, y, 0.25f);
        y += 15;
        if (Object->sentient->bNearestNodeValid != 0)
        {
            const PathNodes::PathNode* node =
                (*(const PathNodes::NodeHandle*)&Object->sentient->mClaimedNode).operator*();
            if (node != nullptr)
            {
                sprintf(tmpstr, "NODE(%d): %d, %d, %d",
                        node->mHandle.mValue,
                        (int)node->mConstant.mOrigin[0],
                        (int)node->mConstant.mOrigin[1],
                        (int)node->mConstant.mOrigin[2]);
                g_inspectorManager.m_currentRgba[0] = 1.0f;
                g_inspectorManager.m_currentRgba[1] = 1.0f;
                g_inspectorManager.m_currentRgba[2] = 1.0f;
                g_inspectorManager.m_currentRgba[3] = 1.0f;
                g_inspectorManager.Print(tmpstr, 100, y, 0.25f);
                Path_DrawDebugNode(node);
            }
        }
        g_drawDebugEntityLos = 1;
        y += 15;
        g_debugThread.m_entityHandle.mHandle.mVal =
            Object->mHandle.mHandle.mVal;
        float ox = Object->r.currentOrigin.v.m128_f32[0];
        float oy = Object->r.currentOrigin.v.m128_f32[1];
        float oz = Object->r.currentOrigin.v.m128_f32[2];
        for (int k = 8; k != 0; --k)
        {
            float startPos[3] = { ox + flrand(-2.0f, 2.0f),
                                  oy + flrand(-2.0f, 2.0f), oz };
            float endPos[3] = { startPos[0] + flrand(-2.0f, 2.0f),
                                startPos[1] + flrand(-2.0f, 2.0f),
                                oz + 200.0f };
            if (g_debugProneCheck.integer != 0)
            {
                G_DebugLine(startPos, endPos, colorRed,
                            g_debugProneCheckDepthCheck.integer, 1);
            }
        }
        const PathNodes::PathNode* chain =
            (*(const PathNodes::NodeHandle*)&Object->sentient->mActualChainPos).operator*();
        if (chain != nullptr)
        {
            sprintf(tmpstr, "Actual Chain: %.1f %.1f %.1f",
                    chain->mConstant.mOrigin[0],
                    chain->mConstant.mOrigin[1],
                    chain->mConstant.mOrigin[2]);
            g_inspectorManager.m_currentRgba[0] = 1.0f;
            g_inspectorManager.m_currentRgba[1] = 1.0f;
            g_inspectorManager.m_currentRgba[2] = 1.0f;
            g_inspectorManager.m_currentRgba[3] = 1.0f;
            g_inspectorManager.Print(tmpstr, 100, y, 0.25f);
            y += 15;
        }
    }
}

// ============================================================================
// DebugThread::Render - ea: 0x50A050
// ============================================================================
void DebugThread::Render()
{
    RenderUniqueIndex(g_renderUniqueIndex);
    char tmpstr[128];
    char textBuff[128];
    float white[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    if (gDebugThread_MessageTicks != 0)
    {
        float alpha = 1.0f;
        if (gDebugThread_MessageAlphaMin != 1.0f)
            alpha = gDebugThread_MessageTicks * 0.0033333334f;
        --gDebugThread_MessageTicks;
        memcpy(g_inspectorManager.m_currentRgba, gDebugThread_MessageRGB, 12);
        g_inspectorManager.m_currentRgba[3] = alpha;
        g_inspectorManager.Print(
            gDebugThread_Message, (int)gDebugThread_MessageXpos,
            (int)gDebugThread_MessageYpos, gDebugThread_MessageScale);
        if (gDebugThread_MessageTicks <= 0)
            gDebugThread_Message = nullptr;
    }
    if (memory_reportBrocPool.integer == 1)
        DisplayPoolTotals(gBrocPool);
    if (memory_reportBrocBackupStackPool.integer == 1)
        DisplayPoolTotals(gAeThreadBackupStackAllocator);
    if (memory_reportCommonPool.integer == 1)
        DisplayPoolTotals(gCommonPoolAllocator);
    if (memory_reportAepsStats.integer == 1)
    {
        FX_ReportFX();
        memory_reportAepsStats.integer = 0;
    }
    if (memory_displayAepsStats.integer == 1)
    {
        apsStats stats;
        apsGetStats(stats);
        g_inspectorManager.m_currentRgba[0] = 0.75f;
        g_inspectorManager.m_currentRgba[1] = 0.75f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print((char*)"SIZE", 320, 48, 0.25f);
        g_inspectorManager.Print((char*)"NUM", 400, 48, 0.25f);
        g_inspectorManager.Print((char*)"USED", 480, 48, 0.25f);
        g_inspectorManager.Print((char*)"PEAK", 560, 48, 0.25f);
        int v78 = 64;
        int v77 = 0;
        float v76 = 0.0f;
        int pool = 0;
        int size, capacity, used, peak;
        if (apsGetPoolInfo(pool, size, capacity, used, peak))
        {
            do
            {
                if ((float)size > v76)
                    v76 = (float)size;
                if (peak == capacity)
                {
                    g_inspectorManager.m_currentRgba[0] = 1.0f;
                    g_inspectorManager.m_currentRgba[1] = 0.25f;
                    g_inspectorManager.m_currentRgba[2] = 0.25f;
                    g_inspectorManager.m_currentRgba[3] = 1.0f;
                }
                else if (peak > 80 * capacity / 100)
                {
                    g_inspectorManager.m_currentRgba[0] = 1.0f;
                    g_inspectorManager.m_currentRgba[1] = 1.0f;
                    g_inspectorManager.m_currentRgba[2] = 0.0f;
                    g_inspectorManager.m_currentRgba[3] = 1.0f;
                }
                else
                {
                    g_inspectorManager.m_currentRgba[0] = 0.5f;
                    g_inspectorManager.m_currentRgba[1] = 0.5f;
                    g_inspectorManager.m_currentRgba[2] = 1.0f;
                    g_inspectorManager.m_currentRgba[3] = 1.0f;
                }
                int y = v78;
                v77 += size * capacity;
                int vals[4] = { size, capacity, used, peak };
                int x = 320;
                for (int c = 0; c < 4; ++c)
                {
                    sprintf(tmpstr, "%d", vals[c]);
                    RE_Text_Paint((float)(x + 2), (float)(y + 2), 5,
                                  scaleScalar * 0.25f, white, tmpstr, 0, 0, 0);
                    RE_Text_Paint((float)x, (float)y, 5, scaleScalar * 0.25f,
                                  g_inspectorManager.m_currentRgba, tmpstr,
                                  0, 0, 0);
                    x += 80;
                }
                v78 += 16;
                ++pool;
            } while (apsGetPoolInfo(pool, size, capacity, used, peak));
        }
        g_inspectorManager.m_currentRgba[0] = 0.75f;
        g_inspectorManager.m_currentRgba[1] = 0.75f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        sprintf(tmpstr, "%dKB total", (v77 + 1023) / 1024);
        int v8 = v78;
        RE_Text_Paint(336.0f, (float)(v8 + 2), 5, scaleScalar * 0.25f, white,
                      tmpstr, 0, 0, 0);
        RE_Text_Paint(320.0f, (float)v8, 5, scaleScalar * 0.25f,
                      g_inspectorManager.m_currentRgba, tmpstr, 0, 0, 0);
        v8 += 16;
        sprintf(tmpstr, "%d active effects", stats.numActiveEffects);
        RE_Text_Paint(336.0f, (float)(v8 + 2), 5, scaleScalar * 0.25f, white,
                      tmpstr, 0, 0, 0);
        RE_Text_Paint(320.0f, (float)v8, 5, scaleScalar * 0.25f,
                      g_inspectorManager.m_currentRgba, tmpstr, 0, 0, 0);
        int v9 = v8 + 16;
        sprintf(tmpstr, "max requested size %d", stats.maxRequestedBlockSize);
        RE_Text_Paint(336.0f, (float)(v9 + 2), 5, scaleScalar * 0.25f, white,
                      tmpstr, 0, 0, 0);
        RE_Text_Paint(320.0f, (float)v9, 5, scaleScalar * 0.25f,
                      g_inspectorManager.m_currentRgba, tmpstr, 0, 0, 0);
        int v10 = v9 + 16;
        sprintf(tmpstr, "%d active particles ( %d peak )",
                stats.numActiveParticles, stats.maxActiveParticles);
        RE_Text_Paint(336.0f, (float)(v10 + 2), 5, scaleScalar * 0.25f, white,
                      tmpstr, 0, 0, 0);
        RE_Text_Paint(320.0f, (float)v10, 5, scaleScalar * 0.25f,
                      g_inspectorManager.m_currentRgba, tmpstr, 0, 0, 0);
        sprintf(tmpstr, "Particle Array (%d/%d)",
                ParticleEffect::sArray.m_size, 256);
        RE_Text_Paint(336.0f, (float)(v10 + 18), 5, scaleScalar * 0.25f,
                      white, tmpstr, 0, 0, 0);
        RE_Text_Paint(320.0f, (float)(v10 + 16), 5, scaleScalar * 0.25f,
                      g_inspectorManager.m_currentRgba, tmpstr, 0, 0, 0);
    }
    if (memory_showStatistics.integer == 1)
    {
        g_inspectorManager.m_currentRgba[0] = 0.75f;
        g_inspectorManager.m_currentRgba[1] = 0.75f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print((char*)"USED", 400, 250, 0.35f);
        g_inspectorManager.Print((char*)"FREE", 475, 250, 0.35f);
        g_inspectorManager.Print((char*)"PEAK", 550, 250, 0.35f);
        g_inspectorManager.m_currentRgba[0] = 0.75f;
        g_inspectorManager.m_currentRgba[1] = 0.75f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print((char*)"MEMORY", 300, 266, 0.35f);
        int usedBytes = mem_get_used_bytes(MEM_HEAP_NONE);
        if (usedBytes <= 0x2000000)
        {
            if (usedBytes > 25165824)
            {
                g_inspectorManager.m_currentRgba[0] = 1.0f;
                g_inspectorManager.m_currentRgba[1] = 1.0f;
                g_inspectorManager.m_currentRgba[2] = 0.0f;
                g_inspectorManager.m_currentRgba[3] = 1.0f;
            }
            else
            {
                g_inspectorManager.m_currentRgba[0] = 0.5f;
                g_inspectorManager.m_currentRgba[1] = 0.5f;
                g_inspectorManager.m_currentRgba[2] = 1.0f;
                g_inspectorManager.m_currentRgba[3] = 1.0f;
            }
        }
        else
        {
            g_inspectorManager.m_currentRgba[0] = 1.0f;
            g_inspectorManager.m_currentRgba[1] = 0.25f;
            g_inspectorManager.m_currentRgba[2] = 0.25f;
            g_inspectorManager.m_currentRgba[3] = 1.0f;
        }
        sprintf(tmpstr, "%5.2f MB", usedBytes * 0.00000095367432f);
        g_inspectorManager.Print(tmpstr, 400, 266, 0.35f);
        g_inspectorManager.m_currentRgba[0] = 0.5f;
        g_inspectorManager.m_currentRgba[1] = 0.5f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        sprintf(tmpstr, "%5.2f MB",
                mem_get_free_bytes(MEM_HEAP_NONE) * 0.00000095367432f);
        g_inspectorManager.Print(tmpstr, 475, 266, 0.35f);
        sprintf(tmpstr, "%5.2f MB",
                mem_get_high_used_bytes(MEM_HEAP_NONE) * 0.00000095367432f);
        g_inspectorManager.Print(tmpstr, 550, 266, 0.35f);
        if (gBrocHeap != nullptr)
        {
            char* heap = (char*)gBrocHeap;
            int size = *(int*)(heap + 0x484);
            int used = *(int*)(heap + 0x488);
            int high = *(int*)(heap + 0x48C);
            g_inspectorManager.m_currentRgba[0] = 0.75f;
            g_inspectorManager.m_currentRgba[1] = 0.75f;
            g_inspectorManager.m_currentRgba[2] = 1.0f;
            g_inspectorManager.m_currentRgba[3] = 1.0f;
            g_inspectorManager.Print((char*)"Broc Heap", 300, 282, 0.35f);
            g_inspectorManager.m_currentRgba[0] = 0.5f;
            g_inspectorManager.m_currentRgba[1] = 0.5f;
            g_inspectorManager.m_currentRgba[2] = 1.0f;
            g_inspectorManager.m_currentRgba[3] = 1.0f;
            sprintf(tmpstr, "%5.2f MB", used * 0.00000095367432f);
            g_inspectorManager.Print(tmpstr, 400, 282, 0.35f);
            sprintf(tmpstr, "%5.2f MB", (size - used) * 0.00000095367432f);
            g_inspectorManager.Print(tmpstr, 475, 282, 0.35f);
            sprintf(tmpstr, "%5.2f MB", high * 0.00000095367432f);
            g_inspectorManager.Print(tmpstr, 550, 282, 0.35f);
        }
    }
    // Thread / selected-entity section
    Entity* selected = nullptr;
    unsigned int selVal = g_debugThread.m_entityHandle.mHandle.mVal;
    unsigned int selIdx = selVal & 0xFFF;
    if (selIdx < 0x540
        && selVal >> 12 == EntityHandleDb::sInst.mElements[selIdx].mKey)
        selected = EntityHandleDb::sInst.mElements[selIdx].mObject;
    char* mgr = (char*)&AeThreadManager::sInst;
    void** list = (void**)(mgr + 4);          // mThreads
    void* m_head = list[0];                   // head node pointer
    void* m_next = m_head != nullptr
        ? *(void**)((char*)m_head + 4) : nullptr;
    void* endNode = (char*)list + 8;          // &mThreads.m_end
    if (m_head == endNode)
        m_next = nullptr;
    int numThreads = 0;
    float numForSelected = 0.0f;
    void* cur = m_next;
    while (cur != nullptr)
    {
        AeThread* t = (AeThread*)cur;
        ++numThreads;
        Entity* owner = nullptr;
        unsigned int own = t->mOwner;
        unsigned int oi = own & 0xFFF;
        if (oi < 0x540
            && own >> 12 == EntityHandleDb::sInst.mElements[oi].mKey)
            owner = EntityHandleDb::sInst.mElements[oi].mObject;
        if (owner == selected)
            ++numForSelected;
        cur = t->mNext;
    }
    sprintf(tmpstr, "Num Threads In Game: %d", numThreads);
    g_inspectorManager.m_currentRgba[0] = 0.0f;
    g_inspectorManager.m_currentRgba[1] = 1.0f;
    g_inspectorManager.m_currentRgba[2] = 0.0f;
    g_inspectorManager.m_currentRgba[3] = 1.0f;
    g_inspectorManager.Print(tmpstr, 100, 50, 0.5f);
    if (selected == nullptr)
    {
        sprintf(tmpstr, "Can't find Selected Entity");
        g_inspectorManager.m_currentRgba[0] = 0.0f;
        g_inspectorManager.m_currentRgba[1] = 1.0f;
        g_inspectorManager.m_currentRgba[2] = 0.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, 100, 74, 0.5f);
        g_SoundOnlyPlay.mHandle.mVal = (unsigned int)-1;
        return;
    }
    const char* name = "<no name>";
    if (selected->targetname.mBlock != nullptr
        && selected->targetname.mBlock->mLength != 0)
        name = selected->targetname.mBlock->mBuff;
    sprintf(tmpstr, "%s(%d)   ID:%d", name,
            selected->mHandle.mHandle.mVal, selected->uniqueIndex);
    g_inspectorManager.m_currentRgba[0] = 0.0f;
    g_inspectorManager.m_currentRgba[1] = 1.0f;
    g_inspectorManager.m_currentRgba[2] = 0.0f;
    g_inspectorManager.m_currentRgba[3] = 1.0f;
    g_inspectorManager.Print(tmpstr, 100, 74, 0.5f);
    float ox = selected->r.currentOrigin.v.m128_f32[0];
    float oy = selected->r.currentOrigin.v.m128_f32[1];
    float oz = selected->r.currentOrigin.v.m128_f32[2];
    float yaw = selected->r.currentAngles.v.m128_f32[1];
    sprintf(tmpstr, "POS: %5.1f, %5.1f, %5.1f   YAW: %3.1f  ",
            ox, oy, oz, yaw);
    g_inspectorManager.m_currentRgba[0] = 0.0f;
    g_inspectorManager.m_currentRgba[1] = 1.0f;
    g_inspectorManager.m_currentRgba[2] = 0.0f;
    g_inspectorManager.m_currentRgba[3] = 1.0f;
    g_inspectorManager.Print(tmpstr, 100, 92, 0.5f);
    int y = 110;
    char* actor = (char*)selected->actor;
    if (actor != nullptr)
    {
        int stateIdx = *(int*)(actor + 0x24);
        sprintf(tmpstr, "%s(%s)",
                AIFuncTable[*(int*)(actor + 8 + 4 * stateIdx)].debugName,
                *(const char**)(actor + 0xB0C));
        g_inspectorManager.m_currentRgba[0] = 1.0f;
        g_inspectorManager.m_currentRgba[1] = 1.0f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, 100, 110, 0.5f);
        textBuff[0] = 0;
        strcat(textBuff, "AI Flags:");
        if (*(int*)(actor + 0x864) != 0)
            strcat(textBuff, "    pacifist");
        if ((selected->flags & 2) != 0
            || *(int*)((char*)selected->sentient + 56) != 0)
            strcat(textBuff, "    ignoreme");
        if (Actor_IsSuppressed(selected->actor) != 0)
            strcat(textBuff, "    suppressed");
        g_inspectorManager.m_currentRgba[0] = 1.0f;
        g_inspectorManager.m_currentRgba[1] = 1.0f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(textBuff, 100, 128, 0.5f);
        y = 146;
        char* sent = (char*)selected->sentient;
        if (*(int*)(sent + 154) != 0)
        {
            const PathNodes::PathNode* node =
                (*(const PathNodes::NodeHandle*)(sent + 116)).operator*();
            if (node != nullptr)
            {
                sprintf(tmpstr, "   NODE(%d): %d, %d, %d",
                        node->mHandle.mValue,
                        (int)node->mConstant.mOrigin[0],
                        (int)node->mConstant.mOrigin[1],
                        (int)node->mConstant.mOrigin[2]);
                Path_DrawDebugNode(node);
                g_inspectorManager.m_currentRgba[0] = 1.0f;
                g_inspectorManager.m_currentRgba[1] = 1.0f;
                g_inspectorManager.m_currentRgba[2] = 1.0f;
                g_inspectorManager.m_currentRgba[3] = 1.0f;
                g_inspectorManager.Print(tmpstr, 100, 146, 0.5f);
                y = 164;
            }
        }
        textBuff[0] = 0;
        int sFlags = *(short*)(sent + 0x72);
        if ((sFlags & 1) != 0)
            strcat(textBuff, "DontProne, ");
        if ((sFlags & 2) != 0)
            strcat(textBuff, "DontStand, ");
        if ((sFlags & 4) != 0)
            strcat(textBuff, "DontCrouch, ");
        if (textBuff[0] != 0)
        {
            g_inspectorManager.Print(textBuff, 100, y, 0.5f);
            y += 18;
        }
        if (selected->mAnimDebug != nullptr)
        {
            unsigned int curHash = *(unsigned int*)selected->mAnimDebug;
            const char* curName =
                gpBrocAPI->mBrocExports.mAnimNameResolver(curHash);
            sprintf(tmpstr, "CurAnim: %s", curName);
            g_inspectorManager.m_currentRgba[0] = 0.0f;
            g_inspectorManager.m_currentRgba[1] = 1.0f;
            g_inspectorManager.m_currentRgba[2] = 0.0f;
            g_inspectorManager.m_currentRgba[3] = 1.0f;
            g_inspectorManager.Print(tmpstr, 100, y + 18, 0.5f);
            unsigned int lastHash = *(unsigned int*)((char*)selected->mAnimDebug + 4);
            const char* lastName =
                gpBrocAPI->mBrocExports.mAnimNameResolver(lastHash);
            sprintf(tmpstr, "LastAnim: %s", lastName);
            g_inspectorManager.m_currentRgba[0] = 0.0f;
            g_inspectorManager.m_currentRgba[1] = 0.8f;
            g_inspectorManager.m_currentRgba[2] = 0.0f;
            g_inspectorManager.m_currentRgba[3] = 1.0f;
            g_inspectorManager.Print(tmpstr, 100, y + 36, 0.5f);
        }
    }
    // Thread window
    int total = (int)numForSelected;
    int scrollStart = g_debugThread.m_menuScrollStartIndex;
    int page = g_debugThread.m_menuMaxOnPage;
    int dispStart = scrollStart;
    if (total - scrollStart < page)
    {
        dispStart = total - page;
        scrollStart = total - page;
        if (total - page < 0)
        {
            dispStart = 0;
            scrollStart = 0;
        }
    }
    int dispEnd = dispStart + page;
    if (dispEnd >= total)
        dispEnd = total;
    int line = y + 24;
    int seen = 0;
    cur = m_next;
    while (cur != nullptr)
    {
        AeThread* t = (AeThread*)cur;
        Entity* owner = nullptr;
        unsigned int own = t->mOwner;
        unsigned int oi = own & 0xFFF;
        if (oi < 0x540
            && own >> 12 == EntityHandleDb::sInst.mElements[oi].mKey)
            owner = EntityHandleDb::sInst.mElements[oi].mObject;
        if (owner == selected)
        {
            if (seen >= scrollStart && seen < dispEnd)
            {
                if (t->mFuncName != nullptr)
                {
                    ae_fixed_string<64, unsigned char> tStr;
                    tStr.mBuff[52] = 0;
                    t->GetCondText(tStr);
                    sprintf(tmpstr, "%s ***%s***", t->mFuncName, tStr.mBuff);
                }
                else
                {
                    sprintf(tmpstr, "Unknown: Line %d, File %s",
                            t->mLine, t->mFile);
                }
                if ((t->mFlags & 0x10) != 0)
                {
                    g_inspectorManager.m_currentRgba[0] = 0.6f;
                    g_inspectorManager.m_currentRgba[1] = 0.6f;
                }
                else
                {
                    g_inspectorManager.m_currentRgba[0] = 1.0f;
                    g_inspectorManager.m_currentRgba[1] = 1.0f;
                }
                g_inspectorManager.m_currentRgba[2] = 1.0f;
                g_inspectorManager.m_currentRgba[3] = 1.0f;
                g_inspectorManager.Print(tmpstr, 100, line, 0.5f);
                line += 18;
            }
            ++seen;
        }
        cur = t->mNext;
    }
    sprintf(tmpstr, "Disp: %d-%d / %d", dispStart + 1, dispEnd, total);
    g_inspectorManager.m_currentRgba[0] = 0.0f;
    g_inspectorManager.m_currentRgba[1] = 1.0f;
    g_inspectorManager.m_currentRgba[2] = 0.0f;
    g_inspectorManager.m_currentRgba[3] = 1.0f;
    g_inspectorManager.Print(tmpstr, 100, line, 0.5f);
    float entityPos[3] = { ox, oy, oz };
    DisplayEntitySound((const math::Position3*)entityPos, 100, line, 18, 0.5f);
    g_SoundOnlyPlay.mHandle.mVal = (unsigned int)-1;
}

// ============================================================================
// DebugThread::DisplayMessage - ea: 0x4F4620
// ============================================================================
char* DebugThread::DisplayMessage(char* msg, int xpos, int ypos, float r,
                                  float g, float b, float scale,
                                  float alphaMin)
{
    gDebugThread_MessageRGB[0] = r;
    gDebugThread_MessageRGB[1] = g;
    gDebugThread_MessageRGB[2] = b;
    gDebugThread_MessageScale = scale;
    gDebugThread_MessageXpos = (float)xpos;
    gDebugThread_Message = msg;
    gDebugThread_MessageTicks = 300;
    gDebugThread_MessageYpos = (float)ypos;
    gDebugThread_MessageAlphaMin = (int)alphaMin;
    return msg;
}

// ============================================================================
// DebugThread::Update - ea: 0x4F46A0
// ============================================================================
void DebugThread::Update()
{
}

extern void* Task_vftable;  // ??_7Task@@6B@

// ea: 0x4F9940
Task::Task(DbLinkedHandle<EntityHandleDb, Entity> handle, unsigned int idTask)
{
    memset(_dlist, 0, 8);
    mTaskId = idTask;
    mEntityHandle = handle;
    mTaskHandle.mVal = 0;
    mFlags = 1;
}

// ea: 0x4F9970
Task::Task(DbLinkedHandle<EntityHandleDb, Entity> handle, int idTask)
{
    memset(_dlist, 0, 8);
    mTaskId = (unsigned int)idTask;
    mEntityHandle = handle;
    mTaskHandle.mVal = 0;
    mFlags = 1;
}

struct HealthRegenTask : Task {
    float mDamageDelay;             // +0x1C
    float mRechargeRate;            // +0x20
    float mTimeSinceDamage;         // +0x24
    float mHealthDelta;             // +0x28
    bool mVeryHurt;                 // +0x2C
    float mTimeSinceVeryHurt;       // +0x30
    float mTimeSinceRecoverySound;  // +0x34

    HealthRegenTask(DbLinkedHandle<EntityHandleDb, Entity> h, float damageDelay,
                    float rechargeRate);
    void Update(Entity* e, float deltaT);  // ea: 0x4F9AC0
};
static_assert(sizeof(HealthRegenTask) == 0x38, "HealthRegenTask size mismatch");

extern void* HealthRegenTask_vftable;  // ??_7HealthRegenTask@@6B@
cvar_t* HealthRegenTask_sDamageDelay;    // game2.o statics
cvar_t* HealthRegenTask_sRechargeRate;   // game2.o statics
extern cvar_t* Cvar_Get(const char* var_name, const char* var_value, int flags);

// ea: 0x4F99A0
HealthRegenTask::HealthRegenTask(DbLinkedHandle<EntityHandleDb, Entity> h,
                                 float damageDelay, float rechargeRate)
    : Task(h, 1213351758)
{
    mTimeSinceDamage = 0.0f;
    mHealthDelta = 0.0f;
    mVeryHurt = false;
    mTimeSinceVeryHurt = 0.0f;
    mTimeSinceRecoverySound = 0.0f;
    if (HealthRegenTask_sDamageDelay == nullptr)
    {
        HealthRegenTask_sDamageDelay = Cvar_Get(
            "hud_healthOverlay_regenPauseTime", "5000", 256);
        HealthRegenTask_sRechargeRate = Cvar_Get(
            "health_recharge_rate", "30", 256);
    }
    float v5 = damageDelay;
    if (damageDelay == -1.0f)
        v5 = HealthRegenTask_sDamageDelay->value * 0.001f;
    mDamageDelay = v5;
    if (rechargeRate == -1.0f)
        mRechargeRate = HealthRegenTask_sRechargeRate->value;
    else
        mRechargeRate = rechargeRate;
}

// ============================================================================
// HealthRegenTask::Update - ea: 0x4F9AC0
// ============================================================================
extern vmCvar_t hud_healthOverlay_pulseStart;
extern vmCvar_t g_player_maxhealth;
extern HashString sDamageStr_0;

void HealthRegenTask::Update(Entity* e, float deltaT)
{
    if ((mFlags & 4) == 0 && e->health > 0)
    {
        if (e->mNotifySet != nullptr
            && EntityNotifySet_GetNotify(e->mNotifySet, sDamageStr_0.mHash)
                != nullptr)
        {
            mTimeSinceDamage = mDamageDelay;
            mTimeSinceVeryHurt = 0.0f;
            mTimeSinceRecoverySound = 0.0f;
        }
        bool mVeryHurt = this->mVeryHurt;
        if (hud_healthOverlay_pulseStart.value
            <= (e->health / g_player_maxhealth.value))
        {
            mTimeSinceVeryHurt = mTimeSinceVeryHurt + deltaT;
            mTimeSinceRecoverySound = mTimeSinceRecoverySound + deltaT;
        }
        else
        {
            this->mVeryHurt = true;
        }
        if (!mVeryHurt)
        {
            mTimeSinceVeryHurt = 0.0f;
            mTimeSinceRecoverySound = 0.0f;
        }
        if (mTimeSinceDamage > 0.0f)
            mTimeSinceDamage = mTimeSinceDamage - deltaT;
        if (mTimeSinceDamage <= 0.0f && e->health < g_player_maxhealth.integer)
            mHealthDelta = (mRechargeRate * deltaT) + mHealthDelta;
        if (mHealthDelta > 1.0f)
        {
            int v8 = (int)mHealthDelta;
            mHealthDelta = mHealthDelta - (float)(int)mHealthDelta;
            int integer = (int)mHealthDelta + e->health;
            if (integer >= g_player_maxhealth.integer)
                integer = g_player_maxhealth.integer;
            e->health = integer;
            if (integer == g_player_maxhealth.integer)
                this->mVeryHurt = false;
            if (mTimeSinceRecoverySound > mDamageDelay)
            {
                mTimeSinceRecoverySound = 0.0f;
                if (gpBrocAPI->mBrocExports.mCallbackHealthRegenRecovering
                    != nullptr)
                {
                    Broc::entity ent{e->mHandle.mHandle.mVal};
                    gpBrocAPI->mBrocExports.mCallbackHealthRegenRecovering(ent);
                }
            }
            if (e->client != nullptr)
            {
                // Decrement the client's damage-scaled health display fields.
                for (int i = 342; i < 358; i += 4)
                {
                    float* base = &e->client->ps.origin.v.m128_f32[i - 1];
                    base[0] = (base[0] - v8 <= 0) ? 0 : base[0] - v8;
                    base[1] = (base[1] - v8 <= 0) ? 0 : base[1] - v8;
                    base[2] = (base[2] - v8 <= 0) ? 0 : base[2] - v8;
                    base[3] = (base[3] - v8 <= 0) ? 0 : base[3] - v8;
                }
            }
        }
    }
}

// ============================================================================
// AnimationPlayer note handler + play method (game2.o)
// ============================================================================
struct AnimNoteHandler {
    DbLinkedHandle<EntityHandleDb, Entity> mEntHandle;  // +0x00
    void* mNotify;       // +0x04
    int mNotifyIndex;    // +0x08
};

class AnimationPlayer {
public:
    struct nalPlayMethod {
        void** __vftable;       // +0x00
        AnimNoteHandler* mNoteHandler;  // +0x04

        nalPlayMethod();       // ea: 0x4FA500
        ~nalPlayMethod();      // ea: 0x50BB30
        void Advance(void* state, float delta);  // ea: 0x50BB80
        void* CreateInstance(void* anim, void* skeleton);  // ea: 0x504C60
        void SetNoteHandlerEntityHandle(
            DbLinkedHandle<EntityHandleDb, Entity> handle);  // ea: 0x4F5D10
        void Release();  // nalPlayMethod::Release (thunk)
    };

    struct nalAnimCallback {
        void** __vftable;       // +0x00
        virtual bool Invoke(AnimationPlayer* player);  // ea: 0x4F5F10
    };

    // nalAnimState - 0x2C (IDA verified)
    struct nalAnimState {
        void* instance;      // +0x00
        float speed;         // +0x04
        float tlimit;        // +0x08
        void* callback;      // +0x0C
        void* play_method;   // +0x10
        float t;             // +0x14
        float t_prev;        // +0x18
        float alpha;         // +0x1C
        float maxAlpha;      // +0x20
        float fadein_rate;   // +0x24
        int state;           // +0x28

        bool Update(AnimationPlayer* player, float delta);  // ?Update@nalAnimState@AnimationPlayer@@QAE_NPAV2@M@Z
        void Compose(nalGeneric::nalGenericPose& pose,
                     nalGeneric::nalGenericPose& tmpPose);  // ?Compose@nalAnimState@AnimationPlayer@@QAEXAAVnalGenericPose@nalGeneric@@0@Z
    };

    // nalPartialAnimState - 0x44 (IDA verified)
    struct nalPartialAnimState {
        nalAnimState base;   // +0x00
        unsigned int CreationAdvanceCount;  // +0x2C
        nalPartialAnimState* next;  // +0x30
        unsigned int mask;   // +0x34
        float priority;      // +0x38
        float fadeout_rate;  // +0x3C
        int type;            // +0x40

        bool Update(AnimationPlayer* player, float delta);  // ?Update@nalPartialAnimState@AnimationPlayer@@QAE_NPAV2@M@Z
    };

    void Advance(float delta);  // ea: 0x4FA550
    static void DebugDump(Entity* ent);  // ea: 0x4F5D30
    void GetPose(nalGeneric::nalGenericPose& Pose,
                 nalGeneric::nalGenericSkeleton* Skeleton,
                 float (*animIKGunOffset)[3]);  // ea: 0x4FA690
};

void* nalPlayMethod_vftable = nullptr;   // ??_7nalPlayMethod@AnimationPlayer@@6B@
extern void* mem_heap_malloc(unsigned int size);

// ea: 0x4FA500
AnimationPlayer::nalPlayMethod::nalPlayMethod()
{
    __vftable = (void**)&nalPlayMethod_vftable;
    mNoteHandler = nullptr;
    AnimNoteHandler* v2 = (AnimNoteHandler*)mem_heap_malloc(0xC);
    if (v2 != nullptr)
    {
        v2->mEntHandle.mHandle.mVal = 0;
        v2->mNotify = nullptr;
        v2->mNotifyIndex = -1;
        mNoteHandler = v2;
    }
    else
    {
        mNoteHandler = nullptr;
    }
}

// ea: 0x4F5D10
void AnimationPlayer::nalPlayMethod::SetNoteHandlerEntityHandle(
    DbLinkedHandle<EntityHandleDb, Entity> handle)
{
    AnimNoteHandler* mNoteHandler = this->mNoteHandler;
    if (mNoteHandler != nullptr)
        mNoteHandler->mEntHandle = handle;
}

// ea: 0x4F5F10
bool AnimationPlayer::nalAnimCallback::Invoke(AnimationPlayer* player)
{
    (void)player;
    return true;
}

// ea: 0x4F5F10 area - nalAnimState/nalPartialAnimState members (stubs)
bool AnimationPlayer::nalAnimState::Update(AnimationPlayer* player, float delta)
{
    (void)player; (void)delta;
    return false;
}

void AnimationPlayer::nalAnimState::Compose(nalGeneric::nalGenericPose& pose,
                                            nalGeneric::nalGenericPose& tmpPose)
{
    (void)pose; (void)tmpPose;
}

bool AnimationPlayer::nalPartialAnimState::Update(AnimationPlayer* player,
                                                  float delta)
{
    (void)player; (void)delta;
    return false;
}

// ============================================================================
// MetaNalBaseAnim - meta-animation wrapper (0x44, IDA verified)
// ============================================================================
struct MetaAnimData {
    void** __vftable;  // +0x00
    // vtable slots:
    //   [0] GetAnimName() -> const tlFixedString*
    //   [1] IsAnimLooping() -> int
    //   [2] IsAnimTrajRelative() -> int
    //   [3] GetAnimDuration() -> float
    //   [4] GetSkeleton() -> nalBaseSkeleton*
    //   [5] DelayCreate(nalAnimClass**, int)
    //   [6] IsDelayCreate() -> int
    //   [7] CreateAnimInst(...)
};

struct nalBaseSkeleton;
struct nalInstanceClass;

struct MetaNalBaseAnim {
    void** __vftable;           // +0x00
    tlFixedString Name;         // +0x04 (32 bytes)
    unsigned char _pad24[0x30 - 0x24];  // +0x24 (nalAnimClass extra)
    void* Skeleton;             // +0x30
    unsigned int Flags;         // +0x34
    float Duration;             // +0x38
    unsigned char _pad3C[4];    // +0x3C (nalAnimClass instance ptr area)
    MetaAnimData* mData;        // +0x40

    MetaNalBaseAnim();                       // ea: 0x4F5F20
    void Create(MetaAnimData* theMetaAnimData);  // ea: 0x4F5F50
    void DelayCreate(void** animArray, int numAnims);  // ea: 0x4F5FE0
    void DelayCreate(void* anim);            // ea: 0x4FAE20
    int IsDelayCreate();                     // ea: 0x4F6010
    void* CreateAnimInst(nalBaseSkeleton* theSkel);  // ea: 0x4F6020
};
static_assert(sizeof(MetaNalBaseAnim) == 0x44, "MetaNalBaseAnim size mismatch");

void* MetaNalBaseAnim_vftable = nullptr;  // ??_7MetaNalBaseAnim@@6B@

typedef void* (*GetAnimNameFn)(void* self);
typedef int (*IsAnimLoopingFn)(void* self);
typedef int (*IsAnimTrajRelativeFn)(void* self);
typedef float (*GetAnimDurationFn)(void* self);
typedef void* (*GetSkeletonFn)(void* self);
typedef void (*DelayCreateFn)(void* self, void** animArray, int numAnims);
typedef int (*IsDelayCreateFn2)(void* self);
typedef void* (*CreateAnimInstFn)(void* self, void* theSkel, void* metaAnim);

// ea: 0x4F5F20
MetaNalBaseAnim::MetaNalBaseAnim()
{
    memset(&Name, 0, sizeof(Name));
    __vftable = (void**)&MetaNalBaseAnim_vftable;
    mData = nullptr;
}

// ea: 0x4F5F50
void MetaNalBaseAnim::Create(MetaAnimData* theMetaAnimData)
{
    mData = theMetaAnimData;
    Flags = 0;
    void** vt = (void**)theMetaAnimData->__vftable;
    if (((IsAnimLoopingFn)vt[1])(theMetaAnimData) != 0)
        Flags |= 1u;
    if (((IsAnimTrajRelativeFn)mData->__vftable[2])(mData) != 0)
        Flags |= 2u;
    Duration = ((GetAnimDurationFn)mData->__vftable[3])(mData);
    Skeleton = ((GetSkeletonFn)mData->__vftable[4])(mData);
    const unsigned int* v5 = (const unsigned int*)
        ((GetAnimNameFn)mData->__vftable[0])(mData);
    Name.hash = v5[0];
    memcpy(Name.str, v5 + 1, 28);
}

// ea: 0x4F5FE0
void MetaNalBaseAnim::DelayCreate(void** animArray, int numAnims)
{
    ((DelayCreateFn)mData->__vftable[5])(mData, animArray, numAnims);
    Create(mData);
}

// ea: 0x4FAE20
void MetaNalBaseAnim::DelayCreate(void* anim)
{
    void* animArray = anim;
    ((DelayCreateFn)mData->__vftable[5])(mData, &animArray, 1);
    Create(mData);
}

// ea: 0x4F6010
int MetaNalBaseAnim::IsDelayCreate()
{
    return ((IsDelayCreateFn2)mData->__vftable[6])(mData);
}

// ea: 0x4F6020
void* MetaNalBaseAnim::CreateAnimInst(nalBaseSkeleton* theSkel)
{
    return ((CreateAnimInstFn)mData->__vftable[7])(mData, theSkel, this);
}

// ============================================================================
// TaskHandler - task dispatch handler (0x30, IDA verified)
// ============================================================================
struct DListNode {
    DListNode* m_next;  // +0x00
    DListNode* m_prev;  // +0x04
};

struct DList {
    DListNode m_end;     // +0x00
    DListNode* m_head;   // +0x08
    DListNode** m_tail;  // +0x0C
    int m_size;          // +0x10
};

struct QuickTaskDeactivation {
    DListNode node;  // next/prev
    unsigned int mEntHandle;  // +0x08
};

struct TaskHandlerImpl {
    void* m_dlist[2];    // +0x00
    unsigned int mTaskId;  // +0x08
    unsigned int mFlags;   // +0x0C
    DList mTaskList;       // +0x10
    DList mQuickDeactivationList;  // +0x20

    TaskHandlerImpl(unsigned int task_id, unsigned int flags);
    ~TaskHandlerImpl();
    void QuickDeactivation(DbLinkedHandle<EntityHandleDb, Entity> h);
    void DeactivateAll();
    Task* GetTaskForEntity(DbLinkedHandle<EntityHandleDb, Entity> h);
    void Update(float deltaT, void* ftor);
};

struct TaskSysImpl2 {
    TaskHandlerImpl* mTaskHandlers[32];  // +0x00
    int m_size;                          // +0x80
    DList mPostQueue;                    // +0x84
    void* mHandleDb[8];                  // +0x98 (HandleDb<Task,32,...>)
    static TaskSysImpl2* sInst;          // ?sInst@TaskSys@@0V1@A
};

TaskSysImpl2* TaskSysImpl2_sInst = nullptr;  // ?TaskSysImpl2_sInst (game2.o)
extern void ae_sized_array_push_back_handler(TaskSysImpl2* self,
                                             TaskHandlerImpl* const* elt);
extern void* mem_heap_malloc_sz(unsigned int size);
extern void HandleDb_ReleaseTaskHandle(void* self, Handle h);

// ea: 0x4FFB20
TaskHandlerImpl::TaskHandlerImpl(unsigned int task_id, unsigned int flags)
{
    mFlags = flags;
    mTaskId = task_id;
    m_dlist[0] = nullptr;
    m_dlist[1] = nullptr;
    mTaskList.m_end.m_next = nullptr;
    mTaskList.m_end.m_prev = nullptr;
    mTaskList.m_head = &mTaskList.m_end;
    mTaskList.m_tail = &mTaskList.m_head;
    mTaskList.m_size = 0;
    mQuickDeactivationList.m_end.m_next = nullptr;
    mQuickDeactivationList.m_end.m_prev = nullptr;
    mQuickDeactivationList.m_head = &mQuickDeactivationList.m_end;
    mQuickDeactivationList.m_tail = &mQuickDeactivationList.m_head;
    mQuickDeactivationList.m_size = 0;
    TaskHandlerImpl* self = this;
    ae_sized_array_push_back_handler(TaskSysImpl2_sInst, &self);
}

// ea: 0x50BAC0
TaskHandlerImpl::~TaskHandlerImpl()
{
    // Delete owned quick-deactivation records (each is a heap block).
    DListNode* q = mQuickDeactivationList.m_head;
    while (q != nullptr && q != &mQuickDeactivationList.m_end)
    {
        DListNode* next = q->m_next;
        QuickTaskDeactivation* rec = (QuickTaskDeactivation*)q;
        mem_heap_free(rec);
        q = next;
    }
    // Delete owned task objects (dlist node is embedded in Task).
    DListNode* t = mTaskList.m_head;
    while (t != nullptr && t != &mTaskList.m_end)
    {
        DListNode* next = t->m_next;
        Task* task = (Task*)((char*)t - 0x4);
        task->~Task();
        mem_heap_free(task);
        t = next;
    }
}

// ea: 0x4FFB80
void TaskHandlerImpl::QuickDeactivation(
    DbLinkedHandle<EntityHandleDb, Entity> h)
{
    QuickTaskDeactivation* v3 =
        (QuickTaskDeactivation*)mem_heap_malloc_sz(0xC);
    if (v3 != nullptr)
    {
        v3->node.m_next = nullptr;
        v3->node.m_prev = nullptr;
        v3->mEntHandle = h.mHandle.mVal;
    }
    DListNode* tail = *mQuickDeactivationList.m_tail;
    v3->node.m_next = &mQuickDeactivationList.m_end;
    v3->node.m_prev = tail;
    tail->m_next = &v3->node;
    *mQuickDeactivationList.m_tail = &v3->node;
    ++mQuickDeactivationList.m_size;
}

// ============================================================================
// TaskHandler::DeactivateAll - ea: 0x504B80
// ============================================================================
void TaskHandlerImpl::DeactivateAll()
{
    mFlags |= 8u;
    DListNode* m_head = mTaskList.m_head;
    DListNode* m_next = m_head != nullptr ? m_head->m_next : nullptr;
    if (m_head != &mTaskList.m_end && m_next != nullptr)
    {
        do
        {
            // Task dlist node is embedded in Task; mFlags at +0x18
            unsigned int* flags = (unsigned int*)((char*)m_head + 0x14);
            *flags |= 4u;
            m_head = m_next;
            m_next = m_next->m_next;
        } while (m_next != nullptr);
    }
}

// ============================================================================
// TaskHandler::GetTaskForEntity - ea: 0x504BC0
// ============================================================================
Task* TaskHandlerImpl::GetTaskForEntity(
    DbLinkedHandle<EntityHandleDb, Entity> h)
{
    DListNode* m_head = mTaskList.m_head;
    DListNode* m_next = m_head != nullptr ? m_head->m_next : nullptr;
    if (m_head == &mTaskList.m_end || m_next == nullptr)
        return nullptr;
    while (*(unsigned int*)((char*)m_head + 0x14) != h.mHandle.mVal)
    {
        m_head = m_next;
        m_next = m_next->m_next;
        if (m_next == nullptr)
            return nullptr;
    }
    return (Task*)((char*)m_head - 0x4);
}

// ============================================================================
// TaskHandler::Update - ea: 0x504990
// ============================================================================

void TaskHandlerImpl::Update(float deltaT, void* ftor)
{
    // Apply quick-deactivation records: mark those entities' tasks.
    DListNode* q = mQuickDeactivationList.m_head;
    while (q != nullptr && q != &mQuickDeactivationList.m_end)
    {
        QuickTaskDeactivation* rec = (QuickTaskDeactivation*)q;
        unsigned int entVal = rec->mEntHandle;
        DListNode* next = q->m_next;
        Entity* ent = EntityHandleDb::sInst.GetObject(entVal);
        if (ent != nullptr)
            ent->mFlags |= 4u;
        mem_heap_free(rec);
        q = next;
    }
    mQuickDeactivationList.m_head = &mQuickDeactivationList.m_end;
    mQuickDeactivationList.m_size = 0;

    // Run each task's Update.
    DListNode* t = mTaskList.m_head;
    while (t != nullptr && t != &mTaskList.m_end)
    {
        Task* task = (Task*)((char*)t - 0x4);
        DListNode* next = t->m_next;
        if (ftor != nullptr)
        {
            // TaskFunctor path: fn(task, entity)
            ((void(*)(Task*, void*))ftor)(task, nullptr);
        }
        else
        {
            // Task::Update(Entity*, float)
            typedef void (*UpdateFn)(Task*, Entity*, float);
            void** vt = *(void***)task;
            UpdateFn fn = (UpdateFn)vt[4];  // vtable slot 4 = Update
            Entity* e = EntityHandleDb::sInst.GetObject(
                task->mEntityHandle.mHandle.mVal);
            fn(task, e, deltaT);
        }
        t = next;
    }
}

// ============================================================================
// TaskSys::ReleaseTask - ea: 0x504970
// ============================================================================
void TaskSys_ReleaseTask(Task* t)
{
    if (t->mTaskHandle.mVal != 0)
        HandleDb_ReleaseTaskHandle(&TaskSysImpl2_sInst->mHandleDb,
                                   t->mTaskHandle);
}

// ============================================================================
// TaskSys::Update - ea: 0x50B8E0
// ============================================================================
TaskHandlerImpl* HealthRegenTask_sHandler = nullptr;  // ?HealthRegenTask_sHandler (game2.o)
TaskHandlerImpl* AnimNotifyTask_sHandler = nullptr;   // ?AnimNotifyTask_sHandler (game2.o)
TaskHandlerImpl* EntityDeathTask_sHandler = nullptr;  // ?EntityDeathTask_sHandler (game2.o)
extern void TaskHandler_Update(TaskHandlerImpl* self, float deltaT,
                               void* ftor);
extern TaskHandlerImpl* TaskSys_LookupHandler(unsigned int id);

void TaskSys_Update(float deltaT)
{
    TaskHandler_Update(HealthRegenTask_sHandler, deltaT, nullptr);
    TaskHandler_Update(AnimNotifyTask_sHandler, deltaT, nullptr);
    TaskHandler_Update(EntityDeathTask_sHandler, deltaT, nullptr);
}

// ============================================================================
// TaskSys::DeactivateTask - ea: 0x50B9C0
// ============================================================================
extern Task* HandleDb_GetTask(void* self, Handle h);  // GetObject on Task HandleDb

void TaskSys_DeactivateTask(Handle taskHandle)
{
    Task* t = HandleDb_GetTask(&TaskSysImpl2_sInst->mHandleDb, taskHandle);
    if (t != nullptr)
        t->mFlags |= 4u;
}

// ============================================================================
// TaskSys::PostTaskAndAllocateHandle - ea: 0x50D810
// ============================================================================
extern void TaskSys_PostTask_glue(Task* t);
extern Handle TaskSys_CreateTaskHandle(Task* t);

Handle TaskSys_PostTaskAndAllocateHandle(Task* t)
{
    TaskSys_PostTask_glue(t);
    return TaskSys_CreateTaskHandle(t);
}

// ============================================================================
// TaskSys::GetTaskForEntity - ea: 0x50BA90
// ============================================================================
extern Task* TaskHandler_GetTaskForEntity(TaskHandlerImpl* self,
    DbLinkedHandle<EntityHandleDb, Entity> h);
extern TaskHandlerImpl* TaskSys_LookupHandler(unsigned int id);

Task* TaskSys_GetTaskForEntity(unsigned int taskId,
    DbLinkedHandle<EntityHandleDb, Entity> eh)
{
    TaskHandlerImpl* v3 = TaskSys_LookupHandler(taskId);
    if (v3 != nullptr)
        return TaskHandler_GetTaskForEntity(v3, eh);
    return nullptr;
}

// ============================================================================
// TaskSys::CreateTaskHandle - ea: 0x50BA00
// ============================================================================
extern void HandleDb_AllocateTaskHandle(void* self, Task** t);
extern void HandleDb_BindTaskObject(void* self, Handle h, Task* obj);

Handle TaskSys_CreateTaskHandle(Task* t)
{
    if (t->mTaskHandle.mVal != 0)
    {
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("handle already assigned"))
            __debugbreak();
    }
    Task* pt = t;
    HandleDb_AllocateTaskHandle(&TaskSysImpl2_sInst->mHandleDb, &pt);
    t->mTaskHandle.mVal = pt->mTaskHandle.mVal;
    HandleDb_BindTaskObject(&TaskSysImpl2_sInst->mHandleDb,
                            t->mTaskHandle, t);
    return t->mTaskHandle;
}

// ============================================================================
// TaskSys::ShutDown - ea: 0x50B920
// ============================================================================
extern void TaskSys_DeliverTasks_glue();

void TaskSys_ShutDown()
{
    TaskSys_DeliverTasks_glue();
    int count = TaskSysImpl2_sInst->m_size;
    for (int i = 0; i < count; ++i)
    {
        TaskHandlerImpl* handler = TaskSysImpl2_sInst->mTaskHandlers[i];
        if (handler != nullptr)
            handler->DeactivateAll();
    }
    TaskHandler_Update(HealthRegenTask_sHandler, 0.01f, nullptr);
    TaskHandler_Update(AnimNotifyTask_sHandler, 0.01f, nullptr);
    TaskHandler_Update(EntityDeathTask_sHandler, 0.01f, nullptr);
}

// ============================================================================
// TaskSys::DeliverTasks - ea: 0x4FF9D0
// ============================================================================
extern void TaskSys_DeliverTasks();

void TaskSys_DeliverTasks()
{
    TaskSysImpl2* sys = TaskSysImpl2_sInst;
    // Move each posted task from mPostQueue to its handler's mTaskList.
    int count = sys->mPostQueue.m_size;
    while (count > 0)
    {
        DListNode* head = sys->mPostQueue.m_head;
        Task* task = (Task*)((char*)head - 0x4);
        sys->mPostQueue.m_head = head->m_next;
        --sys->mPostQueue.m_size;
        unsigned int taskId = *(unsigned int*)((char*)head + 0x8);
        TaskHandlerImpl* handler = TaskSys_LookupHandler(taskId);
        if (handler != nullptr)
        {
            DListNode* node = (DListNode*)&task->_dlist[0];
            node->m_next = &handler->mTaskList.m_end;
            node->m_prev = *handler->mTaskList.m_tail;
            (*handler->mTaskList.m_tail)->m_next = node;
            *handler->mTaskList.m_tail = node;
            ++handler->mTaskList.m_size;
        }
        --count;
    }
}

// ============================================================================
// AnimationPlayer::nalPlayMethod dtor - ea: 0x50BB30
// ============================================================================
extern void mem_heap_free(void* ptr);

AnimationPlayer::nalPlayMethod::~nalPlayMethod()
{
    __vftable = (void**)&nalPlayMethod_vftable;
    AnimNoteHandler* mNoteHandler = this->mNoteHandler;
    if (mNoteHandler != nullptr)
    {
        // NotifyInfo block: [count][NotifyInfo...] freed as one heap block
        mem_heap_free(mNoteHandler->mNotify != nullptr
                          ? (char*)mNoteHandler->mNotify - 4
                          : nullptr);
        mNoteHandler->mNotify = nullptr;
        mem_heap_free(mNoteHandler);
    }
}

// ============================================================================
// AnimationPlayer::nalPlayMethod::Advance - ea: 0x50BB80
// ============================================================================
struct nalAnimStateView {
    void* instance;      // +0x00
    float speed;         // +0x04
    float t;             // +0x08
};

extern void AnimNoteHandler_Advance(void* self, float t);

void AnimationPlayer::nalPlayMethod::Advance(void* state, float delta)
{
    nalAnimStateView* st = (nalAnimStateView*)state;
    float v3 = (*(float*)((char*)st->instance + 0x14)
                * st->speed) * delta + st->t;
    st->t = v3;
    if (mNoteHandler != nullptr)
        AnimNoteHandler_Advance(mNoteHandler, v3);
}

// ============================================================================
// AnimationPlayer::nalPlayMethod::CreateInstance - ea: 0x504C60
// ============================================================================
extern void* nalGenericAnim_CreateInstance(void* anim, void* skeleton);
extern void AnimNoteHandler_ParseNoteTracks(void* self, void* anim);

void* AnimationPlayer::nalPlayMethod::CreateInstance(void* anim, void* skeleton)
{
    void* instance = nalGenericAnim_CreateInstance(anim, skeleton);
    if (mNoteHandler != nullptr)
        AnimNoteHandler_ParseNoteTracks(mNoteHandler, anim);
    return instance;
}

// ============================================================================
// AnimationPlayer::Advance - ea: 0x4FA550
// ============================================================================
void AnimationPlayer::Advance(float delta)
{
    struct AnimationPlayerLayout {
        void* Skeleton;             // +0x00
        unsigned char BackgroundPose[0x10];  // +0x04
        unsigned char tmpPose[0x10];         // +0x14
        int QueueSize;              // +0x24
        nalAnimState* AnimStates[3];  // +0x28
        nalPartialAnimState* PartialAnimStates;   // +0x34
        nalPartialAnimState* PartialAnimStatePool; // +0x38
        unsigned int AdvanceCount;  // +0x3C
    };
    AnimationPlayerLayout* self = (AnimationPlayerLayout*)this;
    ++self->AdvanceCount;
    nalPartialAnimState** p_PartialAnimStates = &self->PartialAnimStates;
    nalPartialAnimState* PartialAnimStates = self->PartialAnimStates;
    while (PartialAnimStates != nullptr)
    {
        nalPartialAnimState* ps = PartialAnimStates;
        if (ps->CreationAdvanceCount != self->AdvanceCount)
        {
            if (ps->Update(this, delta))
            {
                if (*p_PartialAnimStates != PartialAnimStates)
                {
                    nalPartialAnimState* cur = *p_PartialAnimStates;
                    do
                    {
                        p_PartialAnimStates = &cur->next;
                        cur = *p_PartialAnimStates;
                    } while (cur != PartialAnimStates);
                }
                *p_PartialAnimStates = ps->next;
                if (ps->base.callback != nullptr)
                    (*(void(**)(void*))ps->base.callback)(ps->base.callback);
                if (ps->base.play_method != nullptr)
                    (*(void(**)(void*))ps->base.play_method)(ps->base.play_method);
                if (ps->base.instance != nullptr)
                    (*(void(**)(void*, int))ps->base.instance)(ps->base.instance, 1);
                ps->next = self->PartialAnimStatePool;
                self->PartialAnimStatePool = ps;
            }
            else
            {
                p_PartialAnimStates = &ps->next;
            }
            PartialAnimStates = *p_PartialAnimStates;
        }
    }
    int v9 = 0;
    if (self->QueueSize > 0)
    {
        for (;;)
        {
            nalAnimState** v10 = &self->AnimStates[v9];
            nalAnimState* cur = *v10;
            bool v11 = cur->Update(this, delta);
            int QueueSize = self->QueueSize;
            if (v9 < QueueSize)
            {
                do
                {
                    if (*v10 == cur)
                        break;
                    ++v9;
                    ++v10;
                } while (v9 < self->QueueSize);
            }
            if (v11 && v9 < QueueSize)
                break;
            if (++v9 >= self->QueueSize)
                break;
        }
        ++v9;
    }
    int newSize = v9;
    if (v9 < self->QueueSize)
    {
        nalAnimState** deltaa = &self->AnimStates[v9];
        do
        {
            nalAnimState* st = *deltaa;
            if (st->callback != nullptr)
                (*(void(**)(void*))st->callback)(st->callback);
            if (st->play_method != nullptr)
                (*(void(**)(void*))st->play_method)(st->play_method);
            if (st->instance != nullptr)
                (*(void(**)(void*, int))st->instance)(st->instance, 1);
            ++v9;
            ++deltaa;
        } while (v9 < self->QueueSize);
    }
    self->QueueSize = newSize;
}

static const char* AnimDebugTypeString(int type)
{
    switch (type)
    {
    case 0: return "ADD";
    case 1: return "PART";
    case 2: return "FULL";
    default: return "?";
    }
}

void AnimationPlayer::DebugDump(Entity* ent)
{
    if (ent == nullptr)
        return;
    DObj* mDObj = ent->mDObj;
    if (mDObj == nullptr)
        return;
    AnimationPlayer* v2 =
        (AnimationPlayer*)(*(void***)((char*)mDObj + 0x20))[0];
    if (v2 == nullptr)
        return;
    float col[4] = { 1.0f, 0.5f, 0.0f, 1.0f };
    char textBuff[512];
    struct APLayout {
        void* Skeleton;             // +0x00
        unsigned char BackgroundPose[0x10];  // +0x04
        unsigned char tmpPose[0x10];         // +0x14
        int QueueSize;              // +0x24
        AnimationPlayer::nalAnimState* AnimStates[3];  // +0x28
        AnimationPlayer::nalPartialAnimState* PartialAnimStates;   // +0x34
        AnimationPlayer::nalPartialAnimState* PartialAnimStatePool; // +0x38
        unsigned int AdvanceCount;  // +0x3C
    };
    APLayout* self = (APLayout*)v2;
    for (int v3 = self->QueueSize - 1; v3 >= 0; --v3)
    {
        AnimationPlayer::nalAnimState* st = self->AnimStates[v3];
        if (st->state != 0 && st->instance != nullptr)
        {
            _snprintf(textBuff, 0x200u,
                      "%d)%s L=%d a=%.2f t=%.2f",
                      v3,
                      *(const char**)(*(char**)st->instance + 16) + 12,
                      *(int*)(*(char**)st->instance + 52) & 1,
                      st->alpha, st->t);
            DebugRender::RenderText(textBuff, 10, 75 + (self->QueueSize - 1 - v3) * 20,
                                    Color(col[0], col[1], col[2], col[3]), 0.0f, 1.125f);
        }
    }
    // Partial anim states
    AnimationPlayer::nalPartialAnimState* ps = self->PartialAnimStates;
    int y = 75 + (self->QueueSize) * 20 + 20;
    while (ps != nullptr)
    {
        if (ps->base.instance != nullptr)
        {
            _snprintf(textBuff, 0x200u,
                      "p%d)%s %s L=%d a=%.2f t=%.2f",
                      (int)((char*)ps - (char*)self->PartialAnimStates) / (int)sizeof(AnimationPlayer::nalPartialAnimState),
                      *(const char**)(*(char**)ps->base.instance + 16),
                      AnimDebugTypeString(ps->type),
                      *(int*)(*(char**)ps->base.instance + 52) & 1,
                      ps->base.alpha, ps->base.t);
            DebugRender::RenderText(textBuff, 10, y, Color(col[0], col[1], col[2], col[3]), 0.0f, 1.125f);
            y += 20;
        }
        ps = ps->next;
    }
}

// ============================================================================
// AnimationPlayer::GetPose - ea: 0x4FA690
// Compose the current pose from the animation queue (or an active FULL
// partial state) and export the hand/gun world matrices into animIKGunOffset.
// NOTE: original decompile is register-ABI garbled ("local variable allocation
// has failed"); this is a structural reconstruction from the verified layout.
// ============================================================================
void AnimationPlayer::GetPose(nalGeneric::nalGenericPose& Pose,
                              nalGeneric::nalGenericSkeleton* Skeleton,
                              float (*animIKGunOffset)[3])
{
    struct APLayout {
        nalGeneric::nalGenericSkeleton* Skeleton;  // +0x00
        nalGeneric::nalGenericPose BackgroundPose; // +0x04
        nalGeneric::nalGenericPose tmpPose;        // +0x14
        int QueueSize;              // +0x24
        nalAnimState* AnimStates[3];  // +0x28
        nalPartialAnimState* PartialAnimStates;   // +0x34
        nalPartialAnimState* PartialAnimStatePool; // +0x38
        unsigned int AdvanceCount;  // +0x3C
    };
    APLayout* self = (APLayout*)this;
    Pose = self->BackgroundPose;
    static tlFixedString handStr("bip01 r hand");
    static tlFixedString gunStr("TAG_WEAPON_RIGHT");
    nalGeneric::nalGenericBoneHandle handHandle;
    nalGeneric::nalGenericBoneHandle gunHandle;
    handHandle.index = 0;
    handHandle.skeleton = nullptr;
    gunHandle.index = 0;
    gunHandle.skeleton = nullptr;
    Skeleton->GetBoneHandle(handHandle, handStr);
    Skeleton->GetBoneHandle(gunHandle, gunStr);

    // Prefer an active FULL partial state (type 2) at full alpha.
    nalPartialAnimState* active = nullptr;
    for (nalPartialAnimState* ps = self->PartialAnimStates;
         ps != nullptr; ps = ps->next)
    {
        if (ps->type == 2 && ps->base.alpha >= 1.0f)
            active = ps;
    }
    bool offsetDone = false;
    if (active == nullptr)
    {
        for (int i = self->QueueSize - 1; i >= 0; --i)
        {
            nalAnimState* st = self->AnimStates[i];
            if (st == nullptr)
                continue;
            st->Compose(Pose, self->tmpPose);
        }
    }
    else
    {
        for (nalPartialAnimState* ps = active; ps != nullptr; ps = ps->next)
        {
            nalAnimState* st = &ps->base;
            st->Compose(Pose, self->tmpPose);
            if (st->state == 0)
                nalGeneric::BlendTorso(Pose, st->alpha, Pose, self->tmpPose);
            else
                nalGeneric::Blend(Pose, st->alpha, Pose, self->tmpPose);
            st->t_prev = st->t;
        }
    }
    if (animIKGunOffset != nullptr)
    {
        // Hand pose in world space, then expressed relative to the gun bone.
        nalPositionOrientationLocal handPO =
            Pose.GetModelPositionOrientation(handHandle);
        nalPositionOrientationLocal gunPO =
            Pose.GetModelPositionOrientation(gunHandle);
        nalMatrix4x4Local handMat;
        nalMatrix4x4Local gunMat;
        memcpy(&handMat, &handPO, sizeof(nalMatrix4x4Local));
        memcpy(&gunMat, &gunPO, sizeof(nalMatrix4x4Local));
        // 4x4 affine inverse of gunMat (row-major; w row = 0,0,0,1)
        nalMatrix4x4Local inv;
        float det = 0.0f;
        float a[3][3] = {
            { gunMat.m[0][0], gunMat.m[0][1], gunMat.m[0][2] },
            { gunMat.m[1][0], gunMat.m[1][1], gunMat.m[1][2] },
            { gunMat.m[2][0], gunMat.m[2][1], gunMat.m[2][2] },
        };
        for (int c = 0; c < 3; ++c)
            det += a[0][c] * (a[1][(c + 1) % 3] * a[2][(c + 2) % 3]
                              - a[1][(c + 2) % 3] * a[2][(c + 1) % 3]);
        if (fabsf(det) > 1e-9f)
        {
            float invDet = 1.0f / det;
            for (int r = 0; r < 3; ++r)
            {
                for (int c = 0; c < 3; ++c)
                {
                    int r1 = (r + 1) % 3;
                    int r2 = (r + 2) % 3;
                    int c1 = (c + 1) % 3;
                    int c2 = (c + 2) % 3;
                    inv.m[c][r] = (a[r1][c1] * a[r2][c2]
                                   - a[r1][c2] * a[r2][c1]) * invDet;
                }
            }
            inv.m[3][0] = 0.0f;
            inv.m[3][1] = 0.0f;
            inv.m[3][2] = 0.0f;
            inv.m[3][3] = 1.0f;
            float tx = gunMat.m[3][0];
            float ty = gunMat.m[3][1];
            float tz = gunMat.m[3][2];
            inv.m[0][3] = -(inv.m[0][0] * tx + inv.m[0][1] * ty
                            + inv.m[0][2] * tz);
            inv.m[1][3] = -(inv.m[1][0] * tx + inv.m[1][1] * ty
                            + inv.m[1][2] * tz);
            inv.m[2][3] = -(inv.m[2][0] * tx + inv.m[2][1] * ty
                            + inv.m[2][2] * tz);
            for (int r = 0; r < 4; ++r)
            {
                float row[4] = { handMat.m[r][0], handMat.m[r][1],
                                 handMat.m[r][2], handMat.m[r][3] };
                for (int c = 0; c < 4; ++c)
                    gunMat.m[r][c] = row[0] * inv.m[0][c]
                        + row[1] * inv.m[1][c]
                        + row[2] * inv.m[2][c]
                        + row[3] * inv.m[3][c];
            }
        }
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 3; ++j)
                animIKGunOffset[i][j] = gunMat.m[i][j];
        offsetDone = true;
    }
    (void)offsetDone;
}

// ============================================================================
// TestFPS::TestFPS - ea: 0x4FEC20
// ============================================================================
TestFPS::TestFPS()
{
    memset(_pad, 0, sizeof(_pad));
    mStats_size = 0;
    mCells_size = 0;
    mBlock = 0;
    mTesting = false;
    mCellIndex = 0;
    mCurrentAngle = 0;
    mCellX = 0;
    mCellY = 0;
    mZoneIndex = 0;
    mDeltaAngle = 45;
    mDelta = 0.0f;
    mDeltaInverse = 0.0f;
    mCurrentPositionIndex = 0;
    mPlayerHandle.mVal = 0;
    mFile = nullptr;
}

struct PerformanceStats {
    float mPosition[3];   // +0x00
    int   mPositionIndex; // +0x0C
    int   mCell;          // +0x10
    int   mAngle;         // +0x14
    float mDMATime;       // +0x18
    float mSceneSubmitTime;  // +0x1C
    float mRenderTime;    // +0x20
    int   mNodeCount;     // +0x24
    int   mPolyCount;     // +0x28
};
static_assert(sizeof(PerformanceStats) == 0x2C,
              "PerformanceStats size mismatch");

extern bool gUseNfl;  // ?gUseNfl (game2.o)
char buffer[0x4000];  // ?buffer (game2.o)
char temp[0x400];     // ?temp (game2.o)

// ea: 0x4FEC80
void TestFPS::OutputStats()
{
    int m_size = 0;
    if (mStats_size != 0)
    {
        int length = 0;
        char filename[256];
        GetFilename(filename);
        if (_stricmp(filename, mLastFile) == 0)
        {
            if (mFile == nullptr)
            {
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
        }
        else
        {
            if (mFile != nullptr)
            {
                gUseNfl = false;
                fclose((FILE*)mFile);
                mFile = nullptr;
            }
            gUseNfl = false;
            mFile = fopen(filename, "w");
            gUseNfl = true;
            strcpy(mLastFile, filename);
        }
        memset(buffer, 0, 0x4000u);
        buffer[0] = 0;
        temp[0] = 0;
        if (mFile != nullptr)
        {
            int count = mStats_size;
            for (int i = 0; i < count; ++i)
            {
                PerformanceStats* stats =
                    reinterpret_cast<PerformanceStats*>(_pad);
                PerformanceStats& s = stats[i];
                sprintf(temp,
                        "%i, %i, %g %g %g, %i, %g, %g, %g, %i, %i, \n",
                        s.mPositionIndex, s.mCell,
                        s.mPosition[0], s.mPosition[1], s.mPosition[2],
                        s.mAngle, s.mDMATime, s.mSceneSubmitTime,
                        s.mRenderTime, s.mNodeCount, s.mPolyCount);
                length += (int)strlen(temp);
                strcat(buffer, temp);
                if (length > 14336)
                {
                    gUseNfl = false;
                    fwrite(buffer, 1, length, (FILE*)mFile);
                    memset(buffer, 0, 0x4000u);
                    gUseNfl = true;
                    buffer[0] = 0;
                    length = 0;
                }
            }
            gUseNfl = false;
            fwrite(buffer, 1, length, (FILE*)mFile);
            gUseNfl = true;
        }
        else
        {
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("Couldn't open %s for writing", filename))
                __debugbreak();
        }
        fflush((FILE*)mFile);
        mStats_size = 0;
    }
    (void)m_size;
}

// ============================================================================
// TestFPS::StopTest - ea: 0x5018C0
// ============================================================================
extern int gStartTime;  // ?gStartTime (game2.o)
extern void Com_Printf(const char* fmt, ...);
extern void Cvar_Set(const char* var_name, const char* value);

void TestFPS::StopTest()
{
    if (mTesting)
    {
        OutputStats();
        if (mFile != nullptr)
        {
            fclose((FILE*)mFile);
            mFile = nullptr;
        }
        mTesting = false;
        Entity* player = EntityManager::sInst->GetPlayer(currCl);
        player->client->noclip = 0;
        player->client->bFrozen = 0;
        player->client->ps.pm_flags &= ~0x4000u;
        Cvar_Set("g_performanceTest", "0");
        int v3 = Sys_Milliseconds();
        Com_Printf(
            "Performance test finished in %i hours %i minutes %i seconds.\n",
            (int)((v3 - gStartTime) * 0.001) / 60 / 60,
            (int)((v3 - gStartTime) * 0.001) / 60 % 60,
            (int)((v3 - gStartTime) * 0.001) % 60);
    }
}

// ============================================================================
// TestFPS::GatherMetrics - ea: 0x501990
// ============================================================================
struct nglPerfInfoStruct {
    float RenderMS;
    float ListSubmitMS;
    float ListSendMS;
    int NodeCount;
    int TotalPolys;
};

extern nglPerfInfoStruct nglPerfInfo;
extern nglPerfInfoStruct nglSyncPerfInfo;
extern vmCvar_t bg_viewheight_standing;

void TestFPS::GatherMetrics()
{
    if (mTesting)
    {
        float x = mCurrentPosition.x;
        float y = mCurrentPosition.y;
        float z = mCurrentPosition.z;
        mBlock = 0;
        PerformanceStats stat;
        memset(&stat, 0, sizeof(stat));
        stat.mPosition[2] = bg_viewheight_standing.integer + z;
        stat.mPosition[0] = x;
        stat.mPosition[1] = y;
        stat.mPositionIndex = mCurrentPositionIndex;
        stat.mDMATime = nglPerfInfo.ListSendMS;
        stat.mCell = mCellIndex;
        stat.mAngle = mCurrentAngle;
        stat.mSceneSubmitTime = nglPerfInfo.ListSubmitMS;
        stat.mNodeCount = nglSyncPerfInfo.NodeCount;
        stat.mRenderTime = nglPerfInfo.RenderMS;
        stat.mPolyCount = nglSyncPerfInfo.TotalPolys;
        PerformanceStats* stats = reinterpret_cast<PerformanceStats*>(_pad);
        stats[mStats_size] = stat;
        ++mStats_size;
        if (mStats_size >= 1000)
            OutputStats();
    }
}

// ============================================================================
// TestFPS::PositionCamera - ea: 0x509B50
// ============================================================================
extern void TeleportPlayer(Entity* player, const float* origin,
                           const float* angles);
char tr[0x3A0];  // ?tr@@3UtrGlobals_t@@A (render.o)

static int TestFPS_tr_cell_count()
{
    void* world = *(void**)(tr + 0x290);
    if (world == nullptr)
        return 0;
    void* bspTree = *(void**)((char*)world + 0x100);
    if (bspTree == nullptr)
        return 0;
    return *(int*)((char*)bspTree + 0x1C);  // mCells.mSize
}

void TestFPS::PositionCamera(pmove_t* pmove)
{
    PakManager::sInst->FillBanks();
    if (mBlock == 0)
    {
        mBlock = 1;
        Entity* player = EntityManager::sInst->GetPlayer(currCl);
        player->client->noclip = 1;
        player->client->bFrozen = 1;
        int mSize = TestFPS_tr_cell_count();
        mCurrentAngle += mDeltaAngle;
        pmove->ps->pm_flags |= 0x4000u;
        if (mCurrentAngle >= 360)
        {
            mCurrentAngle = 0;
            NextPosition();
        }
        if (mCellIndex >= mSize)
        {
            StopTest();
        }
        else
        {
            float angles[3] = { 0.0f, (float)mCurrentAngle, 0.0f };
            float position[3] = {
                mCurrentPosition.x, mCurrentPosition.y, mCurrentPosition.z
            };
            TeleportPlayer(player, position, angles);
        }
    }
}
